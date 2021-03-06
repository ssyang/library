/*
 * lpu237-hidapi.c
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#include "lpu237-hidapi.h"
#include "hidapi.h"
#include "msr_com.h"
#include "inner_log.h"
#include "manager_dev.h"

#include <dlfcn.h>
#include <inner_worker.h>
#include <inner_rom.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define	_USB_VID_LPU237	0x134b
#define	_USB_PID_LPU237	0x0206
#define	_USB_INF_LPU237	1
#define	_SIZE_UID	16

const char gs_ibutton_post[] = "this_is_ibutton_data";
const int gn_ibutton_data_size = 8;

//
static shared::type_result_fun _flush_callback( void *h_dev );
static shared::type_result_fun _tx_callback( void *h_dev, const shared::type_ptr_buffer & ptr_tx, inner_worker::type_mode mode );
static shared::type_result_fun _rx_callback( void *h_dev, shared::type_ptr_buffer & ptr_rx, inner_worker::type_mode mode );
static int _cmd_tx_rx(
		hid_device *h_dev
		, type_msr_cmd cmd
		, unsigned char c_sub
		, type_dword n_data
		, const unsigned char *s_data
		, type_dword & n_rx	//in - buffer size, out readdone size
		, unsigned char *s_rx
		, bool b_wait_response
		, LPU237_type_callback cb
		, void *p_parameter_for_cb
		, bool b_pump = true
		);

static type_dword _single_cmd( LPU237_HANDLE h_dev,type_msr_cmd cmd, bool b_pump = true );
static type_dword _enter_opos( LPU237_HANDLE h_dev );
static type_dword _leave_opos( LPU237_HANDLE h_dev );

static const string & _get_return_string( type_dword n_result );
/////////////////////////////////////////////////////
//local function body.
shared::type_result_fun _flush_callback( void *h_dev )
{
	shared::type_result_fun result_fun =  shared::result_fun_error;

	do{
		if( h_dev == 0 )
			continue;
		//if( hid_flush( (hid_device*)h_dev ) != 0 )
		//	continue;
		result_fun = shared::result_fun_success;
	}while(0);

	return result_fun;
}

shared::type_result_fun _tx_callback( void *h_dev, const shared::type_ptr_buffer & ptr_tx,inner_worker::type_mode mode )
{
	shared::type_result_fun result_fun =  shared::result_fun_error;
	type_dword n_result = 0;
	type_dword n_offset = 0;
	type_dword n_packet = 0;
	unsigned char n_out_report = 64;
	unsigned char s_out_report[64+1] = {0,};
	int n_written = 0;
	const unsigned char *s_tx = NULL;
	type_dword n_tx = 0;

	do{
		if( ptr_tx == nullptr ){
			LOG_ERROR("ptr_tx == nullptr");
			continue;
		}
		if( ptr_tx->size() == 0 ){
			LOG_ERROR("ptr_tx->size() == 0");
			continue;
		}
		n_tx = (type_dword)ptr_tx->size();
		s_tx = &((*ptr_tx)[0]);
		//
		if( h_dev == 0 ){
			LOG_ERROR("h_dev == 0");
			continue;
		}
		if( n_tx == 0 ){
			LOG_ERROR("n_tx == 0");
			continue;
		}
		if( n_tx > (MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD+3) ){
			LOG_ERROR("n_tx > (MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD+3)");
			continue;
		}
		if( s_tx == NULL ){
			LOG_ERROR("s_tx == NULL");
			continue;
		}
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
				LOG_ERROR("hid_write()");
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
			result_fun =  shared::result_fun_success;
	}while(0);

	return result_fun;
}

shared::type_result_fun _rx_callback( void *h_dev, shared::type_ptr_buffer & ptr_rx, inner_worker::type_mode mode )
{
	shared::type_result_fun result_fun =  shared::result_fun_error;
	int n_result = 0;
	int n_read = 0;
	unsigned char n_in_report = 220;
	unsigned char s_in_report[220+1] = {0,};
	type_dword n_offset = 0;

	do{
		n_result = hid_set_nonblocking((hid_device *)h_dev, 1 );
		if( n_result != 0 ){
			LOG_ERROR("hid_set_nonblocking()");
			continue;
		}
		//
		n_read = hid_read((hid_device*)h_dev, s_in_report, n_in_report);

		if( n_read < 0 ){
			LOG_ERROR("hid_read() : errno = %d",errno);
			continue;
		}
		if( n_read == 0 ){
			result_fun =  shared::result_fun_ing;
			continue;
		}
		//resize rx buffer & copy rx data.
		if( ptr_rx == nullptr ){
			ptr_rx = shared::type_ptr_buffer( new shared::type_buffer(n_read));
		}
		else{
			n_offset = ptr_rx->size();
			ptr_rx->resize(n_offset+n_read);
		}
		memcpy( &((*ptr_rx)[n_offset]), s_in_report, n_read );

		if( mode == inner_worker::mode_msr ||
				mode == inner_worker::mode_key ||
				mode == inner_worker::mode_msr_or_key
				){
			if( n_in_report > ptr_rx->size() ){
				result_fun =  shared::result_fun_ing;
				LOG_INFO("ptr_rx->size() : %d",ptr_rx->size());
				continue;
			}
		}
		else if( mode == inner_worker::mode_system ){
			if( ptr_rx->size() < MSR_SIZE_HOST_PACKET_HEADER ){
				LOG_INFO("ptr_rx->size() < MSR_SIZE_HOST_PACKET_HEADER");
				result_fun =  shared::result_fun_ing;
				continue;
			}

			type_pmsr_host_packet p_packet = (type_pmsr_host_packet)&((*ptr_rx)[0]);
			if( (size_t)(p_packet->c_len+MSR_SIZE_HOST_PACKET_HEADER) > ptr_rx->size() ){
				result_fun =  shared::result_fun_ing;
				LOG_INFO("ptr_rx->size() : %d",ptr_rx->size());
				continue;
			}
		}
		else{
			//unknown mode
			LOG_ERROR("unknown mode");
			continue;
		}

		result_fun =  shared::result_fun_success;
	}while(0);

	do{
		if( result_fun != shared::result_fun_success )
			continue;
		if( ptr_rx == nullptr )
			continue;
		if( ptr_rx->size() < MSR_SIZE_HOST_PACKET_HEADER )
			continue;
		//
		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)&((*ptr_rx)[0]);
		unsigned char *ps_data = &((*ptr_rx)[0]);
		vector<char> v_ibutton_post(sizeof(gs_ibutton_post)+1,0);

		switch( mode ){
		case inner_worker::mode_system:
			if(p_packet->c_rsp != 'R' ){
				ptr_rx->resize(0);//reset buffer
				LOG_ERROR("mismatching mode");
				result_fun = shared::result_fun_ing;
			}
			break;
		case inner_worker::mode_key:
			if(ps_data[0] != 0 || ps_data[1] != 0 || ps_data[2] != 0 ){
				ptr_rx->resize(0);//reset buffer
				LOG_ERROR("mismatching mode");
				result_fun = shared::result_fun_ing;
				break;
			}

			memcpy( &v_ibutton_post[0], &ps_data[3+8], sizeof(gs_ibutton_post) );
			if( strcmp(&v_ibutton_post[0],gs_ibutton_post )!=0 ){
				ptr_rx->resize(0);//reset buffer
				LOG_ERROR("mismatching mode");
				result_fun = shared::result_fun_ing;
			}
			break;
		case inner_worker::mode_msr:
			if(ps_data[0] != 0 || ps_data[1] != 0 || ps_data[2] != 0 )
				break;
			memcpy( &v_ibutton_post[0], &ps_data[3+8], sizeof(gs_ibutton_post) );
			if( strcmp(&v_ibutton_post[0],gs_ibutton_post )==0 ){
				ptr_rx->resize(0);//reset buffer
				LOG_ERROR("mismatching mode");
				result_fun = shared::result_fun_ing;
			}
			break;
		case inner_worker::mode_msr_or_key:
			break;
		default:
			result_fun = shared::result_fun_error;
			LOG_ERROR("unknown mode");
			break;
		}//end switch


	}while(0);

	return result_fun;
}

//return result key
int _cmd_tx_rx(
		hid_device *h_dev
		, type_msr_cmd cmd
		, unsigned char c_sub
		, type_dword n_data
		, const unsigned char *s_data
		, type_dword & n_rx	//in - buffer size, out read-done size
		, unsigned char *s_rx
		, bool b_wait_response
		, LPU237_type_callback cb
		, void *p_parameter_for_cb
		, bool b_pump /*= true*/
		)
{
	int n_result_key = -1;
	type_dword n_tx = MSR_SIZE_HOST_PACKET_HEADER;
	bool b_need_response = false;

	do{
		if( n_data > MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD ){
			LOG_ERROR("n_data > MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD");
			continue;
		}
		if( n_data > 0 )
			n_tx +=  n_data;
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( n_rx > 0 && s_rx != NULL )
			b_need_response = true;
		//
		shared::type_buffer v_tx( n_tx );

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
					, b_pump
					, inner_worker::mode_system
					);
			//
			if( n_result_key < 0 ){
				LOG_ERROR("push_job");
				continue;//error
			}

			inner_event *p_evet = inner_worker::get_instance().get_result_event(n_result_key);
			if( p_evet )
				p_evet->wait( -1 );

			inner_worker::type_job_result result;
			if( !inner_worker::get_instance().get_result_and_delete( n_result_key, result ) ){
				n_result_key = -1;//error
				LOG_ERROR("get_result_and_delete");
				continue;
			}
			if( result.result_fun != shared::result_fun_success ){
				n_result_key = -1;//error
				LOG_ERROR("result_fun = %d",result.result_fun );
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
				, inner_worker::mode_msr_or_key
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("push_job()");
			continue;//error
		}
	}while(0);

	return n_result_key;
}

