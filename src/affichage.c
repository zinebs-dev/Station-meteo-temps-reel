#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "affichage.h"
#include "donnees.h"
#include "jitter.h"
#include "mesure.h"
#include "temps.h"

#define PERIODE_NS 1000000000L /* 1000 ms */

static FILE *meteo = NULL;

void affichage_enregistrer_meteo(const char *fichier) {
    meteo = fopen(fichier, "w");
    if (!meteo) {
        perror("affichage : ouverture du fichier meteo");
        return;
    }
    fprintf(meteo, "temps_s,temperature,humidite\n");
    fflush(meteo);
}

void affichage_fermer_meteo(void) {
    if (meteo) {
        fclose(meteo);
        meteo = NULL;
    }
}

void *affichage_tache(void *arg) {
    (void)arg;
    struct timespec prochain, reel, now, debut;
    mesure_t m;

    clock_gettime(CLOCK_MONOTONIC, &debut);
    prochain = debut;
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

        if (meteo) {
            fprintf(meteo, "%.1f,%.2f,%.2f\n",
                    (double)diff_us(&debut, &reel) / 1e6,
                    m.temperature, m.humidite);
            fflush(meteo);
        }
    }
    return NULL;
}