/* APPLE LOCAL file v7 merge */
/* Test the `vtbx2s8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vtbx2s8 (void)
{
  int8x8_t out_int8x8_t;
  int8x8_t arg0_int8x8_t;
  int8x8x2_t arg1_int8x8x2_t;
  int8x8_t arg2_int8x8_t;

  out_int8x8_t = vtbx2_s8 (arg0_int8x8_t, arg1_int8x8x2_t, arg2_int8x8_t);
}

/* LLVM LOCAL make spaces between comma-separated registers optional */
/* { dg-final { scan-assembler "vtbx\.8\[ 	\]+\[dD\]\[0-9\]+, \\\{((\[dD\]\[0-9\]+-\[dD\]\[0-9\]+)|(\[dD\]\[0-9\]+, ?\[dD\]\[0-9\]+))\\\}, \[dD\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
