//Fork standard library

#include <mutex>
#include <stdint.h>
#include <stdio.h>

//extern "C" disables C++ name mangling

extern "C" void print_int(int64_t i);

extern "C" void print_float(double d);

extern "C" float reconcile_float(double d_old,double d_new,int partx, int ofn);

extern "C" int64_t reconcile_int(int64_t i_old,int64_t i_new,int partx, int ofn);

extern "C" float* malloc_float(int64_t s);

extern "C" int64_t* malloc_int(int64_t s);

extern "C" float* calloc_float(int64_t s);

extern "C" int64_t* calloc_int(int64_t s);

extern "C" void free_float(float* f);

extern "C" void free_int(int64_t* i);




