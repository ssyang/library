
/*
 * share.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_SHARE_H_
#define INC_SHARE_H_

#include <vector>
#include <string>
#include <memory>

using namespace std;

class shared
{
public:
	typedef	vector<unsigned char>	type_buffer;
	typedef	shared_ptr<type_buffer>	type_ptr_buffer;
	typedef	shared_ptr<string>		type_ptr_string;
	typedef	shared_ptr<wstring>		type_ptr_wstring;

	typedef enum{
		result_fun_success = 0,
		result_fun_error = 1,
		result_fun_ing = 2,
		result_fun_cancel = 3
	} type_result_fun;

private://don't call these methods
	shared();
	~shared();
	shared( const shared & );
	shared & operator=( const shared & );
};



#endif /* INC_SHARE_H_ */
