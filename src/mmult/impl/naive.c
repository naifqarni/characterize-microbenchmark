/* naive.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

/* Naive Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void* impl_scalar_naive(void* args)
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

    /* Initialize the output matrix to zero */
    for (size_t i = 0; i < rows_a * cols_b; i++) {
        C[i] = 0;
    }

   
    for (size_t i = 0; i < rows_a; i++) {          
        for (size_t j = 0; j < cols_b; j++) { 
                 
            for (size_t k = 0; k < cols_a; k++) {  
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
        }
    }

    /* Done */
    return NULL;
}
#pragma GCC pop_options
