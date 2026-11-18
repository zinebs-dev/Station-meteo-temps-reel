/* tests/test_alerte.c : test isole du module alerte (sans filtrage ni main) */
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "alerte.h"
#include "journal.h"
#include "temps.h"

#define FICHIER "/tmp/test_alerte_journal.log"

static int echecs = 0;

#define VERIFIER(cond, msg) do {                              \
        if (cond) printf("  OK    : %s\n", msg);              \
        else { printf("  ECHEC : %s\n", msg); echecs++; }     \
    } while (0)

static void attendre_ms(long ms)
{
    struct timespec p = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&p, NULL);
}

/* Compte les lignes du journal contenant "ALERTE" (-1 si fichier absent). */
static int compter_alertes(void)
{
    FILE *f = fopen(FICHIER, "r");
    char ligne[256];
    int n = 0;
    if (f == NULL) return -1;
    while (fgets(ligne, sizeof ligne, f))
        if (strstr(ligne, "ALERTE")) n++;
    fclose(f);
    return n;
}

int main(void)
{
    pthread_t th;
    struct timespec debut, fin;

    journal_init(FICHIER);
    pthread_create(&th, NULL, alerte_tache, NULL);

    printf("Test 1 : thread demarre, aucun declenchement (300 ms)\n");
    attendre_ms(300);
    VERIFIER(compter_alertes() == 0, "aucune entree d'alerte dans le journal");

    printf("Test 2 : declenchement a 45 C\n");
    clock_gettime(CLOCK_MONOTONIC, &debut);
    alerte_declencher(45.0);
    long delai_us = -1;
    for (int i = 0; i < 1000; i++) {
        if (compter_alertes() >= 1) {
            clock_gettime(CLOCK_MONOTONIC, &fin);
            delai_us = diff_us(&debut, &fin);
            break;
        }
        attendre_ms(1);
    }
    printf("  delai mesure : %ld us\n", delai_us);
    VERIFIER(delai_us >= 0 && delai_us < 1000000, "entree de journal en moins d'une seconde");
    attendre_ms(100);
    VERIFIER(compter_alertes() == 1, "exactement une entree dans le journal");

    printf("Test 3 : annulation du thread puis nouveau declenchement\n");
    pthread_cancel(th);
    pthread_join(th, NULL);
    VERIFIER(1, "pthread_cancel + pthread_join terminent");
    alarm(3); /* garde-fou : si le mutex est reste verrouille, le programme est tue apres 3 s */
    alerte_declencher(50.0);
    alarm(0);
    VERIFIER(1, "alerte_declencher ne bloque pas (mutex libere a l'annulation)");

    journal_arreter();
    printf("\n%s (%d echec(s))\n", echecs == 0 ? "TOUS LES TESTS PASSENT" : "ECHEC", echecs);
    return echecs == 0 ? 0 : 1;
}