#include <stdlib.h>
#include <time.h>
#include "capteur.h"

/* Plages : T 15-30 C, H 30-80 %, P 1000-1025 hPa.
   Variation lente : onde triangulaire (sans libm, donc le Makefile
   n'a pas besoin de -lm). A 10 Hz, 6000 pas = un cycle de 10 min. */
#define PERIODE_TH   6000UL
#define PERIODE_P    9000UL

static unsigned long pas = 0;               /* nombre de lectures */
static unsigned int graine = 12345;         /* graine du bruit */
static volatile double temperature_forcee = 0.0;  /* 0 = mode normal */

/* Onde triangulaire entre 0 et 1 */
static double triangle(unsigned long n, unsigned long periode) {
    double x = (double)(n % periode) / (double)periode;
    return (x < 0.5) ? 2.0 * x : 2.0 - 2.0 * x;
}

/* Bruit aleatoire dans [-amp, +amp] */
static double bruit(double amp) {
    return ((double)rand_r(&graine) / (double)RAND_MAX * 2.0 - 1.0) * amp;
}

int capteur_lire(mesure_t *m) {
    if (m == NULL)
        return -1;

    m->temperature = 16.0 + 13.0 * triangle(pas, PERIODE_TH) + bruit(0.2);
    m->humidite    = 75.0 - 40.0 * triangle(pas, PERIODE_TH) + bruit(0.5);
    m->pression    = 1002.0 + 20.0 * triangle(pas, PERIODE_P) + bruit(0.3);

    if (temperature_forcee != 0.0)
        m->temperature = temperature_forcee;

    clock_gettime(CLOCK_MONOTONIC, &m->horodatage);
    pas++;
    return 0;
}

void capteur_forcer_temperature(double t) {
    temperature_forcee = t;
}
