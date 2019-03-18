#include <am.h>
#include <klib.h>

void config_perf_counters();
void reset_perf_counters();
void dump_perf_counters();
void loop(int n);
void cache_flush(int n);
void random_branch(int n);

int main() {
  config_perf_counters();

  reset_perf_counters();
  loop(3);
  dump_perf_counters();

  reset_perf_counters();
  cache_flush(3);
  dump_perf_counters();

  reset_perf_counters();
  random_branch(3);
  dump_perf_counters();

  return 0;
}
