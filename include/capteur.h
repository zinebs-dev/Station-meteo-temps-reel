#ifndef CAPTEUR_H
#define CAPTEUR_H

#include "mesure.h"

/* Remplit m avec une mesure simulee. Retourne 0 si OK, -1 sinon. */
int capteur_lire(mesure_t *m);

/* Force la temperature simulee a t (test de l'alerte).
   t = 0 : retour au mode normal. */
void capteur_forcer_temperature(double t);

#endif
