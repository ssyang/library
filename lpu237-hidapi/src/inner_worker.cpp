/*
 * inner_worker.cpp
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 */

#include "inner_worker.h"
#include <vector>

using namespace neosmart;
using namespace std;

inner_worker & inner_worker::get_instance()
{
	static inner_worker obj;

	return obj;
}

inner_worker::inner_worker() :
		m_n_id(-1)
, m_h_evet_kill(NULL)
, m_h_evet_wakeup(NULL)
, m_n_cur_result_index(-1)
{
	do{
		m_h_evet_kill = CreateEvent( false, false );
		if( m_h_evet_kill == NULL )
			continue;
		m_h_evet_wakeup = CreateEvent( false, false );
		if( m_h_evet_wakeup == NULL )
			continue;
		//
		_start();
	}while(0);
}

inner_worker::~inner_worker()
{
	_stop();

	if( m_h_evet_wakeup )
		DestroyEvent( m_h_evet_wakeup );
	//
	if( m_h_evet_kill )
		DestroyEvent( m_h_evet_kill );
}

bool inner_worker::_job_push(const job_item & item )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_job);

		m_q_job.push(item);
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_job_pop( job_item & item )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_job);

		if( m_q_job.empty() )
			continue;
		item = m_q_job.front();
		m_q_job.pop();
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_job_is_empty()
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_job);
		b_result = m_q_job.empty();
	}while(0);

	return b_result;
}

bool inner_worker::_start()
{
	bool b_result = false;
	int n_result = 0;

	do{
		n_result = pthread_create(&m_n_id, NULL, inner_worker::worker, (void *)this);
		if( n_result != 0 )
			continue;
		//success
		b_result = true;
	}while(0);
	return b_result;
}

bool inner_worker::_stop()
{
	bool b_result = 0;
	int n_status = 0;

	do{
		if( m_n_id == -1 )
			continue;
		if( m_h_evet_kill == NULL )
			continue;
		SetEvent(m_h_evet_kill);

		pthread_join(m_n_id,(void **)&n_status);

		b_result = true;
	}while(0);

	return b_result;
}

void* inner_worker::_worker(void *p_data)
{
	void *p_result = NULL;
	bool b_run = true;
	int n_result_wait = 0;
	int n_index_event = 0;
	vector<neosmart_event_t> v_events(0);

	v_events.push_back( inner_worker::get_instance().m_h_evet_wakeup );
	v_events.push_back( inner_worker::get_instance().m_h_evet_kill );
	v_events.resize(2);
	//
	do{
		n_result_wait = WaitForMultipleEvents(
				&v_events[0], v_events.size(), false, const_worker_timeout_wait, &n_index_event
				);

		switch( n_result_wait ){
		case 0://success
			if( n_index_event >= v_events.size() )
				break;
			//
			if( n_index_event == 1){
				//kill event
				b_run = false;
				break;
			}

			//wakeup event
			b_run = inner_worker::get_instance().job_process();
			break;
		case WAIT_TIMEOUT:
			break;
		default://error
			break;
		}//end switch
	}while(b_run);

	return p_result;
}

bool inner_worker::_job_process()
{
	bool b_continue_run = true;
	unsigned long n_result_job = 0;
	int n_index = -1;

	do{
		if( _job_is_empty() )
			continue;
		//
		job_item item;

		if( !_job_pop( item ) )
			continue;
		//
		if(	item.p_job == NULL )
			continue;

		n_result_job = item.p_job( item.p_job_parameter );

		//n_index = create_result( n_result_job,  );
		if( n_index < 0 )
			continue;

	}while(0);

	return b_continue_run;
}



