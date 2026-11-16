#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jitter.h"
#include "temps.h"

#define JITTER_CAPACITE 20000   /* 60 s : ~960 mesures ; 30 min : ~17 300 */
#define JITTER_NOM_TAILLE 16

typedef struct {
    char tache[JITTER_NOM_TAILLE];
    long long prevu_ns;
    long long reel_ns;
    long ecart_us;
} echantillon_t;

static echantillon_t *tableau = NULL;   /* alloue une seule fois dans jitter_init */
static atomic_uint compteur = 0;        /* prochaine case libre */
static atomic_int plein_signale = 0;
static char chemin[256];

static long long vers_ns(const struct timespec *t) {
    return (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}

void jitter_init(const char *fichier) {
    snprintf(chemin, sizeof chemin, "%s", fichier);
    tableau = calloc(JITTER_CAPACITE, sizeof *tableau);
    if (!tableau) {
        fprintf(stderr, "jitter_init : allocation impossible\n");
        return;
    }
    /* force le passage en memoire des pages avant le debut des mesures */
    memset(tableau, 0, JITTER_CAPACITE * sizeof *tableau);
    atomic_store(&compteur, 0);
    atomic_store(&plein_signale, 0);
}

/* Stockage en memoire seulement : ni malloc, ni mutex, ni E/S. */
void jitter_enregistrer(const char *tache,
                        const struct timespec *prevu,
                        const struct timespec *reel) {
    if (!tableau) return;

    unsigned i = atomic_fetch_add(&compteur, 1);
    if (i >= JITTER_CAPACITE) {
        if (!atomic_exchange(&plein_signale, 1))
            fprintf(stderr, "jitter : tableau plein (%d), enregistrement arrete\n",
                    JITTER_CAPACITE);
        return;
    }
    echantillon_t *e = &tableau[i];
    snprintf(e->tache, sizeof e->tache, "%s", tache);
    e->prevu_ns = vers_ns(prevu);
    e->reel_ns = vers_ns(reel);
    e->ecart_us = (long)diff_us(prevu, reel);
}

void jitter_fermer(void) {
    if (!tableau) return;

    unsigned n = atomic_load(&compteur);
    if (n > JITTER_CAPACITE) n = JITTER_CAPACITE;

    FILE *f = fopen(chemin, "w");
    if (!f) {
        perror("jitter_fermer : ouverture du fichier");
    } else {
        fprintf(f, "tache,instant_prevu_ns,instant_reel_ns,ecart_us\n");
        for (unsigned i = 0; i < n; i++)
            fprintf(f, "%s,%lld,%lld,%ld\n", tableau[i].tache,
                    tableau[i].prevu_ns, tableau[i].reel_ns, tableau[i].ecart_us);
        fclose(f);
        fprintf(stderr, "jitter : %u mesure(s) ecrites dans %s\n", n, chemin);
    }
    free(tableau);
    tableau = NULL;
}