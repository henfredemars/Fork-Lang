
//Implementation of the parallel statement execution manager

#include "parContextManager.h"

StatementContext::StatementContext() {
  //Do nothing
}

/*=================================StatementContext=================================*/
void StatementContext::addIntFuture(std::future<int64_t>& f, const int64_t id) {
  map_mutex.lock();
  assert(int_future_id_map.count(id)==0 && "Statement id already exists");
  int_future_id_map.insert(std::pair<int64_t,std::future<int64_t>>(id,std::move(f)));
  map_mutex.unlock();
}

void StatementContext::addFloatFuture(std::future<double>& f, const int64_t id) {
  map_mutex.lock();
  assert(float_future_id_map.count(id)==0 && "Statement id already exists");
  float_future_id_map.insert(std::pair<int64_t,std::future<double>>(id,std::move(f)));
  map_mutex.unlock();
}

void StatementContext::addIntptrFuture(std::future<int64_t*>& f, const int64_t id) {
  map_mutex.lock();
  assert(intptr_future_id_map.count(id)==0 && "Statement id already exists");
  intptr_future_id_map.insert(std::pair<int64_t,std::future<int64_t*>>(id,std::move(f)));
  map_mutex.unlock();
}

void StatementContext::addFloatptrFuture(std::future<double*>& f, const int64_t id) {
  map_mutex.lock();
  assert(floatptr_future_id_map.count(id)==0 && "Statement id already exists");
  floatptr_future_id_map.insert(std::pair<int64_t,std::future<double*>>(id,std::move(f)));
  map_mutex.unlock();
}

void StatementContext::addVoidFuture(std::future<void>& f, const int64_t id) {
  map_mutex.lock();
  assert(void_future_id_map.count(id)==0 && "Statement id already exists");
  void_future_id_map.insert(std::pair<int64_t,std::future<void>>(id,std::move(f)));
  map_mutex.unlock();
}

std::future<int64_t> StatementContext::getIntFuture(const int64_t id) {
  map_mutex.lock();
  auto f = std::move(int_future_id_map.at(id));
  int_future_id_map.erase(id);
  map_mutex.unlock();
  return f;
}

std::future<double> StatementContext::getFloatFuture(const int64_t id) {
  map_mutex.lock();
  auto f = std::move(float_future_id_map.at(id));
  float_future_id_map.erase(id);
  map_mutex.unlock();
  return f;
}

std::future<int64_t*> StatementContext::getIntptrFuture(const int64_t id) {
  map_mutex.lock();
  auto f = std::move(intptr_future_id_map.at(id));
  intptr_future_id_map.erase(id);
  map_mutex.unlock();
  return f;
}

std::future<double*> StatementContext::getFloatptrFuture(const int64_t id) {
  map_mutex.lock();
  auto f = std::move(floatptr_future_id_map.at(id));
  floatptr_future_id_map.erase(id);
  map_mutex.unlock();
  return f;
}

std::future<void> StatementContext::getVoidFuture(const int64_t id) {
  map_mutex.lock();
  auto f = std::move(void_future_id_map.at(id));
  void_future_id_map.erase(id);
  map_mutex.unlock();
  return f;
}

/*=================================ParContextManager=================================*/
ParContextManager::ParContextManager() {
  set_max_threads();
  thread_count = 0;
  next_cid = 0;
}

int64_t ParContextManager::make_context() {
  mutex.lock();
  int64_t cid = next_cid++;
  mutex.unlock();
  return cid;
}

void ParContextManager::destroy_context(const int64_t cid) {
  mutex.lock();
  assert(context_map.count(cid)==1);
  context_map.erase(cid);
  mutex.unlock();
}

void ParContextManager::sched_int(int64_t (*statement)(void),const int64_t id,const int64_t cid) {
  mutex.lock();
  std::future<int64_t> promise;
  if (thread_count >= max_threads) {
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
  }
  context_map.at(cid).addIntFuture(promise,id);
  mutex.unlock();
}

