#ifndef BUFFER_H
#define BUFFER_H

#include "mesure.h"

#define BUFFER_TAILLE 64

typedef struct {
    mesure_t donnees[BUFFER_TAILLE];
    volatile unsigned tete;    /* prochain indice d'ecriture */
    volatile unsigned queue;   /* prochain indice de lecture */
} buffer_t;

void buffer_init(buffer_t *b);

/* Jamais bloquant. Retourne 0 si OK, -1 si plein. */
int buffer_push(buffer_t *b, const mesure_t *m);

/* Jamais bloquant. Retourne 0 si OK, -1 si vide. */
int buffer_pop(buffer_t *b, mesure_t *m);

#endif
