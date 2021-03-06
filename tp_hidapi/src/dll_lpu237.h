/*
 * dll_lpu237.h
 *
 *  Created on: 2018. 6. 20.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef DLL_LPU237_H_
#define DLL_LPU237_H_

#include <stdio.h>
#include <string>

#include "shared_type.h"
#include "el_lpu237-hidapi.h"

using namespace std;

class dll_lpu237
{
public:
	static dll_lpu237 & get_instance();

	~dll_lpu237();

	bool load( const string & s_dll );

	bool unload();

	bool is_setup_ok();

	type_dword LPU237_dll_on();

	type_dword LPU237_dll_off();

	type_dword LPU237_get_list_w(wchar_t *ss_dev_path);
	type_dword LPU237_get_list_a(char *ss_dev_path);

	LPU237_HANDLE LPU237_open_w( const wchar_t *s_dev_path );
	LPU237_HANDLE LPU237_open_a( const char *s_dev_path );

	type_dword LPU237_close( LPU237_HANDLE h_dev );

	type_dword LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id );

	type_dword LPU237_enable( LPU237_HANDLE h_dev );

	type_dword LPU237_disable( LPU237_HANDLE h_dev );

	type_dword LPU237_cancel_wait( LPU237_HANDLE h_dev );

	type_dword LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev );

	type_dword LPU237_wait_key_with_waits( LPU237_HANDLE h_dev );

	type_dword LPU237_wait_swipe_or_key_with_waits( LPU237_HANDLE h_dev );

	type_dword LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

	type_dword LPU237_wait_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

	type_dword LPU237_wait_swipe_or_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

	type_dword LPU237_get_data( type_dword dw_buffer_index, type_dword dw_iso_track, unsigned char *s_track_data );


private:
	dll_lpu237();
private:
	bool m_b_setup_ok;
	void *m_h_dll;

	type_LPU237_dll_on m_LPU237_dll_on;
	type_LPU237_dll_off m_LPU237_dll_off;
	type_LPU237_get_list_w m_LPU237_get_list_w;
	type_LPU237_get_list_a m_LPU237_get_list_a;

	type_LPU237_open_w m_LPU237_open_w;
	type_LPU237_open_a m_LPU237_open_a;
	type_LPU237_close m_LPU237_close;

	type_LPU237_get_id m_LPU237_get_id;
	type_LPU237_enable m_LPU237_enable;
	type_LPU237_disable m_LPU237_disable;

	type_LPU237_cancel_wait m_LPU237_cancel_wait;
	type_LPU237_wait_swipe_with_waits m_LPU237_wait_swipe_with_waits;
	type_LPU237_wait_key_with_waits m_LPU237_wait_key_with_waits;
	type_LPU237_wait_swipe_or_key_with_waits m_LPU237_wait_swipe_or_key_with_waits;
	type_LPU237_wait_swipe_with_callback m_LPU237_wait_swipe_with_callback;
	type_LPU237_wait_key_with_callback m_LPU237_wait_key_with_callback;
	type_LPU237_wait_swipe_or_key_with_callback m_LPU237_wait_swipe_or_key_with_callback;
	type_LPU237_get_data m_LPU237_get_data;

private://don't call these methods
	dll_lpu237( const dll_lpu237 & );
	dll_lpu237 & operator=( const dll_lpu237 & );


};



#endif /* DLL_LPU237_H_ */
