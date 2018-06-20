//============================================================================
// Name        : tp_hidapi.cpp
// Author      : yss
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "dll_lpu237.h"

using namespace std;

int main()
{
	cout << " = start test = " << endl;

	do{
		dll_lpu237::get_instance().load(string("liblpu237-hidapi.so") );
		if( !dll_lpu237::get_instance().is_setup_ok() ){
			cout << " <> fail load " << string("liblpu237-hidapi.so") << "." << endl;
			continue;
		}

		cout << " = ok load " << string("liblpu237-hidapi.so") << "." << endl;
		//

	}while(0);

	cout << " = exit test = " << endl;
	return 0;
}
