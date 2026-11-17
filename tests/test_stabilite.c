/* tests/test_stabilite.c : le filtre rend-il les valeurs affichees plus stables ?
   Utilise le VRAI capteur simule (module de A) et la VRAIE moyenne glissante.
   On reproduit le rythme du programme : le capteur produit une mesure toutes les 100 ms,
   le filtrage lit 2 mesures par periode de 200 ms puis publie. Pas de thread, pas de sudo. */
#include <stdio.h>

#include "capteur.h"
#include "mesure.h"
#include "moyenne.h"

#define N_PUBLICATIONS  150   /* 150 x 200 ms = 30 s de fonctionnement simule */
#define A_IGNORER         5   /* on ignore le demarrage, tant que la fenetre n'est pas pleine */

static int echecs = 0;

#define VERIFIER(cond, msg)                                   \
    do {                                                      \
        if (cond) {                                           \
            printf("  OK   : %s\n", msg);                     \
        } else {                                              \
            printf("  FAIL : %s\n", msg);                     \
            echecs++;                                         \
        }                                                     \
    } while (0)

/* Mesure du "bruit" d'une suite de valeurs : moyenne des |x[k] - x[k-1]|.
   Une suite stable a de petits ecarts d'une valeur a la suivante. */
typedef struct {
    double precedent;
    double somme;
    int n;
    int vu;
} ecarts_t;

static void ecarts_ajouter(ecarts_t *e, double x)
{
    if (e->vu) {
        double d = x - e->precedent;
        e->somme += (d < 0.0) ? -d : d;
        e->n++;
    }
    e->precedent = x;
    e->vu = 1;
}

static double ecarts_moyenne(const ecarts_t *e)
{
    return e->n > 0 ? e->somme / e->n : 0.0;
}

int main(void)
{
    moyenne_t fenetre;
    mesure_t brute, filtree;
    ecarts_t brut_t = {0}, brut_h = {0}, brut_p = {0};
    ecarts_t filt_t = {0}, filt_h = {0}, filt_p = {0};

    capteur_forcer_temperature(0.0);   /* mode normal */
    moyenne_init(&fenetre);

    printf("Valeurs telles que l'affichage les montrerait (1 decimale) :\n");
    printf("   n | T brute  T filtree | H brute  H filtree | P brute   P filtree\n");

    for (int k = 0; k < N_PUBLICATIONS; k++) {
        /* 2 mesures du capteur (100 ms chacune) = une periode de filtrage (200 ms) */
        capteur_lire(&brute);
        moyenne_ajouter(&fenetre, &brute);
        capteur_lire(&brute);
        moyenne_ajouter(&fenetre, &brute);
        moyenne_calculer(&fenetre, &filtree);

        if (k >= A_IGNORER) {
            ecarts_ajouter(&brut_t, brute.temperature);
            ecarts_ajouter(&brut_h, brute.humidite);
            ecarts_ajouter(&brut_p, brute.pression);
            ecarts_ajouter(&filt_t, filtree.temperature);
            ecarts_ajouter(&filt_h, filtree.humidite);
            ecarts_ajouter(&filt_p, filtree.pression);
        }
        if (k >= A_IGNORER && k < A_IGNORER + 8)
            printf("  %3d | %6.1f   %6.1f   | %6.1f   %6.1f   | %7.1f   %7.1f\n", k,
                   brute.temperature, filtree.temperature,
                   brute.humidite, filtree.humidite,
                   brute.pression, filtree.pression);
    }

    printf("\nEcart moyen entre deux valeurs successives (plus petit = plus stable) :\n");
    printf("  Temperature : brute %.3f C   filtree %.3f C   (filtree = %.0f %% de la brute)\n",
           ecarts_moyenne(&brut_t), ecarts_moyenne(&filt_t),
           100.0 * ecarts_moyenne(&filt_t) / ecarts_moyenne(&brut_t));
    printf("  Humidite    : brute %.3f %%   filtree %.3f %%   (filtree = %.0f %% de la brute)\n",
           ecarts_moyenne(&brut_h), ecarts_moyenne(&filt_h),
           100.0 * ecarts_moyenne(&filt_h) / ecarts_moyenne(&brut_h));
    printf("  Pression    : brute %.3f hPa  filtree %.3f hPa  (filtree = %.0f %% de la brute)\n\n",
           ecarts_moyenne(&brut_p), ecarts_moyenne(&filt_p),
           100.0 * ecarts_moyenne(&filt_p) / ecarts_moyenne(&brut_p));

    VERIFIER(ecarts_moyenne(&filt_t) < 0.7 * ecarts_moyenne(&brut_t), "temperature filtree plus stable que brute");
    VERIFIER(ecarts_moyenne(&filt_h) < 0.7 * ecarts_moyenne(&brut_h), "humidite filtree plus stable que brute");
    VERIFIER(ecarts_moyenne(&filt_p) < 0.7 * ecarts_moyenne(&brut_p), "pression filtree plus stable que brute");

    printf("\n%s\n", echecs == 0 ? "PASS" : "RESULTAT : FAIL");
    return echecs == 0 ? 0 : 1;
}