/* src/temps.c */
#include "temps.h"

void ajouter_ns(struct timespec *t, long ns) {
    (void)t;
    (void)ns;
    /* TODO : ajouter ns a t et normaliser tv_nsec dans [0, 1e9[ */
}

int64_t diff_us(const struct timespec *a, const struct timespec *b) {
    (void)a;
    (void)b;
    /* TODO : retourner b - a en microsecondes */
    return 0;
}