#ifndef ALERTE_H
#define ALERTE_H

/* Seuil de declenchement de l'alerte (en degres C) : alerte si T > ALERTE_SEUIL_C. */
#define ALERTE_SEUIL_C 40.0
/* Tache sporadique : dort jusqu'a un reveil par alerte_declencher(). */
void *alerte_tache(void *arg);

/* Reveille la tache d'alerte (appelee par le filtrage quand T depasse 40 C). */
void alerte_declencher(double temperature);

#endif /* ALERTE_H */