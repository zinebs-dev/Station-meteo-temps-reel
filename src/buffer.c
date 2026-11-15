#include <stdatomic.h>
#include "buffer.h"
void buffer_init(buffer_t *b) {
    if (b == NULL)
        return;
    atomic_store(&b->tete, 0);
    atomic_store(&b->queue, 0);
}
/* Appelee uniquement par le producteur (acquisition). */
int buffer_push(buffer_t *b, const mesure_t *m) {
    if (b == NULL || m == NULL)
        return -1;
    unsigned tete = atomic_load_explicit(&b->tete, memory_order_relaxed);
    unsigned suivant = (tete + 1) % BUFFER_TAILLE;
    /* plein : avancer tete la ferait rejoindre queue */
    if (suivant == atomic_load_explicit(&b->queue, memory_order_acquire))
        return -1;
    b->donnees[tete] = *m;                                          /* 1. ecrire */
    atomic_store_explicit(&b->tete, suivant, memory_order_release); /* 2. publier */
    return 0;
}
/* Appelee uniquement par le consommateur (filtrage). */
int buffer_pop(buffer_t *b, mesure_t *m) {
    if (b == NULL || m == NULL)
        return -1;
    unsigned queue = atomic_load_explicit(&b->queue, memory_order_relaxed);
    /* vide : queue a rattrape tete */
    if (queue == atomic_load_explicit(&b->tete, memory_order_acquire))
        return -1;
    *m = b->donnees[queue];                                         /* 1. lire */
    atomic_store_explicit(&b->queue, (queue + 1) % BUFFER_TAILLE,
                          memory_order_release);                    /* 2. liberer */
    return 0;
}
