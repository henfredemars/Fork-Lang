//Fork standard library

#include "lib.h"

//extern "C" disables C++ name mangling
//DO NOT USE GC HERE --- not compatible with C++11 <thread>

//Never garble writing to the output stream
std::mutex print_mutex;
std::mutex malloc_mutex;

//Parallism manager
ParContextManager manager;

//Public-facing standard library functions
//Note that cmath functions (Ex: sin, cos) are available as well

extern "C" void print_int(int64_t i) {
  print_mutex.lock();
  printf("Outputing Integer: %d\n",(int)i);
  print_mutex.unlock();
}

extern "C" void print_float(double d) {
  print_mutex.lock();
  printf("Outputing Float: %f\n",d);
  print_mutex.unlock();
}

extern "C" float* malloc_float(int64_t s) {
  malloc_mutex.lock();
  float* allocd = (float*)malloc(s*8);
  malloc_mutex.unlock();
  return allocd;
}

extern "C" int64_t* malloc_int(int64_t s) {
  malloc_mutex.lock();
  int64_t* allocd = (int64_t*)malloc(s*8);
  malloc_mutex.unlock();
  return allocd;
}

extern "C" float* calloc_float(int64_t s) {
  malloc_mutex.lock();
  float* allocd = (float*)calloc(s,8);
  malloc_mutex.unlock();
  return allocd;
}

extern "C" int64_t* calloc_int(int64_t s) {
  malloc_mutex.lock();
  int64_t* allocd = (int64_t*)calloc(s,8);
  malloc_mutex.unlock();
  return allocd;
}

extern "C" void free_float(float* f) {
  malloc_mutex.lock();
  free(f);
  malloc_mutex.unlock();
}

extern "C" void free_int(int64_t* i) {
  malloc_mutex.lock();
  free(i);
  malloc_mutex.unlock();
}

//Hidden funcitons implement parallism
//They are not intended to be called by user code

// id - zero-indexed number of the statement being scheduled for the commit
// cid - context identifier obtained from make_context
// CAST TO FUNCTION POINTER MUST BE VALID!
extern "C"  void __fork_sched_int(void* func,void* env,int64_t id,int64_t cid) {
  manager.sched_int((int64_t (*)(void*))func,env,id,cid);
}

extern "C"  void __fork_sched_float(void* func,void* env,int64_t id,int64_t cid) {
  manager.sched_float((double (*)(void*))func,env,id,cid);
}

extern "C"  void __fork_sched_intptr(void* func,void* env,int64_t id,int64_t cid) {
  manager.sched_intptr((int64_t* (*)(void*))func,env,id,cid);
}

extern "C"  void __fork_sched_floatptr(void* func,void* env,int64_t id,int64_t cid) {
  manager.sched_floatptr((double* (*)(void*))func,env,id,cid);
}

extern "C"  void __fork_sched_void(void* func,void* env,int64_t id,int64_t cid) {
  manager.sched_void((void (*)(void*))func,env,id,cid);
}

//Description of function parameters:
//  original - value before the parallel execution
//  known - original value parameter is valid
//  id - index of statement being reconned in this commit, starting at zero
//  max - maximum valid index of statements in this commit
//  cid - context identifier obtained from make_context
//Conflict resolution scheme: none, always return value of the statement
//Right now, none of the function parameters are used
extern "C" int64_t __recon_int(int64_t original,int64_t known,int64_t id,int64_t max,int64_t cid) {
  return manager.recon_int(original,known,id,max,cid);
}

extern "C" double __recon_float(double original,int64_t known,int64_t id,int64_t max,int64_t cid) {
  return manager.recon_float(original,known,id,max,cid);
}

extern "C" int64_t* __recon_intptr(int64_t* original,int64_t known,int64_t id,int64_t max,int64_t cid) {
  return manager.recon_intptr(original,known,id,max,cid);
}

extern "C" double* __recon_floatptr(double* original,int64_t known,int64_t id,int64_t max,int64_t cid) {
  return manager.recon_floatptr(original,known,id,max,cid);
}

extern "C" void  __recon_void(int64_t id,int64_t max,int64_t cid) {
  manager.recon_void(id,max,cid);
}

extern "C" int64_t __make_context() {
  return manager.make_context();
}

extern "C" void __destroy_context(int64_t cid) {
  manager.destroy_context(cid);
}
