//============================================================================
// Name        : tp_hidapi.cpp
// Author      : yss
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
// This code is released under the terms of the MIT License
//============================================================================

#include <iostream>
#include <iomanip>

#include "dll_lpu237.h"
#include "stdlib.h"

#include "inner_event.h"

#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>

#include <string.h>
#include <string>

#include <vector>
#include <algorithm>

#include <dlfcn.h>
#include <pthread.h>

using namespace std;

volatile type_dword gn_result_index = -1;
static void _display_card_data( type_dword n_result_index );
static void _display_key_data( type_dword n_result_index );
static void _display_card_or_key_data( type_dword n_result_index );
static void _LPU237_card_callback(void*p_parameter);
static void _LPU237_key_callback(void*p_parameter);
static void _LPU237_card_or_key_callback(void*p_parameter);
static void *_wait_card_worker(void *p_data);
static void *_wait_key_worker(void *p_data);
static void *_wait_card_or_key_worker(void *p_data);

static void _garbage_processing(bool b_close_need,bool b_off_need,LPU237_HANDLE h_dev );
static bool _load_library();
static bool _start_library();
static bool _get_device_path_mbcs( string & s_path );
static LPU237_HANDLE _open_device_mbcs( const string & s_path );

static bool _get_device_path_wchar( wstring & s_path );
static LPU237_HANDLE _open_device_wchar( const wstring & s_path );

static bool _get_device_id( LPU237_HANDLE h_dev );
static bool _enable_device( LPU237_HANDLE h_dev );
static bool _disable_device( LPU237_HANDLE h_dev );

static bool _read_card_by_sync( LPU237_HANDLE h_dev );
static bool _read_card_by_async( LPU237_HANDLE h_dev );

static bool _read_key_by_sync( LPU237_HANDLE h_dev );
static bool _read_key_by_async( LPU237_HANDLE h_dev );

static bool _read_card_or_key_by_sync( LPU237_HANDLE h_dev );
static bool _read_card_or_key_by_async( LPU237_HANDLE h_dev );

volatile bool gb_read_done = false;

void _LPU237_card_callback(void*p_parameter)
{
	do{
		_display_card_data( gn_result_index );
		cout << " = press enter key. for the next test." << endl;
		gb_read_done = true;
	}while(0);
}

void _LPU237_key_callback(void*p_parameter)
{
	do{
		_display_key_data( gn_result_index );
		cout << " = press enter key. for the next test." << endl;
		gb_read_done = true;
	}while(0);
}

void _LPU237_card_or_key_callback(void*p_parameter)
{
	do{
		_display_card_or_key_data( gn_result_index );
		cout << " = press enter key. for the next test." << endl;
		gb_read_done = true;
	}while(0);
}

void _display_card_data( type_dword n_result_index )
{
	type_dword n_result = 0;
	vector<unsigned char> v_track(0);

	for( type_dword n_track = 0; n_track<3; n_track++ ){
		v_track.resize(0);
		n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, NULL );
		switch(n_result){
		case LPU237_DLL_RESULT_ERROR:
			cout << " <> error msr read - LPU237_DLL_RESULT_ERROR" << endl;
			continue;
		case LPU237_DLL_RESULT_CANCEL:
			n_track = 3;//exit for
			cout << " <> error msr read - LPU237_DLL_RESULT_CANCEL" << endl;
			continue;
		case LPU237_DLL_RESULT_ERROR_MSR:
			cout << " <> error msr read - LPU237_DLL_RESULT_ERROR_MSR" << endl;
			continue;
		default:
			if( n_result == 0){
				cout << " = ok msr read - no data" << endl;
				continue;
			}
			v_track.resize(n_result,0);

			n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, &v_track[0] );
			for_each( begin(v_track), end(v_track), [=](unsigned char c){
				cout << (char)c;
			});
			cout << endl;
			break;
		}//end switch
		//
	}//end for
}


