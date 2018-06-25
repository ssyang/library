//============================================================================
// Name        : tp_hidapi.cpp
// Author      : yss
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "dll_lpu237.h"
#include "inner_util.h"
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

using namespace std;

volatile unsigned long gn_result_index = -1;
static inner_event event_card_done;
static void _display_card_data( unsigned long n_result_index );

static void LPU237_callback(void*p_parameter);
void LPU237_callback(void*p_parameter)
{
	do{

		_display_card_data( gn_result_index );
		//
		event_card_done.set();
	}while(0);
}
void _display_card_data( unsigned long n_result_index )
{
	unsigned long n_result = 0;
	vector<unsigned char> v_track(0);

	for( unsigned long n_track = 0; n_track<3; n_track++ ){
		v_track.resize(0);
		n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, NULL );
		switch(n_result){
		case LPU237_DLL_RESULT_ERROR:
			cout << " <> error msr read - LPU237_DLL_RESULT_ERROR" << endl;
			continue;
		case LPU237_DLL_RESULT_CANCEL:
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
			break;
		}//end switch
		//
		n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, &v_track[0] );
		for_each( begin(v_track), end(v_track), [=](unsigned char c){
			cout << (char)c;
		});
		cout << endl;
	}//end for
}
// 권한 문제의 경우
// KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666", GROUP="plugdev" 를 추가 할 것.
//


int main( int argc, char **argv )
{
	cout << " = start test = " << endl;

	bool b_close_need = false;
	bool b_off_need = false;
	LPU237_HANDLE h_dev = NULL;
	unsigned long n_result(LPU237_DLL_RESULT_SUCCESS);
	unsigned long n_result_index = -1;
	vector<unsigned char> v_track(0);
	int n_test = 0;

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

		//++++++++++++++++++++++++++++++++++++++
		// 1. initial API
		n_result = dll_lpu237::get_instance().LPU237_dll_on();
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_dll_on. "<< endl;
			continue;
		}
		cout << " = ok LPU237_dll_on. "<< endl;
		b_off_need = true;

		//++++++++++++++++++++++++++++++++++++++
		// 2.get device list
		//get buffer size of paths.
		unsigned long n_buffer_size = dll_lpu237::get_instance().LPU237_get_list_a(NULL);
		if( n_buffer_size == 0 ){
			cout << " <> No device. "<< endl;
			continue;//no device
		}
		cout << " = need path buffer size is " << n_buffer_size << endl;

		//get paths
		vector<char> v_paths(n_buffer_size,0);
		unsigned long n_dev = dll_lpu237::get_instance().LPU237_get_list_a(&v_paths[0]);
		if( n_dev == 0 ){
			cout << " <> No device. "<< endl;
			continue;//no device
		}
		cout << " = found device is . "<< n_dev << endl;
		cout << " = the 1'st path = " << &v_paths[0] << endl;

		//++++++++++++++++++++++++++++++++++++++
		// 3. open device
		// open the first device.
		h_dev = dll_lpu237::get_instance().LPU237_open_a(&v_paths[0]);
		if( h_dev == NULL ){
			cout << " <> fail LPU237_open_a. "<< endl;
			continue;
		}
		cout << " = ok LPU237_open_a. "<< endl;
		b_close_need = true;

		//++++++++++++++++++++++++++++++++++++++
		// 4. get device ID.
		unsigned long n_id = dll_lpu237::get_instance().LPU237_get_id( h_dev, NULL );
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

		//++++++++++++++++++++++++++++++++++++++
		// 5. enable reading.
		n_result = dll_lpu237::get_instance().LPU237_enable(h_dev);
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_enable. "<< endl;
			continue;
		}
		cout << " = ok LPU237_enable " << endl;

		//++++++++++++++++++++++++++++++++++++++
		// 6. reading msr by sync pattern.
		for( n_test = 0; n_test<2; n_test++ ){
			cout << " ******* read your card ******* " <<endl;
			n_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_with_waits(h_dev);
			if( n_result_index == LPU237_DLL_RESULT_ERROR ){
				cout << " <> fail LPU237_wait_swipe_with_waits. "<< endl;
				continue;
			}
			cout << " = ok LPU237_wait_swipe_with_waits =  " << n_result_index << endl;

			_display_card_data( n_result_index );

		}//end for n_test times

		//++++++++++++++++++++++++++++++++++++++
		// 7. reading msr by async pattern.
		for( n_test = 0; n_test<10; n_test++ ){
			cout << " ******* read your card ******* " <<endl;
			event_card_done.reset();

			gn_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_with_callback(h_dev,LPU237_callback,NULL);
			if( gn_result_index == LPU237_DLL_RESULT_ERROR ){
				cout << " <> fail LPU237_wait_swipe_with_callback. "<< endl;
				continue;
			}
			cout << " = ok LPU237_wait_swipe_with_callback =  " << gn_result_index << endl;

			event_card_done.wait(-1);
		}

		//++++++++++++++++++++++++++++++++++++++
		// 8. disable reading.
		n_result = dll_lpu237::get_instance().LPU237_disable(h_dev);
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_disable. "<< endl;
			continue;
		}
		cout << " = ok LPU237_disable " << endl;

	}while(0);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//garbage processing
	do{
		if( !dll_lpu237::get_instance().is_setup_ok() ){
			continue;
		}
		//
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


	cout << " = exit test = " << endl;
	return 0;
}
