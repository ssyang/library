/*
 * inner_event.h
 *
 *  Created on: 2018. 6. 18.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_INNER_EVENT_H_
#define INC_INNER_EVENT_H_

#include "pevents.h"

#include <map>

using namespace std;

class inner_event
{
public:
	typedef	map<int,neosmart::neosmart_event_t >	type_map_raw_event;

	static bool set_raw_event_to_map( int n_index,neosmart::neosmart_event_t h_event);
	static neosmart::neosmart_event_t get_raw_event_and_remove( int n_index );

public:
	inner_event();
	inner_event(bool b_manualReset, bool b_initialState);
	~inner_event();

	bool set();
	bool reset();
	int wait( uint64_t time_out_msec );
	neosmart::neosmart_event_t get_handle();
private:
	neosmart::neosmart_event_t m_h_evet;

private:
	static type_map_raw_event m_map_raw_event;

private:
	//don't call these methods
	inner_event( const inner_event & );
	inner_event & operator=( const inner_event & );

};

#endif /* INC_INNER_EVENT_H_ */
