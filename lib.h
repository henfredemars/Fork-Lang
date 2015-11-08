//Fork standard library

#include <cmath>
#include <chrono>
#include <future>
#include <mutex>
#include <thread>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include "gc/include/gc.h"

//extern "C" disables C++ name mangling

extern "C" void print_int(int64_t i);

extern "C" void print_float(double d);

extern "C" float* malloc_float(int64_t s);

extern "C" int64_t* malloc_int(int64_t s);

extern "C" float* calloc_float(int64_t s);

extern "C" int64_t* calloc_int(int64_t s);

extern "C" void free_float(float* f);

extern "C" void free_int(int64_t* i);

extern "C"  void __fork_sched_int(int64_t (*statement)(void),int64_t id);

extern "C"  void __fork_sched_float(double (*statement)(void),int64_t id);

extern "C"  void __fork_sched_intptr(int64_t* (*statement)(void),int64_t id);

extern "C"  void __fork_sched_floatptr(double* (*statement)(void),int64_t id);

extern "C"  void __fork_sched_void(void (*statement)(void),int64_t id);

extern "C" int64_t __recon_int(int64_t original,int64_t known,int64_t update,int64_t id,int64_t max);

extern "C" double __recon_float(double original,int64_t known,double update,int64_t id,int64_t max);

extern "C" int64_t* __recon_intptr(int64_t* original,int64_t known,int64_t* update,int64_t id,int64_t max);

extern "C" double* __recon_floatptr(double* original,int64_t known,double* update,int64_t id,int64_t max);

extern "C" void __recon_void(int64_t id,int64_t max);


