/*-----------------------------------------------------------------------*/
/* Program: STREAM                                                       */
/* Revision: $Id: stream.c,v 5.10 2013/01/17 16:01:06 mccalpin Exp mccalpin $ */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2013: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*           "tuned STREAM benchmark results"                            */
/*           "based on a variant of the STREAM benchmark code"           */
/*         Other comparable, clear, and reasonable labelling is          */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/
# include <am.h>
# include <klib.h>

// For now, we only have L1 cache
// when using BigCores, L1 Dcache size is 16KB(16384 bytes)
// since stream_type is int, so array size should be 4096
// if we set STREAM_ARRAY_SIZE too large, emu won't work.
#define STREAM_ARRAY_SIZE 4096
#define NTIMES 2
#define STREAM_TYPE int

/*-----------------------------------------------------------------------
 * INSTRUCTIONS:
 *
 *	1) STREAM requires different amounts of memory to run on different
 *           systems, depending on both the system cache size(s) and the
 *           granularity of the system timer.
 *     You should adjust the value of 'STREAM_ARRAY_SIZE' (below)
 *           to meet *both* of the following criteria:
 *       (a) Each array must be at least 4 times the size of the
 *           available cache memory. I don't worry about the difference
 *           between 10^6 and 2^20, so in practice the minimum array size
 *           is about 3.8 times the cache size.
 *           Example 1: One Xeon E3 with 8 MB L3 cache
 *               STREAM_ARRAY_SIZE should be >= 4 million, giving
 *               an array size of 30.5 MB and a total memory requirement
 *               of 91.5 MB.  
 *           Example 2: Two Xeon E5's with 20 MB L3 cache each (using OpenMP)
 *               STREAM_ARRAY_SIZE should be >= 20 million, giving
 *               an array size of 153 MB and a total memory requirement
 *               of 458 MB.  
 *       (b) The size should be large enough so that the 'timing calibration'
 *           output by the program is at least 20 clock-ticks.  
 *           Example: most versions of Windows have a 10 millisecond timer
 *               granularity.  20 "ticks" at 10 ms/tic is 200 milliseconds.
 *               If the chip is capable of 10 GB/s, it moves 2 GB in 200 msec.
 *               This means the each array must be at least 1 GB, or 128M elements.
 *
 *      Version 5.10 increases the default array size from 2 million
 *          elements to 10 million elements in response to the increasing
 *          size of L3 caches.  The new default size is large enough for caches
 *          up to 20 MB. 
 *      Version 5.10 changes the loop index variables from "register int"
 *          to "ssize_t", which allows array indices >2^32 (4 billion)
 *          on properly configured 64-bit systems.  Additional compiler options
 *          (such as "-mcmodel=medium") may be required for large memory runs.
 *
 *      Array size can be set at compile time without modifying the source
 *          code for the (many) compilers that support preprocessor definitions
 *          on the compile line.  E.g.,
 *                gcc -O -DSTREAM_ARRAY_SIZE=100000000 stream.c -o stream.100M
 *          will override the default size of 10M with a new size of 100M elements
 *          per array.
 */
#ifndef STREAM_ARRAY_SIZE
#   define STREAM_ARRAY_SIZE	10000000
#endif

/*  2) STREAM runs each kernel "NTIMES" times and reports the *best* result
 *         for any iteration after the first, therefore the minimum value
 *         for NTIMES is 2.
 *      There are no rules on maximum allowable values for NTIMES, but
 *         values larger than the default are unlikely to noticeably
 *         increase the reported performance.
 *      NTIMES can also be set on the compile line without changing the source
 *         code using, for example, "-DNTIMES=7".
 */
#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES	10
#endif
#endif
#ifndef NTIMES
#   define NTIMES	10
#endif

/*  Users are allowed to modify the "OFFSET" variable, which *may* change the
 *         relative alignment of the arrays (though compilers may change the 
 *         effective offset by making the arrays non-contiguous on some systems). 
 *      Use of non-zero values for OFFSET can be especially helpful if the
 *         STREAM_ARRAY_SIZE is set to a value close to a large power of 2.
 *      OFFSET can also be set on the compile line without changing the source
 *         code using, for example, "-DOFFSET=56".
 */
#ifndef OFFSET
#   define OFFSET	0
#endif

/*
 *	3) Compile the code with optimization.  Many compilers generate
 *       unreasonably bad code before the optimizer tightens things up.  
 *     If the results are unreasonably good, on the other hand, the
 *       optimizer might be too smart for me!
 *
 *     For a simple single-core version, try compiling with:
 *            cc -O stream.c -o stream
 *     This is known to work on many, many systems....
 *
 *     To use multiple cores, you need to tell the compiler to obey the OpenMP
 *       directives in the code.  This varies by compiler, but a common example is
 *            gcc -O -fopenmp stream.c -o stream_omp
 *       The environment variable OMP_NUM_THREADS allows runtime control of the 
 *         number of threads/cores used when the resulting "stream_omp" program
 *         is executed.
 *
 *     To run with single-precision variables and arithmetic, simply add
 *         -DSTREAM_TYPE=float
 *     to the compile line.
 *     Note that this changes the minimum array sizes required --- see (1) above.
 *
 *     The preprocessor directive "TUNED" does not do much -- it simply causes the 
 *       code to call separate functions to execute each kernel.  Trivial versions
 *       of these functions are provided, but they are *not* tuned -- they just 
 *       provide predefined interfaces to be replaced with tuned code.
 *
 *
 *	4) Optional: Mail the results to mccalpin@cs.virginia.edu
 *	   Be sure to include info that will help me understand:
 *		a) the computer hardware configuration (e.g., processor model, memory type)
 *		b) the compiler name/version and compilation flags
 *      c) any run-time information (such as OMP_NUM_THREADS)
 *		d) all of the output from the test case.
 *
 * Thanks!
 *
 *-----------------------------------------------------------------------*/

