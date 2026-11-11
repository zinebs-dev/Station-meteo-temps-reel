/* src/temps.c */
#include "temps.h"

#define NS_PAR_S 1000000000L

void ajouter_ns(struct timespec *t, long ns) {
    t->tv_sec  += ns / NS_PAR_S;
    t->tv_nsec += ns % NS_PAR_S;

    /* Normalisation : tv_nsec doit rester dans [0, 1e9[ */
    if (t->tv_nsec >= NS_PAR_S) {
        t->tv_nsec -= NS_PAR_S;
        t->tv_sec++;
    } else if (t->tv_nsec < 0) {
        t->tv_nsec += NS_PAR_S;
        t->tv_sec--;
    }
}

int64_t diff_us(const struct timespec *a, const struct timespec *b) {
    int64_t sec  = (int64_t)b->tv_sec  - (int64_t)a->tv_sec;
    int64_t nsec = (int64_t)b->tv_nsec - (int64_t)a->tv_nsec;
    return sec * 1000000LL + nsec / 1000;
}