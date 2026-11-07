#ifndef MESURE_H
#define MESURE_H

#include <time.h>

typedef struct {
    double temperature;        /* degres C */
    double humidite;           /* pourcentage */
    double pression;           /* hPa */
    struct timespec horodatage;
} mesure_t;

#endif
