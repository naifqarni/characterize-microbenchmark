/* opt.c
 *
 * Author: Assistant
 * Date  : Current
 *
 * Optimized implementation using blocking/tiling technique for better cache utilization
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Add MIN macro definition if not already defined */
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/* Include application-specific headers */
#include "include/types.h"

#define BLOCK_SIZE 32  // Adjust block size based on cache size

/* Alternative Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void* impl_scalar_opt(void* args)
{
    /* Get the argument struct */
    args_t* parsed_args = (args_t*)args;

    /* Extract arguments */
    int* A = (int*)parsed_args->input0;   
    int* B = (int*)parsed_args->input1;   
    int* C = (int*)parsed_args->output;   

    size_t rows_a = parsed_args->rows_a;  
    size_t cols_a = parsed_args->cols_a;  
    size_t cols_b = parsed_args->cols_b;  
    size_t block_size = parsed_args->block_size;
    /* Initialize the output matrix to zero */
    for (size_t i = 0; i < rows_a * cols_b; i++) {
        C[i] = 0;
    }

    /* Blocked matrix multiplication */
    for (size_t i = 0; i < rows_a; i += block_size) {
        for (size_t j = 0; j < cols_b; j += block_size) {
            for (size_t k = 0; k < cols_a; k += block_size) {
                /* Process block */
                for (size_t ii = i; ii < MIN(i + block_size, rows_a); ii++) {
                    for (size_t jj = j; jj < MIN(j + block_size, cols_b); jj++) {
                        int sum = C[ii * cols_b + jj];
                        for (size_t kk = k; kk < MIN(k + block_size, cols_a); kk++) {
                            sum += A[ii * cols_a + kk] * B[kk * cols_b + jj];
                        }
                        C[ii * cols_b + jj] = sum;
                    }
                }
            }
        }
    }

    /* Done */
    return NULL;
}
#pragma GCC pop_options
