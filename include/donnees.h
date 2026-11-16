#ifndef DONNEES_H
#define DONNEES_H

#include "mesure.h"

/* Initialise le mutex a heritage de priorite. A appeler une fois au demarrage (main). */
void donnees_init(void);

/* Ecrit la derniere valeur filtree (protege par le mutex). */
void donnees_ecrire(const mesure_t *m);

/* Copie dans *m la derniere valeur filtree (protege par le mutex). */
void donnees_lire(mesure_t *m);
/* Pour les tests : protocole du mutex tel que relu par pthread_mutexattr_getprotocol()
   apres donnees_init(). Retourne PTHREAD_PRIO_INHERIT si tout est correct,
   et -1 si donnees_init() n'a pas ete appelee. */
   
int donnees_protocole(void);

#endif /* DONNEES_H */