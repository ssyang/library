/*
 * inner_log.h
 *
 *  Created on: 2018. 6. 25.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_INNER_LOG_H_
#define INC_INNER_LOG_H_

#include "log_util.h"
#include "INIReader.h"
#include "inner_util.h"

#include <string>
#include <stdarg.h>
#include <mutex>


#define LOG_TRACE(...)	inner_log::get_instance().log_trace( __FILE__,__func__,__LINE__,__VA_ARGS__)
#define LOG_DEBUG(...)	inner_log::get_instance().log_debug( __FILE__,__func__,__LINE__,__VA_ARGS__)
#define LOG_INFO(...)	inner_log::get_instance().log_info( __FILE__,__func__,__LINE__,__VA_ARGS__)
#define LOG_WARNING(...)	inner_log::get_instance().log_warning( __FILE__,__func__,__LINE__,__VA_ARGS__)
#define LOG_ERROR(...)	inner_log::get_instance().log_error( __FILE__,__func__,__LINE__,__VA_ARGS__)
#define LOG_FATAL(...)	inner_log::get_instance().log_fatal( __FILE__,__func__,__LINE__,__VA_ARGS__)

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
		return LOGsetLevelByIni( n_level );
	}

	int get_level()
	{
		return LOGgetLevelByIni();
	}

	//logging function
	void log_trace( const char *src_file, const char *func, int line_no,const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_TRACE ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('T',src_file,func,line_no,fmt, ap);
			va_end(ap);
		}
	}
	void log_debug( const char *src_file, const char *func, int line_no,const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_DEBUG ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('D',src_file,func,line_no,fmt, ap);
			va_end(ap);
		}
	}
	void log_info( const char *src_file, const char *func, int line_no, const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_INFO ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('I',src_file,func,line_no,fmt, ap);
			va_end(ap);
		}
	}
	void log_warning( const char *src_file, const char *func, int line_no,const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_WARNING ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('W',src_file,func,line_no,fmt, ap);
			va_end(ap);
		}
	}
	void log_error( const char *src_file, const char *func, int line_no,const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_ERROR ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('E',src_file,func,line_no,fmt, ap);
			va_end(ap);
		}
	}
	void log_fatal( const char *src_file, const char *func, int line_no,const char *fmt, ...)
	{
		if( get_level()>=LOG_LVL_FATAL ){
			lock_guard<mutex> _lock (m_mutex);
			va_list ap;
			va_start(ap, fmt);
			LOGlogging('F',src_file,func,line_no,fmt, ap);
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
	mutex m_mutex;

private://don't call these methods
	inner_log();
	inner_log( const inner_log & );
	inner_log & operator=( const inner_log & );

};



#endif /* INC_INNER_LOG_H_ */
