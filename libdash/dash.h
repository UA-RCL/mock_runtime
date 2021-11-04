#pragma once

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zip_op {
  ZIP_ADD,
  ZIP_SUB,
  ZIP_MULT,
  ZIP_DIV
} zip_op_t;

/*
 * Assumes complex input and output of the form input[2*i+0] = real, input[2*i+1] = imaginary
 * "size" specifies the length of the FFT transform, so input and output should be of length 2*size
 */
void DASH_FFT(double* input, double* output, size_t size, bool isForwardTransform);

/*
 * ${Comments about usage of DASH_GEMM}
 */
void DASH_GEMM(double* A_re, double* A_im, double* B_re, double* B_im, double* C_re, double* C_im, size_t Row_A, size_t Col_A, size_t Col_B);

/*
 * Current open questions: 
 * 1. Should we be doing anything to stop the user from shooting themselves in the foot with divide-by-zero with that ZIP_DIV op?
 */
void DASH_ZIP(double* input_1, double* input_2, double* output, size_t size, zip_op_t op);

#ifdef __cplusplus
} // Close 'extern "C"'
#endif
