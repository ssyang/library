/*
 * inner_event.cpp
 *
 *  Created on: 2018. 6. 18.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#include "inner_event.h"

inner_event::type_map_raw_event inner_event::m_map_raw_event;

bool inner_event::set_raw_event_to_map( int n_index,neosmart::neosmart_event_t h_event )
{
	bool b_result = false;

	do{
		if( n_index < 0 )
			continue;
		inner_event::type_map_raw_event::iterator it =  inner_event::m_map_raw_event.find( n_index );
		if( it != end(inner_event::m_map_raw_event) ){
			continue;//already existence key
		}

		inner_event::m_map_raw_event[n_index] = h_event;
		b_result = true;
	}while(0);
	return b_result;
}

neosmart::neosmart_event_t inner_event::get_raw_event_and_remove( int n_index )
{
	neosmart::neosmart_event_t h_evet(NULL);

	inner_event::type_map_raw_event::iterator it =  inner_event::m_map_raw_event.find( n_index );
	if( it != end(inner_event::m_map_raw_event) ){
		h_evet = it->second;
		inner_event::m_map_raw_event.erase( n_index );
	}

	return h_evet;
}

inner_event::inner_event() : m_h_evet(0)
{
	m_h_evet = neosmart::CreateEvent( false, false );//default auto reset, initial status false
}

inner_event::inner_event(bool b_manualReset, bool b_initialState) : m_h_evet(0)
{
	m_h_evet = neosmart::CreateEvent( b_manualReset, b_initialState );
}

inner_event::~inner_event()
{
	if( m_h_evet ){
		neosmart::DestroyEvent(m_h_evet);
	}

}

bool inner_event::set()
{
	bool b_result(false);

	do{
		if( m_h_evet == 0)
			continue;
		//
		neosmart::SetEvent( m_h_evet );
		b_result = true;
	}while(0);
	return b_result;
}
bool inner_event::reset()
{
	bool b_result(false);

	do{
		if( m_h_evet == 0)
			continue;
		//
		neosmart::ResetEvent( m_h_evet );
		b_result = true;
	}while(0);
	return b_result;
}
int inner_event::wait( uint64_t time_out_msec )
{
	int n_result(-1);

	do{
		if( m_h_evet == 0)
			continue;
		n_result = neosmart::WaitForEvent(m_h_evet,time_out_msec );
	}while(0);
	return n_result;
}
neosmart::neosmart_event_t inner_event::get_handle()
{
	return m_h_evet;
}
