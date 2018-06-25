/*
 * lpu237-hidapi.c
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#include "lpu237-hidapi.h"
#include "hidapi.h"
#include "msr_com.h"
#include "inner_log.h"

#include <dlfcn.h>
#include <inner_worker.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define	_USB_VID_LPU237	0x134b
#define	_USB_PID_LPU237	0x0206
#define	_USB_INF_LPU237	1
#define	_SIZE_UID	16
//
static inner_worker::type_result_fun _tx_callback( void *h_dev, const inner_worker::type_ptr_buffer & ptr_tx );
static inner_worker::type_result_fun _rx_callback( void *h_dev, inner_worker::type_ptr_buffer & ptr_rx );
static int _cmd_tx_rx(
		hid_device *h_dev
		, type_msr_cmd cmd
		, unsigned char c_sub
		, unsigned long n_data
		, const unsigned char *s_data
		, unsigned long & n_rx	//in - buffer size, out readdone size
		, unsigned char *s_rx
		, bool b_wait_response
		, LPU237_type_callback cb
		, void *p_parameter_for_cb
		);

static unsigned long _single_cmd( LPU237_HANDLE h_dev,type_msr_cmd cmd );
static unsigned long _enter_opos( LPU237_HANDLE h_dev );
static unsigned long _leave_opos( LPU237_HANDLE h_dev );
/////////////////////////////////////////////////////
//local function body.

inner_worker::type_result_fun _tx_callback( void *h_dev, const inner_worker::type_ptr_buffer & ptr_tx )
{
	inner_worker::type_result_fun result_fun =  inner_worker::result_fun_error;
	unsigned long n_result = 0;
	unsigned long n_offset = 0;
	unsigned long n_packet = 0;
	unsigned char n_out_report = 64;
	unsigned char s_out_report[64+1] = {0,};
	int n_written = 0;
	const unsigned char *s_tx = NULL;
	unsigned long n_tx = 0;

	do{
		if( ptr_tx == nullptr )
			continue;
		if( ptr_tx->size() == 0 )
			continue;
		n_tx = ptr_tx->size();
		s_tx = &((*ptr_tx)[0]);
		//
		if( h_dev == 0 )
			continue;
		if( n_tx == 0 )
			continue;
		if( n_tx > (MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD+3) )
			continue;
		if( s_tx == NULL )
			continue;
		//
		do{
			memset( s_out_report, 0, sizeof(s_out_report) );

			if( (n_tx - n_offset) > n_out_report )
				n_packet = n_out_report;
			else
				n_packet = n_tx - n_offset;

			memcpy( &s_out_report[1], &s_tx[n_offset],n_packet );
			//
			n_written = hid_write( (hid_device*)h_dev,s_out_report, n_out_report+1 );
			if( n_written < 0 ){
				n_result = 0;
				break;//error
			}
			if( (n_tx - n_offset) > n_out_report )
				n_offset += n_out_report;
			else
				n_offset += (n_tx - n_offset);
			//
			n_result = n_offset;

		}while( n_tx > n_offset);

		if( n_result > 0 )
			result_fun =  inner_worker::result_fun_success;
	}while(0);

	return result_fun;
}

inner_worker::type_result_fun _rx_callback( void *h_dev, inner_worker::type_ptr_buffer & ptr_rx )
{
	inner_worker::type_result_fun result_fun =  inner_worker::result_fun_error;
	int n_result = 0;
	int n_read = 0;
	unsigned char n_in_report = 220;
	unsigned char s_in_report[220+1] = {0,};
	unsigned long n_offset = 0;

	do{
		n_result = hid_set_nonblocking((hid_device *)h_dev, 1 );
		if( n_result != 0 )
			continue;
		//
		n_read = hid_read((hid_device*)h_dev, s_in_report, n_in_report);

		if( n_read < 0 )
			continue;
		if( n_read == 0 ){
			result_fun =  inner_worker::result_fun_ing;
			continue;
		}
		//resize rx buffer & copy rx data.
		if( ptr_rx == nullptr ){
			ptr_rx = inner_worker::type_ptr_buffer( new inner_worker::type_buffer(n_read));
		}
		else{
			n_offset = ptr_rx->size();
			ptr_rx->resize(n_offset+n_read);
		}
		memcpy( &((*ptr_rx)[n_offset]), s_in_report, n_read );

		if( ptr_rx->size() < MSR_SIZE_HOST_PACKET_HEADER ){
			result_fun =  inner_worker::result_fun_ing;
			continue;
		}

		// check packet
		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)&((*ptr_rx)[0]);
		if( (size_t)(p_packet->c_len + MSR_SIZE_HOST_PACKET_HEADER) > ptr_rx->size() ){
			result_fun =  inner_worker::result_fun_ing;
			continue;
		}

		result_fun =  inner_worker::result_fun_success;
	}while(0);

	return result_fun;
}

//return result key
int _cmd_tx_rx(
		hid_device *h_dev
		, type_msr_cmd cmd
		, unsigned char c_sub
		, unsigned long n_data
		, const unsigned char *s_data
		, unsigned long & n_rx	//in - buffer size, out read-done size
		, unsigned char *s_rx
		, bool b_wait_response
		, LPU237_type_callback cb
		, void *p_parameter_for_cb
		)
{
	int n_result_key = -1;
	unsigned long n_tx = MSR_SIZE_HOST_PACKET_HEADER;
	bool b_need_response = false;

	do{
		if( n_data > MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD )
			continue;
		if( n_data > 0 )
			n_tx +=  n_data;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( n_rx > 0 && s_rx != NULL )
			b_need_response = true;
		//
		inner_worker::type_buffer v_tx( n_tx );

		type_pmsr_host_packet p_tx = (type_pmsr_host_packet)&(v_tx[0]);
		p_tx->c_cmd = (unsigned char)cmd;
		p_tx->c_sub = c_sub;
		p_tx->c_len = (unsigned char)n_data;
		if( n_data > 0 )
			memcpy( p_tx->s_data, s_data, n_data );
		//
		if( b_wait_response ){
			n_result_key = inner_worker::get_instance().push_job(
					h_dev
					, v_tx
					, cb
					, p_parameter_for_cb
					, b_need_response
					);
			//
			if( n_result_key < 0 ){
				continue;//error
			}

			inner_event *p_evet = inner_worker::get_instance().get_result_event(n_result_key);
			if( p_evet )
				p_evet->wait( -1 );

			inner_worker::type_job_result result;
			if( !inner_worker::get_instance().get_result_and_delete( n_result_key, result ) ){
				n_result_key = -1;//error
				continue;
			}
			if( result.result_fun != inner_worker::result_fun_success ){
				n_result_key = -1;//error
				continue;
			}

			if( result.ptr_rx == nullptr )
				continue;
			if( result.ptr_rx->size() == 0 )
				continue;
			if( n_rx == 0 || s_rx == NULL )
				continue;
			if( n_rx > result.ptr_rx->size())
				n_rx = result.ptr_rx->size();
			memcpy( s_rx, &((*result.ptr_rx)[0]), n_rx );
			continue;
		}

		// no wait part.
		n_result_key = inner_worker::get_instance().push_job(
				h_dev
				, v_tx
				, cb
				, p_parameter_for_cb
				, b_need_response
				);
		//
		if( n_result_key < 0 )
			continue;//error
	}while(0);

	return n_result_key;
}

unsigned long _single_cmd( LPU237_HANDLE h_dev,type_msr_cmd cmd )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		unsigned long n_rx = 220;
		unsigned char s_rx[220+1] = { 0, };

		int result_index = _cmd_tx_rx(
				(hid_device *)h_dev
				, cmd
				, 0
				, 0	, NULL
				, n_rx , s_rx
				, true//wait
				, NULL
				, NULL
				);
		if( result_index < 0 )
			continue;

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;
		if( p_packet->c_rsp != 'R' )
			continue;
		if( p_packet->c_sub == msr_rsp_good ){
			dw_result = LPU237_DLL_RESULT_SUCCESS;
			continue;
		}
	}while(0);

	return dw_result;

}

unsigned long _enter_opos( LPU237_HANDLE h_dev )
{
	return _single_cmd( h_dev, msr_cmd_enter_opos );
}
unsigned long _leave_opos( LPU237_HANDLE h_dev )
{
	return _single_cmd( h_dev, msr_cmd_leave_opos );
}

/////////////////////////////////////////////////////
//exported function body.

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	string s_lib("libhidapi.so");

	inner_log::get_instance().log_info("+LPU237_dll_on");
	do{
		if( inner_worker::get_instance(_tx_callback,_rx_callback).is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().start_worker() )
			continue;
		if( hid_init() != 0 )
			continue;
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);
	inner_log::get_instance().log_info("-LPU237_dll_on");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	inner_log::get_instance().log_info("+LPU237_dll_off");
	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		hid_exit();
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);
	inner_log::get_instance().log_info("-LPU237_dll_off");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_w(wchar_t *ss_dev_path)
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	struct hid_device_info *devs = NULL, *cur_dev=NULL;
	unsigned long n_dev = 0;
	unsigned long n_size_byte = 0;
	wchar_t sw_path[256] = { 0, };
	size_t n_offset = 0;
	size_t n_size = 0;

	inner_log::get_instance().log_info("+LPU237_get_list_w");
	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		devs = hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
		if( devs == NULL ){
			dw_result = 0;
			continue;//no device
		}

		cur_dev = devs;
		while(cur_dev) {
			if( cur_dev->interface_number != _USB_INF_LPU237 ){
				cur_dev = cur_dev->next;
				continue;
			}
			n_dev++;
			//
			//wcslen(cur_dev->path)
			n_size_byte = (strlen(cur_dev->path)+1) * sizeof(wchar_t);

			//convert char* -> wchar_t*
			memset( sw_path, 0, sizeof(sw_path) );
			n_size = mbstowcs(sw_path, cur_dev->path, sizeof(sw_path)/sizeof(wchar_t));
			if( n_size < 0 ){
				break;//exit while
			}
			else{
				wcscpy( &ss_dev_path[n_offset], sw_path );
				n_offset += n_size;
				ss_dev_path[n_offset] = 0;//make zero string.
				n_offset++;

				cur_dev = cur_dev->next;
			}
		}//end while
		hid_free_enumeration(devs);
		if( n_size < 0 ){
			continue;//error exit
		}
		// success
		if( ss_dev_path != NULL )
			dw_result = n_dev;
		else{
			if( n_dev > 0)
				n_size_byte += sizeof(wchar_t);//ad  multi-zero

			dw_result = n_size_byte;//need buffer size.
		}
	}while(0);

	inner_log::get_instance().log_info("-LPU237_get_list_w");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_a(char *ss_dev_path)
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	struct hid_device_info *devs = NULL, *cur_dev=NULL;
	unsigned long n_dev = 0;
	unsigned long n_size_byte = 0;
	size_t n_offset = 0;
	size_t n_size = 0;

	inner_log::get_instance().log_info("+LPU237_get_list_a");
	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		devs = hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
		if( devs == NULL ){
			dw_result = 0;
			continue;//no device
		}

		cur_dev = devs;
		while(cur_dev) {
			if( cur_dev->interface_number != _USB_INF_LPU237 ){
				cur_dev = cur_dev->next;
				continue;
			}
			n_dev++;
			//
			//wcslen(cur_dev->path)
			n_size_byte += ((strlen(cur_dev->path)+1) * sizeof(char));

			if( ss_dev_path ){
				strcpy( &ss_dev_path[n_offset], cur_dev->path );
				n_offset += strlen(cur_dev->path);
				ss_dev_path[n_offset] = 0;//make zero string.
				n_offset++;
			}
			cur_dev = cur_dev->next;
		}//end while
		hid_free_enumeration(devs);
		if( n_size < 0 ){
			continue;//error exit
		}
		// success
		if( ss_dev_path != NULL )
			dw_result = n_dev;
		else{
			if( n_dev > 0)
				n_size_byte += sizeof(char);//ad  multi-zero

			dw_result = n_size_byte;//need buffer size.
		}
	}while(0);

	inner_log::get_instance().log_info("-LPU237_get_list_a");

	return dw_result;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_w( const wchar_t *s_dev_path )
{
	LPU237_HANDLE h_dev = NULL;
	size_t n_size = 0;
	char s_path[256] = {0,};
	hid_device *p_hid = NULL;

	inner_log::get_instance().log_info("+LPU237_open_w");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( s_dev_path == NULL )
			continue;
		//convert wchar_t -> char
		n_size = wcstombs (s_path, s_dev_path, sizeof(s_path));
		if( n_size <= 0)
			continue;

		p_hid = hid_open_path( s_path );
		if( p_hid == NULL )
			continue;

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);
	inner_log::get_instance().log_info("-LPU237_open_w");

	return h_dev;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_a( const char *s_dev_path )
{
	//hid_device * HID_API_EXPORT hid_open_path(const char *path)
	LPU237_HANDLE h_dev = NULL;
	hid_device *p_hid = NULL;

	inner_log::get_instance().log_info("+LPU237_open_a");
	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( s_dev_path == NULL )
			continue;
		//
		p_hid = hid_open_path( s_dev_path );
		if( p_hid == NULL )
			continue;

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_open_a");

	return h_dev;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_close( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_close");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL )
			continue;
		//
		hid_close( (hid_device*)h_dev );

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_close");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_get_id");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		if( s_id == NULL ){
			dw_result = _SIZE_UID;
			continue;
		}
		unsigned long n_rx = 220;
		unsigned char s_rx[220+1] = { 0, };

		int result_index = _cmd_tx_rx(
				(hid_device *)h_dev
				, msr_cmd_read_uid
				, 0
				, 0	, NULL
				, n_rx , s_rx
				, true//wait
				, NULL
				, NULL
				);
		if( result_index < 0 )
			continue;

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;
		if( p_packet->c_rsp != 'R' )
			continue;
		if( p_packet->c_sub != msr_rsp_good )
			continue;
		if( p_packet->c_len > 0 && s_id != NULL ){
			memcpy( s_id,p_packet->s_data, p_packet->c_len );
		}

		// success
		dw_result = _SIZE_UID;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_get_id");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_enable");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		dw_result = _enter_opos( h_dev );
	}while(0);

	inner_log::get_instance().log_info("-LPU237_enable");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_disable");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		dw_result = _leave_opos( h_dev );
	}while(0);

	inner_log::get_instance().log_info("-LPU237_disable");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait_swipe( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_cancel_wait_swipe");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		inner_worker::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void *)h_dev
				, v_tx
				, NULL
				, NULL
				, false
				);
		//
		if( n_result_key < 0 ){
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key );
		if( p_event )
			p_event->wait(-1);

		inner_worker::type_job_result result;
		if( !inner_worker::get_instance().get_result_and_delete( n_result_key, result ) ){
			n_result_key = -1;//error
			continue;
		}
		if( result.result_fun != inner_worker::result_fun_success ){
			n_result_key = -1;//error
			continue;
		}
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_cancel_wait_swipe");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_wait_swipe_with_waits");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		inner_worker::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void*)h_dev
				, v_tx
				, NULL
				, NULL
				, true
				);
		//
		if( n_result_key < 0 ){
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key);
		if( p_event ){
			p_event->wait(-1);
		}

		// success
		dw_result = (unsigned long)n_result_key;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_wait_swipe_with_waits");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	inner_log::get_instance().log_info("+LPU237_wait_swipe_with_callback");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL  )
			continue;
		//
		// no wait part.
		inner_worker::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				h_dev
				, v_tx
				, p_fun, p_parameter
				, true
				);
		//
		if( n_result_key < 0 )
			continue;//error

		// success
		dw_result = (unsigned long)n_result_key;
	}while(0);

	inner_log::get_instance().log_info("-LPU237_wait_swipe_with_callback");

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_data( unsigned long dw_buffer_index, unsigned long dw_iso_track, unsigned char *s_track_data )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	static inner_worker::type_job_result result = { inner_worker::result_fun_success, nullptr, nullptr };
	static int n_cur_result_index = -1;
	int n_new_result_index = (int)dw_buffer_index;
	unsigned char *ps_data = 0;
	int n_offset = 3;
	int i = 0;

	inner_log::get_instance().log_info("+LPU237_get_data");

	do{
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( dw_iso_track == 0 || dw_iso_track > 3 )
			continue;
		//
		if( n_new_result_index != n_cur_result_index ){
			// get new data.
			if( !inner_worker::get_instance().get_result_and_delete(n_new_result_index,result ) )
				continue;
			//
			n_cur_result_index = n_new_result_index;
			//
			result.ptr_event_notify.reset();
		}

		switch( result.result_fun ){
		case inner_worker::result_fun_cancel:
			dw_result = LPU237_DLL_RESULT_CANCEL;
			continue;
		case inner_worker::result_fun_success:
			dw_result = 0;
			if( result.ptr_rx == nullptr ){
				dw_result = LPU237_DLL_RESULT_ERROR_MSR;
				continue;
			}
			if( result.ptr_rx->size()<220){
				dw_result = LPU237_DLL_RESULT_ERROR_MSR;
				continue;
			}

			ps_data = &((*result.ptr_rx)[0]);
			if( ((signed char)ps_data[dw_iso_track-1] )<0 ){
				dw_result = LPU237_DLL_RESULT_ERROR_MSR;
				continue;
			}

			dw_result = (unsigned long)ps_data[dw_iso_track-1];
			if( s_track_data ){
				for( i = 0; i< (int)(dw_iso_track-1); i++ ){
					if( ((signed char)ps_data[i] ) > 0 ){
						n_offset += (int)ps_data[i];
					}
				}//end for

				//convert to ASCII code.
				for( i=0; i<(int)dw_result; i++ ){
					if( dw_iso_track == 1 ){
						s_track_data[i] = ps_data[n_offset+i] + 0x20;
					}
					else{
						s_track_data[i] = ps_data[n_offset+i] + 0x30;
					}
				}//end for
			}
			break;
		case inner_worker::result_fun_ing:
		case inner_worker::result_fun_error:
		default:
			break;
		}//end switch

	}while(0);

	inner_log::get_instance().log_info("-LPU237_get_data");

	return dw_result;
}