void _display_key_data( type_dword n_result_index )
{
	type_dword n_result = 0;
	vector<unsigned char> v_key(0);

	v_key.resize(0);
	n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, 0, NULL );
	switch(n_result){
	case LPU237_DLL_RESULT_ERROR:
		cout << " <> error key read - LPU237_DLL_RESULT_ERROR" << endl;
		break;
	case LPU237_DLL_RESULT_CANCEL:
		cout << " <> error key read - LPU237_DLL_RESULT_CANCEL" << endl;
		break;
	case LPU237_DLL_RESULT_ERROR_MSR:
		cout << " <> error key read - LPU237_DLL_RESULT_ERROR_MSR" << endl;
		break;
	default:
		if( n_result == 0){
			cout << " = ok key read - no data" << endl;
			break;
		}
		v_key.resize(n_result,0);
		n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, 0, &v_key[0] );
		for_each( begin(v_key), end(v_key), [=](unsigned char c){
			cout << internal  << setfill('0') << hex << setw(2) << (int)c;
		});
		cout << endl;
		break;
	}//end switch
	//
}

void _display_card_or_key_data( type_dword n_result_index )
{
	type_dword n_result = 0;
	vector<unsigned char> v_data(0);

	do{
		n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, 0, NULL );
		switch(n_result){
		case LPU237_DLL_RESULT_CANCEL:
			cout << " <> error read - LPU237_DLL_RESULT_CANCEL" << endl;
			continue;
		case LPU237_DLL_RESULT_ERROR:
		case LPU237_DLL_RESULT_ERROR_MSR:
			break;
		default:
			if( n_result == 0){
				cout << " = ok key read - no data" << endl;
			}
			else{
				v_data.resize(n_result,0);
				n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, 0, &v_data[0] );
				for_each( begin(v_data), end(v_data), [=](unsigned char c){
					cout << internal  << setfill('0') << hex << setw(2) << (int)c;
				});
				cout << endl;
			}
			continue;
		}//end switch
		//
		for( type_dword n_track = 0; n_track<3; n_track++ ){
			v_data.resize(0);
			n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, NULL );
			switch(n_result){
			case LPU237_DLL_RESULT_ERROR:
				cout << " <> error msr read - LPU237_DLL_RESULT_ERROR" << endl;
				continue;
			case LPU237_DLL_RESULT_CANCEL:
				n_track = 3;//exit for
				cout << " <> error msr read - LPU237_DLL_RESULT_CANCEL" << endl;
				continue;
			case LPU237_DLL_RESULT_ERROR_MSR:
				cout << " <> error msr read - LPU237_DLL_RESULT_ERROR_MSR" << endl;
				continue;
			default:
				if( n_result == 0){
					cout << " = ok msr read - no data" << endl;
				}
				else{
					v_data.resize(n_result,0);
					n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, &v_data[0] );
					for_each( begin(v_data), end(v_data), [=](unsigned char c){
						cout << (char)c;
					});
					cout << endl;
				}
				continue;
			}//end switch
			//
		}//end for
	}while(0);
}

void *_wait_card_worker(void *p_data)
{
	static int n_return_val = 0;

	do{
		LPU237_HANDLE h_dev = (LPU237_HANDLE)p_data;

		if( h_dev == NULL )
			continue;
		//
		type_dword n_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_with_waits(h_dev);
		if( n_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_swipe_with_waits. "<< endl;
			continue;
		}
		_display_card_data( n_result_index );

	}while(0);

	cout << " = press enter key. for the next test." << endl;
	gb_read_done = true;

	pthread_exit((void*)&n_return_val);
	//return p_result;
}

void *_wait_key_worker(void *p_data)
{
	static int n_return_val = 0;

	do{
		LPU237_HANDLE h_dev = (LPU237_HANDLE)p_data;

		if( h_dev == NULL )
			continue;
		//
		type_dword n_result_index = dll_lpu237::get_instance().LPU237_wait_key_with_waits(h_dev);
		if( n_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_key_with_waits. "<< endl;
			continue;
		}
		_display_key_data( n_result_index );

	}while(0);

	cout << " = press enter key. for the next test." << endl;
	gb_read_done = true;

	pthread_exit((void*)&n_return_val);
	//return p_result;
}


void *_wait_card_or_key_worker(void *p_data)
{
	static int n_return_val = 0;

	do{
		LPU237_HANDLE h_dev = (LPU237_HANDLE)p_data;

		if( h_dev == NULL )
			continue;
		//
		type_dword n_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_or_key_with_waits(h_dev);
		if( n_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_swipe_or_key_with_waits. "<< endl;
			continue;
		}
		_display_card_or_key_data( n_result_index );

	}while(0);

	cout << " = press enter key. for the next test." << endl;
	gb_read_done = true;

	pthread_exit((void*)&n_return_val);
	//return p_result;
}
// 권한 문제의 경우
// KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666", GROUP="plugdev" 를 추가 할 것.
//

