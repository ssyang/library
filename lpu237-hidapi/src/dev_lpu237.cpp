/*
 * dev_lpu237.cpp
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */


#include "dev_lpu237.h"

const string & get_class_name()
{
	static string s_name("dev_lpu237");
	return s_name;
}

dev_lpu237::dev_lpu237() :	dev_hid()
{
	m_map_in_report_size[0] = 220;
}

dev_lpu237::~dev_lpu237()
{

}

