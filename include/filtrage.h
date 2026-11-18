#ifndef FILTRAGE_H
#define FILTRAGE_H
#include "buffer.h"



void filtrage_brancher(buffer_t *b);

/* Tache periodique a 5 Hz (200 ms) : lit le buffer, filtre, ecrit dans donnees. */
void *filtrage_tache(void *arg);

#endif /* FILTRAGE_H */