type_dword _single_cmd( LPU237_HANDLE h_dev,type_msr_cmd cmd, bool b_pump /*= true*/ )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		type_dword n_rx = 220;
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
				, b_pump
				);
		if( result_index < 0 ){
			LOG_ERROR("_cmd_tx_rx()");
			continue;
		}

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;
		if( p_packet->c_rsp != 'R' ){
			LOG_ERROR("c_rsp = 0x%x",p_packet->c_rsp );
			continue;
		}
		if( p_packet->c_sub != msr_rsp_good ){
			LOG_ERROR("c_sub = 0x%x",p_packet->c_sub );
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;

}

type_dword _enter_opos( LPU237_HANDLE h_dev )
{
	return _single_cmd( h_dev, msr_cmd_enter_opos, true );
}
type_dword _leave_opos( LPU237_HANDLE h_dev )
{
	return _single_cmd( h_dev, msr_cmd_leave_opos, false );
}

const string & _get_return_string( type_dword n_result )
{
	static string s_result("");

	switch( n_result ){
	case LPU237_DLL_RESULT_SUCCESS:
		s_result = "LPU237_DLL_RESULT_SUCCESS";
		break;
	case LPU237_DLL_RESULT_ERROR:
		s_result = "LPU237_DLL_RESULT_ERROR";
		break;
	case LPU237_DLL_RESULT_CANCEL:
		s_result = "LPU237_DLL_RESULT_CANCEL";
		break;
	case LPU237_DLL_RESULT_ERROR_MSR:
		s_result = "LPU237_DLL_RESULT_ERROR_MSR";
		break;
	case LPU237_DLL_RESULT_ICC_INSERTED:
		s_result = "LPU237_DLL_RESULT_ICC_INSERTED";
		break;
	case LPU237_DLL_RESULT_ICC_REMOVED:
		s_result = "LPU237_DLL_RESULT_ICC_REMOVED";
		break;
	default:
		s_result = to_string(n_result);
		break;
	}//end switch

	return s_result;
}

