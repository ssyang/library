/*
 * lpu237-hidapi.c
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#include "lpu237-hidapi.h"
#include "el_hidapi.h"

#include "msr_com.h"

#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define	_USB_VID_LPU237	0x134b
#define	_USB_PID_LPU237	0x0206
#define	_USB_INF_LPU237	1

#define	_GET_HANDLE	(void*)0xFFFFFFFF
/////////////////////////////////////////////////////
//local function prototype.
static void *_dll_handle( void *p_dll );
static type_hid_init _init( type_hid_init p_fun );
static type_hid_exit _exit_my( type_hid_exit p_fun );
static type_hid_enumerate _enumerate( type_hid_enumerate p_fun );
static type_hid_free_enumeration _free_enumeration( type_hid_free_enumeration p_fun );
static type_hid_open_ex _open_ex( type_hid_open_ex p_fun );
static type_hid_open _open( type_hid_open p_fun );
static type_hid_open_path _open_path( type_hid_open_path p_fun );
static type_hid_write _write( type_hid_write p_fun );
static type_hid_read_timeout _read_timeout( type_hid_read_timeout p_fun );
static type_hid_read _read( type_hid_read p_fun );
static type_hid_set_nonblocking _set_nonblocking( type_hid_set_nonblocking p_fun );
static type_hid_send_feature_report _send_feature_report( type_hid_send_feature_report p_fun );
static type_hid_get_feature_report _get_feature_report( type_hid_get_feature_report p_fun );
static type_hid_close _close( type_hid_close p_fun );
static type_hid_get_manufacturer_string _get_manufacturer_string( type_hid_get_manufacturer_string p_fun );
static type_hid_get_product_string _get_product_string( type_hid_get_product_string p_fun );
static type_hid_get_serial_number_string _get_serial_number_string( type_hid_get_serial_number_string p_fun );
static type_hid_get_indexed_string _get_indexed_string( type_hid_get_indexed_string p_fun );
static type_hid_error _error( type_hid_error p_fun );

//
static unsigned long _tx(  hid_device *h_dev, type_msr_cmd cmd, unsigned char c_sub, unsigned long n_data, const unsigned char *s_data );

/////////////////////////////////////////////////////
//local function body.
void *_dll_handle( void *p_dll /*= 0xFFFFFFFF*/ )
{
	static void *h_dll = NULL;

	if( p_dll != _GET_HANDLE ){
		h_dll = p_dll;
	}

	return h_dll;
}

type_hid_init _init( type_hid_init p_fun /*= NULL*/ )
{
	static type_hid_init p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}

type_hid_exit _exit_my( type_hid_exit p_fun /*= NULL*/ )
{
	static type_hid_exit p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}

