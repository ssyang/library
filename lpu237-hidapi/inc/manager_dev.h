/*
 * manager_dev.h
 *
 *  Created on: 2018. 7. 6.
 *      Author: totoro
 */

#ifndef INC_MANAGER_DEV_H_
#define INC_MANAGER_DEV_H_

#include <map>
#include <memory>
#include <mutex>

#include "dev_hid.h"
#include "dev_lpu237.h"

using namespace	std;

class manager_dev
{
private:
	typedef	shared_ptr<dev_lpu237>		type_ptr_dev_lpu237;
	typedef	map<dev_hid::type_handle, type_ptr_dev_lpu237>	type_map_lpu237;
public:
	static manager_dev & get_instance()
	{
		static manager_dev obj;
		return obj;
	}

	~manager_dev(){}

	dev_lpu237 *get_device( dev_hid::type_handle h_dev )
	{
		dev_lpu237 *p_obj(NULL);

		do{
			lock_guard<mutex> _lock (m_mutex_map_lpu237);
			if( h_dev == NULL )
				continue;
			//
			type_map_lpu237::iterator it = m_map_lpu237.find(h_dev);
			if( it == end(m_map_lpu237) )
				continue;
			p_obj = it->second.get();
		}while(0);
		return p_obj;
	}

	dev_lpu237 *add_device( dev_hid::type_handle h_dev )
	{
		dev_lpu237 *p_obj(NULL);

		do{
			lock_guard<mutex> _lock (m_mutex_map_lpu237);
			if( h_dev == NULL )
				continue;
			//
			type_map_lpu237::iterator it = m_map_lpu237.find(h_dev);
			if( it != end(m_map_lpu237) )
				continue;

			m_map_lpu237[h_dev] = type_ptr_dev_lpu237( new dev_lpu237());
		}while(0);
		return p_obj;
	}

	bool remove_device( dev_hid::type_handle h_dev )
	{
		bool b_result(false);

		do{
			lock_guard<mutex> _lock (m_mutex_map_lpu237);
			if( h_dev == NULL )
				continue;
			//
			type_map_lpu237::iterator it = m_map_lpu237.find(h_dev);
			if( it == end(m_map_lpu237) )
				continue;

			m_map_lpu237.erase(it);
			b_result = true;
		}while(0);
		return b_result;
	}

private:
	manager_dev(){}

private:
	mutex m_mutex_map_lpu237;
	type_map_lpu237 m_map_lpu237;

private://don't call these method.
	manager_dev( const manager_dev & );
	manager_dev & operator=( const manager_dev & );

};


#endif /* INC_MANAGER_DEV_H_ */
