/*
 * inner_worker.h
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 */

#ifndef INC_INNER_WORKER_H_
#define INC_INNER_WORKER_H_

#include <mutex>
#include <queue>
#include <map>
#include <memory>
#include <vector>

#include <pthread.h>

#include "lpu237-hidapi.h"
#include "inner_event.h"

using namespace std;

class inner_worker{

public:
	typedef enum{
		result_fun_success = 0,
		result_fun_error = 1,
		result_fun_ing = 2,
		result_fun_cancel = 3
	} type_result_fun;

	typedef	shared_ptr<inner_event>	type_ptr_event;
	typedef	vector<unsigned char>	type_buffer;
	typedef	shared_ptr<type_buffer>	type_ptr_buffer;

	typedef	type_result_fun(*type_fun_tx)(void *h_dev, const type_ptr_buffer & ptr_tx );
	typedef	type_result_fun(*type_fun_rx)(void *h_dev, type_ptr_buffer & ptr_rx );

	typedef struct tag_job_item{
		void *h_dev;
		type_ptr_buffer ptr_tx;
		bool b_rx;	//need response.

		LPU237_type_callback fun_wait;	//callback for wait thread
		void *p_parameter_for_fun_wait;

		int n_index;	//the index of m_map_result
	}type_job_item;

	typedef struct tag_job_process{
		bool b_process;

		void *h_dev;

		type_ptr_buffer ptr_tx;
		type_ptr_buffer ptr_rx;

		LPU237_type_callback fun_wait;	//callback for wait thread
		void *p_parameter_for_fun_wait;

		int n_index;	//the index of m_map_result
	}type_job_process;

	typedef struct tag_job_result{
		type_result_fun result_fun;
		type_ptr_buffer ptr_rx;
		type_ptr_event ptr_event_notify;
	}type_job_result;

	typedef	map< int,type_job_result >	type_map_result;

private:
	enum{
		const_worker_timeout_wait = 10	//msec
	};
public:
	~inner_worker();
	static inner_worker & get_instance(type_fun_tx fun_tx=NULL, type_fun_rx fun_rx=NULL);

	bool is_setup_ok();
	void kill_worker();

	// return result map key
	int push_job(
			void *h_dev,
			type_buffer & v_tx,
			LPU237_type_callback fun_wait,
			void *p_parameter_for_fun_wait,
			bool b_need_rx = true
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
	inner_worker(type_fun_tx fun_tx, type_fun_rx fun_rx);

	bool _start();
	bool _stop();

	bool _job_process();
	bool _idle_process();

	bool _job_push(const type_job_item & item );
	bool _job_pop( type_job_item & item );
	bool _job_is_empty();

	bool _create_result( int n_index );
	bool _set_result( int n_index, type_result_fun result_fun, const type_buffer & v_rx );
	bool _delete_result( int n_index );

	void _save_job_item( const type_job_item & item
			, bool b_enable_process = true
			, bool b_save_tx = false );

	void _notify_result(
			int n_index,
			type_result_fun result_fun,
			const type_buffer & v_rx
			);

private:
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

	type_job_process m_cur_job;	//current processing job data

private://don't call these method.
	inner_worker();
	inner_worker( const inner_worker & );
	inner_worker & operator=( const inner_worker & );
};


#endif /* INC_INNER_WORKER_H_ */
