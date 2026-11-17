#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdatomic.h>
#include <stddef.h>
#include <time.h>

#include "buffer.h"
#include "donnees.h"
#include "filtrage.h"
#include "jitter.h"
#include "mesure.h"
#include "moyenne.h"
#include "temps.h"

#ifdef FILTRAGE_DEBUG
#include <stdio.h>
#endif
 
#define PERIODE_NS 200000000L /* 200 ms = 5 Hz */
 

static buffer_t *tampon_entree = NULL;

void filtrage_brancher(buffer_t *b)
{
    tampon_entree = b;
}



void *filtrage_tache(void *arg)
{
    atomic_int *arret = (atomic_int *)arg; /* NULL = ne s'arrete jamais */
    struct timespec prochain, reel;
    moyenne_t fenetre;
    mesure_t brute, filtree;
    int r;

    moyenne_init(&fenetre);

    clock_gettime(CLOCK_MONOTONIC, &prochain);

    while (arret == NULL || atomic_load(arret) == 0) {
        /* Prochain reveil = reveil precedent + 200 ms :
         * les retards ne s'accumulent pas. */
        ajouter_ns(&prochain, PERIODE_NS);
        do {
            r = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &prochain, NULL);
        } while (r == EINTR); /* interrompu par un signal : on se rendort jusqu'a l'heure prevue */

        clock_gettime(CLOCK_MONOTONIC, &reel); /* heure reelle du reveil */
        jitter_enregistrer("filtrage", &prochain, &reel);

        /* 1. Vider le buffer. buffer_pop() ne bloque jamais : -1 si vide.
         *    La boucle est bornee a BUFFER_TAILLE lectures : duree maximale connue. */
        unsigned nouvelles = 0;
        if (tampon_entree != NULL) {
            for (unsigned i = 0; i < BUFFER_TAILLE; i++) {
                if (buffer_pop(tampon_entree, &brute) != 0)
                    break;
                moyenne_ajouter(&fenetre, &brute);
                nouvelles++;
            }
        }

        /* 2. Publier seulement s'il y a du nouveau. Buffer vide : on ne fait rien,
         *    donnees garde la derniere valeur publiee. */
        if (nouvelles > 0 && moyenne_calculer(&fenetre, &filtree) == 0) {
            donnees_ecrire(&filtree);
#ifdef FILTRAGE_DEBUG
            printf("filtrage : %u nouvelle(s), T filtree = %.2f\n", nouvelles, filtree.temperature);
#endif
        }
    }
    return NULL;
}