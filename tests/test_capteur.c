#include <stdio.h>
#include <math.h>
#include "capteur.h"

static int echecs = 0;

#define VERIFIER(cond, msg) do { \
    if (!(cond)) { printf("ECHEC : %s\n", msg); echecs++; } \
} while (0)

int main(void) {
    mesure_t m, prec;

    /* Pointeur NULL */
    VERIFIER(capteur_lire(NULL) == -1, "NULL doit renvoyer -1");

    /* Lectures : plages et pas de saut brutal (20000 lectures = cycles complets) */
    capteur_lire(&prec);
    for (int i = 0; i < 20000; i++) {
        VERIFIER(capteur_lire(&m) == 0, "lecture doit renvoyer 0");
        VERIFIER(m.temperature >= 15.0 && m.temperature <= 30.0, "temperature hors plage");
        VERIFIER(m.humidite    >= 30.0 && m.humidite    <= 80.0, "humidite hors plage");
        VERIFIER(m.pression    >= 1000.0 && m.pression  <= 1025.0, "pression hors plage");
        VERIFIER(fabs(m.temperature - prec.temperature) < 1.0, "saut de temperature");
        VERIFIER(fabs(m.humidite - prec.humidite) < 2.0, "saut d'humidite");
        VERIFIER(fabs(m.pression - prec.pression) < 1.0, "saut de pression");
        prec = m;
        if (echecs > 10) break;
    }

    /* Forcage a 45 C puis retour au mode normal */
    capteur_forcer_temperature(45.0);
    capteur_lire(&m);
    VERIFIER(m.temperature == 45.0, "forcage a 45 doit renvoyer 45");

    capteur_forcer_temperature(0);
    capteur_lire(&m);
    VERIFIER(m.temperature <= 30.0, "retour au mode normal");

    if (echecs == 0)
        printf("Tous les tests capteur passent\n");
    return echecs ? 1 : 0;
}
