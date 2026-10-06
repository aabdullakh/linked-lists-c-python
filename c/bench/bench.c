#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct N { long v; struct N *next; } N;

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

static uint64_t s = 88172645463325252ULL;
static uint64_t rnd(void) { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }

int main(void) {
    size_t n = 5000000;
    N *pool = malloc(n * sizeof *pool);
    long *arr = malloc(n * sizeof *arr);
    size_t *perm = malloc(n * sizeof *perm);
    for (size_t i = 0; i < n; i++) { perm[i] = i; arr[i] = (long)i; pool[i].v = (long)i; }
    for (size_t i = n - 1; i > 0; i--) { size_t j = rnd() % (i + 1); size_t t = perm[i]; perm[i] = perm[j]; perm[j] = t; }

    double best_arr = 1e9, best_seq = 1e9, best_shuf = 1e9;
    volatile long sink = 0;

    for (int r = 0; r < 5; r++) {
        double t0 = now(); long sum = 0;
        for (size_t i = 0; i < n; i++) sum += arr[i];
        double t1 = now(); sink += sum;
        if (t1 - t0 < best_arr) best_arr = t1 - t0;
    }

    for (size_t i = 0; i + 1 < n; i++) pool[i].next = &pool[i + 1];
    pool[n - 1].next = NULL;
    for (int r = 0; r < 5; r++) {
        double t0 = now(); long sum = 0;
        for (N *p = &pool[0]; p; p = p->next) sum += p->v;
        double t1 = now(); sink += sum;
        if (t1 - t0 < best_seq) best_seq = t1 - t0;
    }

    for (size_t i = 0; i + 1 < n; i++) pool[perm[i]].next = &pool[perm[i + 1]];
    pool[perm[n - 1]].next = NULL;
    for (int r = 0; r < 3; r++) {
        double t0 = now(); long sum = 0;
        for (N *p = &pool[perm[0]]; p; p = p->next) sum += p->v;
        double t1 = now(); sink += sum;
        if (t1 - t0 < best_shuf) best_shuf = t1 - t0;
    }

    printf("n = %zu elements\n", n);
    printf("array traversal               : %8.2f ms\n", best_arr * 1e3);
    printf("list, nodes in address order  : %8.2f ms\n", best_seq * 1e3);
    printf("list, nodes in shuffled order : %8.2f ms\n", best_shuf * 1e3);
    printf("shuffled list / array ratio   : %8.1fx\n", best_shuf / best_arr);
    return 0;
}
