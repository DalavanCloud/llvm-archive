/* Test warning for non-existent selectors.  */
/* This is the "-fgnu-runtime" variant of objc.dg/selector-1.m.  */
/* { dg-options "-Wselector -fgnu-runtime" } */
/* { dg-do compile } */
/* APPLE LOCAL radar 4894756 */
/* { dg-require-effective-target ilp32 } */

typedef struct objc_object { struct objc_class *class_pointer; } *id;
typedef const struct objc_selector    *SEL;

__attribute__((objc_root_class)) @interface Foo
- (void) foo;
- (void) bar;
@end

@implementation Foo
- (void) bar
{
}

- (void) foo
{
  SEL a,b,c;
  a = @selector(b1ar);
  b = @selector(bar);
}
@end /* { dg-warning "creating selector for nonexistent method .b1ar." } */

