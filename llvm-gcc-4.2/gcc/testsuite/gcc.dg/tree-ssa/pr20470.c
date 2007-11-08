/* { dg-do compile } */
/* { dg-options "-fdump-tree-gimple" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

/* PR tree-optimization/20470:
    Missing fold to abs(x) when x == MINUS_EXPR.  */
#define abs(x)  ((x) >= 0 ? (x) : -(x))

int i,j,k;
void f1()
{
  i = abs(j-k);
}

/* { dg-final { scan-tree-dump-times "ABS_EXPR" 1 "gimple" } } */
/* { dg-final { cleanup-tree-dump "gimple" } } */
