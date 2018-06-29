/*
 * inner_worker.cpp
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#include <inner_worker.h>
#include <vector>
#include <string.h>
#include <mutex>

#include <inner_log.h>

using namespace std;

inner_worker & inner_worker::get_instance(
		type_fun_tx fun_tx/*=NULL*/
		, type_fun_rx fun_rx/*=NULL*/
		,  type_fun_flush fun_flush /*=NULL*/)
{
	static inner_worker obj(fun_tx,fun_rx,fun_flush);//Don't use "-nostartfiles" linker option

	return obj;
}

bool inner_worker::is_setup_ok()
{
	return m_b_setup;
}

inner_worker::inner_worker(type_fun_tx fun_tx, type_fun_rx fun_rx, type_fun_flush fun_flush) :
m_h_pump(NULL)
,m_b_setup(false)
, m_n_id(-1)
, m_n_cur_result_index(1)
, m_evet_kill(true,false)
, m_fun_tx(fun_tx)
, m_fun_rx(fun_rx)
, m_fun_flush(fun_flush)
{
	do{
		if( m_evet_kill.get_handle() == NULL ){
			LOG_ERROR("m_evet_kill.get_handle() == NULL");
			continue;
		}
		if( m_evet_wakeup.get_handle() == NULL ){
			LOG_ERROR("m_evet_wakeup.get_handle() == NULL");
			continue;
		}
		//
		m_cur_job.b_process = 0;
		m_cur_job.mode = mode_msr;
		m_cur_job.h_dev = 0;
		m_cur_job.n_index = -1;
		m_cur_job.ptr_rx = nullptr;
		m_cur_job.ptr_tx = nullptr;
		m_cur_job.fun_wait = NULL;
		m_cur_job.p_parameter_for_fun_wait = NULL;
		//
	}while(0);
}

void inner_worker::kill_worker()
{
	_stop();
}

inner_worker::~inner_worker()
{
	kill_worker();
}

bool inner_worker::_job_push(const type_job_item & item )
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

bool inner_worker::_job_pop( type_job_item & item )
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

bool inner_worker::start_worker()
{
	bool b_result = false;
	int n_result = 0;

	do{
		if( m_n_id != (pthread_t)-1 ){
			LOG_WARNING("already started.");
			continue;//already started
		}
		//
		n_result = pthread_create(&m_n_id, NULL, inner_worker::_worker, (void *)this);
		if( n_result != 0 ){
			m_n_id = -1;
			LOG_ERROR("pthread_create()");
			continue;
		}
		//success
		m_b_setup = true;
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_stop()
{
	bool b_result = 0;
	void *p_status = NULL;

	do{
		if( m_n_id == (pthread_t)-1 ){
			LOG_WARNING("already stop");
			continue;
		}
		if( m_evet_kill.get_handle() == NULL ){
			LOG_ERROR("m_evet_kill.get_handle() == NULL");
			continue;
		}
		m_evet_kill.set();

		pthread_join(m_n_id,&p_status);

		b_result = true;
	}while(0);

	return b_result;
}

void* inner_worker::_worker(void *p_data)
{
	static int n_return_val = 0;

	//void *p_result = NULL;
	bool b_run = true;
	int n_result_wait = 0;
	int n_index_event = 0;
	neosmart::neosmart_event_t events[] = {
			inner_worker::get_instance().m_evet_wakeup.get_handle(),
			inner_worker::get_instance().m_evet_kill.get_handle()
	};

	//
	do{
		n_result_wait = WaitForMultipleEvents(
				events, 2, false, const_worker_timeout_wait, n_index_event
				);

		switch( n_result_wait ){
		case 0://success
			if( n_index_event >= 2 )
				break;
			//
			if( n_index_event == 1){
				//kill event
				b_run = false;
				break;
			}

			//wakeup event
			b_run = inner_worker::get_instance()._job_process();
			break;
		case WAIT_TIMEOUT:
			b_run = inner_worker::get_instance()._idle_process();
			break;
		default://error
			break;
		}//end switch
	}while(b_run);

	pthread_exit((void*)&n_return_val);
	//return p_result;
}

bool inner_worker::_job_process()
{
	bool b_continue_run = true;

	do{
		if( _job_is_empty() )
			continue;
		if( m_cur_job.b_process ){
			//cancel current job
			m_cur_job.b_process = false;
			if( m_cur_job.ptr_rx == nullptr )
				m_cur_job.ptr_rx = shared::type_ptr_buffer( new shared::type_buffer(0));
			_notify_result( shared::result_fun_cancel );
		}
		//
		type_job_item item;

		if( !_job_pop( item ) ){
			LOG_ERROR("!_job_pop()");
			continue;
		}
		//
		if( item.b_pump_rx )
			m_h_pump = item.h_dev;
		else
			m_h_pump = NULL;
		//
		if( item.ptr_tx == nullptr ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item, shared::result_fun_success, shared::type_buffer(0) );
			}
			continue;
		}
		if( item.ptr_tx->size() == 0 ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item, shared::result_fun_success, shared::type_buffer(0) );
			}
			continue;
		}
		if( m_fun_tx == NULL ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item, shared::result_fun_success, shared::type_buffer(0) );
			}
			continue;
		}
		// tx part.
		bool b_tx_continue = false;
		do{
			b_tx_continue = false;
			//
			shared::type_result_fun result_fun = m_fun_tx( item.h_dev, item.ptr_tx, item.mode );
			switch( result_fun ){
			case shared::result_fun_success:
				if( item.b_rx ){
					_save_job_item( item );
					break;
				}
			case shared::result_fun_error:
			case shared::result_fun_cancel:
				_notify_result( item, result_fun, shared::type_buffer(0) );
				break;
			case shared::result_fun_ing:
				b_tx_continue = true;
				break;
			default:
				LOG_WARNING("m_fun_tx() return unknown");
				break;
			}//end switch

		}while(b_tx_continue);

	}while(0);

	return b_continue_run;
}

