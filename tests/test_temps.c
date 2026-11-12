/* tests/test_temps.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "temps.h"

#define PERIODE_NS 100000000L /* 100 ms */
#define NB_ITER    50

static int echecs = 0;

#define VERIFIER(cond, msg) do { \
    if (!(cond)) { printf("ECHEC : %s\n", msg); echecs++; } \
} while (0)

static void tests_unitaires(void) {
    struct timespec t, a, b;

    /* ajouter_ns sans retenue */
    t.tv_sec = 1; t.tv_nsec = 100;
    ajouter_ns(&t, 200);
    VERIFIER(t.tv_sec == 1 && t.tv_nsec == 300, "ajout simple");

    /* ajouter_ns avec retenue */
    t.tv_sec = 1; t.tv_nsec = 900000000L;
    ajouter_ns(&t, 200000000L);
    VERIFIER(t.tv_sec == 2 && t.tv_nsec == 100000000L, "retenue sur les secondes");

    /* ajouter_ns avec ns > 1 s */
    t.tv_sec = 0; t.tv_nsec = 500000000L;
    ajouter_ns(&t, 2500000000L);
    VERIFIER(t.tv_sec == 3 && t.tv_nsec == 0, "ns superieur a 1 s");

    /* ajouter_ns avec valeur negative */
    t.tv_sec = 2; t.tv_nsec = 100000000L;
    ajouter_ns(&t, -200000000L);
    VERIFIER(t.tv_sec == 1 && t.tv_nsec == 900000000L, "valeur negative");

    /* diff_us : b - a */
    a.tv_sec = 1; a.tv_nsec = 500000000L;
    b.tv_sec = 2; b.tv_nsec = 250000000L;
    VERIFIER(diff_us(&a, &b) == 750000, "diff_us positif");
    VERIFIER(diff_us(&b, &a) == -750000, "diff_us negatif");
    VERIFIER(diff_us(&a, &a) == 0, "diff_us nul");
}

static void test_boucle_100ms(void) {
    struct timespec prochain, reel;
    long long min = 0, max = 0, somme = 0;

    clock_gettime(CLOCK_MONOTONIC, &prochain);

    for (int i = 0; i < NB_ITER; i++) {
        ajouter_ns(&prochain, PERIODE_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &prochain, NULL);
        clock_gettime(CLOCK_MONOTONIC, &reel);

        long long ecart = diff_us(&prochain, &reel); /* reel - prevu */
        if (i == 0 || ecart < min) min = ecart;
        if (i == 0 || ecart > max) max = ecart;
        somme += ecart;
        printf("iter %2d : ecart = %lld us\n", i, ecart);
    }

    printf("\nJitter sur %d iterations : min=%lld us, moy=%lld us, max=%lld us\n",
           NB_ITER, min, somme / NB_ITER, max);
}

int main(void) {
    tests_unitaires();
    if (echecs == 0) printf("Tests unitaires : OK\n\n");
    else printf("Tests unitaires : %d echec(s)\n\n", echecs);

    test_boucle_100ms();
    return echecs == 0 ? 0 : 1;
}