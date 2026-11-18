/* tests/test_donnees.c : tests unitaires du module donnees (sans sudo). */
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "donnees.h"
#include "mesure.h"

#define N_ECRITURES 200000

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

/* Ecrivain : publie des mesures dont les 3 champs sont toujours egaux (i, i, i). */
static void *ecrivain(void *arg)
{
    (void)arg;
    for (int i = 1; i <= N_ECRITURES; i++) {
        mesure_t m;
        memset(&m, 0, sizeof m);
        m.temperature = i;
        m.humidite = i;
        m.pression = i;
        donnees_ecrire(&m);
    }
    return NULL;
}

int main(void)
{
    mesure_t m;

    printf("Test 1 : avant donnees_init()\n");
    VERIFIER(donnees_protocole() == -1, "protocole inconnu (-1) avant l'initialisation");

    donnees_init();

    printf("Test 2 : mutex a heritage de priorite\n");
    VERIFIER(donnees_protocole() == PTHREAD_PRIO_INHERIT,
             "pthread_mutexattr_getprotocol == PTHREAD_PRIO_INHERIT");

    printf("Test 3 : lecture avant toute ecriture\n");
    memset(&m, 0xFF, sizeof m);
    donnees_lire(&m);
    VERIFIER(m.temperature == 0.0 && m.humidite == 0.0 && m.pression == 0.0,
             "valeurs initiales a zero");

    printf("Test 4 : ecriture puis lecture\n");
    mesure_t e = { .temperature = 21.5, .humidite = 48.0, .pression = 1013.2 };
    e.horodatage.tv_sec = 42;
    e.horodatage.tv_nsec = 7;
    donnees_ecrire(&e);
    memset(&m, 0, sizeof m);
    donnees_lire(&m);
    VERIFIER(m.temperature == 21.5 && m.humidite == 48.0 && m.pression == 1013.2,
             "temperature, humidite et pression relues identiques");
    VERIFIER(m.horodatage.tv_sec == 42 && m.horodatage.tv_nsec == 7,
             "horodatage relu identique");

    printf("Test 5 : un ecrivain et un lecteur en parallele (%d ecritures)\n", N_ECRITURES);
    pthread_t th;
    long lectures = 0, incoherentes = 0;
    /* Etat de depart coherent (0, 0, 0) : sinon la valeur du test 4 serait comptee a tort. */
    mesure_t zero;
    memset(&zero, 0, sizeof zero);
    donnees_ecrire(&zero);
    pthread_create(&th, NULL, ecrivain, NULL);
    for (;;) {
        donnees_lire(&m);
        lectures++;
        if (m.temperature != m.humidite || m.humidite != m.pression)
            incoherentes++;
        if (m.temperature == N_ECRITURES)
            break;
    }
    pthread_join(th, NULL);
    printf("  %ld lectures, %ld incoherentes\n", lectures, incoherentes);
    VERIFIER(incoherentes == 0, "aucune lecture a moitie ecrite");

    printf("\n%s\n", echecs == 0 ? "PASS" : "RESULTAT : FAIL");
    return echecs == 0 ? 0 : 1;
}