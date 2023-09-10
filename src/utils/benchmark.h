#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "sys/time.h"

#define MAX_BENCH_ENTRIES 64

typedef struct bench_entry_t
{
    const char* module_name;
    double time;
} bench_entry_t;

extern unsigned bench_entries_count;
extern bench_entry_t bench_entries[MAX_BENCH_ENTRIES];
extern double bench_busy_time; // Time spent outside sleeping for the next frame

void init_benchmarking();
void save_benchmark_time(const char* module, double time);
void bench_begin_frame();
void bench_end_frame();

#define START_BENCHMARK(module) \
    double bench_impl_##module##_start = elapsed_time();

#define END_BENCHMARK(module, str_name) \
    save_benchmark_time(str_name, elapsed_time() - bench_impl_##module##_start); \

#endif // BENCHMARK_H
