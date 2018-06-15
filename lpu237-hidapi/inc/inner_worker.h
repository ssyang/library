/*
 * inner_worker.h
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 */

#ifndef INC_INNER_WORKER_H_
#define INC_INNER_WORKER_H_

#include <pthread.h>
#include "pevents.h"

#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <memory>
#include <vector>

using namespace neosmart;
using namespace std;

class inner_worker{

public:
	typedef	unsigned long (*type_fun_job)(void*);
	typedef	vector<unsigned char>	type_buffer;
	typedef	shared_ptr<type_buffer>	type_ptr_buffer;

	typedef struct{
		neosmart_event_t h_event_notify;
		type_fun_job p_job;
		void* p_job_parameter;
		int n_index;	//the index of m_map_result
	}job_item;

	typedef struct{
		unsigned char n_result;
		type_ptr_buffer result_buffer;
	}job_result;

	typedef	map< int,job_result >	type_map_result;

private:
	enum{
		const_worker_timeout_wait = 10	//msec
	};
public:
	~inner_worker();
	static inner_worker & get_instance();

private:
	static void* _worker(void *p_data);

	bool push_job( const job_item & item );//<<
	bool get_result( int n_result_index, job_result & result );//<<

private:
	inner_worker();
	bool _start();
	bool _stop();

	bool _job_process();

	bool _job_push(const job_item & item );
	bool _job_pop( job_item & item );
	bool _job_is_empty();

	int _create_result( int n_index, unsigned char n_result, const type_buffer & result_buffer ); //<<
	bool _delete_result( int n_index );//<<

private:
	pthread_t m_n_id;
	neosmart_event_t m_h_evet_kill;
	neosmart_event_t m_h_evet_wakeup;

	queue<job_item> m_q_job;
	mutex m_mutex_job;

	type_map_result m_map_result;
	mutex m_mutex_result;
	int m_n_cur_result_index;

private://don't call these method.
	inner_worker( const inner_worker & );
	inner_worker & operator=( const inner_worker & );
};


#endif /* INC_INNER_WORKER_H_ */
