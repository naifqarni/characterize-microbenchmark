/* ref.c
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

/* Reference Implementation */
void* impl_ref(void* args)
{
    /* Get the argument struct */
    args_t* parsed_args = (args_t*)args;

    /* Extract arguments */
    int* A = (int*)parsed_args->input0;   // Matrix A
    int* B = (int*)parsed_args->input1;   // Matrix B
    int* C = (int*)parsed_args->output;   // Matrix C (output)

    size_t rows_a = parsed_args->rows_a;  // Rows in Matrix A
    size_t cols_a = parsed_args->cols_a;  // Columns in Matrix A (and rows in B)
    size_t cols_b = parsed_args->cols_b;  // Columns in Matrix B

    /* Initialize the output matrix to zero */
    for (size_t i = 0; i < rows_a * cols_b; i++) {
        C[i] = 0;
    }

    /* Perform matrix multiplication */
    for (size_t i = 0; i < rows_a; i++) {          // Loop over rows of A
        for (size_t j = 0; j < cols_b; j++) {      // Loop over columns of B
            for (size_t k = 0; k < cols_a; k++) {  // Loop over elements in the row/column
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
        }
    }

    /* Done */
    return NULL;
}
