
//Parallel statement execution management object

//DO NOT USE GC HERE --- not compatible with C++11 <thread>

#ifndef __PARCONTEXTMANAGER_H
#define __PARCONTEXTMANAGER_H

#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <cassert>
#include <stdint.h>
#include <stdio.h>

//All methods can be safely called from any thread and in parallel

class StatementContext {
public:
	StatementContext();
	StatementContext(StatementContext&& sc);
	void addIntFuture(std::future<int64_t>& f, const int64_t id);
	void addFloatFuture(std::future<double>& f, const int64_t id);
	void addIntptrFuture(std::future<int64_t*>& f, const int64_t id);
	void addFloatptrFuture(std::future<double*>& f, const int64_t id);
	void addVoidFuture(std::future<void>& f, const int64_t id);
	std::future<int64_t> getIntFuture(const int64_t id);
	std::future<double> getFloatFuture(const int64_t id);
	std::future<int64_t*> getIntptrFuture(const int64_t id);
	std::future<double*> getFloatptrFuture(const int64_t id);
	std::future<void> getVoidFuture(const int64_t id);
private:
	std::unordered_map<int64_t,std::future<int64_t>> int_future_id_map;
	std::unordered_map<int64_t,std::future<double>> float_future_id_map;
	std::unordered_map<int64_t,std::future<int64_t*>> intptr_future_id_map;
	std::unordered_map<int64_t,std::future<double*>> floatptr_future_id_map;
	std::unordered_map<int64_t,std::future<void>> void_future_id_map;
	std::mutex map_mutex;
};

class ParContextManager {
public:
	ParContextManager();
	int64_t make_context();
	void destroy_context(const int64_t cid);
	void sched_int(int64_t (*statement)(void*),void* env,const int64_t id,const int64_t cid);
	void sched_float(double (*statement)(void*),void* env,const int64_t id,const int64_t cid);
	void sched_intptr(int64_t* (*statement)(void*),void* env,int64_t id,const int64_t cid);
	void sched_floatptr(double* (*statement)(void*),void* env,int64_t id,const int64_t cid);
	void sched_void(void (*statement)(void*),void* env,int64_t id,const int64_t cid);
	int64_t recon_int(const int64_t original,const int64_t known,
		const int64_t id,const int64_t max,const int64_t cid);
	double recon_float(const double original,const int64_t known,
		const int64_t id,const int64_t max,const int64_t cid);
	int64_t* recon_intptr(const int64_t* original,const int64_t known,
		const int64_t id,const int64_t max,const int64_t cid);
	double* recon_floatptr(const double* original,const int64_t known,
		const int64_t id,const int64_t max,const int64_t cid);
	void recon_void(const int64_t id,const int64_t max,const int64_t cid);
private:
	void set_max_threads();
	std::unordered_map<int64_t,StatementContext> context_map;
	int64_t thread_count;
	int64_t max_threads;
	int64_t next_cid;
	std::mutex mutex;
};

#endif /* __PARCONTEXTMANAGER_H */