void ParContextManager::sched_float(double (*statement)(void),const int64_t id,const int64_t cid) {
  mutex.lock();
  std::future<double> promise;
  if (thread_count >= max_threads) {
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
  }
  context_map.at(cid).addFloatFuture(promise,id);
  mutex.unlock();
}

void ParContextManager::sched_intptr(int64_t* (*statement)(void),int64_t id,const int64_t cid) {
  mutex.lock();
  std::future<int64_t*> promise;
  if (thread_count >= max_threads) {
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
  }
  context_map.at(cid).addIntptrFuture(promise,id);
  mutex.unlock();
}

void ParContextManager::sched_floatptr(double* (*statement)(void),int64_t id,const int64_t cid) {
  mutex.lock();
  std::future<double*> promise;
  if (thread_count >= max_threads) {
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
  }
  context_map.at(cid).addFloatptrFuture(promise,id);
  mutex.unlock();
}

void ParContextManager::sched_void(void (*statement)(void),int64_t id,const int64_t cid) {
  mutex.lock();
  std::future<void> promise;
  if (thread_count >= max_threads) {
    promise = std::async(std::launch::deferred,statement);
  } else {
    promise = std::async(std::launch::async,statement);
    thread_count++;
  }
  context_map.at(cid).addVoidFuture(promise,id);
  mutex.unlock();
}

int64_t ParContextManager::recon_int(const int64_t original,const int64_t known,const int64_t update,
                const int64_t id,const int64_t max,const int64_t cid) {
  mutex.lock();
  std::chrono::milliseconds span(0);
  std::future<int64_t> fv = context_map.at(cid).getIntFuture(id);
  int64_t v;
  if (fv.wait_for(span)==std::future_status::deferred) {
    v = fv.get();
  } else {
    v = fv.get();
    thread_count--;
  }
  mutex.unlock();
  return v;
}

double ParContextManager::recon_float(const double original,const int64_t known,const double update,
                const int64_t id,const int64_t max,const int64_t cid) {
  mutex.lock();
  std::chrono::milliseconds span(0);
  std::future<double> fv = context_map.at(cid).getFloatFuture(id);
  double v;
  if (fv.wait_for(span)==std::future_status::deferred) {
    v = fv.get();
  } else {
    v = fv.get();
    thread_count--;
  }
  mutex.unlock();
  return v;
}

int64_t* ParContextManager::recon_intptr(const int64_t* original,const int64_t known,const int64_t* update,
                const int64_t id,const int64_t max,const int64_t cid) {
  mutex.lock();
  std::chrono::milliseconds span(0);
  std::future<int64_t*> fv = context_map.at(cid).getIntptrFuture(id);
  int64_t* v;
  if (fv.wait_for(span)==std::future_status::deferred) {
    v = fv.get();
  } else {
    v = fv.get();
    thread_count--;
  }
  mutex.unlock();
  return v;
}

double* ParContextManager::recon_floatptr(const double* original,const int64_t known,const double* update,
                const int64_t id,const int64_t max,const int64_t cid) {
  mutex.lock();
  std::chrono::milliseconds span(0);
  std::future<double*> fv = context_map.at(cid).getFloatptrFuture(id);
  double* v;
  if (fv.wait_for(span)==std::future_status::deferred) {
    v = fv.get();
  } else {
    v = fv.get();
    thread_count--;
  }
  mutex.unlock();
  return v;
}

void ParContextManager::recon_void(const int64_t id,const int64_t max,const int64_t cid) {
  mutex.lock();
  std::chrono::milliseconds span(0);
  std::future<void> fv = context_map.at(cid).getVoidFuture(id);
  if (fv.wait_for(span)==std::future_status::deferred) {
    fv.get();
  } else {
    fv.get();
    thread_count--;
  }
  mutex.unlock();
}

void ParContextManager::set_max_threads() {
  unsigned long dth = std::thread::hardware_concurrency()-1;
  printf("Detected %d additional compute elements.\n",(int)dth);
  if (dth < 0) max_threads = 0;
  else if (dth > 3) max_threads = 3;
  else max_threads = dth;
  printf("Setting max execution threads to: %d\n",(int)max_threads+1);
}

