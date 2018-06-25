/*
 * inner_log.h
 *
 *  Created on: 2018. 6. 25.
 *      Author: totoro
 */

#ifndef INC_INNER_LOG_H_
#define INC_INNER_LOG_H_

#include "log_util.h"

#include <string>
#include <stdarg.h>

using namespace std;

class inner_log
{
public:
	static inner_log & get_instance()
	{
		static inner_log obj( string("/var/tmp/elpusk/lpu237"), string("hidapi") );
		return obj;
	}

	~inner_log()
	{

	}

	bool is_setup_ok()
	{
		return m_b_setup_ok;
	}

	int set_level(int n_level)
	{
		return LOGsetLevel( n_level );
	}

	int get_level()
	{
		return LOGgetLevel();
	}

	//logging function
	void log_trace( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_TRACE ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('T',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	void log_debug( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_DEBUG ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('D',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	void log_info( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_INFO ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('I',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	void log_warning( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_WARNING ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('W',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	void log_error( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_ERROR ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('E',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	void log_fatal( const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_FATAL ){
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('F',__FILE__,__func__,__LINE__,fmt, ap);
			va_end(ap);
		}
	}
	//

private:

	inner_log(const string & s_dir,const string & s_prefix ) :
		m_s_dir(s_dir)
		,m_s_prefix(s_prefix)
		,m_b_setup_ok(false)
	{
		if( LOGsetInfo(m_s_dir.c_str(), m_s_prefix.c_str()) == 0 )
			m_b_setup_ok = true;
	}


private:
	string m_s_dir;
	string m_s_prefix;
	bool m_b_setup_ok;

private://don't call these methods
	inner_log();
	inner_log( const inner_log & );
	inner_log & operator=( const inner_log & );

};



#endif /* INC_INNER_LOG_H_ */
