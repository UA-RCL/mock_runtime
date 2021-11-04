#include "dash.h"
#include <cstdio>
#include <cstdlib>
#include <gsl/gsl_fft_complex.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CPU_ONLY)
extern void enqueue_kernel(const char* kernel_name, ...);
#endif

void DASH_FFT_cpu(double** input, double** output, size_t* size, bool* isForwardTransform) {
  thread_local double* data;
  data = (double *)malloc((*size)*2*sizeof(double));

  for (size_t i=0; i < (*size); i++){
    data[2*i] = (*input)[2*i];  
    data[(2*i)+1] = (*input)[(2*i)+1];
  }

  int check = gsl_fft_complex_radix2_forward(data, 1, (*size));

  if (check != 0){
    fprintf(stderr, "[libdash] Faild to complete DASH_FFT_cpu using libgsl with message %d!\n", check);
    exit(1);
  }

  for (size_t i=0; i < (*size); i++){
    (*output)[2*i] = data[2*i];  
    (*output)[(2*i)+1] = data[(2*i)+1];
  }
  free(data);
  return;
}


void DASH_GEMM_cpu(double** A_re, double** A_im, double** B_re, double** B_im, double** C_re, double** C_im, size_t* A_ROWS, size_t* A_COLS, size_t* B_COLS) {
  double res1, res2, res3, res4;
  double term1, term2, term3, term4;

  for (int i = 0; i < (*A_ROWS); i++) {
    for (int j = 0; j < (*B_COLS); j++) {
      res1 = 0; res2 = 0; res3 = 0; res4 = 0;
      // A_COLS better equal B_ROWS, I'm trusting you here >:(
      for (int k = 0; k < (*A_COLS); k++) {
        term1 = (*A_re)[i * (*A_COLS) + k] * (*B_re)[k * (*B_COLS) + j];
        res1 += term1;

        term2 = (*A_im)[i * (*A_COLS) + k] * (*B_im)[k * (*B_COLS) + j];
        res2 += term2;

        term3 = (*A_re)[i * (*A_COLS) + k] * (*B_im)[k * (*B_COLS) + j];
        res3 += term3;

        term4 = (*A_im)[i * (*A_COLS) + k] * (*B_re)[k * (*B_COLS) + j];
        res4 += term4;
      }
      (*C_re)[i * (*B_COLS) + j] = res1 - res2;
      (*C_im)[i * (*B_COLS) + j] = res3 + res4;
    }
  }
}

void DASH_ZIP_cpu(double** input_1, double** input_2, double** output, size_t* size, zip_op_t* op) {
  for (size_t i = 0; i < *size; i++) {
    switch (*op) {
      case ZIP_ADD:
        (*output)[i] = (*input_1)[i] + (*input_2)[i];
        break;
      case ZIP_SUB:
        (*output)[i] = (*input_1)[i] - (*input_2)[i];
        break;
      case ZIP_MULT:
        (*output)[i] = (*input_1)[i] * (*input_2)[i];
        break;
      case ZIP_DIV:
        (*output)[i] = (*input_1)[i] / (*input_2)[i];
        break;
    }
  }
}

void DASH_FFT(double* input, double* output, size_t size, bool isForwardTransform) {
#if defined(CPU_ONLY)
  DASH_FFT_cpu(&input, &output, &size, &isForwardTransform);
#else
  pthread_barrier_t barrier;
  pthread_barrier_init(&barrier, nullptr, 2);
  enqueue_kernel("DASH_FFT", &input, &output, &size, &isForwardTransform, &barrier);
  pthread_barrier_wait(&barrier);
  pthread_barrier_destroy(&barrier);
#endif
}

void DASH_GEMM(double* A_re, double* A_im, double* B_re, double* B_im, double* C_re, double* C_im, size_t Row_A, size_t Col_A, size_t Col_B) {
#if defined(CPU_ONLY)
  DASH_GEMM_cpu(&A_re, &A_im, &B_re, &B_im, &C_re, &C_im, &Row_A, &Col_A, &Col_B);
#else
  pthread_barrier_t barrier;
  pthread_barrier_init(&barrier, nullptr, 2);
  enqueue_kernel("DASH_GEMM", &A_re, &A_im, &B_re, &B_im, &C_re, &C_im, &Row_A, &Col_A, &Col_B, &barrier);
  pthread_barrier_wait(&barrier);
  pthread_barrier_destroy(&barrier);
#endif
}

void DASH_ZIP(double* input_1, double* input_2, double* output, size_t size, zip_op_t op) {
#if defined(CPU_ONLY)
  DASH_ZIP_cpu(&input_1, &input_2, &output, &size, &op);
#else
  pthread_barrier_t barrier;
  pthread_barrier_init(&barrier, nullptr, 2);
  enqueue_kernel("DASH_ZIP", &input_1, &input_2, &output, &size, &op, &barrier);
  pthread_barrier_wait(&barrier);
  pthread_barrier_destroy(&barrier);
#endif
}
/* End of baseline API implementations */

#ifdef __cplusplus
} // Close 'extern "C"'
#endif
