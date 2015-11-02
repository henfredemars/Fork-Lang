//Fork standard library

#include <mutex>
#include <stdint.h>
#include <stdio.h>

//extern "C" disables C++ name mangling

//Never garble writing to the output stream
std::mutex print_mutex;
std::mutex malloc_mutex;

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

extern "C" float reconcile_float(double d_old,double d_new,int partx, int ofn) {
  return d_new; //Update always wins for now
}

extern "C" int64_t reconcile_int(int64_t i_old,int64_t i_new,int partx, int ofn) {
  return i_new; //Update always wins for now
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