/////////////////////////////////////////////////////
//exported function body.

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on()
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+");
	do{
		if( inner_worker::get_instance(_tx_callback,_rx_callback, _flush_callback).is_setup_ok() ){
			LOG_WARNING("is_setup_ok()");
			continue;
		}
		if( !inner_worker::get_instance().start_worker() ){
			LOG_ERROR("start_worker()");
			continue;
		}
		if( hid_init() != 0 ){
			LOG_ERROR("hid_init()");
			continue;
		}
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);
	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off()
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+");
	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( hid_exit() != 0 ){
			LOG_ERROR("hid_exit()");
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);
	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_w(wchar_t *ss_dev_path)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;
	struct hid_device_info *devs = NULL, *cur_dev=NULL;
	type_dword n_dev = 0;
	type_dword n_size_byte = 0;
	wchar_t sw_path[256] = { 0, };
	size_t n_offset = 0;
	size_t n_size = 0;

	LOG_INFO("+ : 0x%X",ss_dev_path);
	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		//
		devs = hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
		if( devs == NULL ){
			dw_result = 0;
			LOG_INFO("hid_enumerate");
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
			n_size_byte += (strlen(cur_dev->path)+1) * sizeof(wchar_t);

			//convert char* -> wchar_t*
			memset( sw_path, 0, sizeof(sw_path) );
			n_size = mbstowcs(sw_path, cur_dev->path, sizeof(sw_path)/sizeof(wchar_t));
			if( n_size < 0 ){
				LOG_ERROR("mbstowcs");
				break;//exit while
			}
			else{
				if( ss_dev_path ){
					wcscpy( &ss_dev_path[n_offset], sw_path );
					n_offset += n_size;
					ss_dev_path[n_offset] = 0;//make zero string.
					n_offset++;
				}

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

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_a(char *ss_dev_path)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;
	struct hid_device_info *devs = NULL, *cur_dev=NULL;
	type_dword n_dev = 0;
	type_dword n_size_byte = 0;
	size_t n_offset = 0;

	LOG_INFO("+ : 0x%X",ss_dev_path);
	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		//
		devs = hid_enumerate(_USB_VID_LPU237, _USB_PID_LPU237);
		if( devs == NULL ){
			dw_result = 0;
			LOG_INFO("hid_enumerate");
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

		// success
		if( ss_dev_path != NULL )
			dw_result = n_dev;
		else{
			if( n_dev > 0)
				n_size_byte += sizeof(char);//ad  multi-zero

			dw_result = n_size_byte;//need buffer size.
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_w( const wchar_t *s_dev_path )
{
	LPU237_HANDLE h_dev = NULL;
	size_t n_size = 0;
	char s_path[256] = {0,};
	hid_device *p_hid = NULL;

	if( s_dev_path )	LOG_INFO("+ : %s",s_dev_path);
	else LOG_INFO("+ : NULL");

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( s_dev_path == NULL ){
			LOG_ERROR("s_dev_path == NULL");
			continue;
		}
		//convert wchar_t -> char
		n_size = wcstombs (s_path, s_dev_path, sizeof(s_path));
		if( n_size <= 0){
			LOG_ERROR("wcstombs");
			continue;
		}

		p_hid = hid_open_path( s_path );
		if( p_hid == NULL ){
			LOG_INFO("hid_open_path");
			continue;
		}

		if( manager_dev::get_instance().add_device(p_hid) == NULL ){
			hid_close( p_hid );
			LOG_INFO("add_device");
			continue;
		}

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);
	LOG_INFO("- : 0x%X",h_dev );

	return h_dev;
}

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_a( const char *s_dev_path )
{
	//hid_device * HID_API_EXPORT hid_open_path(const char *path)
	LPU237_HANDLE h_dev = NULL;
	hid_device *p_hid = NULL;

	if( s_dev_path )	LOG_INFO("+ : %s",s_dev_path);
	else LOG_INFO("+ : NULL");

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( s_dev_path == NULL ){
			LOG_ERROR("s_dev_path == NULL");
			continue;
		}
		//
		p_hid = hid_open_path( s_dev_path );
		if( p_hid == NULL ){
			LOG_INFO("hid_open_path");
			continue;
		}

		if( manager_dev::get_instance().add_device(p_hid) == NULL ){
			hid_close( p_hid );
			LOG_INFO("add_device");
			continue;
		}

		h_dev = (LPU237_HANDLE)p_hid;
	}while(0);

	LOG_INFO("- : 0x%X",h_dev);

	return h_dev;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_close( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x",h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		manager_dev::get_instance().remove_device( (hid_device*)h_dev );
		hid_close( (hid_device*)h_dev );
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x, 0x%x", h_dev,s_id );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		if( s_id == NULL ){
			dw_result = _SIZE_UID;
			continue;
		}
		type_dword n_rx = 220;
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
		if( result_index < 0 ){
			LOG_ERROR("result_index < 0");
			continue;
		}

		type_pmsr_host_packet p_packet = (type_pmsr_host_packet)s_rx;

		LOG_DEBUG("p_packet->c_rsp = 0x%X",p_packet->c_rsp);
		LOG_DEBUG("p_packet->c_sub = 0x%X",p_packet->c_sub);
		LOG_DEBUG("p_packet->c_len = 0x%X",p_packet->c_len);

		if( p_packet->c_rsp != 'R' ){
			LOG_ERROR("p_packet->c_rsp != R");
			continue;
		}
		if( p_packet->c_sub != msr_rsp_good ){
			LOG_ERROR("p_packet->c_sub != msr_rsp_good");
			continue;
		}
		if( p_packet->c_len > 0 && s_id != NULL ){
			memcpy( s_id,p_packet->s_data, p_packet->c_len );
		}

		// success
		dw_result = _SIZE_UID;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		dw_result = _enter_opos( h_dev );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		dw_result = _leave_opos( h_dev );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void *)h_dev
				, v_tx
				, NULL
				, NULL
				, false
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key );
		if( p_event )
			p_event->wait(-1);

		inner_worker::type_job_result result;
		if( !inner_worker::get_instance().get_result_and_delete( n_result_key, result ) ){
			n_result_key = -1;//error
			LOG_ERROR("get_result_and_delete()");
			continue;
		}
		if( result.result_fun != shared::result_fun_success ){
			n_result_key = -1;//error
			LOG_ERROR("result_fun(%d) != result_fun_success",(int)result.result_fun );
			continue;
		}
		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void*)h_dev
				, v_tx
				, NULL
				, NULL
				, true
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key);
		if( p_event ){
			p_event->wait(-1);
		}
		else{
			LOG_ERROR("get_result_event()");
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_waits( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void*)h_dev
				, v_tx
				, NULL
				, NULL
				, true
				, true
				, inner_worker::mode_key
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key);
		if( p_event ){
			p_event->wait(-1);
		}
		else{
			LOG_ERROR("get_result_event()");
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_waits( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				(void*)h_dev
				, v_tx
				, NULL
				, NULL
				, true
				, true
				, inner_worker::mode_msr_or_key
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		inner_event *p_event = inner_worker::get_instance().get_result_event(n_result_key);
		if( p_event ){
			p_event->wait(-1);
		}
		else{
			LOG_ERROR("get_result_event()");
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x, 0x%x, 0x%x,", h_dev,p_fun,p_parameter );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		// no wait part.
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				h_dev
				, v_tx
				, p_fun, p_parameter
				, true
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x, 0x%x, 0x%x,", h_dev,p_fun,p_parameter );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		// no wait part.
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				h_dev
				, v_tx
				, p_fun, p_parameter
				, true
				, true
				, inner_worker::mode_key
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x, 0x%x, 0x%x,", h_dev,p_fun,p_parameter );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//
		// no wait part.
		shared::type_buffer v_tx(0);
		int n_result_key = inner_worker::get_instance().push_job(
				h_dev
				, v_tx
				, p_fun, p_parameter
				, true
				, true
				, inner_worker::mode_msr_or_key
				);
		//
		if( n_result_key < 0 ){
			LOG_ERROR("result_index < 0");
			continue;//error
		}

		// success
		dw_result = (type_dword)n_result_key;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );

	return dw_result;
}

/*
 * dw_iso_track = 0 : ibutton data.
 * dw_iso_track = 1 : MSR ISO1 track data
 * dw_iso_track = 2 : MSR ISO2 track data
 * dw_iso_track = 3 : MSR ISO3 track data
 */
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_data( type_dword dw_buffer_index, type_dword dw_iso_track, unsigned char *s_track_data )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;
	static inner_worker::type_job_result result = { shared::result_fun_success, nullptr, nullptr };
	static int n_cur_result_index = -1;
	int n_new_result_index = (int)dw_buffer_index;
	unsigned char *ps_data = NULL;
	type_pmsr_host_packet p_packet = NULL;
	int n_offset = 3;
	int i = 0;

	LOG_INFO("+ : %d, %u, 0x%x", (int)dw_buffer_index, dw_iso_track, s_track_data);

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( dw_iso_track > 3 ){
			LOG_ERROR( "dw_iso_track > 3");
			continue;
		}
		//
		if( n_new_result_index != n_cur_result_index ){
			// get new data.
			if( !inner_worker::get_instance().get_result_and_delete(n_new_result_index,result ) ){
				LOG_ERROR("get_result_and_delete()");
				continue;
			}
			//
			n_cur_result_index = n_new_result_index;
			result.ptr_event_notify.reset();
		}

		switch( result.result_fun ){
		case shared::result_fun_cancel:
			dw_result = LPU237_DLL_RESULT_CANCEL;
			continue;
		case shared::result_fun_success:
			dw_result = 0;
			if( result.ptr_rx == nullptr ){
				LOG_ERROR("result.ptr_rx == nullptr");
				continue;
			}
			if( result.ptr_rx->size()<220){
				LOG_ERROR("result.ptr_rx->size()<220");
				continue;
			}
			p_packet = (type_pmsr_host_packet)&((*result.ptr_rx)[0]);
			ps_data = &((*result.ptr_rx)[0]);

			if( ps_data[0]==0 && ps_data[1]==0 && ps_data[2]==0 ){
				if( dw_iso_track != 0 ){
					break;//NO msr data.
				}
				vector<char> v_ibutton_post(sizeof(gs_ibutton_post)+1,0);
				memcpy( &v_ibutton_post[0], &ps_data[3+8], sizeof(gs_ibutton_post) );
				if( strcmp(&v_ibutton_post[0],gs_ibutton_post )==0 ){
					//ibutton data.
					dw_result = (type_dword)gn_ibutton_data_size;
					if( s_track_data )
						memcpy( s_track_data,&ps_data[3],gn_ibutton_data_size );
				}
				else{
					LOG_ERROR("no i-button postfix");
					dw_result = LPU237_DLL_RESULT_ERROR;
				}
				break;
			}

			if( dw_iso_track == 0 ){
				LOG_ERROR("no i-button data.");
				dw_result = LPU237_DLL_RESULT_ERROR;
				break;
			}
			if( p_packet->c_rsp == 'R'){
				dw_result = LPU237_DLL_RESULT_ERROR;
				LOG_ERROR("mismatching mode");
				break;
			}

			// case MSR data
			if( ((signed char)ps_data[dw_iso_track-1] )<0 ){
				LOG_ERROR("track %u : error code = 0x%x",dw_iso_track,ps_data[dw_iso_track-1]);
				dw_result = LPU237_DLL_RESULT_ERROR_MSR;
				continue;
			}

			dw_result = (type_dword)ps_data[dw_iso_track-1];
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
		case shared::result_fun_ing:
			LOG_ERROR("result_fun == result_fun_ing");
			break;
		case shared::result_fun_error:
			LOG_ERROR("result_fun == result_fun_error");
			break;
		default:
			LOG_ERROR("result_fun == unknown");
			break;
		}//end switch

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str());

	return dw_result;
}

