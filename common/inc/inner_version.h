
/*
 * share.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_INNER_VERSION_H_
#define INC_INNER_VERSION_H_

#include <vector>
#include <string>
#include <sstream>

using namespace std;

class inner_version
{
public:
	typedef	vector<unsigned char>	type_version;

	enum{
		const_size_of_type_version = 4
	};

public:
	inner_version() : m_version(const_size_of_type_version,0)
	{

	}

	inner_version(const inner_version & v )
	{
		m_version = v.m_version;
	}

	inner_version(const type_version & v_version ) : m_version(const_size_of_type_version,0)
	{
		int n_loop(v_version.size());

		if( v_version.size() > const_size_of_type_version ){
			n_loop = const_size_of_type_version;
		}

		for( int i = 0; i<n_loop; i++ ){
			m_version[i] = v_version[i];
		}//end for
	}

	inner_version( unsigned char c_major, unsigned char c_minor, unsigned char c_fix, unsigned char c_build ):
		m_version(const_size_of_type_version,0)
	{
		m_version[0] = c_major;
		m_version[1] = c_minor;
		m_version[2] = c_fix;
		m_version[3] = c_build;
	}
	~inner_version()
	{

	}

	inner_version & operator=( const inner_version & v )
	{
		this->m_version = v.m_version;
		return *this;
	}

	inner_version & operator=( const type_version & v )
	{
		this->m_version = v;
		return *this;
	}

	bool operator==( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] != v.m_version[i])
				return false;
		}
		return true;
	}

	bool operator!=( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] != v.m_version[i])
				return true;
		}
		return false;
	}

	bool operator>( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] > v.m_version[i])
				return true;
			else if( this->m_version[i] < v.m_version[i])
				return false;
		}
		return false;
	}

	bool operator>=( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] > v.m_version[i])
				return true;
			else if( this->m_version[i] < v.m_version[i])
				return false;
		}
		return true;
	}

	bool operator<( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] > v.m_version[i])
				return false;
			else if( this->m_version[i] < v.m_version[i])
				return true;
		}
		return false;
	}

	bool operator<=( const inner_version & v )
	{
		for( int i = 0; i<const_size_of_type_version; i++ ){
			if( this->m_version[i] > v.m_version[i])
				return false;
			else if( this->m_version[i] < v.m_version[i])
				return true;
		}
		return true;
	}

	unsigned char get_major(){	return m_version[0];	}
	unsigned char get_minor(){	return m_version[1];	}
	unsigned char get_fix(){	return m_version[2];	}
	unsigned char get_build(){	return m_version[3];	}
	//
	string get_string()
	{
		ostringstream oss(ostringstream::out);
		oss << m_version[0] << "." << m_version[1] << "." << m_version[2] << "." << m_version[3];
		return oss.str();
	}

	void set( unsigned char c_major, unsigned char c_minor, unsigned char c_fix, unsigned char c_build )
	{
		m_version[0] = c_major;
		m_version[1] = c_minor;
		m_version[2] = c_fix;
		m_version[3] = c_build;
	}
private:
	type_version m_version;
};



#endif /* INC_INNER_VERSION_H_ */
