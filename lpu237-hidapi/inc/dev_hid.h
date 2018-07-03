/*
 * dev_hid.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 */

#ifndef INC_DEV_HID_H_
#define INC_DEV_HID_H_

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <list>
#include <string.h>

#include "hidapi.h"
#include "shared.h"

using namespace std;

class dev_hid
{
public:
	typedef	hid_device*	type_handle;
	typedef	map<int,int>	type_map_report_size;

	typedef struct tag_hid_device_info {
		shared::type_ptr_string ptr_s_path;
		unsigned short vendor_id;
		unsigned short product_id;
		shared::type_ptr_wstring ptr_ws_serial_number;
		unsigned short release_number;
		shared::type_ptr_wstring ptr_ws_manufacturer_string;
		shared::type_ptr_wstring ptr_ws_product_string;
		unsigned short usage_page;
		unsigned short usage;
		int interface_number;
	}type_hid_device_info;

	typedef	list<type_hid_device_info>	type_list_hid_device_info;

public:
	static int get_device_list( type_list_hid_device_info & list_dev )
	{
		int n_dev(0);

		do{
			list_dev.clear();

			struct hid_device_info *devs = NULL, *cur_dev=NULL;

			devs = hid_enumerate(0, 0);
			if( devs == NULL ){
				continue;//no device
			}

			cur_dev = devs;
			while(cur_dev) {

				type_hid_device_info info =
				{
					shared::type_ptr_string( new string(cur_dev->path) )
					,cur_dev->vendor_id
					,cur_dev->product_id
					,shared::type_ptr_wstring( new wstring(cur_dev->serial_number) )
					,cur_dev->release_number
					,shared::type_ptr_wstring( new wstring(cur_dev->manufacturer_string) )
					,shared::type_ptr_wstring( new wstring(cur_dev->product_string) )
					,cur_dev->usage_page
					,cur_dev->usage
					,cur_dev->interface_number
				};
				list_dev.push_back(info);

				n_dev++;
				cur_dev = cur_dev->next;
			}//end while
			hid_free_enumeration(devs);

		}while(0);

		return n_dev;
	}

public:
	virtual const string & get_class_name()=0;
	virtual int get_vid()=0;
	virtual int get_pid()=0;
	virtual int get_inf()=0;

public:
	dev_hid()
	{
		m_map_in_report_size[0] = 64;
		m_map_out_report_size[0] = 64;
	}

	dev_hid(const type_map_report_size & map_in_report_size, const type_map_report_size & map_out_report_size) :
		m_map_in_report_size(map_in_report_size)
		,m_map_out_report_size(map_out_report_size)
	{
	}
	virtual ~dev_hid()
	{

	}

	type_handle open( const string & s_path )
	{
		type_handle h_dev(NULL);

		do{
			if( s_path.empty() )
				continue;

			h_dev = hid_open_path( s_path.c_str() );
			if( h_dev == NULL )
				continue;

		}while(0);
		return h_dev;
	}

	bool close( type_handle h_dev )
	{
		bool b_result(false);

		do{
			if( h_dev == NULL )
				hid_close(h_dev);

			b_result = true;
		}while(0);

		return b_result;
	}

protected:

	shared::type_result_fun write( type_handle h_dev, const shared::type_buffer & v_tx,int n_report_number = 0)
	{
		shared::type_result_fun result(shared::result_fun_ing);
		do{
			if( h_dev == NULL ){
				result = shared::result_fun_error;
				continue;
			}

			if( v_tx.empty() ){
				result = shared::result_fun_success;
				continue;
			}

			type_map_report_size::iterator it = m_map_out_report_size.find(n_report_number);
			if( it == end(m_map_out_report_size) ){
				result = shared::result_fun_error;
				continue;
			}
			if( it->second <= 0 ){
				result = shared::result_fun_error;
				continue;
			}

			int n_out_report = it->second;
			shared::type_buffer v_out_report(n_out_report+1,0);
			v_out_report[0] = (unsigned char)n_report_number;//set report number

			int n_offset = 0;
			int n_tx = (int)v_tx.size();
			int	n_written = 0;

			do{

				if( (n_tx-n_offset) > n_out_report ){
					memcpy( &v_out_report[1], &v_tx[n_offset], (unsigned long)n_out_report );
				}
				else{
					memcpy( &v_out_report[1], &v_tx[n_offset], (n_tx-n_offset) );
				}

				n_written = hid_write( h_dev,&v_out_report[0], n_out_report+1 );
				if( n_written < 0 ){
					result = shared::result_fun_error;
					break;//exit while
				}

				n_offset += (n_written-1);
				fill( begin(v_out_report), end(v_out_report), 0 );
				v_out_report[0] = (unsigned char)n_report_number;//set report number
			}while( n_tx > n_offset );

			if(n_tx > n_offset )
				continue;//error
			//
			result = shared::result_fun_success;
		}while(0);

		return result;
	}
	shared::type_result_fun read( type_handle h_dev, shared::type_buffer & v_rx ,int n_report_number = 0 )
	{
		shared::type_result_fun result(shared::result_fun_ing);

		do{
			if( h_dev == NULL ){
				result = shared::result_fun_error;
				continue;
			}
			type_map_report_size::iterator it = m_map_in_report_size.find(n_report_number);
			if( it == end(m_map_in_report_size) ){
				result = shared::result_fun_error;
				continue;
			}
			if( it->second <= 0 ){
				result = shared::result_fun_error;
				continue;
			}

			int n_in_report(it->second);
			int n_in_report_add(0);

			if( m_map_in_report_size.size() > 1 ){
				n_in_report_add = 1;
			}
			shared::type_buffer v_in_report(n_in_report+n_in_report_add,0);
			if( m_map_in_report_size.size() > 1 ){
				v_in_report[0] = (unsigned char)n_report_number;//set report number
			}

			int n_read(0);

			n_read = hid_read(h_dev, &v_in_report[0], n_in_report+n_in_report_add);
			if( n_read < 0 ){
				result = shared::result_fun_error;
				continue;
			}

			if( n_read >= (n_in_report+n_in_report_add) )
				result = shared::result_fun_success;
			//
			int n_rx = v_rx.size();
			v_rx.resize( n_rx+n_read );
			memcpy( &v_rx[n_rx],&v_in_report[0],n_read );
		}while(0);

		return result;
	}

protected :
	type_map_report_size m_map_in_report_size;
	type_map_report_size m_map_out_report_size;
};




#endif /* INC_DEV_HID_H_ */