/*********************************************************************************************
 * system interface.
 */
/*!
* function
*	save the current lpu237 device setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*
* return
*	if success, return LPU237_DLL_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_save_setting( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( (hid_device*)h_dev );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_system_parameters( (hid_device*)h_dev ) ){
			LOG_ERROR("df_get_system_parameters()");
			continue;
		}

		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	resetting  lpu237 device with saved setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*
* return
*	if success, return LPU237_DLLW_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_recover_setting( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( (hid_device*)h_dev );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_set_system_parameters( (hid_device*)h_dev ) ){
			LOG_ERROR("df_set_system_parameters()");
			continue;
		}

		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	get device internal name.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*	sName : [in/out] A pointer to the buffer that save the device name.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else the size of internal name.[unit byte]
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_get_name(
		LPU237_HANDLE h_dev
		, unsigned char *s_name
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( (hid_device*)h_dev );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		dev_lpu237::type_name v_name = p_dev->get_name();
		if( v_name.empty() ){
			if( !p_dev->df_get_name( (hid_device*)h_dev ) ){
				LOG_ERROR("df_get_name()");
				continue;
			}
			v_name = p_dev->get_name();
		}

		if( v_name.empty() ){
			LOG_ERROR("empty()");
			continue;
		}

		dw_result = (type_dword)v_name.size();
		if( s_name ){
			memcpy( s_name, &v_name[0], v_name.size() );
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}


/*!
* function
*	get device firmware version.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*	sName : [in/out] A pointer to the buffer that save the device firmware version.( version 4 bytes )
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else the size of version.[unit byte]
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version( LPU237_HANDLE h_dev, unsigned char *s_version )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( (hid_device*)h_dev );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		inner_version ver = p_dev->get_version();
		if( ver == inner_version() ){
			if( !p_dev->df_get_version_and_system_type((hid_device*)h_dev) ){
				LOG_ERROR("df_get_version_and_system_type()");
				continue;
			}

			ver = p_dev->get_version();
		}

		if( ver == inner_version() ){
			LOG_ERROR("ver == inner_version()");
			continue;
		}

		dw_result = inner_version::const_size_of_type_version;
		if( s_version ){
			s_version[0] = ver.get_major();
			s_version[1] = ver.get_minor();
			s_version[2] = ver.get_fix();
			s_version[3] = ver.get_build();
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	get major number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_sys_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else major version number.
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version_major( const unsigned char *s_version )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_version == NULL )
			continue;
		dw_result = (type_dword)s_version[0];
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	get minor number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_sys_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else minor version number.
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version_minor( const unsigned char *s_version )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_version == NULL )
			continue;
		dw_result = (type_dword)s_version[1];
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	stop operation of LPU237_sys_update_x.
*
* parameters
*
* return
*	if success, return LPU237_DLL_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_cancel_update( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}
		//TODO.
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}


/*!
* function
*	start firmware update.( unicode version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cbUpdate : [in] callback function for serving updating information.
*	pUser : [in] user data pointer for calling cbUpdate().
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR.
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_update_callback_w
		(
		const unsigned char *s_id
		, type_lpu237_update_callback cb_update
		, void *p_user
		, const wchar_t *s_rom_file_name
		, type_dword dw_index
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}

		size_t n_size = 0;
		char s_rom[256] = {0,};

		//convert wchar_t -> char
		n_size = wcstombs (s_rom, s_rom_file_name, sizeof(s_rom));
		if( n_size <= 0){
			LOG_ERROR("wcstombs");
			continue;
		}

		dw_result = LPU237_sys_update_callback_a( s_id, cb_update, p_user, s_rom, dw_index );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	start firmware update..( Multi Byte Code Set version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cbUpdate : [in] callback function for serving updating information.
*	pUser : [in] user data pointer for calling cbUpdate().
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_update_callback_a
		(
		const unsigned char *s_id
		, type_lpu237_update_callback cb_update
		, void *p_user
		, const char *s_rom_file_name
		, type_dword dw_index
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_id == NULL ){
			LOG_ERROR("s_id == NULL");
			continue;
		}
		if( cb_update == NULL ){
			LOG_ERROR("cb_update == NULL");
			continue;
		}
		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}
		//TODO.

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	load rom file.( unicode version )
*
* parameters
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_load_w( const wchar_t *s_rom_file_name )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}

		size_t n_size = 0;
		char s_rom[256] = {0,};

		//convert wchar_t -> char
		n_size = wcstombs (s_rom, s_rom_file_name, sizeof(s_rom));
		if( n_size <= 0){
			LOG_ERROR("wcstombs");
			continue;
		}

		dw_result = LPU237_sys_rom_load_a( s_rom );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	load rom file.( Multi Byte Code Set version )
*
* parameters
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*
* return
* 	if error, return LPU237_FW_RESULT_ERROR
*	else LPU237_FW_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_load_a( const char *s_rom_file_name )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		inner_rom::type_result result_rom(inner_rom::result_error);
		inner_rom::ROMFILE_HEAD header;
		string s_rom;

		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}

		s_rom = s_rom_file_name;
		result_rom = inner_rom::get_instance().load_header( s_rom, &header );
		if( result_rom != inner_rom::result_success ){
			LOG_ERROR("load_header()");
			continue;
		}

		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	get firmware index value( greater then equal 0 ) in rom file.( unicode version ).
*
* parameters
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else firmware index value( greater then equal 0 )
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_get_index_w(
		const wchar_t *s_rom_file_name
		, const unsigned char *s_name
		, const unsigned char *s_version
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}
		size_t n_size = 0;
		char s_rom[256] = {0,};

		//convert wchar_t -> char
		n_size = wcstombs (s_rom, s_rom_file_name, sizeof(s_rom));
		if( n_size <= 0){
			LOG_ERROR("wcstombs");
			continue;
		}

		dw_result = LPU237_sys_rom_get_index_a( s_rom, s_name, s_version );

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*!
* function
*	get firmware index value( greater then equal 0 ) in rom file.( multi-byte-code-set version ).
*
* parameters
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
*
* return
* 	if error, return LPU237_FW_RESULT_ERROR
*	else firmware index value( greater then equal 0 )
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_get_index_a(
		const char *s_rom_file_name
		, const unsigned char *s_name
		, const unsigned char *s_version
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+" );

	do{
		if( s_name == NULL ){
			LOG_ERROR("s_name == NULL");
			continue;
		}
		if( s_version == NULL ){
			LOG_ERROR("s_version == NULL");
			continue;
		}
		if( s_rom_file_name == NULL ){
			LOG_ERROR("s_rom_file_name == NULL");
			continue;
		}
		//
		inner_rom::type_result result_rom(inner_rom::result_error);
		inner_rom::ROMFILE_HEAD header;
		string s_rom;

		s_rom = s_rom_file_name;
		result_rom = inner_rom::get_instance().load_header( s_rom, &header );
		if( result_rom != inner_rom::result_success ){
			LOG_ERROR("load_header()");
			continue;
		}

		int n_index = inner_rom::get_instance().get_updateble_item_index(
				&header
				,s_name
				,s_version[0]
				,s_version[1]
				,s_version[2]
				,s_version[3]);
		if( n_index < 0){
			LOG_ERROR("get_updateble_item_index()");
		}

		dw_result = (type_dword)n_index;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

/*
 * ***********************************************************************************
 * system functions .......
 * ***********************************************************************************
 */