int main( int argc, char **argv )
{
#if __x86_64__
/* 64-bit */
	cout << " = start test(64 bits) = " << sizeof(unsigned int) << endl;
#else
/* 32-bit */
	cout << " = start test(32 bits) = " << sizeof(unsigned long) << endl;
#endif



	bool b_close_need = false;
	bool b_off_need = false;
	LPU237_HANDLE h_dev = NULL;
	vector<unsigned char> v_track(0);
	int n_test = 0;
	bool b_use_mbcs = true;

	do{
		if( argc == 2 ){
			if( argv[1][0] == 'u' )
				b_use_mbcs =false;
		}

		if( b_use_mbcs )
			cout << " $ test by multi byte code set $ " <<endl;
		else
			cout << " $ test by wide-character $ " <<endl;
		//
		if( !_load_library() )
			continue;

		//++++++++++++++++++++++++++++++++++++++
		// 1. initial API
		if( !_start_library() )
			continue;
		b_off_need = true;

		//++++++++++++++++++++++++++++++++++++++
		// 2.get device list
		string s_path;
		wstring s_wpath;

		if( b_use_mbcs ){
			if( !_get_device_path_mbcs( s_path ) )
				continue;
		}
		else{
			if( !_get_device_path_wchar( s_wpath ) )
				continue;
		}

		//++++++++++++++++++++++++++++++++++++++
		// 3. open device
		if( b_use_mbcs )
			h_dev = _open_device_mbcs( s_path );
		else
			h_dev = _open_device_wchar( s_wpath );

		if( h_dev == NULL )
			continue;
		b_close_need = true;

		//++++++++++++++++++++++++++++++++++++++
		// 4. get device ID.
		if( !_get_device_id( h_dev ) )
			continue;

		//++++++++++++++++++++++++++++++++++++++
		// 5. enable reading.
		if( !_enable_device( h_dev ) )
			continue;

		//++++++++++++++++++++++++++++++++++++++
		// 6. reading msr by sync pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_card_by_sync( h_dev );
		}//end for n_test times

		//++++++++++++++++++++++++++++++++++++++
		// 7. reading msr by async pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_card_by_async(h_dev);
		}

		//++++++++++++++++++++++++++++++++++++++
		// 8. reading key by sync pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_key_by_sync( h_dev );
		}//end for n_test times

		//++++++++++++++++++++++++++++++++++++++
		// 9. reading key by async pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_key_by_async(h_dev);
		}

		//++++++++++++++++++++++++++++++++++++++
		// 10. reading msr or key by sync pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_card_or_key_by_sync( h_dev );
		}//end for n_test times

		//++++++++++++++++++++++++++++++++++++++
		// 11. reading msr or key by async pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			_read_card_or_key_by_async(h_dev);
		}

		//++++++++++++++++++++++++++++++++++++++
		// 12. disable reading.
		if( !_disable_device( h_dev ) )
			continue;
	}while(0);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//garbage processing
	_garbage_processing( b_close_need,b_off_need,h_dev );

	cout << " = exit test = " << endl;
	return 0;
}

void _garbage_processing(bool b_close_need,bool b_off_need,LPU237_HANDLE h_dev )
{
	do{
		if( !dll_lpu237::get_instance().is_setup_ok() ){
			continue;
		}
		//
		type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

		if( b_close_need ){
			n_result = dll_lpu237::get_instance().LPU237_close( h_dev );
			if( n_result != LPU237_DLL_RESULT_SUCCESS )
				cout << " <> fail LPU237_close. "<< endl;
			else
				cout << " = ok LPU237_close. "<< endl;
		}

		if( b_off_need ){
			n_result = dll_lpu237::get_instance().LPU237_dll_off();
			if( n_result != LPU237_DLL_RESULT_SUCCESS )
				cout << " <> fail LPU237_dll_off. "<< endl;
			else
				cout << " = ok LPU237_dll_off. "<< endl;
		}

	}while(0);

}

