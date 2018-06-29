/*
 * dev_lpu237.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_DEV_LPU237_H_
#define INC_DEV_LPU237_H_

#include "dev_hid.h"

using namespace std;

class dev_lpu237 : public dev_hid
{
public:
	const string & get_class_name();

public:
	dev_lpu237();
	~dev_lpu237();

private:

private:
};



#endif /* INC_DEV_LPU237_H_ */
