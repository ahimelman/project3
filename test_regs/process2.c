#include "util.h"
#include "printf.h"
#include "syslib.h"

void _start(void)
{
  /* Unlike process 1
   * this function tries to
   * clobber registers as quickly
   * as possible
   */

  for(;;)
  {
    int32_t
      eax0 = rand(),
      ebx0 = rand(),
      ecx0 = rand(),
      edx0 = rand(),
      esi0 = rand(),
      edi0 = rand();

    asm volatile(
      /* Set initial values for other regs */
      "   movl %0, %%eax     ;\n"
      "   movl %1, %%ebx     ;\n"
      "   movl %2, %%ecx     ;\n"
      "   movl %3, %%edx     ;\n"
      "   movl %4, %%esi     ;\n"
      "   movl %5, %%edi      \n"
    ::
      /* Inputs */
      "m" (eax0),
      "m" (ebx0),
      "m" (ecx0),
      "m" (edx0),
      "m" (esi0),
      "m" (edi0)
    );

    yield();
  }
}
