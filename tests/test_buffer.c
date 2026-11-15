#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include "buffer.h"

static int echecs = 0;

#define VERIFIER(cond, msg) do { \
    if (!(cond)) { printf("ECHEC : %s\n", msg); echecs++; } \
} while (0)

static mesure_t faire(double v) {
    mesure_t m = {0};
    m.temperature = v;
    return m;
}

static void test_vide(void) {
    buffer_t b; mesure_t m;
    buffer_init(&b);
    VERIFIER(buffer_pop(&b, &m) == -1, "pop sur buffer vide doit renvoyer -1");
}

static void test_plein(void) {
    buffer_t b; mesure_t m;
    int acceptes = 0;
    buffer_init(&b);
    for (int i = 0; i < BUFFER_TAILLE + 10; i++) {
        m = faire(i);
        if (buffer_push(&b, &m) == 0) acceptes++;
    }
    VERIFIER(acceptes == BUFFER_TAILLE - 1, "capacite attendue : BUFFER_TAILLE - 1");
    m = faire(999);
    VERIFIER(buffer_push(&b, &m) == -1, "push sur buffer plein doit renvoyer -1");
    /* le rejet ne doit rien ecraser : on relit dans l'ordre */
    for (int i = 0; i < acceptes; i++)
        VERIFIER(buffer_pop(&b, &m) == 0 && m.temperature == i, "ordre FIFO apres rejet");
    VERIFIER(buffer_pop(&b, &m) == -1, "vide apres avoir tout lu");
}

static void test_tour_complet(void) {
    buffer_t b; mesure_t m;
    int ecrit = 0, lu = 0;
    buffer_init(&b);
    /* 10 x BUFFER_TAILLE elements par paquets de 5 : les indices font plusieurs tours */
    while (ecrit < 10 * BUFFER_TAILLE) {
        for (int k = 0; k < 5; k++) {
            m = faire(ecrit);
            VERIFIER(buffer_push(&b, &m) == 0, "push pendant le tour complet");
            ecrit++;
        }
        for (int k = 0; k < 5; k++) {
            VERIFIER(buffer_pop(&b, &m) == 0 && m.temperature == lu, "ordre FIFO sur plusieurs tours");
            lu++;
        }
    }
    VERIFIER(buffer_pop(&b, &m) == -1, "vide a la fin");
}

/* ---- test a deux threads ---- */
#define N 1000000
static buffer_t partage;
static long erreurs_thread = 0;

static void *producteur(void *arg) {
    (void)arg;
    for (int i = 0; i < N; i++) {
        mesure_t m = faire(i);
        while (buffer_push(&partage, &m) != 0)
            sched_yield();              /* plein : on laisse la place au consommateur */
    }
    return NULL;
}

static void *consommateur(void *arg) {
    (void)arg;
    for (int i = 0; i < N; i++) {
        mesure_t m;
        while (buffer_pop(&partage, &m) != 0)
            sched_yield();              /* vide : on laisse la place au producteur */
        if (m.temperature != (double)i)  /* perte ou doublon => la suite est decalee */
            erreurs_thread++;
    }
    return NULL;
}

int main(void) {
    test_vide();
    test_plein();
    test_tour_complet();

    pthread_t p, c;
    buffer_init(&partage);
    pthread_create(&p, NULL, producteur, NULL);
    pthread_create(&c, NULL, consommateur, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    VERIFIER(erreurs_thread == 0, "2 threads : perte, doublon ou desordre detecte");

    if (echecs == 0)
        printf("Tous les tests buffer passent (dont %d elements a 2 threads)\n", N);
    return echecs ? 1 : 0;
}
