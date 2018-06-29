/*
 * inner_worker.h
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_INNER_WORKER_H_
#define INC_INNER_WORKER_H_

#include <mutex>
#include <queue>
#include <map>
#include <memory>
#include <vector>

#include <pthread.h>

#include <shared.h>
#include "lpu237-hidapi.h"
#include "inner_event.h"

using namespace std;

class inner_worker{

public:
	typedef enum{
		mode_msr = 1,
		mode_key = 2,
		mode_msr_or_key = 3,
		mode_system = 4
	}type_mode;

	typedef	shared_ptr<inner_event>	type_ptr_event;

	typedef	shared::type_result_fun(*type_fun_tx)(void *h_dev, const shared::type_ptr_buffer & ptr_tx, type_mode mode );
	typedef	shared::type_result_fun(*type_fun_rx)(void *h_dev, shared::type_ptr_buffer & ptr_rx, type_mode mode );
	typedef	shared::type_result_fun(*type_fun_flush)(void *h_dev );

	typedef struct tag_job_item{
		void *h_dev;
		shared::type_ptr_buffer ptr_tx;
		bool b_rx;	//need response.
		bool b_pump_rx;// when idle pumping rx.
		type_mode mode;

		LPU237_type_callback fun_wait;	//callback for wait thread
		void *p_parameter_for_fun_wait;

		int n_index;	//the index of m_map_result
	}type_job_item;

	typedef struct tag_job_process{
		bool b_process;
		type_mode mode;

		void *h_dev;

		shared::type_ptr_buffer ptr_tx;
		shared::type_ptr_buffer ptr_rx;

		LPU237_type_callback fun_wait;	//callback for wait thread
		void *p_parameter_for_fun_wait;

		int n_index;	//the index of m_map_result
	}type_job_process;

	typedef struct tag_job_result{
		shared::type_result_fun result_fun;
		shared::type_ptr_buffer ptr_rx;
		type_ptr_event ptr_event_notify;
		type_mode mode;
	}type_job_result;

	typedef	map< int,type_job_result >	type_map_result;

private:
	enum{
		const_worker_timeout_wait = 5	//msec
	};
public:
	~inner_worker();
	static inner_worker & get_instance(type_fun_tx fun_tx=NULL, type_fun_rx fun_rx=NULL, type_fun_flush fun_flush=NULL);

	bool is_setup_ok();
	bool start_worker();
	void kill_worker();

	// return result map key
	int push_job(
			void *h_dev,
			shared::type_buffer & v_tx,
			LPU237_type_callback fun_wait,
			void *p_parameter_for_fun_wait,
			bool b_need_rx = true,
			bool b_pump_rx = true,
			type_mode mode = mode_msr
			 );

	// return result map key
	int push_job( type_job_item & item );

	//get from m_map_result, end erase it.
	bool get_result_and_delete( int n_result_index, type_job_result & result );
	//get event from m_map_result.
	inner_event *get_result_event( int n_result_index );

private:
	static void* _worker(void *p_data);
private:
	inner_worker(type_fun_tx fun_tx, type_fun_rx fun_rx, type_fun_flush fun_flush);

	bool _stop();

	bool _job_process();
	bool _idle_process();

	bool _job_push(const type_job_item & item );
	bool _job_pop( type_job_item & item );
	bool _job_is_empty();

	bool _create_result( int n_index, type_mode mode );
	bool _set_result( int n_index, shared::type_result_fun result_fun, const shared::type_buffer & v_rx );
	bool _delete_result( int n_index );

	void _save_job_item( const type_job_item & item
			, bool b_enable_process = true
			, bool b_save_tx = false );

	void _notify_result(
			const type_job_item & item,
			shared::type_result_fun result_fun,
			const shared::type_buffer & v_rx
			);
	void _notify_result(
			shared::type_result_fun result_fun
			);

private:
	void *m_h_pump;
	bool m_b_setup;
	pthread_t m_n_id;	// main worker thread id

	int m_n_cur_result_index;//will be allocated result index of m_map_result.

	inner_event m_evet_kill;	//main thread kill event
	inner_event m_evet_wakeup;//main thread wakeup event

	queue<type_job_item> m_q_job;	//job queue
	mutex m_mutex_job;

	type_map_result m_map_result;	//result map
	mutex m_mutex_result;

	type_fun_tx m_fun_tx;
	type_fun_rx m_fun_rx;
	type_fun_flush m_fun_flush;

	type_job_process m_cur_job;	//current processing job data

private://don't call these method.
	inner_worker();
	inner_worker( const inner_worker & );
	inner_worker & operator=( const inner_worker & );
};


#endif /* INC_INNER_WORKER_H_ */
