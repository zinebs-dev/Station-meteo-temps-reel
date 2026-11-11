/* src/donnees.c */
#include "donnees.h"

void donnees_init(void) {
    /* TODO : mutex PTHREAD_PRIO_INHERIT */
}

void donnees_ecrire(const mesure_t *m) {
    (void)m;
    /* TODO : verrouiller, copier, deverrouiller */
}

void donnees_lire(mesure_t *m) {
    (void)m;
    /* TODO : verrouiller, copier, deverrouiller */
}