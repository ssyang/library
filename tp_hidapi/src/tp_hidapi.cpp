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

#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>

#include <string.h>
#include <string>

#include <vector>
#include <algorithm>

#include <dlfcn.h>

// 권한 문제의 경우
// KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666", GROUP="plugdev" 를 추가 할 것.
//
using namespace std;

int main( int argc, char **argv )
{
	cout << " = start test = " << endl;

	bool b_close_need = false;
	bool b_off_need = false;
	LPU237_HANDLE h_dev = NULL;
	unsigned long n_result(LPU237_DLL_RESULT_SUCCESS);

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
		cout << " ******* read your card ******* " <<endl;
		unsigned long n_result_index = dll_lpu237::get_instance().LPU237_wait_swipe_with_waits(h_dev);
		if( n_result_index == LPU237_DLL_RESULT_ERROR ){
			cout << " <> fail LPU237_wait_swipe_with_waits. "<< endl;
			continue;
		}
		cout << " = ok LPU237_wait_swipe_with_waits =  " << n_result_index << endl;

		unsigned char n_track = 0;
		vector<unsigned char> v_track(0);

		for( n_track = 0; n_track<3; n_track++ ){
			v_track.resize(0);

			n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, NULL );
			switch(n_result){
			case LPU237_DLL_RESULT_ERROR:
				cout << " <> error msr read - LPU237_DLL_RESULT_ERROR" << endl;
				break;
			case LPU237_DLL_RESULT_CANCEL:
				cout << " <> error msr read - LPU237_DLL_RESULT_CANCEL" << endl;
				break;
			case LPU237_DLL_RESULT_ERROR_MSR:
				cout << " <> error msr read - LPU237_DLL_RESULT_ERROR_MSR" << endl;
				break;
			default:
				if( n_result == 0){
					cout << " = ok msr read - no data" << endl;
					break;
				}
				//
				v_track.resize(n_result,0);
				continue;
			}//end switch

			//
			if( v_track.size() == 0 )
				continue;
			//
			n_result = dll_lpu237::get_instance().LPU237_get_data(n_result_index, n_track+1, &v_track[0] );
			for_each( begin(v_track), end(v_track), [=](unsigned char c){
				cout << (char)c << ',';
			});
			cout << endl;
		}//end for


		//++++++++++++++++++++++++++++++++++++++
		// 7. disable reading.
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
