#include <am.h>
#include <klib.h>

#define str(x) #x

#define set_csr(reg, val) \
  asm volatile ("csrsi %0, %1" :: "i"(reg), "rK"(val))

#define clear_csr(reg, val) \
  asm volatile ("csrci %0, %1" :: "i"(reg), "rK"(val))

#undef read_csr
#undef write_csr

#define read_csr(reg) ({ \
    long long val; \
    asm volatile ("csrr %0, %1" : "=r"(val) : "i"(reg)); \
    val; })

#define write_csr(reg, val) \
  asm volatile ("csrw %0, %1" :: "i"(reg), "rK"(val))

// for now, rocketchip-pard only have 8 perf counters
enum {
  mcycle = 0xb00,
  minstret = 0xb02,
  mhpmcounter3 = 0xb03,
  mhpmcounter4,
  mhpmcounter5,
  mhpmcounter6,
  mhpmcounter7,
  mhpmcounter8,
  mhpmcounter9,
  mhpmcounter10,

  mhpmevent3 = 0x323,
  mhpmevent4,
  mhpmevent5,
  mhpmevent6,
  mhpmevent7,
  mhpmevent8,
  mhpmevent9,
  mhpmevent10,
};

void reset_perf_counters() {
  write_csr(mhpmcounter3, 0ULL);
  write_csr(mhpmcounter4, 0ULL);
  write_csr(mhpmcounter5, 0ULL);
  write_csr(mhpmcounter6, 0ULL);
  write_csr(mhpmcounter7, 0ULL);
  write_csr(mhpmcounter8, 0ULL);
  write_csr(mhpmcounter9, 0ULL);
  write_csr(mhpmcounter10, 0ULL);
}

uint64_t get_event_selector(const char *event_name);

void config_perf_counters() {
  write_csr(mhpmevent3, get_event_selector("load"));
  write_csr(mhpmevent4, get_event_selector("store"));
  write_csr(mhpmevent5, get_event_selector("branch"));
  write_csr(mhpmevent6, get_event_selector("branch misprediction"));
  write_csr(mhpmevent7, get_event_selector("I$ access"));
  write_csr(mhpmevent8, get_event_selector("I$ miss"));
  write_csr(mhpmevent9, get_event_selector("D$ miss"));
}

void print_llx(long long value) {
  const char hex_map[] = "0123456789abcdef";
  char *p = (char *)&value;
  for (int i = 7; i >= 0; i--) {
    printf("%c%c", hex_map[(p[i] >> 4) & 0xf], hex_map[p[i] & 0xf]);
  }
}

#define print_perf_counter(name, reg) \
  printf(#name": "); \
print_llx(reg); \
printf("\n")

void dump_perf_counters() {
  // first read all these registers out
  // do not read out and printf one by one
  // or printf function will be counted by perf-counters
  uint64_t perf_counters[9];
  perf_counters[0] = read_csr(mcycle);
  perf_counters[1] = read_csr(minstret);
  perf_counters[2] = read_csr(mhpmcounter3);
  perf_counters[3] = read_csr(mhpmcounter4);
  perf_counters[4] = read_csr(mhpmcounter5);
  perf_counters[5] = read_csr(mhpmcounter6);
  perf_counters[6] = read_csr(mhpmcounter7);
  perf_counters[7] = read_csr(mhpmcounter8);
  perf_counters[8] = read_csr(mhpmcounter9);

  print_perf_counter(cycle, perf_counters[0]);
  print_perf_counter(instret, perf_counters[1]);
  print_perf_counter(nLoad, perf_counters[2]);
  print_perf_counter(nStore, perf_counters[3]);
  print_perf_counter(nBranch, perf_counters[4]);
  print_perf_counter(nBranchMisprediction, perf_counters[5]);
  print_perf_counter(nICacheAccess, perf_counters[6]);
  print_perf_counter(nICacheMiss, perf_counters[7]);
  print_perf_counter(nDCacheMiss, perf_counters[8]);
}

// event_mask bit field width can not exceed 24
// See http://www.catb.org/esr/structure-packing/#_bitfields:
// The thing to know about bitfields is that they are implemented with
// word- and byte-level mask and rotate instructions operating on machine words,
// and cannot cross word boundaries.
// C99 guarentees that bit-fields will be packed as tightly as possible,
// provided they don’t cross storage unit boundaries (6.7.2.1 #10).
typedef union {
  struct {
    uint32_t set_id     : 8;
    uint32_t event_mask : 24;
    uint32_t pad        : 32;
  };
  uint64_t val;
} __attribute__((packed)) EventSelector;


// each event set can have at most 32 different events
#define MAX_SET_SIZE 32

// this is static, non-initialized entries will default to zero(null)
static const char *events[][MAX_SET_SIZE] = {
  {
    "exception", "load", "store", "amo", "system", "arith", "branch", "jal",
    "jalr", "mul", "div", "fp load", "fp store", "fp add", "fp mul", "fp mul-add",
    "fp div/sqrt", "fp other"
  },
  {
    "load-use interlock",
    "long-latency interlock",
    "csr interlock",
    "I$ blocked",
    "D$ blocked",
    "branch misprediction",
    "control-flow target misprediction",
    "flush",
    "replay",
    "mul/div interlock",
    "fp interlock"
  },
  {
    "I$ access",
    "I$ miss",
    "D$ miss",
    "D$ release",
    "ITLB miss",
    "DTLB miss",
    "L2 TLB miss"
  }
};

uint64_t get_event_selector(const char *event_name) {
  int n_set = sizeof(events) / sizeof(char *[MAX_SET_SIZE]);
  for (int i = 0; i < n_set; i++) {
    for (int j = 0; j < MAX_SET_SIZE; j++) {
      const char *event = events[i][j];
      if (!event)
        break;
      if (!strcmp(event, event_name)) {
        EventSelector e;
        e.set_id = i;
        e.event_mask = 1U << j;
        return e.val;
      }
    }
  }

  // should never reach here
  assert(0);
  return 0;
}
