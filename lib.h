//Fork standard library

//DO NOT USE GC HERE --- not compatible with C++11 <thread>

#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include "parContextManager.h"

//extern "C" disables C++ name mangling

extern "C" void print_int(int64_t i);

extern "C" void print_float(double d);

extern "C" float* malloc_float(int64_t s);

extern "C" int64_t* malloc_int(int64_t s);

extern "C" float* calloc_float(int64_t s);

extern "C" int64_t* calloc_int(int64_t s);

extern "C" void free_float(float* f);

extern "C" void free_int(int64_t* i);

extern "C"  void __fork_sched_int(void* func,void* env,int64_t id,int64_t cid);

extern "C"  void __fork_sched_float(void* func,void* end,int64_t id,int64_t cid);

extern "C"  void __fork_sched_intptr(void* func,void* env,int64_t id,int64_t cid);

extern "C"  void __fork_sched_floatptr(void* func,void* env,int64_t id,int64_t cid);

extern "C"  void __fork_sched_void(void* func,void* env,int64_t id,int64_t cid);

extern "C" int64_t __recon_int(int64_t original,int64_t known,int64_t id,int64_t max,int64_t cid);

extern "C" double __recon_float(double original,int64_t known,int64_t id,int64_t max,int64_t cid);

extern "C" int64_t* __recon_intptr(int64_t* original,int64_t known,int64_t id,int64_t max,int64_t cid);

extern "C" double* __recon_floatptr(double* original,int64_t known,int64_t id,int64_t max,int64_t cid);

extern "C" void __recon_void(int64_t id,int64_t max,int64_t cid);

extern "C" int64_t __make_context();

extern "C" void __destroy_context(int64_t cid);

