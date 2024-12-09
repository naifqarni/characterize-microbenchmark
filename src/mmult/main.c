/* main.c
 *
 * Author: Khalid Al-Hawaj
 * Date  : 12 Nov. 2023
 *
 * This file is structured to call different implementation of the same
 * algorithm/microbenchmark. The file will allocate 3 output arrays one
 * for: scalar naive impl, scalar opt impl, vectorized impl. As it stands
 * the file will allocate and initialize with random data one input array
 * of type 'byte'. To check correctness, the file allocate a 'ref' array;
 * to calculate this 'ref' array, the file will invoke a ref_impl, which
 * is supposed to be functionally correct and act as a reference for
 * the functionality. The file also adds a guard word at the end of the
 * output arrays to check for buffer overruns.
 *
 * The file will invoke each implementation n number of times. It will
 * record the runtime of _each_ invocation through the following Linux
 * API:
 *    clock_gettime(), with the clk_id set to CLOCK_MONOTONIC
 * Then, the file will calculate the standard deviation and calculate
 * an outlier-free average by excluding runtimes that are larger than
 * 2 standard deviation of the original average.
 */

/* Set features         */
#define _GNU_SOURCE

/* Standard C includes  */
/*  -> Standard Library */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/*  -> Scheduling       */
#include <sched.h>
/*  -> Types            */
#include <stdbool.h>
#include <inttypes.h>
/*  -> Runtimes         */
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/ref.h"
#include "impl/naive.h"
#include "impl/opt.h"
#include "impl/vec.h"
#include "impl/para.h"

/* Include common headers */
#include "common/types.h"
#include "common/macros.h"

/* Include application-specific headers */
#include "include/types.h"


int main(int argc, char** argv)
{
  /* Set the buffer for printf to NULL */
  setbuf(stdout, NULL);

  /* Arguments */
  int nthreads = 1;
  int cpu      = 0;

  int nruns    = 10000;
  int nstdevs  = 3;

  /* Data */
  size_t rows_a = 10;
  size_t cols_a = 20;
  size_t cols_b = 15;  

  /* Parse arguments */
  /* Function pointers */
  void* (*impl_scalar_naive_ptr)(void* args) = impl_scalar_naive;
  void* (*impl_scalar_opt_ptr  )(void* args) = impl_scalar_opt;
  void* (*impl_vector_ptr      )(void* args) = impl_vector;
  void* (*impl_parallel_ptr    )(void* args) = impl_parallel;

  /* Chosen */
  void* (*impl)(void* args) = NULL;
  const char* impl_str      = NULL;

  bool help = false;
  for (int i = 1; i < argc; i++) {
    /* Implementations */
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {
      assert (++i < argc);
      if (strcmp(argv[i], "naive") == 0) {
        impl = impl_scalar_naive_ptr; impl_str = "scalar_naive";
    } else if (strcmp(argv[i], "opt") == 0 || strcmp(argv[i], "vec") == 0 || strcmp(argv[i], "para") == 0) {
        printf("\n");
        printf("The selected implementation \"%s\" is not implemented yet.\n", argv[i]);
        exit(0); // Exit the program gracefully
    } else {
        impl = NULL;
        impl_str = "unknown";
    }


      continue;
    }

    /* Input/output data size */

    if (strcmp(argv[i], "--rows_a") == 0) {
      assert (++i < argc);
      rows_a = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "--cols_a") == 0) {
      assert (++i < argc);
      cols_a = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "--cols_b") == 0) {
      assert (++i < argc);
      cols_b = atoi(argv[i]);

      continue;
    }

    /* Run parameterization */
    if (strcmp(argv[i], "--nruns") == 0) {
      assert (++i < argc);
      nruns = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "--nstdevs") == 0) {
      assert (++i < argc);
      nstdevs = atoi(argv[i]);

      continue;
    }

    /* Parallelization */
    if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nthreads") == 0) {
      assert (++i < argc);
      nthreads = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cpu") == 0) {
      assert (++i < argc);
      cpu = atoi(argv[i]);

      continue;
    }

    /* Help */
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      help = true;

      continue;
    }
  }

  if (help || impl == NULL) {
    if (!help) {
      if (impl_str != NULL) {
        printf("\n");
        printf("ERROR: Unknown \"%s\" implementation.\n", impl_str);
      } else {
        printf("\n");
        printf("ERROR: No implementation was chosen.\n");
      }
    }
    printf("\n");
    printf("Usage:\n");
    printf("  %s {-i | --impl} impl_str [Options]\n", argv[0]);
    printf("  \n");
    printf("  Required:\n");
    printf("    -i | --impl      Available implementations = {naive, opt, vec, para}\n");
    printf("    \n");
    printf("  Options:\n");
    printf("    -h | --help      Print this message\n");
    printf("    -n | --nthreads  Set number of threads available (default = %d)\n", nthreads);
    printf("    -c | --cpu       Set the main CPU for the program (default = %d)\n", cpu);
    printf("         --rows_a    Set the number of rows in Matrix A (default = %zu)\n", rows_a);
    printf("         --cols_a    Set the number of columns in Matrix A (default = %zu)\n", cols_a);
    printf("         --cols_b    Set the number of columns in Matrix B (default = %zu)\n", cols_b);
    printf("         --nruns     Number of runs to the implementation (default = %d)\n", nruns);
    printf("         --stdevs    Number of standard deviation to exclude outliers (default = %d)\n", nstdevs);
    printf("\n");
    printf("  Examples:\n");
    printf("    ./mmlut -i naive --rows_a 11 --cols_a 21 --cols_b 16 \n");
    printf("\n");
    printf("  Warning!: setting high values for rows_a, cols_a, and cols_b will take a long time to run or result in a memory allocation error.\n");

    exit(help? 0 : 1);
  }

  /* Set our priority the highest */
  int nice_level = -20;

  printf("Setting up schedulers and affinity:\n");
  printf("  * Setting the niceness level:\n");
  do {
    errno = 0;
    printf("      -> trying niceness level = %d\n", nice_level);
    int __attribute__((unused)) ret = nice(nice_level);
  } while (errno != 0 && nice_level++);

  printf("    + Process has niceness level = %d\n", nice_level);

  /* If we are on an apple operating system, skip the scheduling  *
   * routine; Darwin does not support sched_set* system calls ... *
   *                                                              *
   * hawajkm: and here I was--thinking that MacOS is POSIX ...    *
   *          Silly me!                                           */
