#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "acquisition.h"
#include "affichage.h"
#include "buffer.h"
#include "donnees.h"
#include "filtrage.h"

#define PRIO_ACQUISITION 60
#define PRIO_FILTRAGE    50
#define PRIO_AFFICHAGE   40

static volatile sig_atomic_t arret = 0;
static buffer_t tampon; /* buffer partage acquisition -> filtrage */

static void sur_signal(int sig) {
    (void)sig;
    arret = 1;
}

/* repere 8.2 : creation d'un thread en SCHED_FIFO */
static int creer_tache_fifo(pthread_t *th, void *(*fn)(void *), void *arg, int priorite) {
    pthread_attr_t attr;
    struct sched_param sp = { .sched_priority = priorite };

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &sp);
    int r = pthread_create(th, &attr, fn, arg);
    pthread_attr_destroy(&attr);
    return r; /* 0 si OK ; EPERM si on oublie sudo */
}

int main(int argc, char **argv) {
    int duree = 0; /* 0 = jusqu'a Ctrl+C */
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(argv[i], "--duree") == 0) duree = atoi(argv[i + 1]);

    struct sigaction sa = { .sa_handler = sur_signal };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    donnees_init();
    buffer_init(&tampon);

    struct { const char *nom; void *(*fn)(void *); void *arg; int prio; pthread_t th; int ok; } t[] = {
        { "acquisition", acquisition_tache, &tampon, PRIO_ACQUISITION, 0, 0 },
        { "filtrage",    filtrage_tache,    NULL,    PRIO_FILTRAGE,    0, 0 },
        { "affichage",   affichage_tache,   NULL,    PRIO_AFFICHAGE,   0, 0 },
    };
    int n = (int)(sizeof t / sizeof t[0]);
    int erreur = 0;

    for (int i = 0; i < n; i++) {
        int r = creer_tache_fifo(&t[i].th, t[i].fn, t[i].arg, t[i].prio);
        if (r != 0) {
            fprintf(stderr, "Erreur : creation de la tache %s (priorite %d) : %s%s\n",
                    t[i].nom, t[i].prio, strerror(r),
                    r == EPERM ? " -> lancer avec sudo" : "");
            erreur = 1;
            break;
        }
        t[i].ok = 1;
    }

    if (!erreur) {
        struct timespec pas = { 0, 100000000L }; /* 100 ms */
        int ticks = 0;
        while (!arret && (duree == 0 || ticks < duree * 10)) {
            nanosleep(&pas, NULL);
            ticks++;
        }
    }

    for (int i = 0; i < n; i++)
        if (t[i].ok) pthread_cancel(t[i].th);
    for (int i = 0; i < n; i++)
        if (t[i].ok) pthread_join(t[i].th, NULL);

    return erreur ? EXIT_FAILURE : EXIT_SUCCESS;
}
