#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "filtrage.h"
#include "jitter.h"
#include "temps.h"

#define MAX_ECH 2000
#define PERIODE_US 200000L
#define SEUIL_P99_US 20000L /* retard P99 tolere : 20 ms (a ajuster apres mesures) */

/* ---- Faux module jitter ---- */

static struct timespec g_prevu[MAX_ECH];
static struct timespec g_reel[MAX_ECH];
static int g_n = 0;
static char g_nom[32] = "";

void jitter_init(const char *fichier)
{
    (void)fichier;
}

void jitter_fermer(void)
{
}

void jitter_enregistrer(const char *tache,
                        const struct timespec *prevu,
                        const struct timespec *reel)
{
    if (g_n == 0) {
        strncpy(g_nom, tache, sizeof g_nom - 1);
        g_nom[sizeof g_nom - 1] = '\0';
    }

    if (g_n < MAX_ECH) {
        g_prevu[g_n] = *prevu;
        g_reel[g_n] = *reel;
        g_n++;
    }
}

/* Comparaison pour qsort (ordre croissant). */
static int cmp_long(const void *a, const void *b)
{
    long x = *(const long *)a;
    long y = *(const long *)b;
    return (x > y) - (x < y);
}

/* ---- Creation du thread de filtrage ---- */

static int lancer(pthread_t *th, atomic_int *arret)
{
    pthread_attr_t attr;
    struct sched_param sp = { .sched_priority = 50 };
    int r;

    r = pthread_attr_init(&attr);
    if (r != 0) {
        return r;
    }

    /* Demande explicite de SCHED_FIFO et de la priorite 50. */
    r = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (r == 0) {
        r = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    }
    if (r == 0) {
        r = pthread_attr_setschedparam(&attr, &sp);
    }

    if (r != 0) {
        fprintf(stderr, "Erreur configuration SCHED_FIFO : %s\n", strerror(r));
        pthread_attr_destroy(&attr);
        return r;
    }

    r = pthread_create(th, &attr, filtrage_tache, arret);
    pthread_attr_destroy(&attr);

    /* Si SCHED_FIFO est refuse (pas de sudo), on lance en mode normal. */
    if (r == EPERM) {
        printf("SCHED_FIFO refuse (lancer avec sudo) : ordonnancement normal.\n");

        r = pthread_create(th, NULL, filtrage_tache, arret);

        if (r == 0) {
            int policy;
            struct sched_param param;

            if (pthread_getschedparam(*th, &policy, &param) == 0) {
                printf("Politique active : %s, priorite %d\n",
                       (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                       (policy == SCHED_FIFO)  ? "SCHED_FIFO"  : "AUTRE",
                       param.sched_priority);
            }
        }
        return r;
    }

    if (r != 0) {
        return r;
    }

    /* Verification que le thread utilise bien SCHED_FIFO, priorite 50. */
    {
        int policy;
        struct sched_param param;

        r = pthread_getschedparam(*th, &policy, &param);

        if (r != 0 || policy != SCHED_FIFO || param.sched_priority != 50) {
            fprintf(stderr, "Erreur : SCHED_FIFO priorite 50 non confirme.\n");

            atomic_store(arret, 1);
            pthread_join(*th, NULL);

            return (r != 0) ? r : EINVAL;
        }
    }

    printf("SCHED_FIFO actif (priorite 50 verifiee).\n");
    return 0;
}

/* ---- Programme principal ---- */

int main(int argc, char **argv)
{
    int duree = (argc > 1) ? atoi(argv[1]) : 60;
    atomic_int arret;
    pthread_t th;

    int r;
    int i;
    int ok = 1;

    long attendu;
    long somme = 0;
    long pmin = 0;
    long pmax = 0;

    long retards[MAX_ECH];
    long retard_max = 0;
    long avance_max = 0;
    long p99;

    long derive_us;
    double moyenne;

    if (duree < 2 || duree > 390) {
        fprintf(stderr, "usage : %s [duree entre 2 et 390 secondes]\n", argv[0]);
        return 2;
    }

    atomic_init(&arret, 0);

    r = lancer(&th, &arret);
    if (r != 0) {
        fprintf(stderr, "Erreur lancement thread : %s\n", strerror(r));
        return 2;
    }

    printf("Mesure pendant %d secondes...\n", duree);

    sleep((unsigned)duree);

    atomic_store(&arret, 1);
    pthread_join(th, NULL);

    attendu = (long)duree * 5;

    if (g_n < 2) {
        printf("FAIL : seulement %d echantillon(s).\n", g_n);
        return 1;
    }

    /* Periodes reelles entre deux reveils consecutifs. */
    for (i = 1; i < g_n; i++) {
        long periode = (long)diff_us(&g_reel[i - 1], &g_reel[i]);

        somme += periode;

        if (i == 1 || periode < pmin) {
            pmin = periode;
        }
        if (i == 1 || periode > pmax) {
            pmax = periode;
        }
    }

    /* Retard (reel - prevu) de chaque reveil. */
    for (i = 0; i < g_n; i++) {
        long ecart = (long)diff_us(&g_prevu[i], &g_reel[i]);

        retards[i] = ecart;

        if (ecart > retard_max) {
            retard_max = ecart;
        }
        if (ecart < 0 && -ecart > avance_max) {
            avance_max = -ecart;
        }
    }

    qsort(retards, (size_t)g_n, sizeof retards[0], cmp_long);
    p99 = retards[(g_n - 1) * 99 / 100];

    moyenne = (double)somme / (double)(g_n - 1);

    /* Espacement des heures prevues : doit etre exactement n * 200 ms. */
    derive_us = (long)diff_us(&g_prevu[0], &g_prevu[g_n - 1])
                - (long)(g_n - 1) * PERIODE_US;

    /* ---- Affichage ---- */

    printf("\n========== RESULTATS DU TEST ==========\n");
    printf("Tache : %s\n", g_nom);
    printf("Reveils enregistres : %d (attendu %ld +/- 2)\n", g_n, attendu);
    printf("Periode reelle (us) : min %ld | moyenne %.1f | max %ld\n",
           pmin, moyenne, pmax);
    printf("Retard au reveil    : P99 %ld us | max %ld us (info)\n",
           p99, retard_max);
    printf("Avance au reveil    : max %ld us (doit etre 0)\n", avance_max);
    printf("Derive des heures prevues : %ld us (doit etre 0)\n", derive_us);
    printf("=======================================\n\n");

    /* ---- Criteres ---- */

    if (labs((long)g_n - attendu) > 2) {
        printf("FAIL : nombre de reveils incorrect.\n");
        ok = 0;
    }

    if (moyenne < 199000.0 || moyenne > 201000.0) {
        printf("FAIL : periode moyenne hors de 200 ms +/- 1 ms.\n");
        ok = 0;
    }

    if (derive_us != 0) {
        printf("FAIL : derive des heures prevues (verifier ajouter_ns).\n");
        ok = 0;
    }

    if (avance_max != 0) {
        printf("FAIL : reveil en avance sur l'heure prevue.\n");
        ok = 0;
    }

    if (p99 > SEUIL_P99_US) {
        printf("FAIL : retard P99 superieur a %ld us.\n", SEUIL_P99_US);
        ok = 0;
    }

    printf("%s\n", ok ? "PASS" : "RESULTAT : FAIL");

    return ok ? 0 : 1;
}