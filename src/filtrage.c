/* src/filtrage.c */
#define _POSIX_C_SOURCE 200809L
 
#include <errno.h>
#include <stdatomic.h>
#include <stddef.h>
#include <time.h>
 
#include "filtrage.h"
#include "jitter.h"
#include "temps.h"
 
#ifdef FILTRAGE_DEBUG
#include <stdio.h>
#endif
 
#define PERIODE_NS 200000000L /* 200 ms = 5 Hz */
 
/* Derniere valeur traitee (volatile : evite que le compilateur supprime le calcul). */
static volatile double derniere_sortie;

static double valeur_provisoire(unsigned long n)
{
    static unsigned long etat = 12345UL;
    double bruit;
 
    etat = etat * 1103515245UL + 12345UL;
    bruit = ((double)((etat >> 16) & 0x7FFFUL) / 32767.0 - 0.5) * 0.4; 
    return 20.0 + (double)(n % 100UL) * 0.1 + bruit;
}

/* Traitement de la valeur */
static double traiter(double x)
{
    return x;
}

void *filtrage_tache(void *arg)
{
    atomic_int *arret = (atomic_int *)arg; /* NULL = ne s'arrete jamais */
    struct timespec prochain, reel;
    unsigned long n = 0;
    int r;
 
    clock_gettime(CLOCK_MONOTONIC, &prochain);
 
    while (arret == NULL || atomic_load(arret) == 0) {
        /* Prochain reveil = reveil precedent + 200 ms 
         * les retards ne s'accumulent pas. */
        ajouter_ns(&prochain, PERIODE_NS);
        do {
            r = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &prochain, NULL);
        } while (r == EINTR); /* interrompu par un signal : on se rendort jusqu'a l'heure prevue */
 
        clock_gettime(CLOCK_MONOTONIC, &reel); /* heure reelle du reveil */
        jitter_enregistrer("filtrage", &prochain, &reel);
 
        derniere_sortie = traiter(valeur_provisoire(n));
#ifdef FILTRAGE_DEBUG
        printf("filtrage #%lu : %.2f\n", n, derniere_sortie);
#endif
        n++;
    }
    return NULL;
}