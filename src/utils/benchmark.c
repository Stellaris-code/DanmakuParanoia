#include "benchmark.h"

#include "sys/cleanup.h"
#include "sys/log.h"

unsigned bench_entries_count;
bench_entry_t bench_entries[MAX_BENCH_ENTRIES];
double bench_busy_time;

static double bench_frame_start;

void save_benchmark_time(const char *module, double time)
{
    if (bench_entries_count >= MAX_BENCH_ENTRIES)
    {
        trace_log(LOG_INFO, "Couldn't benchmark module '%s' : out of benchmark entries\n", module);
        return;
    }

    bench_entries[bench_entries_count].module_name = module;
    bench_entries[bench_entries_count].time = time;
    ++bench_entries_count;
}

void bench_begin_frame()
{
    bench_entries_count = 0;

    bench_frame_start = elapsed_time();
}

void bench_end_frame()
{
    bench_busy_time = elapsed_time() - bench_frame_start;
}
