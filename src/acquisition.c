#include <stddef.h>
#include <time.h>
#include "acquisition.h"
#include "capteur.h"
#include "temps.h"
#include "jitter.h"

#ifdef ACQ_DEBUG
#include <stdio.h>
#endif

#define PERIODE_NS 100000000L   /* 100 ms = 10 Hz */

void *acquisition_tache(void *arg) {
    (void)arg;

    mesure_t m;
    struct timespec prochain, reel;

    clock_gettime(CLOCK_MONOTONIC, &prochain);

#ifdef ACQ_DEBUG
    struct timespec debut = prochain;
    double precedent = 0.0;
    long n = 0;
#endif

    for (;;) {
        /* Prochain reveil = reveil precedent + periode (pas de derive) */
        ajouter_ns(&prochain, PERIODE_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &prochain, NULL);

        /* Heure reelle du reveil, pour mesurer le jitter */
        clock_gettime(CLOCK_MONOTONIC, &reel);
        jitter_enregistrer("acquisition", &prochain, &reel);

        /* Travail de la tache : lecture du capteur */
        capteur_lire(&m);
        /* TODO apres : buffer_push((buffer_t *)arg, &m); */

#ifdef ACQ_DEBUG
        n++;
        double t = (reel.tv_sec - debut.tv_sec) * 1000.0
                 + (reel.tv_nsec - debut.tv_nsec) / 1e6;
        /* n, intervalle depuis le reveil precedent (ms), derive cumulee (ms) */
        printf("%ld,%.3f,%.3f\n", n, t - precedent, t - n * 100.0);
        precedent = t;
#endif
    }
    return NULL;
}
