/*
 * dll_lpu237.cpp
 *
 *  Created on: 2018. 6. 20.
 *      Author: totoro
 */


#include "dll_lpu237.h"

#include <dlfcn.h>

using namespace std;

dll_lpu237 & dll_lpu237::get_instance()
{
	static dll_lpu237 dll;
	return dll;
}

dll_lpu237::~dll_lpu237()
{
	if( m_h_dll )
		dlclose( m_h_dll );
}

bool dll_lpu237::load( const string & s_dll )
{
	bool b_result = false;

	do{
		if( m_b_setup_ok )
			continue;

		if( s_dll.empty() )
			continue;
		// load dll
		m_h_dll = dlopen( s_dll.c_str(), RTLD_NOW);//RTLD_LAZY
		if( m_h_dll == NULL )
			continue;
		//
		m_LPU237_dll_on = (type_LPU237_dll_on)dlsym( m_h_dll, "LPU237_dll_on");
		if( m_LPU237_dll_on == NULL )
			continue;
		m_LPU237_dll_off = (type_LPU237_dll_off)dlsym( m_h_dll, "LPU237_dll_off");
		if( m_LPU237_dll_off == NULL )
			continue;
		m_LPU237_get_list_w = (type_LPU237_get_list_w)dlsym( m_h_dll, "LPU237_get_list_w");
		if( m_LPU237_get_list_w == NULL )
			continue;
		m_LPU237_get_list_a = (type_LPU237_get_list_a)dlsym( m_h_dll, "LPU237_get_list_a");
		if( m_LPU237_get_list_a == NULL )
			continue;
		m_LPU237_open_w = (type_LPU237_open_w)dlsym( m_h_dll, "LPU237_open_w");
		if( m_LPU237_open_w == NULL )
			continue;
		m_LPU237_open_a = (type_LPU237_open_a)dlsym( m_h_dll, "LPU237_open_a");
		if( m_LPU237_open_a == NULL )
			continue;
		m_LPU237_close = (type_LPU237_close)dlsym( m_h_dll, "LPU237_close");
		if( m_LPU237_close == NULL )
			continue;
		m_LPU237_get_id = (type_LPU237_get_id)dlsym( m_h_dll, "LPU237_get_id");
		if( m_LPU237_get_id == NULL )
			continue;
		m_LPU237_enable = (type_LPU237_enable)dlsym( m_h_dll, "LPU237_enable");
		if( m_LPU237_enable == NULL )
			continue;
		m_LPU237_disable = (type_LPU237_disable)dlsym( m_h_dll, "LPU237_disable");
		if( m_LPU237_disable == NULL )
			continue;
		m_LPU237_cancel_wait_swipe = (type_LPU237_cancel_wait_swipe)dlsym( m_h_dll, "LPU237_cancel_wait_swipe");
		if( m_LPU237_cancel_wait_swipe == NULL )
			continue;
		m_LPU237_wait_swipe_with_waits = (type_LPU237_wait_swipe_with_waits)dlsym( m_h_dll, "LPU237_wait_swipe_with_waits");
		if( m_LPU237_wait_swipe_with_waits == NULL )
			continue;
		m_LPU237_wait_swipe_with_callback = (type_LPU237_wait_swipe_with_callback)dlsym( m_h_dll, "LPU237_wait_swipe_with_callback");
		if( m_LPU237_wait_swipe_with_callback == NULL )
			continue;
		m_LPU237_get_data = (type_LPU237_get_data)dlsym( m_h_dll, "LPU237_get_data");
		if( m_LPU237_get_data == NULL )
			continue;
		//
		m_b_setup_ok = true;
		b_result = true;
	}while(0);

	if( m_b_setup_ok == false && m_h_dll != NULL  ){
		dlclose( m_h_dll );
		m_h_dll = NULL;
		m_LPU237_dll_on = 0;
		m_LPU237_dll_off = 0;
		m_LPU237_get_list_w = 0;
		m_LPU237_get_list_a = 0;
		m_LPU237_open_w = 0;
		m_LPU237_open_a = 0;
		m_LPU237_close = 0;
		m_LPU237_get_id = 0;
		m_LPU237_enable = 0;
		m_LPU237_disable = 0;
		m_LPU237_cancel_wait_swipe = 0;
		m_LPU237_wait_swipe_with_waits = 0;
		m_LPU237_wait_swipe_with_callback = 0;
		m_LPU237_get_data = 0;
	}

	return b_result;
}

