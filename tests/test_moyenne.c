/* tests/test_moyenne.c : tests unitaires de la moyenne glissante (sans thread, sans sudo). */
#include <stdio.h>
#include <string.h>

#include "moyenne.h"
#include "mesure.h"

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

/* Egalite a 1e-9 pres (les doubles ne sont pas toujours exacts). */
static int proche(double a, double b)
{
    double d = a - b;
    return d > -1e-9 && d < 1e-9;
}

/* Mesure de test : T = t, H = 10*t, P = 1000 + t. */
static mesure_t mesure(double t)
{
    mesure_t m;
    memset(&m, 0, sizeof m);
    m.temperature = t;
    m.humidite = 10.0 * t;
    m.pression = 1000.0 + t;
    return m;
}

int main(void)
{
    moyenne_t f;
    mesure_t m, r;

    printf("Test 1 : fenetre vide\n");
    moyenne_init(&f);
    VERIFIER(moyenne_calculer(&f, &r) == -1, "moyenne_calculer renvoie -1 sur une fenetre vide");

    printf("Test 2 : une seule valeur\n");
    m = mesure(7.0);
    moyenne_ajouter(&f, &m);
    VERIFIER(moyenne_calculer(&f, &r) == 0 && proche(r.temperature, 7.0)
             && proche(r.humidite, 70.0) && proche(r.pression, 1007.0),
             "la moyenne d'une valeur est cette valeur");

    printf("Test 3 : fenetre partielle (2 valeurs : 10 et 20)\n");
    moyenne_init(&f);
    m = mesure(10.0); moyenne_ajouter(&f, &m);
    m = mesure(20.0); moyenne_ajouter(&f, &m);
    moyenne_calculer(&f, &r);
    VERIFIER(proche(r.temperature, 15.0) && proche(r.humidite, 150.0) && proche(r.pression, 1015.0),
             "moyenne des 2 valeurs presentes = 15 / 150 / 1015");

    printf("Test 4 : sequence connue 1,2,3,4,5 puis 6, 7\n");
    moyenne_init(&f);
    for (int i = 1; i <= 5; i++) { m = mesure(i); moyenne_ajouter(&f, &m); }
    moyenne_calculer(&f, &r);
    VERIFIER(proche(r.temperature, 3.0) && proche(r.humidite, 30.0) && proche(r.pression, 1003.0),
             "fenetre {1..5} -> T=3, H=30, P=1003");
    m = mesure(6.0); moyenne_ajouter(&f, &m);
    moyenne_calculer(&f, &r);
    VERIFIER(proche(r.temperature, 4.0) && proche(r.humidite, 40.0) && proche(r.pression, 1004.0),
             "fenetre {2..6} -> T=4, H=40, P=1004 (le 1 est oublie)");
    m = mesure(7.0); moyenne_ajouter(&f, &m);
    moyenne_calculer(&f, &r);
    VERIFIER(proche(r.temperature, 5.0) && proche(r.humidite, 50.0) && proche(r.pression, 1005.0),
             "fenetre {3..7} -> T=5, H=50, P=1005");

    printf("Test 5 : longue sequence 1..100 (tour complet de la fenetre, plusieurs fois)\n");
    moyenne_init(&f);
    int ok = 1;
    for (int i = 1; i <= 100; i++) {
        m = mesure(i);
        moyenne_ajouter(&f, &m);
        moyenne_calculer(&f, &r);
        /* A partir de la 5e valeur, la moyenne de {i-4 .. i} vaut i - 2. */
        if (i >= 5 && !proche(r.temperature, i - 2.0))
            ok = 0;
    }
    VERIFIER(ok, "pour i >= 5, la moyenne vaut i - 2 a chaque pas");

    printf("Test 6 : horodatage de la derniere mesure\n");
    moyenne_init(&f);
    m = mesure(1.0); m.horodatage.tv_sec = 100; m.horodatage.tv_nsec = 5; moyenne_ajouter(&f, &m);
    m = mesure(2.0); m.horodatage.tv_sec = 200; m.horodatage.tv_nsec = 9; moyenne_ajouter(&f, &m);
    moyenne_calculer(&f, &r);
    VERIFIER(r.horodatage.tv_sec == 200 && r.horodatage.tv_nsec == 9,
             "l'horodatage du resultat est celui de la derniere mesure");

    printf("Test 7 : le filtre rend un signal bruite plus stable\n");
    moyenne_init(&f);
    double brut_min = 1e9, brut_max = -1e9, filt_min = 1e9, filt_max = -1e9;
    for (int i = 0; i < 50; i++) {
        m = mesure((i % 2 == 0) ? 19.0 : 21.0);     /* bruit : 19, 21, 19, 21 ... */
        moyenne_ajouter(&f, &m);
        moyenne_calculer(&f, &r);
        if (i >= MOYENNE_FENETRE) {                  /* fenetre pleine */
            if (m.temperature < brut_min) brut_min = m.temperature;
            if (m.temperature > brut_max) brut_max = m.temperature;
            if (r.temperature < filt_min) filt_min = r.temperature;
            if (r.temperature > filt_max) filt_max = r.temperature;
        }
    }
    printf("  amplitude brute = %.2f, amplitude filtree = %.2f\n",
           brut_max - brut_min, filt_max - filt_min);
    VERIFIER((filt_max - filt_min) < (brut_max - brut_min), "amplitude filtree < amplitude brute");

    printf("Test 8 : pointeurs NULL (aucun plantage)\n");
    moyenne_init(NULL);
    moyenne_ajouter(NULL, &m);
    moyenne_ajouter(&f, NULL);
    VERIFIER(moyenne_calculer(NULL, &r) == -1 && moyenne_calculer(&f, NULL) == -1,
             "moyenne_calculer renvoie -1 avec un pointeur NULL");

    printf("\n%s\n", echecs == 0 ? "PASS" : "RESULTAT : FAIL");
    return echecs == 0 ? 0 : 1;
}