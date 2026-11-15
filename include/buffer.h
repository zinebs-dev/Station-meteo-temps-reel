#ifndef BUFFER_H
#define BUFFER_H
#include <stdatomic.h>
#include "mesure.h"
#define BUFFER_TAILLE 64
/* File circulaire pour UN producteur et UN consommateur, sans mutex.
   Une case reste toujours libre : capacite utile = BUFFER_TAILLE - 1. */
typedef struct {
    mesure_t donnees[BUFFER_TAILLE];
    atomic_uint tete;    /* prochain indice d'ecriture (ecrit par le producteur) */
    atomic_uint queue;   /* prochain indice de lecture (ecrit par le consommateur) */
} buffer_t;
void buffer_init(buffer_t *b);
/* Jamais bloquant. Retourne 0 si OK, -1 si plein. */
int buffer_push(buffer_t *b, const mesure_t *m);
/* Jamais bloquant. Retourne 0 si OK, -1 si vide. */
int buffer_pop(buffer_t *b, mesure_t *m);
#endif

