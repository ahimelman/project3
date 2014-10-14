#include "syslib.h"
#include "printf.h"
#include "util.h"

static void error(int row, const char *s);

void _start(void)
{
  printf(1,0, "Please wait...");

  //srand( get_timer() );
  int32_t
    /*eax0 = rand(),
    ebx0 = rand(),
    ecx0 = rand(),
    edx0 = rand(),
    esi0 = rand(),
    edi0 = rand(),*/
/* set some random number to the registers, using rand() will cause some unknown bugs
 * in current environment. 10/8/2012
 */
		eax0 = 3765463,
    ebx0 = 4744567,
    ecx0 = 553462,
    edx0 = 64561,
    esi0 = 584456,
    edi0 = 5664,
    ebp0 = 0,
    esp0 = 0,
    flags0 = 0;

  int32_t
    eax1 = 0,
    ebx1 = 0,
    ecx1 = 0,
    edx1 = 0,
    esi1 = 0,
    edi1 = 0,
    ebp1 = 0,
    esp1 = 0,
    flags1 = 0;
  int32_t countdown = 2000000;


  /* Note: this first loop will check
   * everything except the flags
   */
  asm volatile(
    /* Save initial value of sp, bp, flags */
    "   movl %%esp, %0       ;\n"
    "   movl %%ebp, %1       ;\n"
    "   movl $0, %%eax       ;\n"
    "   lahf                 ;\n"
    "   movl %%eax, %2       ;\n"
    /* Set initial values for other regs */
    "   movl %12, %%eax      ;\n"
    "   movl %13, %%ebx      ;\n"
    "   movl %14, %%ecx      ;\n"
    "   movl %15, %%edx      ;\n"
    "   movl %16, %%esi      ;\n"
    "   movl %17, %%edi      ;\n"
    /* Do a really long loop, so if preemption
     * works at all, we will be preempted.
     * Note: decl modifies Z flag.
     */
    "myfoo:                  ;\n"
    "   decl  %18            ;\n"
    "   jnz   myfoo          ;\n"
    /* Now, save final values of regs */
    "   movl %%eax, %3       ;\n"
    "   movl %%ebx, %4       ;\n"
    "   movl %%ecx, %5       ;\n"
    "   movl %%edx, %6       ;\n"
    "   movl %%esi, %7       ;\n"
    "   movl %%edi, %8       ;\n"
    "   movl %%ebp, %9       ;\n"
    "   movl %%esp, %10      ;\n"
    "   movl $0, %%eax       ;\n"
    "   lahf                 ;\n"
    "   movl %%eax, %11       \n"
  :
    /* Outputs */
    "=m" (esp0),
    "=m" (ebp0),
    "=m" (flags0),
    "=m" (eax1),
    "=m" (ebx1),
    "=m" (ecx1),
    "=m" (edx1),
    "=m" (esi1),
    "=m" (edi1),
    "=m" (ebp1),
    "=m" (esp1),
    "=m" (flags1)
  :
    /* Inputs */
    "m" (eax0),
    "m" (ebx0),
    "m" (ecx0),
    "m" (edx0),
    "m" (esi0),
    "m" (edi0),
    "m" (countdown)
  );

  /* make gcc think we use the countdown variable */
  (void) countdown;
  /* make gcc think we take the address of countdown,
   * and so it cannot put countdown into a reg
   */
  (void) &countdown;
  if( eax0 != eax1 )
    error(5,"eax is clobbered\n");
  if( ebx0 != ebx1 )
    error(6,"ebx is clobbered\n");
  if( ecx0 != ecx1 )
    error(7,"ecx is clobbered\n");
  if( edx0 != edx1 )
    error(8,"edx is clobbered\n");
  if( esi0 != esi1 )
    error(9,"esi is clobbered\n");
  if( edi0 != edi1 )
    error(10,"edi is clobbered\n");
  if( ebp0 != ebp1 )
    error(11,"ebp is clobbered\n");
  if( esp0 != esp1 )
    error(12,"esp is clobbered\n");

  /* This second loop will check
   * everything except ecx
   */
  asm volatile(
    /* Save initial value of sp, bp, flags */
    "   movl %%esp, %0       ;\n"
    "   movl %%ebp, %1       ;\n"
    "   movl $0, %%eax       ;\n"
    "   lahf                 ;\n"
    "   movl %%eax, %2       ;\n"
    /* Set initial values for other regs */
    "   movl %12, %%eax      ;\n"
    "   movl %13, %%ebx      ;\n"
    "   movl %14, %%ecx      ;\n"
    "   movl %15, %%edx      ;\n"
    "   movl %16, %%esi      ;\n"
    "   movl %17, %%edi      ;\n"
    /* Do a really long loop, so if preemption
     * works at all, we will be preempted.
     * Note: loop modifies ecx
     */
    "   movl $2000000, %%ecx ;\n"
    "mybar:                  ;\n"
    "   loop  mybar          ;\n"
    /* Now, save final values of regs */
    "   movl %%eax, %3       ;\n"
    "   movl %%ebx, %4       ;\n"
    "   movl %%ecx, %5       ;\n"
    "   movl %%edx, %6       ;\n"
    "   movl %%esi, %7       ;\n"
    "   movl %%edi, %8       ;\n"
    "   movl %%ebp, %9       ;\n"
    "   movl %%esp, %10      ;\n"
    "   movl $0, %%eax       ;\n"
    "   lahf                 ;\n"
    "   movl %%eax, %11       \n"
  :
    /* Outputs */
    "=m" (esp0),
    "=m" (ebp0),
    "=m" (flags0),
    "=m" (eax1),
    "=m" (ebx1),
    "=m" (ecx1),
    "=m" (edx1),
    "=m" (esi1),
    "=m" (edi1),
    "=m" (ebp1),
    "=m" (esp1),
    "=m" (flags1)
  :
    /* Inputs */
    "m" (eax0),
    "m" (ebx0),
    "m" (ecx0),
    "m" (edx0),
    "m" (esi0),
    "m" (edi0),
    "m" (countdown)
  );

  /* make gcc think we use the countdown variable */
  (void) countdown;
  /* make gcc think we take the address of countdown,
   * and so it cannot put countdown into a reg
   */
  (void) &countdown;
  if( eax0 != eax1 )
    error(5,"eax is clobbered\n");
  if( ebx0 != ebx1 )
    error(6,"ebx is clobbered\n");
  if( edx0 != edx1 )
    error(8,"edx is clobbered\n");
  if( esi0 != esi1 )
    error(9,"esi is clobbered\n");
  if( edi0 != edi1 )
    error(10,"edi is clobbered\n");
  if( ebp0 != ebp1 )
    error(11,"ebp is clobbered\n");
  if( esp0 != esp1 )
    error(12,"esp is clobbered\n");
  if( flags0 != flags1 )
    error(13,"flags clobbered\n");


  printf(15, 2, "%s", "If you only see this line without getting any \"clobbered\" words above,\n");
	printf(16, 2, "%s", "probably you are fine.\n");

  for(;;)
    { /* inf loop */ }

  shutdown();
}

static void error(int row, const char *s)
{
  int i;
  for(i=0; s[i]; ++i)
    write_serial(s[i]);

  printf(row, 5, "%s", s);
}

