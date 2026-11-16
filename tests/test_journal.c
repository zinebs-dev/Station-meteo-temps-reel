#include <stdio.h>
#include <time.h>
#include "journal.h"
#include "temps.h"

#define N 10000

int main(void) {
    journal_init("/tmp/journal_test.log");
    struct timespec a, b, debut, fin;
    long max_us = 0;

    clock_gettime(CLOCK_MONOTONIC, &debut);
    for (int i = 0; i < N; i++) {
        clock_gettime(CLOCK_MONOTONIC, &a);
        journal_ecrire("message %d", i);
        clock_gettime(CLOCK_MONOTONIC, &b);
        long d = (long)diff_us(&a, &b);
        if (d > max_us) max_us = d;
    }
    clock_gettime(CLOCK_MONOTONIC, &fin);
    printf("rafale de %d messages en %ld us, appel le plus long : %ld us\n",
           N, (long)diff_us(&debut, &fin), max_us);

    journal_arreter();
    return 0;
}