#if !defined(__APPLE__)
  /* Set scheduling to reduce context switching */
  /*    -> Set scheduling scheme                */
  printf("  * Setting up FIFO scheduling scheme and high priority ... ");
  pid_t pid    = 0;
  int   policy = SCHED_FIFO;
  struct sched_param param;

  param.sched_priority = sched_get_priority_max(policy);
  int res = sched_setscheduler(pid, policy, &param);
  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }

  /*    -> Set affinity                         */
  printf("  * Setting up scheduling affinity ... ");
  cpu_set_t cpumask;

  CPU_ZERO(&cpumask);
  for (int i = 0; i < nthreads; i++) {
    CPU_SET(cpu + i, &cpumask);
  }

  res = sched_setaffinity(pid, sizeof(cpumask), &cpumask);

  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }
#endif
  printf("\n");

  /* Statistics */
  __DECLARE_STATS(nruns, nstdevs);

  /* Initialize Rand */
  srand(0xdeadbeef);

  /* Datasets */
  /* Allocation and initialization */


  byte* A = __ALLOC_INIT_DATA(byte, rows_a * cols_a * sizeof(float));
  byte* B = __ALLOC_INIT_DATA(byte, cols_a * cols_b * sizeof(float) );
  byte* C = __ALLOC_INIT_DATA(byte, rows_a * cols_b * sizeof(float) + 4);
  byte* D = __ALLOC_INIT_DATA(byte, rows_a * cols_b * sizeof(float) + 4);

  int sizeofarray = rows_a * cols_b;
  int full_size = rows_a * cols_b * sizeof(float);

    /* Initialize matrices A and B with random values */
  for (size_t i = 0; i < rows_a * cols_a; i++) {
      A[i] = rand() % 10; // Random values for Matrix A
  }
  for (size_t i = 0; i < cols_a * cols_b; i++) {
      B[i] = rand() % 10; // Random values for Matrix B
  }

  /* Setting a guards, which is 0xdeadcafe.
     The guard should not change or be touched. */
  __SET_GUARD(C , full_size);
  __SET_GUARD(D, full_size);

  /* Generate ref data */
  /* Arguments for the functions */
  args_t args_ref;

  args_ref.output   = (byte*)D;
  args_ref.input0   = (byte*)A;
  args_ref.input1   = (byte*)B;
  args_ref.rows_a   = rows_a;
  args_ref.cols_a   = cols_a;
  args_ref.cols_b   = cols_b;

  args_ref.cpu      = cpu;
  args_ref.nthreads = nthreads;

  /* Running the reference function */
  impl_ref(&args_ref);

  /* Execute the requested implementation */
  /* Arguments for the function */
  args_t args;


  args.input0   = (byte*)A;
  args.input1   = (byte*)B;
  args.output   = (byte*)C;
  args.rows_a   = rows_a;
  args.cols_a   = cols_a;
  args.cols_b   = cols_b;

  args.cpu      = cpu;
  args.nthreads = nthreads;

  /* Start execution */
  printf("Running \"%s\" implementation:\n", impl_str);

  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __SET_START_TIME();
    for (int j = 0; j < 16; j++) {
      (*impl)(&args);
    }
    __SET_END_TIME();
    runtimes[i] = __CALC_RUNTIME() / 16;
  }
  printf("Finished\n");

  /* Verfication */
  printf("  * Verifying results .... ");
  bool match = __CHECK_MATCH(C, D, full_size);
  bool guard = __CHECK_GUARD(C, full_size);
  if (match && guard) {
    printf("Success\n");
  } else if (!match && guard) {
    printf("Fail, but no buffer overruns\n");
  } else if (match && !guard) {
    printf("Success, but failed buffer overruns check\n");
  } else if(!match && !guard) {
    printf("Failed, and failed buffer overruns check\n");
  }

  /* Running analytics */
  uint64_t min     = -1;
  uint64_t max     =  0;

  uint64_t avg     =  0;
  uint64_t avg_n   =  0;

  uint64_t std     =  0;
  uint64_t std_n   =  0;

  int      n_msked =  0;
  int      n_stats =  0;

  for (int i = 0; i < num_runs; i++)
    runtimes_mask[i] = true;

  printf("  * Running statistics:\n");
  do {
    n_stats++;
    printf("    + Starting statistics run number #%d:\n", n_stats);
    avg_n =  0;
    avg   =  0;

    /*   -> Calculate min, max, and avg */
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] < min) {
          min = runtimes[i];
        }
        if (runtimes[i] > max) {
          max = runtimes[i];
        }
        avg += runtimes[i];
        avg_n += 1;
      }
    }
    avg = avg / avg_n;

    /*   -> Calculate standard deviation */
    std   =  0;
    std_n =  0;

    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        std   += ((runtimes[i] - avg) *
                  (runtimes[i] - avg));
        std_n += 1;
      }
    }
    std = sqrt(std / std_n);

    /*   -> Calculate outlier-free average (mean) */
    n_msked = 0;
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] > avg) {
          if ((runtimes[i] - avg) > (nstd * std)) {
            runtimes_mask[i] = false;
            n_msked += 1;
          }
        } else {
          if ((avg - runtimes[i]) > (nstd * std)) {
            runtimes_mask[i] = false;
            n_msked += 1;
          }
        }
      }
    }

    printf("      - Standard deviation = %" PRIu64 "\n", std);
    printf("      - Average = %" PRIu64 "\n", avg);
    printf("      - Number of active elements = %" PRIu64 "\n", avg_n);
    printf("      - Number of masked-off = %d\n", n_msked);
  } while (n_msked > 0);
  /* Display information */
  printf("  * Runtimes (%s): ", __PRINT_MATCH(match));
  printf(" %" PRIu64 " ns\n"  , avg                 );

  /* Dump */
  printf("  * Dumping runtime informations:\n");
  FILE * fp;
  char filename[256];
  strcpy(filename, impl_str);
  strcat(filename, "_runtimes.csv");
  printf("    - Filename: %s\n", filename);
  printf("    - Opening file .... ");
  fp = fopen(filename, "w");

  if (fp != NULL) {
    printf("Succeeded\n");
    printf("    - Writing runtimes ... ");
    fprintf(fp, "impl,%s", impl_str);

    fprintf(fp, "\n");
    fprintf(fp, "num_of_runs,%d", num_runs);

    fprintf(fp, "\n");
    fprintf(fp, "runtimes");
    for (int i = 0; i < num_runs; i++) {
      fprintf(fp, ", ");
      fprintf(fp, "%" PRIu64 "", runtimes[i]);
    }

    fprintf(fp, "\n");
    fprintf(fp, "avg,%" PRIu64 "", avg);
    printf("Finished\n");
    printf("    - Closing file handle .... ");
    fclose(fp);
    printf("Finished\n");
  } else {
    printf("Failed\n");
  }
  printf("\n");

  /* Manage memory */

  free(A);
  free(B);
  free(C);
  free(D);

  /* Finished with statistics */
  __DESTROY_STATS();

  /* Done */
  return 0;
}