type_dword LPU237_sys_enter_config( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		if( !p_dev->df_enter_config(h_hid) ){
			LOG_ERROR("df_enter_config()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_leave_config( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_leave_config(h_hid) ){
			LOG_ERROR("df_leave_config()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}
type_dword LPU237_sys_apply_config( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_apply_config(h_hid) ){
			LOG_ERROR("df_apply_config()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_goto_boot( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_goto_bootloader(h_hid) ){
			LOG_ERROR("df_apply_config()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_bypass_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tx
		, type_dword dw_tx
		, unsigned char *ps_rx
		, type_dword dw_rx
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		shared::type_buffer v_tx(dw_tx, 0 );
		memcpy( &v_tx[0], ps_tx, dw_tx );

		shared::type_buffer v_rx(0);
		if( !p_dev->df_bypass_uart(h_hid,v_tx, v_rx ) ){
			LOG_ERROR("df_bypass_uart()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}
type_dword LPU237_sys_enter_opos( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_enter_opos(h_hid)){
			LOG_ERROR("df_enter_opos()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_leave_opos( LPU237_HANDLE h_dev )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_leave_opos(h_hid)){
			LOG_ERROR("df_enter_opos()");
			continue;
		}

		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_interface( LPU237_HANDLE h_dev, type_dword dw_interface  )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->set_interface((dev_lpu237::type_interface)dw_interface).df_set_interface(h_hid) ){
			LOG_ERROR("df_set_interface()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_language( LPU237_HANDLE h_dev, type_dword dw_language )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->set_language((dev_lpu237::type_language_map_index)dw_language).df_set_language(h_hid) ){
			LOG_ERROR("df_set_language()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_buzzer( LPU237_HANDLE h_dev, type_dword dw_frequency )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->set_buzzer_frequency((int)dw_frequency).df_set_buzzer(h_hid ) ){
			LOG_ERROR("df_set_buzzer()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_track_status(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, type_dword b_enable
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		bool enable(false);
		if( b_enable )
			enable = true;
		//
		if( !p_dev->set_enable_track( (dev_lpu237::type_msr_track_number)dw_track,enable).df_set_enable_track(h_hid) ){
			LOG_ERROR("df_set_enable_track()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_global_prefix(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_global_prefix( v_tag ).df_set_global_prefix(h_hid ) ){
			LOG_ERROR("df_set_global_prefix()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_global_postfix(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_global_postfix( v_tag ).df_set_global_postfix(h_hid ) ){
			LOG_ERROR("df_set_global_postfix()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_private_prefix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_private_prefix( (dev_lpu237::type_msr_track_number)dw_track, v_tag ).df_set_private_prefix(h_hid ) ){
			LOG_ERROR("df_set_private_prefix()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_private_postfix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_private_postfix( (dev_lpu237::type_msr_track_number)dw_track, v_tag ).df_set_private_postfix(h_hid ) ){
			LOG_ERROR("df_set_private_postfix()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_prefix_ibutton(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_ibutton_prefix( v_tag ).df_set_prefix_ibutton(h_hid ) ){
			LOG_ERROR("df_set_prefix_ibutton()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_postfix_ibutton(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_ibutton_postfix( v_tag ).df_set_postfix_ibutton(h_hid ) ){
			LOG_ERROR("df_set_postfix_ibutton()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_prefix_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_uart_prefix( v_tag ).df_set_prefix_uart(h_hid ) ){
			LOG_ERROR("df_set_prefix_uart()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_postfix_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		dev_lpu237::type_tag v_tag(dw_tag);
		memcpy( &v_tag[0], ps_tag, dw_tag );

		if( !p_dev->set_uart_postfix( v_tag ).df_set_postfix_uart(h_hid) ){
			LOG_ERROR("df_set_postfix_uart()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_set_global_prepostfix_send_condition(
		LPU237_HANDLE h_dev
		, type_dword b_all_track_good
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		bool is_all_track_good(false);

		if(b_all_track_good )
			is_all_track_good = true;

		if( !p_dev->set_global_prepostfix_send_condition(is_all_track_good).df_set_global_prepostfix_send_condition(h_hid) ){
			LOG_ERROR("df_set_global_prepostfix_send_condition()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_global_prepostfix_send_condition( LPU237_HANDLE h_dev,type_dword *pb_all_track_no_error )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}

		if( !p_dev->df_get_global_prepostfix_send_condition(h_hid) ){
			LOG_ERROR("df_get_global_prepostfix_send_condition()");
			continue;
		}

		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( pb_all_track_no_error == NULL ){
			continue;
		}

		*pb_all_track_no_error = 0;

		if( p_dev->get_global_prepostfix_send_condition() )
			*pb_all_track_no_error = 1;

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_uid( LPU237_HANDLE h_dev, unsigned char  *ps_uid )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_uid(h_hid)){
			LOG_ERROR("df_get_uid()");
			continue;
		}

		dev_lpu237::type_uid uid = p_dev->get_uid();
		if( uid.size() != dev_lpu237::const_size_uid ){
			LOG_ERROR("uid.size() != dev_lpu237::const_size_uid");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( ps_uid == NULL ){
			continue;
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_version_and_system_type(
		LPU237_HANDLE h_dev
		, unsigned char *s_version
		, type_dword *pdw_system_type
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_version_and_system_type(h_hid)){
			LOG_ERROR("df_get_version_and_system_type()");
			continue;
		}
		type_dword type = p_dev->get_system_type();
		inner_version  v = p_dev->get_version();
		dw_result = LPU237_DLL_RESULT_SUCCESS;

		if( s_version ){
			s_version[0] = v.get_major();
			s_version[1] = v.get_minor();
			s_version[2] = v.get_fix();
			s_version[3] = v.get_build();
		}
		if( pdw_system_type ){
			*pdw_system_type = type;
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_structure_version( LPU237_HANDLE h_dev, unsigned char *s_version )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_structure_version(h_hid)){
			LOG_ERROR("df_get_structure_version()");
			continue;
		}
		inner_version v = p_dev->get_structure_version();
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( s_version ){
			s_version[0] = v.get_major();
			s_version[1] = v.get_minor();
			s_version[2] = v.get_fix();
			s_version[3] = v.get_build();
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_interface( LPU237_HANDLE h_dev,type_dword *pdw_interface )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_interface(h_hid)){
			LOG_ERROR("df_get_interface()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		dev_lpu237::type_interface inf = p_dev->get_interface();
		if( pdw_interface ){
			*pdw_interface = (type_dword)inf;
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_language( LPU237_HANDLE h_dev,type_dword *pdw_language )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_language(h_hid)){
			LOG_ERROR("df_get_language()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		dev_lpu237::type_language_map_index language = p_dev->get_language_map_index();
		if( pdw_language ){
			*pdw_language = (type_dword)language;
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_buzzer( LPU237_HANDLE h_dev,type_dword *pdw_buzzer )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_buzzer(h_hid)){
			LOG_ERROR("df_get_buzzer()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( pdw_buzzer ){
			*pdw_buzzer = (type_dword)p_dev->get_buzzer_frequency();
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_msd_run_time( LPU237_HANDLE h_dev,type_dword *pdw_run_time )
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_msd_runtime(h_hid)){
			LOG_ERROR("df_get_msd_runtime()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( pdw_run_time ){
			*pdw_run_time = (type_dword)p_dev->get_msd_bootloader_runtime();
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_track_status(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, type_dword *pb_enable
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_enable_track(h_hid, (dev_lpu237::type_msr_track_number)dw_track)){
			LOG_ERROR("df_get_enable_track()");
			continue;
		}
		dw_result = LPU237_DLL_RESULT_SUCCESS;
		if( pb_enable ){
			*pb_enable = 0;
			if( p_dev->get_enable_track((dev_lpu237::type_msr_track_number)dw_track) )
				*pb_enable = 1;
		}
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_global_prefix(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_global_prefix( h_hid ) ){
			LOG_ERROR("df_get_global_prefix()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_global_prefix();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_global_postfix(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_global_postfix( h_hid ) ){
			LOG_ERROR("df_get_global_postfix()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_global_postfix();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_private_prefix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_private_prefix( h_hid,(dev_lpu237::type_msr_track_number)dw_track ) ){
			LOG_ERROR("df_get_private_prefix()");
			continue;
		}
		dev_lpu237::type_tag v_tag = p_dev->get_private_prefix((dev_lpu237::type_msr_track_number)dw_track);
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_private_postfix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_private_postfix( h_hid,(dev_lpu237::type_msr_track_number)dw_track ) ){
			LOG_ERROR("df_get_private_postfix()");
			continue;
		}
		dev_lpu237::type_tag v_tag = p_dev->get_private_postfix((dev_lpu237::type_msr_track_number)dw_track);
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_prefix_ibutton(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_prefix_ibutton( h_hid ) ){
			LOG_ERROR("df_get_prefix_ibutton()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_prefix_ibutton();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_postfix_ibutton(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_postfix_ibutton( h_hid ) ){
			LOG_ERROR("df_get_postfix_ibutton()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_postfix_ibutton();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );

	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_prefix_uart(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_prefix_uart( h_hid ) ){
			LOG_ERROR("df_get_prefix_uart()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_prefix_uart();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}

type_dword LPU237_sys_get_postfix_uart(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		)
{
	type_dword dw_result = LPU237_DLL_RESULT_ERROR;

	LOG_INFO("+ : 0x%x", h_dev );

	do{
		if( !inner_worker::get_instance().is_setup_ok() ){
			LOG_ERROR("!is_setup_ok()");
			continue;
		}
		if( h_dev == NULL  ){
			LOG_ERROR("h_dev == NULL");
			continue;
		}

		dev_hid::type_handle h_hid = (dev_hid::type_handle)h_dev;
		dev_lpu237 *p_dev = manager_dev::get_instance().get_device( h_hid );
		if( p_dev == NULL ){
			LOG_ERROR("get_device()");
			continue;
		}
		if( !p_dev->df_get_postfix_uart( h_hid ) ){
			LOG_ERROR("df_get_postfix_uart()");
			continue;
		}

		dev_lpu237::type_tag v_tag = p_dev->get_postfix_uart();
		dw_result = v_tag.size();
		if( dw_tag < v_tag.size() || ps_tag == NULL )
			continue;
		//
		memcpy( ps_tag, &v_tag[0],v_tag.size() );
	}while(0);

	LOG_INFO("- : %s",_get_return_string(dw_result).c_str() );
	return dw_result;
}


