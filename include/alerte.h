#ifndef ALERTE_H
#define ALERTE_H

/* Tache sporadique : dort jusqu'a un reveil par alerte_declencher(). */
void *alerte_tache(void *arg);

/* Reveille la tache d'alerte (appelee par le filtrage quand T depasse 40 C). */
void alerte_declencher(double temperature);

#endif /* ALERTE_H */