type_hid_enumerate _enumerate( type_hid_enumerate p_fun /*= NULL*/ )
{
	static type_hid_enumerate p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_free_enumeration _free_enumeration( type_hid_free_enumeration p_fun /*= NULL*/ )
{
	static type_hid_free_enumeration p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_open_ex _open_ex( type_hid_open_ex p_fun /*= NULL*/ )
{
	static type_hid_open_ex p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_open _open( type_hid_open p_fun /*= NULL*/ )
{
	static type_hid_open p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_open_path _open_path( type_hid_open_path p_fun /*= NULL*/ )
{
	static type_hid_open_path p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_write _write( type_hid_write p_fun /*= NULL*/ )
{
	static type_hid_write p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_read_timeout _read_timeout( type_hid_read_timeout p_fun /*= NULL*/ )
{
	static type_hid_read_timeout p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_read _read( type_hid_read p_fun /*= NULL*/ )
{
	static type_hid_read p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_set_nonblocking _set_nonblocking( type_hid_set_nonblocking p_fun /*= NULL*/ )
{
	static type_hid_set_nonblocking p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_send_feature_report _send_feature_report( type_hid_send_feature_report p_fun /*= NULL*/ )
{
	static type_hid_send_feature_report p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_get_feature_report _get_feature_report( type_hid_get_feature_report p_fun /*= NULL*/ )
{
	static type_hid_get_feature_report p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_close _close( type_hid_close p_fun /*= NULL*/ )
{
	static type_hid_close p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_get_manufacturer_string _get_manufacturer_string( type_hid_get_manufacturer_string p_fun /*= NULL*/ )
{
	static type_hid_get_manufacturer_string p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_get_product_string _get_product_string( type_hid_get_product_string p_fun /*= NULL*/ )
{
	static type_hid_get_product_string p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_get_serial_number_string _get_serial_number_string( type_hid_get_serial_number_string p_fun /*= NULL*/ )
{
	static type_hid_get_serial_number_string p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_get_indexed_string _get_indexed_string( type_hid_get_indexed_string p_fun /*= NULL*/ )
{
	static type_hid_get_indexed_string p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}
type_hid_error _error( type_hid_error p_fun /*= NULL*/ )
{
	static type_hid_error p_hid = NULL;
	if( p_fun )
		p_hid = p_fun;
	//
	return p_hid;
}

//


unsigned long _tx( hid_device *h_dev, type_msr_cmd cmd, unsigned char c_sub, unsigned long n_data, const unsigned char *s_data )
{
	unsigned long n_result = 0;
	unsigned long n_tx = MSR_SIZE_HOST_PACKET_HEADER;
	type_msr_host_packet req;
	unsigned char n_out_report = 64;
	unsigned char s_out_report[64+1] = {0,};
	unsigned long n_offset = 0;
	unsigned char *p_tx = (unsigned char *)&req;
	unsigned long n_packet = 0;
	int n_written = 0;

	do{
		if( n_data > MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD )
			continue;
		//
		if( n_data > 0 )
			n_tx +=  n_data;
		//
		memset( &req, 0, sizeof(req) );
		req.c_cmd = (unsigned char)cmd;
		req.c_sub = c_sub;
		req.c_len = (unsigned char)n_data;
		if( n_data > 0 )
			memcpy( req.s_data, s_data, n_data );

		//3+37+104+76 = 220
		do{
			memset( s_out_report, 0, sizeof(s_out_report) );

			if( (n_tx - n_offset) > n_out_report )
				n_packet = n_out_report;
			else
				n_packet = n_tx - n_offset;
			//
			memcpy( &s_out_report[1], &p_tx[n_offset], n_packet);

			n_written = _write(0)( h_dev,s_out_report, n_packet+1 );
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
		}while(n_offset<n_tx);

	}while(0);

	return n_result;
}

/////////////////////////////////////////////////////
//exported function body.

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;
	void *h_dll = NULL;

	do{
		//load library
		h_dll = _dll_handle( dlopen( "libhidapi.so", RTLD_NOW) );//RTLD_LAZY
		if( h_dll == NULL ){
			continue;
		}
		//
		if( _init( (type_hid_init)dlsym(_dll_handle(_GET_HANDLE), "hid_init") ) == NULL ){
			continue;
		}
		if( _exit_my( (type_hid_exit)dlsym(_dll_handle(_GET_HANDLE), "hid_exit") ) == NULL ){
			continue;
		}
		if( _enumerate( (type_hid_enumerate)dlsym(_dll_handle(_GET_HANDLE), "hid_enumerate") ) == NULL ){
			continue;
		}
		if( _free_enumeration( (type_hid_free_enumeration)dlsym(_dll_handle(_GET_HANDLE), "hid_free_enumeration") ) == NULL ){
			continue;
		}
		if( _open_ex( (type_hid_open_ex)dlsym(_dll_handle(_GET_HANDLE), "hid_open_ex") ) == NULL ){
			continue;
		}
		if( _open( (type_hid_open)dlsym(_dll_handle(_GET_HANDLE), "hid_open") ) == NULL ){
			continue;
		}
		if( _open_path( (type_hid_open_path)dlsym(_dll_handle(_GET_HANDLE), "hid_open_path") ) == NULL ){
			continue;
		}
		if( _write( (type_hid_write)dlsym(_dll_handle(_GET_HANDLE), "hid_write") ) == NULL ){
			continue;
		}
		if( _read_timeout( (type_hid_read_timeout)dlsym(_dll_handle(_GET_HANDLE), "hid_read_timeout") ) == NULL ){
			continue;
		}
		if( _read( (type_hid_read)dlsym(_dll_handle(_GET_HANDLE), "hid_read") ) == NULL ){
			continue;
		}
		if( _set_nonblocking( (type_hid_set_nonblocking)dlsym(_dll_handle(_GET_HANDLE), "hid_set_nonblocking") ) == NULL ){
			continue;
		}
		if( _send_feature_report( (type_hid_send_feature_report)dlsym(_dll_handle(_GET_HANDLE), "hid_send_feature_report") ) == NULL ){
			continue;
		}
		if( _get_feature_report( (type_hid_get_feature_report)dlsym(_dll_handle(_GET_HANDLE), "hid_get_feature_report") ) == NULL ){
			continue;
		}
		if( _close( (type_hid_close)dlsym(_dll_handle(_GET_HANDLE), "hid_close") ) == NULL ){
			continue;
		}
		if( _get_manufacturer_string( (type_hid_get_manufacturer_string)dlsym(_dll_handle(_GET_HANDLE), "hid_get_manufacturer_string") ) == NULL ){
			continue;
		}
		if( _get_product_string( (type_hid_get_product_string)dlsym(_dll_handle(_GET_HANDLE), "hid_get_product_string") ) == NULL ){
			continue;
		}
		if( _get_serial_number_string( (type_hid_get_serial_number_string)dlsym(_dll_handle(_GET_HANDLE), "hid_get_serial_number_string") ) == NULL ){
			continue;
		}
		if( _get_indexed_string( (type_hid_get_indexed_string)dlsym(_dll_handle(_GET_HANDLE), "hid_get_indexed_string") ) == NULL ){
			continue;
		}
		if( _error( (type_hid_error)dlsym(_dll_handle(_GET_HANDLE), "hid_error") ) == NULL ){
			continue;
		}

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	if( dw_result != LPU237_DLL_RESULT_SUCCESS ){
		if( _dll_handle(_GET_HANDLE) ){
			dlclose( _dll_handle(_GET_HANDLE) );
			_dll_handle(0);
		}
	}
	else{
		if( _init(0)() != 0 ){
			dlclose( _dll_handle(_GET_HANDLE) );
			_dll_handle(0);
		}
	}

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off()
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	if( _dll_handle(_GET_HANDLE) ){
		_exit_my(0)();
		dlclose( _dll_handle(_GET_HANDLE) );
		_dll_handle(0);
	}

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
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		//
		devs = _enumerate(0)(_USB_VID_LPU237, _USB_PID_LPU237);
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
		_free_enumeration(0)(devs);
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
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		//
		devs = _enumerate(0)(_USB_VID_LPU237, _USB_PID_LPU237);
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
		_free_enumeration(0)(devs);
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
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		if( s_dev_path == NULL )
			continue;
		//convert wchar_t -> char
		n_size = wcstombs (s_path, s_dev_path, sizeof(s_path));
		if( n_size <= 0)
			continue;

		p_hid = _open_path(0)( s_path );
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
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		if( s_dev_path == NULL )
			continue;
		//
		p_hid = _open_path(0)( s_dev_path );
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
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		if( h_dev == NULL )
			continue;
		//
		_close(0)( (hid_device*)h_dev );

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait_swipe( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
{
	unsigned long dw_result = LPU237_DLL_RESULT_ERROR;

	do{
		if( _dll_handle(_GET_HANDLE) == NULL )
			continue;
		// TODO.......

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

		// success
		dw_result = LPU237_DLL_RESULT_SUCCESS;
	}while(0);

	return dw_result;
}

