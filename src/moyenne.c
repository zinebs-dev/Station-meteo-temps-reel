/* src/moyenne.c : moyenne glissante sur les MOYENNE_FENETRE dernieres mesures. */
#include <stddef.h>

#include "moyenne.h"

void moyenne_init(moyenne_t *f)
{
    if (f == NULL)
        return;

    for (unsigned i = 0; i < MOYENNE_FENETRE; i++) {
        f->temperature[i] = 0.0;
        f->humidite[i] = 0.0;
        f->pression[i] = 0.0;
    }
    f->horodatage.tv_sec = 0;
    f->horodatage.tv_nsec = 0;
    f->nb = 0;
    f->suivant = 0;
}

void moyenne_ajouter(moyenne_t *f, const mesure_t *m)
{
    if (f == NULL || m == NULL)
        return;

    f->temperature[f->suivant] = m->temperature;
    f->humidite[f->suivant] = m->humidite;
    f->pression[f->suivant] = m->pression;
    f->horodatage = m->horodatage;

    f->suivant = (f->suivant + 1) % MOYENNE_FENETRE;   /* on revient a 0 apres la derniere case */
    if (f->nb < MOYENNE_FENETRE)
        f->nb++;
}

int moyenne_calculer(const moyenne_t *f, mesure_t *resultat)
{
    if (f == NULL || resultat == NULL || f->nb == 0)
        return -1;

    double somme_t = 0.0, somme_h = 0.0, somme_p = 0.0;

    /* On re-somme a chaque fois (au plus 5 valeurs) : pas d'erreur qui s'accumule. */
    for (unsigned i = 0; i < f->nb; i++) {
        somme_t += f->temperature[i];
        somme_h += f->humidite[i];
        somme_p += f->pression[i];
    }

    resultat->temperature = somme_t / f->nb;
    resultat->humidite = somme_h / f->nb;
    resultat->pression = somme_p / f->nb;
    resultat->horodatage = f->horodatage;
    return 0;
}