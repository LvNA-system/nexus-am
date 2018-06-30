#include <am.h>
#include <klib.h>

volatile int *cpbase = (int *)0x900;
static const char *colname [] = {
  "waymask", 
  "cache access", 
  "cache miss", 
  "cache usage", 
  "bucket size", 
  "bucket freq", 
  "bucket increase", 
  "memory read", 
  "memory write" 
};

#define NR_COL (sizeof(colname) / sizeof(colname[0]))

void print_cp_statistics() {
  // waymask，access，miss，usage, sizes，freqs，incs，read，write
  //printf("control plane statistics: ");
  for (int i = 0; i < NR_COL; i++)
    printf("%s: %x\n", colname[i], cpbase[i * 8]);
  printf("\n");
}

int main() {
  printf("start to access control plane by mmio...\n");
  while (1) {
    print_cp_statistics();
  }
  return 0;
}
