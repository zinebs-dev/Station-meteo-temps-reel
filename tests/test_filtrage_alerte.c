/* tests/test_filtrage_alerte.c : le filtrage declenche l'alerte au franchissement de 40 C */
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "alerte.h"
#include "buffer.h"
#include "donnees.h"
#include "filtrage.h"
#include "jitter.h"
#include "journal.h"
#include "mesure.h"

#define FICHIER_JOURNAL "/tmp/test_filtrage_alerte_journal.log"
#define FICHIER_JITTER  "/tmp/test_filtrage_alerte_jitter.csv"

static buffer_t tampon;
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

/* Depose n mesures identiques a la temperature donnee, comme le ferait l'acquisition. */
static void pousser(double temperature, int n)
{
    for (int i = 0; i < n; i++) {
        mesure_t m;
        memset(&m, 0, sizeof m);
        m.temperature = temperature;
        m.humidite = 50.0;
        m.pression = 1013.0;
        clock_gettime(CLOCK_MONOTONIC, &m.horodatage);
        if (buffer_push(&tampon, &m) != 0)
            printf("  (buffer plein : mesure perdue)\n");
    }
}

static int compter_alertes(void)
{
    FILE *f = fopen(FICHIER_JOURNAL, "r");
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
    atomic_int arret = 0;
    pthread_t th_filtrage, th_alerte;

    journal_init(FICHIER_JOURNAL);
    jitter_init(FICHIER_JITTER);
    donnees_init();
    buffer_init(&tampon);
    filtrage_brancher(&tampon);

    pthread_create(&th_alerte, NULL, alerte_tache, NULL);
    pthread_create(&th_filtrage, NULL, filtrage_tache, &arret);

    printf("Phase A : 10 mesures a 20 C (mode normal)\n");
    pousser(20.0, 10);
    attendre_ms(600);
    VERIFIER(compter_alertes() == 0, "aucune alerte en dessous de 40 C");

    printf("Phase B : 10 mesures a 45 C (franchissement du seuil)\n");
    pousser(45.0, 10);
    attendre_ms(600);
    VERIFIER(compter_alertes() == 1, "une alerte declenchee");

    printf("Phase C : la temperature reste a 45 C pendant plusieurs cycles\n");
    pousser(45.0, 3);
    attendre_ms(300);
    pousser(45.0, 3);
    attendre_ms(400);
    VERIFIER(compter_alertes() == 1, "pas de nouvelle alerte tant que T reste au-dessus du seuil");

    printf("Phase D : retour a 20 C\n");
    pousser(20.0, 10);
    attendre_ms(600);
    VERIFIER(compter_alertes() == 1, "retour a la normale : pas d'alerte supplementaire");

    printf("Phase E : nouveau franchissement a 46 C\n");
    pousser(46.0, 10);
    attendre_ms(600);
    VERIFIER(compter_alertes() == 2, "l'alerte est rearmee : deuxieme alerte declenchee");

    atomic_store(&arret, 1);
    pthread_join(th_filtrage, NULL);
    pthread_cancel(th_alerte);
    pthread_join(th_alerte, NULL);
    journal_arreter();
    jitter_fermer();

    printf("\n%s (%d echec(s))\n", echecs == 0 ? "TOUS LES TESTS PASSENT" : "ECHEC", echecs);
    return echecs == 0 ? 0 : 1;
}