#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "affichage.h"
#include "donnees.h"
#include "jitter.h"
#include "mesure.h"
#include "temps.h"

#define PERIODE_NS 1000000000L /* 1000 ms */

void *affichage_tache(void *arg) {
    (void)arg;
    struct timespec prochain, reel, now;
    mesure_t m;

    clock_gettime(CLOCK_MONOTONIC, &prochain);
    for (;;) {
        ajouter_ns(&prochain, PERIODE_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &prochain, NULL);
        clock_gettime(CLOCK_MONOTONIC, &reel);
        jitter_enregistrer("affichage", &prochain, &reel);

        donnees_lire(&m);
        clock_gettime(CLOCK_REALTIME, &now);
        struct tm tm;
        localtime_r(&now.tv_sec, &tm);
        printf("[%02d:%02d:%02d.%03ld] T=%.1f C  H=%.1f %%  P=%.1f hPa\n",
               tm.tm_hour, tm.tm_min, tm.tm_sec, now.tv_nsec / 1000000L,
               m.temperature, m.humidite, m.pression);
        fflush(stdout);
    }
    return NULL;
}
