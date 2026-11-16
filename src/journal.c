#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "journal.h"

#define JOURNAL_CAPACITE   4096
#define JOURNAL_MSG_TAILLE 128

/* buffer circulaire de messages, protege par un mutex a heritage de priorite */
static char file_msgs[JOURNAL_CAPACITE][JOURNAL_MSG_TAILLE];
static unsigned tete = 0;     /* prochain indice d'ecriture */
static unsigned queue = 0;    /* prochain indice de lecture */
static unsigned long pertes = 0;
static pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

static pthread_t thread_ecriture;
static FILE *sortie = NULL;
static atomic_int actif = 0;
static atomic_int arret_demande = 0;

/* Thread de faible priorite : vide le buffer et ecrit dans le fichier. */
static void *ecriture_tache(void *arg) {
    (void)arg;
    char msg[JOURNAL_MSG_TAILLE];

    for (;;) {
        int fin = atomic_load(&arret_demande); /* lu AVANT de vider */
        int vide;
        do {
            vide = 1;
            pthread_mutex_lock(&verrou);
            if (queue != tete) {
                memcpy(msg, file_msgs[queue], sizeof msg);
                queue = (queue + 1) % JOURNAL_CAPACITE;
                vide = 0;
            }
            pthread_mutex_unlock(&verrou);
            if (!vide) {
                fputs(msg, sortie);
                fputc('\n', sortie);
            }
        } while (!vide);
        fflush(sortie);
        if (fin) break;
        struct timespec pas = { 0, 10000000L }; /* 10 ms */
        nanosleep(&pas, NULL);
    }
    return NULL;
}

void journal_init(const char *fichier) {
    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setprotocol(&ma, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&verrou, &ma);
    pthread_mutexattr_destroy(&ma);

    sortie = fopen(fichier, "w");
    if (!sortie) {
        perror("journal_init : ouverture du fichier");
        return;
    }
    tete = queue = 0;
    pertes = 0;
    atomic_store(&arret_demande, 0);

    /* faible priorite : SCHED_OTHER explicite, sous toutes les taches FIFO */
    pthread_attr_t attr;
    struct sched_param sp = { .sched_priority = 0 };
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_setschedparam(&attr, &sp);
    int r = pthread_create(&thread_ecriture, &attr, ecriture_tache, NULL);
    pthread_attr_destroy(&attr);
    if (r != 0) {
        fprintf(stderr, "journal_init : creation du thread impossible (%d)\n", r);
        fclose(sortie);
        sortie = NULL;
        return;
    }
    atomic_store(&actif, 1);
}

/* Formate, horodate, depose dans le buffer. Aucune entree-sortie, jamais bloquant. */
void journal_ecrire(const char *fmt, ...) {
    if (!atomic_load(&actif)) return;

    char msg[JOURNAL_MSG_TAILLE];
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    int n = snprintf(msg, sizeof msg, "[%ld.%06ld] ", (long)t.tv_sec, t.tv_nsec / 1000);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg + n, sizeof msg - (size_t)n, fmt, ap);
    va_end(ap);

    size_t len = strlen(msg) + 1;
    pthread_mutex_lock(&verrou); /* section tres courte : copie en memoire seulement */
    if ((tete + 1) % JOURNAL_CAPACITE == queue) {
        pertes++; /* buffer plein : message abandonne */
    } else {
        memcpy(file_msgs[tete], msg, len);
        tete = (tete + 1) % JOURNAL_CAPACITE;
    }
    pthread_mutex_unlock(&verrou);
}

/* Vide le buffer, ecrit le bilan, ferme le fichier. */
void journal_arreter(void) {
    if (!atomic_load(&actif)) return;
    atomic_store(&actif, 0);
    atomic_store(&arret_demande, 1);
    pthread_join(thread_ecriture, NULL); /* le thread vide le buffer avant de finir */

    fprintf(sortie, "[journal] messages perdus : %lu\n", pertes);
    fclose(sortie);
    sortie = NULL;
    fprintf(stderr, "journal : %lu message(s) perdu(s)\n", pertes);
}