bool dll_lpu237::unload()
{
	bool b_result = false;

	do{
		if( !m_b_setup_ok )
			continue;
		//
		dlclose( m_h_dll );
		m_h_dll = NULL;
		m_LPU237_dll_on = 0;
		m_LPU237_dll_off = 0;
		m_LPU237_get_list_w = 0;
		m_LPU237_get_list_a = 0;
		m_LPU237_open_w = 0;
		m_LPU237_open_a = 0;
		m_LPU237_close = 0;
		m_LPU237_get_id = 0;
		m_LPU237_enable = 0;
		m_LPU237_disable = 0;
		m_LPU237_cancel_wait_swipe = 0;
		m_LPU237_wait_swipe_with_waits = 0;
		m_LPU237_wait_swipe_with_callback = 0;
		m_LPU237_get_data = 0;
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool dll_lpu237::is_setup_ok()
{
	return m_b_setup_ok;
}

	unsigned long dll_lpu237::LPU237_dll_on()
	{
		if( m_LPU237_dll_on )
			return m_LPU237_dll_on();
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_dll_off()
	{
		if( m_LPU237_dll_off )
			return m_LPU237_dll_off();
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_get_list_w(wchar_t *ss_dev_path)
	{
		if( m_LPU237_get_list_w )
			return m_LPU237_get_list_w(ss_dev_path);
		else
			return LPU237_DLL_RESULT_ERROR;
	}
	unsigned long dll_lpu237::LPU237_get_list_a(char *ss_dev_path)
	{
		if( m_LPU237_get_list_a )
			return m_LPU237_get_list_a(ss_dev_path);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	LPU237_HANDLE dll_lpu237::LPU237_open_w( const wchar_t *s_dev_path )
	{
		if( m_LPU237_open_w )
			return m_LPU237_open_w(s_dev_path);
		else
			return NULL;
	}
	LPU237_HANDLE dll_lpu237::LPU237_open_a( const char *s_dev_path )
	{
		if( m_LPU237_open_a )
			return m_LPU237_open_a(s_dev_path);
		else
			return NULL;
	}

	unsigned long dll_lpu237::LPU237_close( LPU237_HANDLE h_dev )
	{
		if( m_LPU237_close )
			return m_LPU237_close(h_dev);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id )
	{
		if( m_LPU237_get_id )
			return m_LPU237_get_id(h_dev,s_id);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_enable( LPU237_HANDLE h_dev )
	{
		if( m_LPU237_enable )
			return m_LPU237_enable(h_dev);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_disable( LPU237_HANDLE h_dev )
	{
		if( m_LPU237_disable )
			return m_LPU237_disable(h_dev);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_cancel_wait_swipe( LPU237_HANDLE h_dev )
	{
		if( m_LPU237_cancel_wait_swipe )
			return m_LPU237_cancel_wait_swipe(h_dev);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev )
	{
		if( m_LPU237_wait_swipe_with_waits )
			return m_LPU237_wait_swipe_with_waits(h_dev);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter )
	{
		if( m_LPU237_wait_swipe_with_callback )
			return m_LPU237_wait_swipe_with_callback(h_dev,p_fun,p_parameter);
		else
			return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long dll_lpu237::LPU237_get_data( unsigned long dw_buffer_index, unsigned long dw_iso_track, unsigned char *s_track_data )
	{
		if( m_LPU237_get_data )
			return m_LPU237_get_data(dw_buffer_index,dw_iso_track,s_track_data);
		else
			return LPU237_DLL_RESULT_ERROR;
	}


	dll_lpu237::dll_lpu237() :
	m_b_setup_ok(false)
	,m_h_dll(0)
	,m_LPU237_dll_on(0)
	,m_LPU237_dll_off(0)
	,m_LPU237_get_list_w(0)
	,m_LPU237_get_list_a(0)
	,m_LPU237_open_w(0)
	,m_LPU237_open_a(0)
	,m_LPU237_close(0)
	,m_LPU237_get_id(0)
	,m_LPU237_enable(0)
	,m_LPU237_disable(0)
	,m_LPU237_cancel_wait_swipe(0)
	,m_LPU237_wait_swipe_with_waits(0)
	,m_LPU237_wait_swipe_with_callback(0)
	,m_LPU237_get_data(0)
	{

	}

