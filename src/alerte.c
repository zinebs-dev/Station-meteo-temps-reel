/* src/alerte.c */
#include <stddef.h>
#include "alerte.h"

void *alerte_tache(void *arg) {
    (void)arg;
    /* TODO : attente sur variable de condition */
    return NULL;
}

void alerte_declencher(double temperature) {
    (void)temperature;
    /* TODO : reveiller alerte_tache */
}