# define HLINE "-------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

static STREAM_TYPE	a[STREAM_ARRAY_SIZE+OFFSET],
			b[STREAM_ARRAY_SIZE+OFFSET],
			c[STREAM_ARRAY_SIZE+OFFSET];

#define UINT_MAX 4294967295UL

static uint32_t	avgtime[4] = {0}, maxtime[4] = {0},
		mintime[4] = {UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX};

static char	*label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

extern uint32_t mysecond();
#ifdef TUNED
extern void tuned_STREAM_Copy();
extern void tuned_STREAM_Scale(STREAM_TYPE scalar);
extern void tuned_STREAM_Add();
extern void tuned_STREAM_Triad(STREAM_TYPE scalar);
#endif
int
main()
    {
    int			BytesPerWord;
    int			k;
    ssize_t		j;
    STREAM_TYPE		scalar;
    uint32_t	times[4][NTIMES];

    /* --- SETUP --- determine precision and check timing --- */

    printf(HLINE);
    printf("STREAM version $Revision: 5.10 $\n");
    printf(HLINE);
    BytesPerWord = sizeof(STREAM_TYPE);
    printf("This system uses %d bytes per array element.\n",
	BytesPerWord);

    printf(HLINE);
#ifdef N
    printf("*****  WARNING: ******\n");
    printf("      It appears that you set the preprocessor variable N when compiling this code.\n");
    printf("      This version of the code uses the preprocesor variable STREAM_ARRAY_SIZE to control the array size\n");
    printf("      Reverting to default value of STREAM_ARRAY_SIZE=%u\n",(uint32_t) STREAM_ARRAY_SIZE);
    printf("*****  WARNING: ******\n");
#endif

    printf("Array size = %u (elements), Offset = %d (elements)\n" , (uint32_t) STREAM_ARRAY_SIZE, OFFSET);
    printf("Memory per array = %u MiB (= %u GiB).\n", 
	BytesPerWord * ( (uint32_t) STREAM_ARRAY_SIZE / 1024/1024),
	BytesPerWord * ( (uint32_t) STREAM_ARRAY_SIZE / 1024/1024/1024));
    printf("Total memory required = %u MiB (= %u GiB).\n",
	(3 * BytesPerWord) * ( (uint32_t) STREAM_ARRAY_SIZE / 1024/1024),
	(3 * BytesPerWord) * ( (uint32_t) STREAM_ARRAY_SIZE / 1024/1024/1024));
    printf("Each kernel will be executed %d times.\n", NTIMES);

    /* Get initial value for system clock. */
    for (j=0; j<STREAM_ARRAY_SIZE; j++) {
	    a[j] = 1;
	    b[j] = 2;
	    c[j] = 0;
	}

    printf(HLINE);

    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    scalar = 3;
    for (k=0; k<NTIMES; k++)
	{
	times[0][k] = mysecond();
	for (j=0; j<STREAM_ARRAY_SIZE; j++)
	    c[j] = a[j];
	times[0][k] = mysecond() - times[0][k];
	
	times[1][k] = mysecond();
	for (j=0; j<STREAM_ARRAY_SIZE; j++)
	    b[j] = scalar*c[j];
	times[1][k] = mysecond() - times[1][k];
	
	times[2][k] = mysecond();
	for (j=0; j<STREAM_ARRAY_SIZE; j++)
	    c[j] = a[j]+b[j];
	times[2][k] = mysecond() - times[2][k];
	
	times[3][k] = mysecond();
	for (j=0; j<STREAM_ARRAY_SIZE; j++)
	    a[j] = b[j]+scalar*c[j];
	times[3][k] = mysecond() - times[3][k];
	}

    /*	--- SUMMARY --- */

    for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
	{
	for (j=0; j<4; j++)
	    {
	    avgtime[j] = avgtime[j] + times[j][k];
	    mintime[j] = MIN(mintime[j], times[j][k]);
	    maxtime[j] = MAX(maxtime[j], times[j][k]);
	    }
	}
    
    printf("Function   Avg cycle\n");
    for (j=0; j<4; j++) {
		avgtime[j] = avgtime[j]/(uint32_t)(NTIMES-1);

		// scale clock cycle to 50MHz
		printf("%s%u\n", label[j],
	       avgtime[j]);
    }
    printf(HLINE);
    return 0;
}

int
checktick()
    {
	  // assume 100Hz timer
	  return 10000;
    }


#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

uint32_t mysecond()
{
  return (uint32_t)read_csr(mcycle);
}
