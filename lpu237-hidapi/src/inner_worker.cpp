/*
 * inner_worker.cpp
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 */

#include <inner_worker.h>
#include <vector>
#include <string.h>
#include <mutex>

using namespace std;

inner_worker & inner_worker::get_instance(type_fun_tx fun_tx/*=NULL*/, type_fun_rx fun_rx/*=NULL*/)
{
	static inner_worker obj(fun_tx,fun_rx);//Don't use "-nostartfiles" linker option

	return obj;
}

bool inner_worker::is_setup_ok()
{
	return m_b_setup;
}

inner_worker::inner_worker(type_fun_tx fun_tx, type_fun_rx fun_rx) :
 m_b_setup(false)
, m_n_id(-1)
, m_n_cur_result_index(1)
{
	do{
		if( m_evet_kill.get_handle() == NULL )
			continue;
		if( m_evet_wakeup.get_handle() == NULL )
			continue;
		//
		m_cur_job.b_process = 0;
		m_cur_job.b_rx = 0;
		m_cur_job.h_dev = 0;
		m_cur_job.h_event_notify = 0;
		m_cur_job.n_index = -1;
		m_cur_job.ptr_rx = nullptr;
		m_cur_job.ptr_tx = nullptr;
		//
		m_b_setup = _start();
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

bool inner_worker::_start()
{
	bool b_result = false;
	int n_result = 0;

	do{
		if( m_n_id >= 0 )
			continue;//already started
		//
		n_result = pthread_create(&m_n_id, NULL, inner_worker::_worker, (void *)this);
		if( n_result != 0 ){
			m_n_id = -1;
			continue;
		}
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
		if( m_n_id == (pthread_t)-1 )
			continue;
		if( m_evet_kill.get_handle() == NULL )
			continue;
		m_evet_kill.set();

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
	vector<neosmart::neosmart_event_t> v_events(0);

	v_events.push_back( inner_worker::get_instance().m_evet_wakeup.get_handle() );
	v_events.push_back( inner_worker::get_instance().m_evet_kill.get_handle() );
	v_events.resize(2);
	//
	do{
		n_result_wait = WaitForMultipleEvents(
				&v_events[0], v_events.size(), false, const_worker_timeout_wait, n_index_event
				);

		switch( n_result_wait ){
		case 0://success
			if( n_index_event >= (int)v_events.size() )
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

	return p_result;
}

bool inner_worker::_job_process()
{
	bool b_continue_run = true;

	do{
		if( _job_is_empty() )
			continue;
		if( m_cur_job.b_process ){
			//cancel current job
			m_cur_job.b_process = 0;
			_notify_result( m_cur_job.n_index, result_fun_cancel, m_cur_job.ptr_rx,m_cur_job.h_event_notify );
		}
		//
		type_job_item item;

		if( !_job_pop( item ) )
			continue;
		if( item.ptr_tx == nullptr ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item.n_index, result_fun_success, nullptr, item.h_event_notify );
			}
			continue;
		}
		if( item.ptr_tx->size() == 0 ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item.n_index, result_fun_success, nullptr, item.h_event_notify );
			}
			continue;
		}
		if( m_fun_tx == NULL ){
			if( item.b_rx )
				_save_job_item( item );
			else{
				//cancel case
				_notify_result( item.n_index, result_fun_success, nullptr, item.h_event_notify );
			}
			continue;
		}
		// tx part.
		bool b_tx_continue = false;
		do{
			b_tx_continue = false;
			//
			type_result_fun result_fun = m_fun_tx( item.h_dev, item.ptr_tx );
			switch( result_fun ){
			case result_fun_success:
				if( item.b_rx == 1 ){
					_save_job_item( item );
					break;
				}
			case result_fun_error:
			case result_fun_cancel:
				_notify_result( item.n_index, result_fun, nullptr, item.h_event_notify );
				break;
			case result_fun_ing:
				b_tx_continue = true;
				break;
			default:
				break;
			}//end switch

		}while(b_tx_continue);

	}while(0);

	return b_continue_run;
}

bool inner_worker::_idle_process()
{
	bool b_continue_run = true;

	do{
		if( m_cur_job.b_process==0 )
			continue;
		if(	m_cur_job.b_rx == 0 )
			continue;
		if( m_fun_rx == 0 )
			continue;
		//
		type_result_fun result_fun = m_fun_rx( m_cur_job.h_dev, m_cur_job.ptr_tx );

		m_cur_job.b_process = 0;//stop idle process

		switch( result_fun ){
		case result_fun_error:
			m_cur_job.ptr_rx->resize(0);
		case result_fun_cancel:
		case result_fun_success:
			_notify_result( m_cur_job.n_index, result_fun, m_cur_job.ptr_rx, m_cur_job.h_event_notify );
			break;
		case result_fun_ing:
			m_cur_job.b_process = 0;	//continue idle process.
			break;
		default:
			break;
		}//end switch

	}while(0);

	return b_continue_run;
}

bool inner_worker::_create_result( int n_index, type_result_fun result_fun, const type_ptr_buffer & ptr_rx )
{
	bool b_result = false;

	do{
		if( n_index < 0 )
			continue;
		//
		type_map_result::iterator it =  m_map_result.find(n_index);
		if( it != end(m_map_result) )
			continue;//already exist result.
		//
		type_job_result result = { result_fun, ptr_rx };
		m_map_result[n_index] = result;
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool inner_worker::_delete_result( int n_index )
{
	bool b_result = false;

	do{
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
		inner_event & event_notify,
		void *h_dev,
		const unsigned char *s_tx, unsigned long n_tx,
		bool b_need_rx /*= true*/,
		int n_result_index /*= -1*/ )
{
	return push_job(
			event_notify.get_handle()
			, h_dev
			, s_tx
			, n_tx
			, b_need_rx
			, n_result_index
			);
}

int inner_worker::push_job(
		neosmart::neosmart_event_t h_event_notify,
		void *h_dev,
		const unsigned char *s_tx,
		unsigned long n_tx,
		bool b_need_rx /*= true*/,
		int n_result_index /*= -1*/ )
{
	int n_index = -1;
	unsigned char b_rx = 0;

	do{
		if( n_tx < 0 )
			n_tx = 0;
		if( b_need_rx )
			b_rx = 1;
		//
		type_job_item item = {
				h_event_notify,
				h_dev,
				type_ptr_buffer( new type_buffer(n_tx) ),
				b_rx,
				n_result_index };
		if( n_tx > 0 )
			memcpy( &((*item.ptr_tx)[0]), s_tx, n_tx );
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
			item.n_index = m_n_cur_result_index;
			m_n_cur_result_index++;
			if( m_n_cur_result_index < 0)
				m_n_cur_result_index = 1;//reset
			n_index = item.n_index;
		}

		if( !_job_push( item ) ){
			n_index = -1;
			continue;
		}

	}while(0);

	return n_index;
}

bool inner_worker::get_result( int n_result_index, type_job_result & result )
{
	bool b_result = false;

	do{
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

void inner_worker::_save_job_item(
		const type_job_item & item
		,bool b_enable_process /*=true*/
		, bool b_save_tx /*= false*/
		)
{
	if( b_enable_process )
		m_cur_job.b_process = 1;
	else
		m_cur_job.b_process = 0;
	//
	m_cur_job.b_rx = item.b_rx;
	m_cur_job.h_dev = item.h_dev;
	m_cur_job.h_event_notify = item.h_event_notify;
	m_cur_job.n_index = item.n_index;
	m_cur_job.ptr_rx = type_ptr_buffer( new type_buffer(0) );

	m_cur_job.ptr_tx = nullptr;
	do{
		if( !b_save_tx )
			continue;
		if( item.ptr_tx == nullptr )
			continue;
		if( item.ptr_tx->size() == 0 )
			continue;
		//deep copy tx data.
		m_cur_job.ptr_tx = type_ptr_buffer( new type_buffer(begin(*item.ptr_tx),end(*item.ptr_tx)) );
	}while(0);
}

void inner_worker::_notify_result(
		int n_index,
		type_result_fun result_fun,
		const type_ptr_buffer & ptr_rx,
		neosmart::neosmart_event_t h_event_notify
		)
{
	do{
		if( !_create_result( n_index, result_fun, ptr_rx ) )
			continue;
		if( h_event_notify == NULL )
			continue;
		SetEvent( h_event_notify );//notify event
	}while(0);
}


