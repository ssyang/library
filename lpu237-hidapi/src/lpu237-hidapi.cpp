/*
 * lpu237-hidapi.c
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#include "lpu237-hidapi.h"
#include "el_hidapi.h"
#include "msr_com.h"

#include "inner_dll.h"

#include <dlfcn.h>
#include <inner_worker.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define	_USB_VID_LPU237	0x134b
#define	_USB_PID_LPU237	0x0206
#define	_USB_INF_LPU237	1
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
		);


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
			n_written = inner_dll::get_instance().hid_write( (hid_device*)h_dev,s_out_report, n_packet+1 );
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
		n_result = inner_dll::get_instance().hid_set_nonblocking((hid_device *)h_dev, 1 );
		if( n_result != 0 )
			continue;
		//
		n_read = inner_dll::get_instance().hid_read((hid_device*)h_dev, s_in_report, n_in_report);

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
			inner_event event_wait;
			n_result_key = inner_worker::get_instance().push_job(
					event_wait
					, h_dev
					, &(v_tx[0]), v_tx.size()
					, b_need_response
					);
			//
			if( n_result_key < 0 ){
				continue;//error
			}

			event_wait.wait(-1);//wait result
			inner_worker::type_job_result result;
			if( !inner_worker::get_instance().get_result(n_result_key, result ) ){
				n_result_key = -1;//error
				continue;
			}
			if( result.result_fun != inner_worker::result_fun_success ){
				n_result_key = -1;//error
				continue;
			}
			continue;
		}

		// no wait part.
		neosmart::neosmart_event_t h_evet = neosmart::CreateEvent();
		if( h_evet == NULL )
			continue;
		n_result_key = inner_worker::get_instance().push_job(
				h_evet
				, h_dev
				, &(v_tx[0]), v_tx.size()
				, b_need_response
				);
		//
		if( n_result_key < 0 )
			continue;//error
		if( !inner_event::set_raw_event_to_map(n_result_key,h_evet ) ){
			//todo ???????
			continue;
		}
		//
	}while(0);

	return n_result_key;
}

/////////////////////////////////////////////////////
//exported function body.

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() ){
			if( !inner_dll::get_instance().load( string("libhidapi.so")) )
				continue;
		}
		if( !inner_worker::get_instance(_tx_callback,_rx_callback).is_setup_ok() )
			continue;
		if( inner_dll::get_instance().hid_init() != 0 )
			continue;
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		inner_dll::get_instance().hid_exit();
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

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

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		devs = inner_dll::get_instance().hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
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
		inner_dll::get_instance().hid_free_enumeration(devs);
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

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		devs = inner_dll::get_instance().hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
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
			n_size_byte = (strlen(cur_dev->path)+1) * sizeof(char);

			strcpy( &ss_dev_path[n_offset], cur_dev->path );
			ss_dev_path[n_offset] = 0;//make zero string.
			n_offset++;
			cur_dev = cur_dev->next;
		}//end while
		inner_dll::get_instance().hid_free_enumeration(devs);
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

	return dw_result;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_w( const wchar_t *s_dev_path )
{
	LPU237_HANDLE h_dev = NULL;
	size_t n_size = 0;
	char s_path[256] = {0,};
	hid_device *p_hid = NULL;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( s_dev_path == NULL )
			continue;
		//convert wchar_t -> char
		n_size = wcstombs (s_path, s_dev_path, sizeof(s_path));
		if( n_size <= 0)
			continue;

		p_hid = inner_dll::get_instance().hid_open_path( s_path );
		if( p_hid == NULL )
			continue;

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);

	return h_dev;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_a( const char *s_dev_path )
{
	//hid_device * HID_API_EXPORT hid_open_path(const char *path)
	LPU237_HANDLE h_dev = NULL;
	hid_device *p_hid = NULL;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( s_dev_path == NULL )
			continue;
		//
		p_hid = inner_dll::get_instance().hid_open_path( s_dev_path );
		if( p_hid == NULL )
			continue;

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);

	return h_dev;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_close( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		if( h_dev == NULL )
			continue;
		//
		inner_dll::get_instance().hid_close( (hid_device*)h_dev );

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		unsigned long n_rx = 220;
		unsigned char s_rx[220+1] = { 0, };

		int result_index = _cmd_tx_rx(
				(hid_device *)h_dev
				, msr_cmd_read_uid
				, 0
				, 0	, NULL
				, n_rx , s_rx
				, true//wait
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
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		unsigned long n_rx = 220;
		unsigned char s_rx[220+1] = { 0, };

		int result_index = _cmd_tx_rx(
				(hid_device *)h_dev
				, msr_cmd_leave_cs
				, 0
				, 0	, NULL
				, n_rx , s_rx
				, true//wait
				);
		if( result_index < 0 )
			continue;

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;
		if( p_packet->c_rsp != 'R' )
			continue;
		if( p_packet->c_sub != msr_rsp_good )
			continue;

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		unsigned long n_rx = 220;
		unsigned char s_rx[220+1] = { 0, };

		int result_index = _cmd_tx_rx(
				(hid_device *)h_dev
				, msr_cmd_enter_cs
				, 0
				, 0	, NULL
				, n_rx , s_rx
				, true//wait
				);
		if( result_index < 0 )
			continue;

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;
		if( p_packet->c_rsp != 'R' )
			continue;
		if( p_packet->c_sub != msr_rsp_good )
			continue;
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait_swipe( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		inner_event event_wait;
		int n_result_key = inner_worker::get_instance().push_job(
				event_wait
				, h_dev
				, NULL, 0
				, false
				);
		//
		if( n_result_key < 0 ){
			continue;//error
		}

		event_wait.wait(-1);//wait result
		inner_worker::type_job_result result;
		if( !inner_worker::get_instance().get_result(n_result_key, result ) ){
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

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		inner_event event_wait;
		int n_result_key = inner_worker::get_instance().push_job(
				event_wait
				, h_dev
				, NULL, 0
				, true
				);
		//
		if( n_result_key < 0 ){
			continue;//error
		}

		event_wait.wait(-1);//wait result

		// success
		dw_result = (unsigned long)n_result_key;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;
		//
		// no wait part.
		neosmart::neosmart_event_t h_evet = neosmart::CreateEvent();
		if( h_evet == NULL )
			continue;
		int n_result_key = inner_worker::get_instance().push_job(
				h_evet
				, h_dev
				, NULL, 0
				, true
				);
		//
		if( n_result_key < 0 )
			continue;//error
		if( !inner_event::set_raw_event_to_map(n_result_key,h_evet ) ){
			//todo ???????
			continue;
		}

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_data( unsigned long dw_buffer_index, unsigned long dw_iso_track, unsigned char *s_track_data )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		// TODO.......
		if( !inner_dll::get_instance().is_setup_ok() )
			continue;
		if( !inner_worker::get_instance().is_setup_ok() )
			continue;

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

