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

#include <dlfcn.h>

using namespace std;

int main( int argc, char **argv )
{
	cout << " = start test = " << endl;

	do{
		//ok -> string s_path("//home//totoro//work//library//tp_hidapi//Debug//liblpu237-hidapi.so");
		//ok in console ->string s_path("./liblpu237-hidapi.so");
		//ok in console ->string s_path(".//liblpu237-hidapi.so");
		string s_path("liblpu237-hidapi.so");
		//string s_path("//home//totoro//work//library//tp_hidapi//Debug//liblpu237-hidapi.so");
		if( !dll_lpu237::get_instance().load( s_path ) ){

			/*
			string s_new_path = inner_util::get_changed_fullpath(
					inner_util::get_current_module_fullpath(),
					s_path
					);

			dll_lpu237::get_instance().load( s_new_path );
			*/
		}
		//
		if( !dll_lpu237::get_instance().is_setup_ok() ){
			cout << " <> fail load " << s_path << "." << endl;
			cout << " <> fail load " << dlerror() << "." << endl;
			continue;
		}

		cout << " = ok load " << s_path << "." << endl;

		// 1.
		unsigned long n_result(LPU237_DLL_RESULT_SUCCESS);

		n_result = dll_lpu237::get_instance().LPU237_dll_on();
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_dll_on. "<< endl;
			continue;
		}
		cout << " = ok LPU237_dll_on. "<< endl;

		n_result = dll_lpu237::get_instance().LPU237_dll_off();
		if( n_result != LPU237_DLL_RESULT_SUCCESS ){
			cout << " <> fail LPU237_dll_off. "<< endl;
			continue;
		}
		cout << " = ok LPU237_dll_off. "<< endl;


	}while(0);

	cout << " = exit test = " << endl;
	return 0;
}
