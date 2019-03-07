#include <am.h>
#include "riscv64.h"

int _cpu(void) {
  return read_csr(mhartid);
}
