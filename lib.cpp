//Fork standard library

#include "lib.h"

//extern "C" disables C++ name mangling

//Never garble writing to the output stream
std::mutex print_mutex;
std::mutex malloc_mutex;

//Parallism data structures
int64_t thread_count = 0;
int64_t max_threads = -1;
std::map<int,std::future<int64_t>> int_future_id_map;
std::map<int,std::future<double>> float_future_id_map;
std::map<int,std::future<int64_t*>> intptr_future_id_map;
std::map<int,std::future<double*>> floatptr_future_id_map;
std::map<int,std::future<void>> void_future_id_map;
std::mutex thread_count_mutex;
std::mutex map_mutex;

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
//Only the main thread may call these functions

void set_max_threads() {
  if (max_threads != -1) return;
  unsigned long dth = std::thread::hardware_concurrency()-1;
  printf("Detected %d additional compute elements.\n",(int)dth);
  if (dth < 0) max_threads = 0;
  else if (dth > 3) max_threads = 3;
  else max_threads = dth;
  printf("Setting max execution threads to: %d\n",(int)max_threads+1);
}

extern "C"  void __fork_sched_int(int64_t (*statement)(void),int64_t id) {
  set_max_threads();
  std::future<int64_t> promise;
  thread_count_mutex.lock();
  if (thread_count >= max_threads) {
    thread_count_mutex.unlock();
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
    thread_count_mutex.unlock();
  }
  map_mutex.lock();
  int_future_id_map.insert(std::pair<int64_t,std::future<int64_t>>(id,std::move(promise)));
  map_mutex.unlock();
}

extern "C"  void __fork_sched_float(double (*statement)(void),int64_t id) {
  set_max_threads();
  std::future<double> promise;
  thread_count_mutex.lock();
  if (thread_count >= max_threads) {
    thread_count_mutex.unlock();
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
    thread_count_mutex.unlock();
  }
  map_mutex.lock();
  float_future_id_map.insert(std::pair<int64_t,std::future<double>>(id,std::move(promise)));
  map_mutex.unlock();
}

extern "C"  void __fork_sched_intptr(int64_t* (*statement)(void),int64_t id) {
  set_max_threads();
  std::future<int64_t*> promise;
  thread_count_mutex.lock();
  if (thread_count >= max_threads) {
    thread_count_mutex.unlock();
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
    thread_count_mutex.unlock();
  }
  map_mutex.lock();
  intptr_future_id_map.insert(std::pair<int64_t,std::future<int64_t*>>(id,std::move(promise)));
  map_mutex.unlock();
}

extern "C"  void __fork_sched_floatptr(double* (*statement)(void),int64_t id) {
  set_max_threads();
  std::future<double*> promise;
  thread_count_mutex.lock();
  if (thread_count >= max_threads) {
    thread_count_mutex.unlock();
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
    thread_count_mutex.unlock();
  }
  map_mutex.lock();
  floatptr_future_id_map.insert(std::pair<int64_t,std::future<double*>>(id,std::move(promise)));
  map_mutex.unlock();
}

extern "C"  void __fork_sched_void(void (*statement)(void),int64_t id) {
  set_max_threads();
  std::future<void> promise;
  thread_count_mutex.lock();
  if (thread_count >= max_threads) {
    thread_count_mutex.unlock();
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
    thread_count_mutex.unlock();
  }
  map_mutex.lock();
  void_future_id_map.insert(std::pair<int64_t,std::future<void>>(id,std::move(promise)));
  map_mutex.unlock();
}

extern "C" int64_t __recon_int(int64_t original,int64_t known,int64_t update,int64_t id,int64_t max) {
  std::chrono::milliseconds span(0);
  map_mutex.lock();
  auto fv = std::move(int_future_id_map.at(id));
  int_future_id_map.erase(id);
  map_mutex.unlock();
  if (fv.wait_for(span)==std::future_status::deferred) {
    return fv.get();
  } else {
    auto v = fv.get();
    thread_count_mutex.lock();
    thread_count--;
    thread_count_mutex.unlock();
    return v;
  }
}

extern "C" double __recon_float(double original,int64_t known,double update,int64_t id,int64_t max) {
  std::chrono::milliseconds span(0);
  map_mutex.lock();
  auto fv = std::move(float_future_id_map.at(id));
  float_future_id_map.erase(id);
  map_mutex.unlock();
  if (fv.wait_for(span)==std::future_status::deferred) {
    return fv.get();
  } else {
    auto v = fv.get();
    thread_count_mutex.lock();
    thread_count--;
    thread_count_mutex.unlock();
    return v;
  }
}

extern "C" int64_t* __recon_intptr(int64_t* original,int64_t known,int64_t* update,int64_t id,int64_t max) {
  std::chrono::milliseconds span(0);
  map_mutex.lock();
  auto fv = std::move(intptr_future_id_map.at(id));
  intptr_future_id_map.erase(id);
  map_mutex.unlock();
  if (fv.wait_for(span)==std::future_status::deferred) {
    return fv.get();
  } else {
    auto v = fv.get();
    thread_count_mutex.lock();
    thread_count--;
    thread_count_mutex.unlock();
    return v;
  }
}

extern "C" double* __recon_floatptr(double* original,int64_t known,double* update,int64_t id,int64_t max) {
  std::chrono::milliseconds span(0);
  map_mutex.lock();
  auto fv = std::move(floatptr_future_id_map.at(id));
  floatptr_future_id_map.erase(id);
  map_mutex.unlock();
  if (fv.wait_for(span)==std::future_status::deferred) {
    return fv.get();
  } else {
    auto v = fv.get();
    thread_count_mutex.lock();
    thread_count--;
    thread_count_mutex.unlock();
    return v;
  }
}

extern "C" void  __recon_void(int64_t id,int64_t max) {
  std::chrono::milliseconds span(0);
  map_mutex.lock();
  auto fv = std::move(void_future_id_map.at(id));
  void_future_id_map.erase(id);
  map_mutex.unlock();
  if (fv.wait_for(span)==std::future_status::deferred) {
    fv.get();
  } else {
    fv.get();
    thread_count_mutex.lock();
    thread_count--;
    thread_count_mutex.unlock();
  }
}

