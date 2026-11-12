#ifndef DONNEES_H
#define DONNEES_H

#include "mesure.h"

/* Initialise le mutex a heritage de priorite. A appeler une fois au demarrage (main). */
void donnees_init(void);

/* Ecrit la derniere valeur filtree (protege par le mutex). */
void donnees_ecrire(const mesure_t *m);

/* Copie dans *m la derniere valeur filtree (protege par le mutex). */
void donnees_lire(mesure_t *m);

#endif /* DONNEES_H */