bool _load_library()
{
	bool b_result(false);

	do{
		string s_path("liblpu237-hidapi.so");
		dll_lpu237::get_instance().load( s_path );
		//
		if( !dll_lpu237::get_instance().is_setup_ok() ){
			cout << " <> fail load " << s_path << "." << endl;
			cout << " <> fail load " << dlerror() << "." << endl;
			continue;
		}

		cout << " = ok load " << s_path << "." << endl;
		b_result = true;
	}while(0);

	return b_result;
}

bool _start_library()
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		n_result = dll_lpu237::get_instance().LPU237_dll_on();
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_dll_on. "<< endl;
			continue;
		}
		cout << " = ok LPU237_dll_on. "<< endl;

		b_result = true;
	}while(0);

	return b_result;
}

bool _get_device_path_mbcs( string & s_path )
{
	bool b_result(false);

	do{
		s_path="";
		//get buffer size of paths.
		type_dword n_buffer_size = dll_lpu237::get_instance().LPU237_get_list_a(NULL);
		if( n_buffer_size == 0 ){
			cout << " <> No device. "<< endl;
			continue;//no device
		}
		cout << " = need path buffer size is " << n_buffer_size << endl;

		//get paths
		vector<char> v_paths(n_buffer_size,0);
		type_dword n_dev = dll_lpu237::get_instance().LPU237_get_list_a(&v_paths[0]);
		if( n_dev == 0 ){
			cout << " <> No device. "<< endl;
			continue;//no device
		}

		s_path = &v_paths[0];

		cout << " = found device is . "<< n_dev << endl;
		cout << " = the 1'st path = " << s_path << endl;

		b_result = true;
	}while(0);

	return b_result;
}

bool _get_device_path_wchar( wstring & s_path )
{
	bool b_result(false);

	do{
		s_path=L"";
		//get buffer size of paths.
		type_dword n_buffer_size = dll_lpu237::get_instance().LPU237_get_list_w(NULL);
		if( n_buffer_size == 0 ){
			wcout << L" <> No device. "<< endl;
			continue;//no device
		}
		wcout << L" = need path buffer size is " << n_buffer_size << endl;

		//get paths
		vector<wchar_t> v_paths(n_buffer_size,0);
		type_dword n_dev = dll_lpu237::get_instance().LPU237_get_list_w(&v_paths[0]);
		if( n_dev == 0 ){
			wcout << L" <> No device. "<< endl;
			continue;//no device
		}

		s_path = &v_paths[0];

		wcout << L" = found device is . "<< n_dev << endl;
		wcout << L" = the 1'st path = " << s_path << endl;

		b_result = true;
	}while(0);

	return b_result;
}

LPU237_HANDLE _open_device_mbcs( const string & s_path )
{
	LPU237_HANDLE h_dev = NULL;

	do{
		// open the first device.
		h_dev = dll_lpu237::get_instance().LPU237_open_a(s_path.c_str());
		if( h_dev == NULL ){
			cout << " <> fail LPU237_open_a. "<< endl;
			continue;
		}
		cout << " = ok LPU237_open_a. "<< endl;

	}while(0);

	return h_dev;
}

LPU237_HANDLE _open_device_wchar( const wstring & s_path )
{
	LPU237_HANDLE h_dev = NULL;

	do{
		// open the first device.
		h_dev = dll_lpu237::get_instance().LPU237_open_w(s_path.c_str());
		if( h_dev == NULL ){
			wcout << L" <> fail LPU237_open_a. "<< endl;
			continue;
		}
		wcout << L" = ok LPU237_open_a. "<< endl;

	}while(0);

	return h_dev;
}

bool _get_device_id( LPU237_HANDLE h_dev )
{
	bool b_result(false);

	do{
		type_dword n_id = dll_lpu237::get_instance().LPU237_get_id( h_dev, NULL );
		if( n_id == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_get_id. "<< endl;
			continue;
		}
		cout << " = ok LPU237_get_id size = "<< n_id << endl;

		vector<unsigned char> v_id(n_id,0);
		n_id = dll_lpu237::get_instance().LPU237_get_id( h_dev, &v_id[0] );
		if( n_id == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_get_id. "<< endl;
			continue;
		}
		cout << " = ok LPU237_get_id = ";
		for_each( begin(v_id), end(v_id),[=]( unsigned char c ){
			cout << hex << (int)c << ',';
		} );
		cout << hex << endl;

		b_result = true;
	}while(0);

	return b_result;
}

