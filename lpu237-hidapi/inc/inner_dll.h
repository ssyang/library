/*
 * inner_dll.h
 *
 *  Created on: 2018. 6. 19.
 *      Author: totoro
 */

#ifndef INC_INNER_DLL_H_
#define INC_INNER_DLL_H_

#include "el_hidapi.h"

#include <string>
#include <dlfcn.h>

using namespace std;

class inner_dll
{
public:
	static inner_dll & get_instance()
	{
		static inner_dll dll;

		return dll;
	}

	~inner_dll()
	{
		if( m_h_dll )
			dlclose( m_h_dll );
	}

	bool load(const string & s_dll)
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
			m_fun_init = (type_hid_init)dlsym( m_h_dll, "hid_init");
			if( m_fun_init == NULL )
				continue;
			m_fun_exit = (type_hid_exit)dlsym( m_h_dll, "hid_exit");
			if( m_fun_exit == NULL )
				continue;
			m_fun_enumerate = (type_hid_enumerate)dlsym( m_h_dll, "hid_enumerate");
			if( m_fun_enumerate == NULL )
				continue;
			m_fun_free_enumeration = (type_hid_free_enumeration)dlsym( m_h_dll, "hid_free_enumeration");
			if( m_fun_free_enumeration == NULL )
				continue;
			m_fun_open_ex = (type_hid_open_ex)dlsym( m_h_dll, "hid_open_ex");
			if( m_fun_open_ex == NULL )
				continue;
			m_fun_open = (type_hid_open)dlsym( m_h_dll, "hid_open");
			if( m_fun_open == NULL )
				continue;
			m_fun_open_path = (type_hid_open_path)dlsym( m_h_dll, "hid_open_path");
			if( m_fun_open_path == NULL )
				continue;
			m_fun_write = (type_hid_write)dlsym( m_h_dll, "hid_write");
			if( m_fun_write == NULL )
				continue;
			m_fun_read_timeout = (type_hid_read_timeout)dlsym( m_h_dll, "hid_read_timeout");
			if( m_fun_read_timeout == NULL )
				continue;
			m_fun_read = (type_hid_read)dlsym( m_h_dll, "hid_read");
			if( m_fun_read == NULL )
				continue;
			m_fun_set_nonblocking = (type_hid_set_nonblocking)dlsym( m_h_dll, "hid_set_nonblocking");
			if( m_fun_set_nonblocking == NULL )
				continue;
			m_fun_send_feature_report = (type_hid_send_feature_report)dlsym( m_h_dll, "hid_send_feature_report");
			if( m_fun_send_feature_report == NULL )
				continue;
			m_fun_get_feature_report = (type_hid_get_feature_report)dlsym( m_h_dll, "hid_get_feature_report");
			if( m_fun_get_feature_report == NULL )
				continue;
			m_fun_close = (type_hid_close)dlsym( m_h_dll, "hid_close");
			if( m_fun_close == NULL )
				continue;
			m_fun_get_manufacturer_string = (type_hid_get_manufacturer_string)dlsym( m_h_dll, "hid_get_manufacturer_string");
			if( m_fun_get_manufacturer_string == NULL )
				continue;
			m_fun_get_product_string = (type_hid_get_product_string)dlsym( m_h_dll, "hid_get_product_string");
			if( m_fun_get_product_string == NULL )
				continue;
			m_fun_get_serial_number_string = (type_hid_get_serial_number_string)dlsym( m_h_dll, "hid_get_serial_number_string");
			if( m_fun_get_serial_number_string == NULL )
				continue;
			m_fun_get_indexed_string = (type_hid_get_indexed_string)dlsym( m_h_dll, "hid_get_indexed_string");
			if( m_fun_get_indexed_string == NULL )
				continue;
			m_fun_error = (type_hid_error)dlsym( m_h_dll, "hid_error");
			if( m_fun_error == NULL )
				continue;
			//
			m_b_setup_ok = true;
			b_result = true;
		}while(0);

		if( m_b_setup_ok == false && m_h_dll != NULL  ){
			dlclose( m_h_dll );
			m_h_dll = NULL;
			m_fun_init = 0;
			m_fun_exit = 0;
			m_fun_enumerate = 0;
			m_fun_free_enumeration = 0;
			m_fun_open_ex = 0;
			m_fun_open = 0;
			m_fun_open_path = 0;
			m_fun_write = 0;
			m_fun_read_timeout = 0;
			m_fun_read = 0;
			m_fun_set_nonblocking = 0;
			m_fun_send_feature_report = 0;
			m_fun_get_feature_report = 0;
			m_fun_close = 0;
			m_fun_get_manufacturer_string = 0;
			m_fun_get_product_string = 0;
			m_fun_get_serial_number_string = 0;
			m_fun_get_indexed_string = 0;
			m_fun_error = 0;
		}

		return b_result;
	}

	bool unload()
	{
		bool b_result = false;

		do{
			if( !m_b_setup_ok )
				continue;
			//
			dlclose( m_h_dll );
			m_h_dll = NULL;
			m_fun_init = 0;
			m_fun_exit = 0;
			m_fun_enumerate = 0;
			m_fun_free_enumeration = 0;
			m_fun_open_ex = 0;
			m_fun_open = 0;
			m_fun_open_path = 0;
			m_fun_write = 0;
			m_fun_read_timeout = 0;
			m_fun_read = 0;
			m_fun_set_nonblocking = 0;
			m_fun_send_feature_report = 0;
			m_fun_get_feature_report = 0;
			m_fun_close = 0;
			m_fun_get_manufacturer_string = 0;
			m_fun_get_product_string = 0;
			m_fun_get_serial_number_string = 0;
			m_fun_get_indexed_string = 0;
			m_fun_error = 0;
			//
			b_result = true;
		}while(0);

		return b_result;
	}

	bool is_setup_ok()
	{
		return m_b_setup_ok;
	}

	int hid_init(void)
	{
		if(m_fun_init)
			return m_fun_init();
		else
			return -1;
	}
	int hid_exit(void)
	{
		if(m_fun_exit)
			return m_fun_exit();
		else
			return -1;
	}
	struct hid_device_info *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
	{
		if( m_fun_enumerate )
			return m_fun_enumerate( vendor_id, product_id );
		else
			return NULL;
	}
	void hid_free_enumeration(struct hid_device_info *devs)
	{
		if( m_fun_free_enumeration )
			m_fun_free_enumeration( devs );
	}
	hid_device *hid_open_ex(unsigned short vendor_id, unsigned short product_id, int interface_number, wchar_t *serial_number)
	{
		if( m_fun_open_ex )
			return m_fun_open_ex( vendor_id, product_id,interface_number, serial_number );
		else
			return NULL;
	}
	hid_device *hid_open(unsigned short vendor_id, unsigned short product_id, wchar_t *serial_number)
	{
		if( m_fun_open )
			return m_fun_open( vendor_id,product_id,serial_number );
		else
			return NULL;
	}
	hid_device *hid_open_path(const char *path)
	{
		if( m_fun_open_path )
			return m_fun_open_path( path );
		else
			return NULL;
	}
	int hid_write(hid_device *device, const unsigned char *data, size_t length)
	{
		if( m_fun_write )
			return m_fun_write( device, data, length );
		else
			return -1;
	}
	int hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
	{
		if( m_fun_read_timeout )
			return m_fun_read_timeout( dev, data, length, milliseconds );
		else
			return -1;
	}
	int hid_read(hid_device *device, unsigned char *data, size_t length)
	{
		if( m_fun_read )
			return m_fun_read( device, data, length );
		else
			return -1;
	}
	int hid_set_nonblocking(hid_device *device, int nonblock)
	{
		if( m_fun_set_nonblocking )
			return m_fun_set_nonblocking( device, nonblock );
		else
			return -1;
	}
	int hid_send_feature_report(hid_device *device, const unsigned char *data, size_t length)
	{
		if( m_fun_send_feature_report )
			return m_fun_send_feature_report( device, data, length );
		else
			return -1;
	}
	int hid_get_feature_report(hid_device *device, unsigned char *data, size_t length)
	{
		if( m_fun_get_feature_report )
			return m_fun_get_feature_report( device, data, length );
		else
			return -1;
	}
	void hid_close(hid_device *device)
	{
		if( m_fun_close )
			m_fun_close( device );
	}
	int hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen)
	{
		if( m_fun_get_manufacturer_string )
			return m_fun_get_manufacturer_string( device, string, maxlen );
		else
			return -1;
	}
	int hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen)
	{
		if( m_fun_get_product_string )
			return m_fun_get_product_string( device, string, maxlen );
		else
			return -1;
	}
	int hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen)
	{
		if( m_fun_get_serial_number_string )
			return m_fun_get_serial_number_string( device, string, maxlen );
		else
			return -1;
	}
	int hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen)
	{
		if( m_fun_get_indexed_string )
			return m_fun_get_indexed_string( device, string_index, string, maxlen );
		else
			return -1;
	}
	const wchar_t* hid_error(hid_device *device)
	{
		if( m_fun_error )
			return hid_error( device );
		else
			return NULL;
	}

