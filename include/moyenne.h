#ifndef MOYENNE_H
#define MOYENNE_H

#include <time.h>
#include "mesure.h"

/* Nombre de valeurs prises en compte par la moyenne glissante. */
#define MOYENNE_FENETRE 5

/* Fenetre glissante sur les MOYENNE_FENETRE dernieres mesures.
   Pas de thread, pas de mutex : c'est un simple calcul, donc facile a tester. */
typedef struct {
    double temperature[MOYENNE_FENETRE];
    double humidite[MOYENNE_FENETRE];
    double pression[MOYENNE_FENETRE];
    struct timespec horodatage;   /* horodatage de la derniere mesure ajoutee */
    unsigned nb;                  /* valeurs valides : 0 .. MOYENNE_FENETRE */
    unsigned suivant;             /* case ou ecrire la prochaine valeur */
} moyenne_t;

/* Vide la fenetre. */
void moyenne_init(moyenne_t *f);

/* Ajoute une mesure. Quand la fenetre est pleine, la plus ancienne est remplacee. */
void moyenne_ajouter(moyenne_t *f, const mesure_t *m);

/* Ecrit dans *resultat la moyenne des valeurs presentes (temperature, humidite,
   pression) et l'horodatage de la derniere mesure ajoutee.
   Retourne 0 si OK, -1 si la fenetre est vide ou si un pointeur est NULL.
   Tant que la fenetre n'est pas pleine, on moyenne les valeurs deja recues. */
int moyenne_calculer(const moyenne_t *f, mesure_t *resultat);

#endif /* MOYENNE_H */