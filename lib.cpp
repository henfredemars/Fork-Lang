//Fork standard library

#include <mutex>
#include <stdint.h>
#include <stdio.h>

//extern "C" disables C++ name mangling

//Never garble writing to the output stream
std::mutex print_mutex;

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

extern "C" float reconcilef(double d_old,double d_new,int partx, int ofn) {
  return d_new; //Update always wins for now
}

extern "C" int64_t reconcilei(int64_t i_old,int64_t i_new,int partx, int ofn) {
  return i_new; //Update always wins for now
}

//Just call the system malloc/free implementation

