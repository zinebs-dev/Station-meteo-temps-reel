/* tests/test_filtrage_buffer.c : filtrage branche sur un vrai buffer (sans sudo).
   Verifie : buffer vide -> pas de blocage ; valeurs lues puis moyennees puis publiees. */
#define _GNU_SOURCE
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "buffer.h"
#include "donnees.h"
#include "filtrage.h"
#include "jitter.h"
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

/* Faux module jitter : compte les reveils de la tache (prouve qu'elle continue de tourner). */
static atomic_int reveils = 0;

void jitter_enregistrer(const char *tache, const struct timespec *prevu,
                        const struct timespec *reel)
{
    (void)tache; (void)prevu; (void)reel;
    atomic_fetch_add(&reveils, 1);
}

static void attendre_ms(long ms)
{
    struct timespec d = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&d, NULL);
}

/* Arrete le thread et attend sa fin au maximum 2 s. Retourne 0 si il s'est arrete, -1 sinon. */
static int arreter(pthread_t th, atomic_int *arret)
{
    struct timespec limite;
    atomic_store(arret, 1);
    clock_gettime(CLOCK_REALTIME, &limite);
    limite.tv_sec += 2;
    return pthread_timedjoin_np(th, NULL, &limite) == 0 ? 0 : -1;
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

static int proche(double a, double b)
{
    double d = a - b;
    return d > -1e-9 && d < 1e-9;
}

int main(void)
{
    static buffer_t tampon;
    atomic_int arret;
    pthread_t th;
    mesure_t m, lu;

    donnees_init();
    buffer_init(&tampon);
    filtrage_brancher(&tampon);

    /* ---------- Scenario 1 : buffer vide ---------- */
    printf("Scenario 1 : buffer vide pendant 1,3 s\n");
    atomic_init(&arret, 0);
    atomic_store(&reveils, 0);
    pthread_create(&th, NULL, filtrage_tache, &arret);
    attendre_ms(1300);
    int n = atomic_load(&reveils);
    printf("  %d reveils en 1,3 s\n", n);
    VERIFIER(n >= 4 && n <= 7, "la tache continue de se reveiller toutes les 200 ms (4 a 7 reveils, attendu 6)");
    VERIFIER(arreter(th, &arret) == 0, "la tache s'arrete sans rester bloquee");
    donnees_lire(&lu);
    VERIFIER(lu.temperature == 0.0 && lu.humidite == 0.0 && lu.pression == 0.0,
             "rien n'est publie quand il n'y a rien a lire");

    /* ---------- Scenario 2 : valeurs connues ---------- */
    printf("Scenario 2 : 5 mesures T=1..5 puis 2 mesures T=6,7\n");
    buffer_init(&tampon);
    for (int i = 1; i <= 5; i++) {
        m = mesure(i);
        buffer_push(&tampon, &m);
    }
    atomic_store(&arret, 0);
    pthread_create(&th, NULL, filtrage_tache, &arret);
    attendre_ms(500);
    donnees_lire(&lu);
    printf("  apres {1..5} : T=%.2f H=%.2f P=%.2f\n", lu.temperature, lu.humidite, lu.pression);
    VERIFIER(proche(lu.temperature, 3.0) && proche(lu.humidite, 30.0) && proche(lu.pression, 1003.0),
             "moyenne publiee de {1..5} = T 3, H 30, P 1003");

    for (int i = 6; i <= 7; i++) {
        m = mesure(i);
        buffer_push(&tampon, &m);
    }
    attendre_ms(500);
    donnees_lire(&lu);
    printf("  apres {3..7} : T=%.2f H=%.2f P=%.2f\n", lu.temperature, lu.humidite, lu.pression);
    VERIFIER(proche(lu.temperature, 5.0) && proche(lu.humidite, 50.0) && proche(lu.pression, 1005.0),
             "moyenne publiee de {3..7} = T 5, H 50, P 1005");

    mesure_t reste;
    VERIFIER(buffer_pop(&tampon, &reste) == -1, "le buffer a ete entierement vide par le filtrage");
    VERIFIER(arreter(th, &arret) == 0, "la tache s'arrete proprement");

    printf("\n%s\n", echecs == 0 ? "PASS" : "RESULTAT : FAIL");
    return echecs == 0 ? 0 : 1;
}