bool inner_worker::_idle_process()
{
	bool b_continue_run = true;
	shared::type_result_fun result_fun = shared::result_fun_success;
	static shared::type_ptr_buffer ptr_bump(nullptr);

	do{
		if( m_fun_rx == 0 )
			continue;
		if( !m_cur_job.b_process ){
			if( m_h_pump == NULL )
				continue;
			//
			result_fun = m_fun_rx( m_h_pump, ptr_bump, mode_msr_or_key );
			continue;
		}
		//
		result_fun = m_fun_rx( m_cur_job.h_dev, m_cur_job.ptr_rx, m_cur_job.mode );

		m_cur_job.b_process = false;//stop idle process

		switch( result_fun ){
		case shared::result_fun_error:
			m_cur_job.ptr_rx = nullptr;
		case shared::result_fun_cancel:
		case shared::result_fun_success:
			if( m_cur_job.ptr_rx == nullptr )
				m_cur_job.ptr_rx = shared::type_ptr_buffer( new shared::type_buffer(0) );
			_notify_result( result_fun  );
			break;
		case shared::result_fun_ing:
			m_cur_job.b_process = true;	//continue idle process.
			break;
		default:
			LOG_WARNING("m_fun_rx() return unknown");
			break;
		}//end switch

	}while(0);

	return b_continue_run;
}

bool inner_worker::_create_result( int n_index,type_mode mode )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_result);
		if( n_index < 0 )
			continue;
		//
		type_map_result::iterator it =  m_map_result.find(n_index);
		if( it != end(m_map_result) )
			continue;//already exist result.
		//
		type_job_result result =
		{
				shared::result_fun_ing
				, nullptr
				, type_ptr_event( new inner_event(true,false) )
				, mode
		};
		m_map_result[n_index] = result;
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_set_result( int n_index, shared::type_result_fun result_fun, const shared::type_buffer & v_rx )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_result);
		if( n_index < 0 )
			continue;
		//
		type_map_result::iterator it =  m_map_result.find(n_index);
		if( it == end(m_map_result) )
			continue;//not found
		//
		it->second.result_fun = result_fun;
		it->second.ptr_rx = shared::type_ptr_buffer( new shared::type_buffer(v_rx) );
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_delete_result( int n_index )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_result);
		if( n_index < 0 )
			continue;
		//
		type_map_result::iterator it =  m_map_result.find(n_index);
		if( it == end(m_map_result) )
			continue;//not found result.

		m_map_result.erase( it );
		//
		b_result = true;
	}while(0);

	return b_result;
}