private:
	inner_dll() :
		m_b_setup_ok(false)
		,m_h_dll(0)
		,m_fun_init(0)
		,m_fun_exit(0)
		,m_fun_enumerate(0)
		,m_fun_free_enumeration(0)
		,m_fun_open_ex(0)
		,m_fun_open(0)
		,m_fun_open_path(0)
		,m_fun_write(0)
		,m_fun_read_timeout(0)
		,m_fun_read(0)
		,m_fun_set_nonblocking(0)
		,m_fun_send_feature_report(0)
		,m_fun_get_feature_report(0)
		,m_fun_close(0)
		,m_fun_get_manufacturer_string(0)
		,m_fun_get_product_string(0)
		,m_fun_get_serial_number_string(0)
		,m_fun_get_indexed_string(0)
		,m_fun_error(0)
	{


	}

private:
	bool m_b_setup_ok;
	void *m_h_dll;

	type_hid_init m_fun_init;
	type_hid_exit m_fun_exit;
	type_hid_enumerate m_fun_enumerate;
	type_hid_free_enumeration m_fun_free_enumeration;
	type_hid_open_ex m_fun_open_ex;
	type_hid_open m_fun_open;
	type_hid_open_path m_fun_open_path;
	type_hid_write m_fun_write;
	type_hid_read_timeout m_fun_read_timeout;
	type_hid_read m_fun_read;
	type_hid_set_nonblocking m_fun_set_nonblocking;
	type_hid_send_feature_report m_fun_send_feature_report;
	type_hid_get_feature_report m_fun_get_feature_report;
	type_hid_close m_fun_close;
	type_hid_get_manufacturer_string m_fun_get_manufacturer_string;
	type_hid_get_product_string m_fun_get_product_string;
	type_hid_get_serial_number_string m_fun_get_serial_number_string;
	type_hid_get_indexed_string m_fun_get_indexed_string;
	type_hid_error m_fun_error;


private://don't call these methods
	inner_dll( const inner_dll & );
	inner_dll & operator=( const inner_dll & );

};

#endif /* INC_INNER_DLL_H_ */