bool _enable_device( LPU237_HANDLE h_dev )
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		n_result = dll_lpu237::get_instance().LPU237_enable(h_dev);
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_enable. "<< endl;
			continue;
		}
		cout << " = ok LPU237_enable " << endl;

		b_result = true;
	}while(0);

	return b_result;
}

bool _disable_device( LPU237_HANDLE h_dev )
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		n_result = dll_lpu237::get_instance().LPU237_disable(h_dev);
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_disable. "<< endl;
			continue;
		}
		cout << " = ok LPU237_disable " << endl;

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_card_by_sync( LPU237_HANDLE h_dev )
{
	bool b_result(false);

	do{
		gb_read_done = false;
		pthread_t n_id = -1;
		void *p_status = NULL;
		//
		int n_result = pthread_create(&n_id, NULL, _wait_card_worker, h_dev );
		if( n_result != 0 ){
			cout << " <> fail create waiting worker. "<< endl;
			continue;
		}
		//
		cout << " ******* read your card by sync-method ******* " <<endl;
		cout << " = waiting status =  " << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		pthread_join(n_id,&p_status);

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_card_by_async( LPU237_HANDLE h_dev )
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		cout << " ******* read your card by async-method ******* " <<endl;
		gb_read_done = false;
		//
		gn_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_with_callback(h_dev,_LPU237_card_callback,NULL);
		if( gn_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_swipe_with_callback. "<< endl;
			continue;
		}

		cout << " = waiting status =  " << gn_result_index << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_key_by_sync( LPU237_HANDLE h_dev )
{
	bool b_result(false);

	do{
		gb_read_done = false;
		pthread_t n_id = -1;
		void *p_status = NULL;
		//
		int n_result = pthread_create(&n_id, NULL, _wait_key_worker, h_dev );
		if( n_result != 0 ){
			cout << " <> fail create waiting worker. "<< endl;
			continue;
		}
		//
		cout << " ******* read your ibutton by sync-method ******* " <<endl;
		cout << " = waiting status =  " << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		pthread_join(n_id,&p_status);

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_key_by_async( LPU237_HANDLE h_dev )
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		cout << " ******* read your key by async-method ******* " <<endl;
		gb_read_done = false;
		//
		gn_result_index = dll_lpu237::get_instance().LPU237_wait_key_with_callback(h_dev,_LPU237_key_callback,NULL);
		if( gn_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_key_with_callback. "<< endl;
			continue;
		}

		cout << " = waiting status =  " << gn_result_index << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_card_or_key_by_sync( LPU237_HANDLE h_dev )
{
	bool b_result(false);

	do{
		gb_read_done = false;
		pthread_t n_id = -1;
		void *p_status = NULL;
		//
		int n_result = pthread_create(&n_id, NULL, _wait_card_or_key_worker, h_dev );
		if( n_result != 0 ){
			cout << " <> fail create waiting worker. "<< endl;
			continue;
		}
		//
		cout << " ******* read your card or key by sync-method ******* " <<endl;
		cout << " = waiting status =  " << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		pthread_join(n_id,&p_status);

		b_result = true;
	}while(0);

	return b_result;
}

bool _read_card_or_key_by_async( LPU237_HANDLE h_dev )
{
	bool b_result(false);
	type_dword n_result(LPU237_DLL_RESULT_SUCCESS);

	do{
		cout << " ******* read your card or key by async-method ******* " <<endl;
		gb_read_done = false;
		//
		gn_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_or_key_with_callback(h_dev,_LPU237_card_or_key_callback,NULL);
		if( gn_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_swipe_or_key_with_callback. "<< endl;
			continue;
		}

		cout << " = waiting status =  " << gn_result_index << endl;
		cout << " = if you want to cancel, press enter key." << endl;
		char c_in(0);
		cin.get() >> c_in;

		if( !gb_read_done ){
			n_result = dll_lpu237::get_instance().LPU237_cancel_wait(h_dev);
			if( n_result == LPU237_DLL_RESULT_SUCCESS )
				cout << " = ok cancel" << endl;
			else
				cout << " <> fail cancel" << endl;
		}

		b_result = true;
	}while(0);

	return b_result;
}