int inner_worker::push_job(
		void *h_dev,
		shared::type_buffer & v_tx,
		LPU237_type_callback fun_wait,
		void *p_parameter_for_fun_wait,
		bool b_need_rx, /*= true*/
		bool b_pump_rx, /*= true */
		type_mode mode /*= mode_msr*/
		)
{
	int n_index = -1;

	do{
		type_job_item item = {
				h_dev,
				shared::type_ptr_buffer( new shared::type_buffer(v_tx) ),
				b_need_rx,
				b_pump_rx,
				mode,
				fun_wait,
				p_parameter_for_fun_wait,

				-1 //auto generation
		};
		//
		n_index = push_job( item );
	}while(0);

	return n_index;
}

int inner_worker::push_job( type_job_item & item )
{
	int n_index = -1;

	do{//m_n_cur_result_index
		if( item.n_index == -1 ){
			//auto generation
			n_index = item.n_index = m_n_cur_result_index;
			m_n_cur_result_index++;
			if( m_n_cur_result_index < 0)
				m_n_cur_result_index = 1;//reset
		}

		if( !_job_push( item ) ){
			n_index = -1;
			LOG_ERROR("!_job_push()");
			continue;
		}

		if( !_create_result( n_index, item.mode ) ){
			type_job_item item;
			_job_pop( item );
			n_index = -1;
			LOG_ERROR("!_create_result()");
			continue;
		}

		m_evet_wakeup.set();
	}while(0);

	return n_index;
}

bool inner_worker::get_result_and_delete( int n_result_index, type_job_result & result )
{
	bool b_result = false;

	do{
		lock_guard<mutex> _lock (m_mutex_result);
		type_map_result::iterator it =  m_map_result.find(n_result_index);
		if( it == end(m_map_result) )
			continue;//not found result.
		//
		result = it->second;

		m_map_result.erase(it);
		//
		b_result = true;
	}while(0);

	return b_result;
}

inner_event *inner_worker::get_result_event( int n_result_index )
{
	inner_event *p_evet = NULL;

	do{
		lock_guard<mutex> _lock (m_mutex_result);
		type_map_result::iterator it =  m_map_result.find(n_result_index);
		if( it == end(m_map_result) )
			continue;//not found result.

		p_evet = it->second.ptr_event_notify.get();
	}while(0);

	return p_evet;
}

void inner_worker::_save_job_item(
		const type_job_item & item
		,bool b_enable_process /*=true*/
		, bool b_save_tx /*= false*/
		)
{
	do{
		m_cur_job.b_process = b_enable_process;
		m_cur_job.mode = item.mode;
		m_cur_job.h_dev = item.h_dev;
		m_cur_job.n_index = item.n_index;
		m_cur_job.ptr_rx = nullptr;
		m_cur_job.ptr_tx = nullptr;
		m_cur_job.fun_wait = item.fun_wait;
		m_cur_job.p_parameter_for_fun_wait = item.p_parameter_for_fun_wait;

		//
		if( item.h_dev ){
			//flush file.
			if( m_fun_flush )
				m_fun_flush( item.h_dev );
		}

		if( !b_save_tx )
			continue;
		if( item.ptr_tx == nullptr )
			continue;
		if( item.ptr_tx->size() == 0 )
			continue;
		//deep copy tx data.
		m_cur_job.ptr_tx = shared::type_ptr_buffer( new shared::type_buffer(begin(*item.ptr_tx),end(*item.ptr_tx)) );
	}while(0);
}

void inner_worker::_notify_result(
		const type_job_item & item,
		shared::type_result_fun result_fun,
		const shared::type_buffer & v_rx
		)
{
	do{
		if( !_set_result( item.n_index, result_fun, v_rx ) ){
			LOG_ERROR("!_set_result()");
			continue;
		}
		inner_event *p_evet = get_result_event( item.n_index );
		if( p_evet == NULL ){
			LOG_ERROR("p_evet == NULL");
			continue;
		}
		p_evet->set();

		//call callback
		if( item.fun_wait )
			item.fun_wait( item.p_parameter_for_fun_wait );
	}while(0);
}

void inner_worker::_notify_result( shared::type_result_fun result_fun )
{
	do{
		if( !_set_result( m_cur_job.n_index, result_fun, *(m_cur_job.ptr_rx) ) ){
			LOG_ERROR("!_set_result()");
			continue;
		}
		inner_event *p_evet = get_result_event( m_cur_job.n_index );
		if( p_evet == NULL ){
			LOG_ERROR("p_evet == NULL");
			continue;
		}
		p_evet->set();

		//call callback
		if( m_cur_job.fun_wait )
			m_cur_job.fun_wait( m_cur_job.p_parameter_for_fun_wait );
	}while(0);
}

