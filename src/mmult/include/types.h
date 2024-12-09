/* types.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 13 Nov. 2023
 * 
 * This file contains all required types decalartions.
*/

#ifndef __INCLUDE_TYPES_H_
#define __INCLUDE_TYPES_H_

typedef struct {
  byte*   input0;
  byte*   input1;
  byte*   output;
  size_t rows_a;     
  size_t cols_a;     
  size_t cols_b; 



  int     cpu;
  int     nthreads;
} args_t;

#endif //__INCLUDE_TYPES_H_
