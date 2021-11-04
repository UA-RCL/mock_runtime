#include <cstdio>
#include <cstddef>
#include "dash.h"

#define SIZE 10

int main(void) {
  double arr1[SIZE];
  double arr2[SIZE];
  double out[SIZE];
  const size_t size = SIZE;

  for (size_t i = 0; i < SIZE; i++) {
    arr1[i] = i;
    arr2[i] = i;
  }

  DASH_ZIP(arr1, arr2, out, size, ZIP_ADD);

  printf("Array 1: ");
  for (size_t i = 0; i < SIZE; i++) {
    printf("%lf ", arr1[i]);
  }
  printf("\n");
  
  printf("Array 2: ");
  for (size_t i = 0; i < SIZE; i++) {
    printf("%lf ", arr2[i]);
  }
  printf("\n");
  
  printf("Output: ");
  for (size_t i = 0; i < SIZE; i++) {
    printf("%lf ", out[i]);
  }
  printf("\n");
}
