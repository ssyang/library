/*
 * inner_util.h
 *
 *  Created on: 2018. 6. 21.
 *      Author: totoro
 */

#ifndef INC_INNER_UTIL_H_
#define INC_INNER_UTIL_H_

#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string>
#include <libgen.h>

#include <vector>

using namespace std;

class inner_util
{
public:
	static string get_current_module_fullpath()
	{
		char arg1[20];
		char exepath[PATH_MAX + 1] = {0};

		sprintf( arg1, "/proc/%d/exe", getpid() );
		ssize_t len = readlink( arg1, exepath, sizeof(exepath)-1 );
		return string( exepath );
	}

	static string get_changed_fullpath( const string & s_full_path, const string & s_new_basename )
	{
		string s_new_full_path;

		do{
			if( s_full_path.empty() ){
				s_new_full_path = s_new_basename;
				continue;
			}
			if( s_new_basename.empty() ){
				s_new_full_path = s_full_path;
				continue;
			}
			//
			vector<char> v_full_path(s_full_path.size()+1,0 );
			strcpy( &v_full_path[0], s_full_path.c_str() );
			string s_dir( dirname(&v_full_path[0]) );

			s_new_full_path = s_dir + '/' + s_new_basename;

		}while(0);

		return s_new_full_path;
	}
private:

public:
	~inner_util()
	{

	}
private:

private://don't call these method
	inner_util();
	inner_util( const inner_util & );
	inner_util & operator=(const inner_util & );

};

#endif /* INC_INNER_UTIL_H_ */
