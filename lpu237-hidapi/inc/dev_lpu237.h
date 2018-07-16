/*
 * dev_lpu237.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_DEV_LPU237_H_
#define INC_DEV_LPU237_H_

#include "dev_hid.h"
#include "inner_version.h"
#include "shared.h"
#include "msr_com.h"
#include "info_msr.h"

#include <algorithm>
#include <vector>

using namespace std;

class dev_lpu237 : public dev_hid
{
public:
	const string & get_class_name();
	int get_vid();
	int get_pid();
	int get_inf();

public:
	dev_lpu237();
	~dev_lpu237();

public:
	enum{
		cont_the_number_of_tracks = 3,
		const_size_uid = 4*4,
		const_default_buzzer_frequency = 2500
	};

	enum type_fun : type_dword
	{
		ft_none	= 0x00000000,
		ft_msr		= 0x00000001,
		ft_ibutton	= 0x00000002
	};

	enum type_bl
	{
		bl_none,
		bl_drive,
		bl_hid
	};

	enum type_interface
	{
		inf_unknown	=-1,
		inf_usb_kb		= 0,
		inf_usb_msr	= 1,
		inf_uart		= 10,
		inf_ps2_stand_alone = 20,
		inf_ps2_bypass	= 21,
		inf_ps2		= 22,
		inf_by_hw_setting	= 100
	};

	enum type_language_map_index
	{
		lang_map_index_unknown	= -1,
		lang_map_index_english	= 0,
		lang_map_index_spanish	= 1,
		lang_map_index_danish	= 2,
		lang_map_index_french	= 3,
		lang_map_index_german	= 4,
		lang_map_index_italian	= 5,
		lang_map_index_norwegian	= 6,
		lang_map_index_swedish	= 7,
		lang_map_index_uk_english	= 8,
		lang_map_index_israel	= 9,
		lang_map_index_turkey	= 10
	};

	enum type_msr_track_number
	{
		mt_unknown = -1,
		mt_iso1	= 0,
		mt_iso2	= 1,
		mt_iso3	= 2
	};

	typedef	shared::type_buffer	type_tag;
	typedef	shared::type_buffer	type_name;
	typedef	shared::type_buffer	type_uid;
	typedef	vector<bool>			type_vector_bool;

	inline bool is_ibutton_reader( type_dword dw_system_type )
	{
		return ( dw_system_type==ft_ibutton );
	}
	inline bool is_standard_reader( type_dword dw_system_type )
	{
		return ( dw_system_type==(ft_msr|ft_ibutton) );
	}
	inline bool is_compact_reader( type_dword dw_system_type )
	{
		return ( dw_system_type==ft_msr );
	}

private:
	enum{
		_const_the_number_of_common_df = 12,
		_const_the_number_of_ibutton_uart_tag = 4,
		_const_hid_keymap_address_offset = 0x400,
		_const_ps2_keymap_address_offset = 0x800
	};
	enum{
		_const_io_wait_timeout_slice = 2,
		_const_write_retry_count = 500,
	};

public:
	class config{
	public:
		config():
			m_global_preposfix_send_condition(true)
			,m_interface(inf_usb_kb)
			,m_language_index(lang_map_index_english)
			,m_n_buzzer_frequency(const_default_buzzer_frequency)
			,m_v_enable_track(cont_the_number_of_tracks,true)
		{}

		config(const config & c) : m_v_enable_track(cont_the_number_of_tracks,true)
		{
			m_global_preposfix_send_condition = c.m_global_preposfix_send_condition;
			m_interface = c.m_interface;
			m_language_index = c.m_language_index;
			m_n_buzzer_frequency = c.m_n_buzzer_frequency;


			copy( begin(c.m_v_enable_track), end(c.m_v_enable_track),begin(m_v_enable_track));
			copy( begin(c.m_v_global_prefix), end(c.m_v_global_prefix), back_insert_iterator<type_tag>(m_v_global_prefix) );
			copy( begin(c.m_v_global_postfix), end(c.m_v_global_postfix), back_insert_iterator<type_tag>(m_v_global_postfix) );

			for( int i=0; i<cont_the_number_of_tracks; i++ ){
				copy( begin(c.m_v_private_prefix[i]), end(c.m_v_private_prefix[i]), back_insert_iterator<type_tag>(m_v_private_prefix[i]) );
				copy( begin(c.m_v_private_postfix[i]), end(c.m_v_private_postfix[i]), back_insert_iterator<type_tag>(m_v_private_postfix[i]) );
			}//end for

			copy( begin(c.m_v_prefix_ibutton), end(c.m_v_prefix_ibutton), back_insert_iterator<type_tag>(m_v_prefix_ibutton) );
			copy( begin(c.m_v_postfix_ibutton), end(c.m_v_postfix_ibutton), back_insert_iterator<type_tag>(m_v_postfix_ibutton) );

			copy( begin(c.m_v_prefix_uart), end(c.m_v_prefix_uart), back_insert_iterator<type_tag>(m_v_prefix_uart) );
			copy( begin(c.m_v_postfix_uart), end(c.m_v_postfix_uart), back_insert_iterator<type_tag>(m_v_postfix_uart) );
		}

		config & operator=( const config & c )
		{
			m_global_preposfix_send_condition = c.m_global_preposfix_send_condition;
			m_interface = c.m_interface;
			m_language_index = c.m_language_index;
			m_n_buzzer_frequency = c.m_n_buzzer_frequency;

			copy( begin(c.m_v_enable_track), end(c.m_v_enable_track),begin(m_v_enable_track));
			m_v_global_prefix.clear();	copy( begin(c.m_v_global_prefix), end(c.m_v_global_prefix), back_insert_iterator<type_tag>(m_v_global_prefix) );
			m_v_global_postfix.clear();	copy( begin(c.m_v_global_postfix), end(c.m_v_global_postfix), back_insert_iterator<type_tag>(m_v_global_postfix) );

			for( int i=0; i<cont_the_number_of_tracks; i++ ){
				m_v_private_prefix[i].clear();	copy( begin(c.m_v_private_prefix[i]), end(c.m_v_private_prefix[i]), back_insert_iterator<type_tag>(m_v_private_prefix[i]) );
				m_v_private_postfix[i].clear();	copy( begin(c.m_v_private_postfix[i]), end(c.m_v_private_postfix[i]), back_insert_iterator<type_tag>(m_v_private_postfix[i]) );
			}//end for

			m_v_prefix_ibutton.clear();	copy( begin(c.m_v_prefix_ibutton), end(c.m_v_prefix_ibutton), back_insert_iterator<type_tag>(m_v_prefix_ibutton) );
			m_v_postfix_ibutton.clear();	copy( begin(c.m_v_postfix_ibutton), end(c.m_v_postfix_ibutton), back_insert_iterator<type_tag>(m_v_postfix_ibutton) );

			m_v_prefix_uart.clear();	copy( begin(c.m_v_prefix_uart), end(c.m_v_prefix_uart), back_insert_iterator<type_tag>(m_v_prefix_uart) );
			m_v_postfix_uart.clear();	copy( begin(c.m_v_postfix_uart), end(c.m_v_postfix_uart), back_insert_iterator<type_tag>(m_v_postfix_uart) );
			return *this;
		}

		//getter
		bool get_global_preposfix_send_condition(){	return m_global_preposfix_send_condition;	}
		type_interface get_interface(){	return m_interface;	}
		type_language_map_index get_language_map_index(){	return m_language_index;	}
		int get_buzzer_frequency(){		return m_n_buzzer_frequency;	}
		bool get_enable_tack( type_msr_track_number track ){	return m_v_enable_track[track];	}

		type_tag get_global_prefix(){	return m_v_global_prefix;	}
		type_tag get_global_postfix(){	return m_v_global_postfix;	}

		type_tag get_private_prefix( type_msr_track_number track ){		return m_v_private_prefix[track];}
		type_tag get_private_postfix( type_msr_track_number track ){	return m_v_private_postfix[track];}

		type_tag get_prefix_ibutton(){	return m_v_prefix_ibutton;	}
		type_tag get_postfix_ibutton(){	return m_v_postfix_ibutton;	}

		type_tag get_prefix_uart(){	return m_v_prefix_uart;	}
		type_tag get_postfix_uart(){	return m_v_postfix_uart;	}

		//setter
		void set_global_preposfix_send_condition( bool b_condition )
		{	m_global_preposfix_send_condition = b_condition; }
		void set_interface( type_interface inf ){	m_interface = inf;	}
		void set_language_index( type_language_map_index index ){	m_language_index = index;	}
		void set_buzzer_frequency( int n_freq ){	m_n_buzzer_frequency = n_freq;	}
		void set_enable_track( type_msr_track_number track, bool b_enable = true )
		{
			m_v_enable_track[track] = b_enable;
		}

		void set_global_prefix( const type_tag & tag ){	m_v_global_prefix = tag;	}
		void set_global_postfix( const type_tag & tag ){	m_v_global_postfix = tag;	}
		void set_global_prefix(){	m_v_global_prefix.clear();	}
		void set_global_postfix(){	m_v_global_postfix.clear();	}

		void set_private_prefix( type_msr_track_number track, const type_tag & tag )
		{
			m_v_private_prefix[track] = tag;
		}
		void set_private_postfix( type_msr_track_number track, const type_tag & tag )
		{
			m_v_private_postfix[track] = tag;
		}
		void set_private_prefix( type_msr_track_number track )
		{
			m_v_private_prefix[track].clear();
		}
		void set_private_postfix( type_msr_track_number track )
		{
			m_v_private_postfix[track].clear();
		}

		void set_prefix_ibutton( const type_tag & tag ){	m_v_prefix_ibutton = tag;	}
		void set_postfix_ibutton( const type_tag & tag ){	m_v_postfix_ibutton = tag;	}
		void set_prefix_ibutton(){	m_v_prefix_ibutton.clear();	}
		void set_postfix_ibutton(){	m_v_postfix_ibutton.clear();	}

		void set_prefix_uart( const type_tag & tag ){	m_v_prefix_uart = tag;	}
		void set_postfix_uart( const type_tag & tag ){	m_v_postfix_uart = tag;	}
		void set_prefix_uart(){	m_v_prefix_uart.clear();	}
		void set_postfix_uart(){	m_v_postfix_uart.clear();	}

		void set_global_prefix( const string & s_tag ){	_set_tag( m_v_global_prefix,s_tag );	}
		void set_global_postfix( const string & s_tag ){	_set_tag( m_v_global_postfix,s_tag );	}

		void set_private_prefix( type_msr_track_number track, const string & s_tag )
		{
			_set_tag( m_v_private_prefix[track], s_tag );
		}
		void set_private_postfix( type_msr_track_number track, const string & s_tag )
		{
			_set_tag( m_v_private_postfix[track], s_tag );
		}
		void set_prefix_ibutton( const string & s_tag ){	_set_tag( m_v_prefix_ibutton, s_tag );	}
		void set_postfix_ibutton( const string & s_tag ){	_set_tag( m_v_postfix_ibutton, s_tag );	}
		void set_prefix_uart( const string & s_tag ){	_set_tag( m_v_prefix_uart, s_tag );	}
		void set_postfix_uart( const string & s_tag ){	_set_tag( m_v_postfix_uart, s_tag );	}

	private:
		void _set_tag( type_tag & tag, const string & s_tag )
		{
			do{
				tag.clear();

				if( s_tag.empty() ){
					continue;
				}
				//
				vector<string> v_token;
				size_t next(-1);
				size_t current(-1);
				string delimiters = " ";
				do{
					current = next + 1;
					next = s_tag.find_first_of( delimiters, current );
					v_token.push_back(s_tag.substr( current, next - current ));
				}while (next != string::npos);
				//
				for_each(begin(v_token),end(v_token),[&](string & s_token ){
					try{
						if( s_token.empty() ){
							unsigned char c = (unsigned char)stoul( s_token, nullptr, 0);
							tag.push_back(c);
						}
					}
					catch(invalid_argument e){
						if( s_token.size()==1 ){
							unsigned char c = (unsigned char)s_token[0];
							tag.push_back(0xFF);
							tag.push_back(c);
						}

					}
					catch( out_of_range e ){
						return;
					}
					catch(...){
						return;
					}
				});

			}while(0);
		}
	private:
		bool m_global_preposfix_send_condition;
		type_interface m_interface;
		type_language_map_index m_language_index;
		int m_n_buzzer_frequency;//unit Hz
		type_vector_bool m_v_enable_track;

		type_tag m_v_global_prefix;
		type_tag m_v_global_postfix;

		type_tag m_v_private_prefix[cont_the_number_of_tracks];
		type_tag m_v_private_postfix[cont_the_number_of_tracks];

		type_tag m_v_prefix_ibutton;
		type_tag m_v_postfix_ibutton;

		type_tag m_v_prefix_uart;
		type_tag m_v_postfix_uart;
	};//the end of config class

	string get_system_type_string();
	string get_bootloader_type_string();
	string get_interface_string();
	string get_language_string();
	string get_name_string();
	string get_global_prepostfix_send_condition_string();

	bool df_bypass_uart( dev_hid::type_handle h_dev, const shared::type_buffer & v_tx_data, shared::type_buffer & v_rx_data );
	bool df_debug_interface(dev_hid::type_handle h_dev);
	bool df_goto_bootloader(dev_hid::type_handle h_dev);
	bool df_enter_config(dev_hid::type_handle h_dev);
	bool df_leave_config(dev_hid::type_handle h_dev);
	bool df_apply_config(dev_hid::type_handle h_dev);
	bool df_enter_opos(dev_hid::type_handle h_dev);
	bool df_leave_opos(dev_hid::type_handle h_dev);

	bool df_get_system_parameters(dev_hid::type_handle h_dev);

	bool df_set_system_parameters(dev_hid::type_handle h_dev);

	bool df_set_interface(dev_hid::type_handle h_dev);
	bool df_set_language(dev_hid::type_handle h_dev);
	bool df_set_buzzer(dev_hid::type_handle h_dev);
	bool df_set_enable_track(dev_hid::type_handle h_dev);

	bool df_set_global_prefix(dev_hid::type_handle h_dev);
	bool df_set_global_postfix(dev_hid::type_handle h_dev);
	bool df_set_private_prefix(dev_hid::type_handle h_dev);
	bool df_set_private_postfix(dev_hid::type_handle h_dev);

	bool df_set_prefix_ibutton(dev_hid::type_handle h_dev);
	bool df_set_postfix_ibutton(dev_hid::type_handle h_dev);

	bool df_set_prefix_uart(dev_hid::type_handle h_dev);
	bool df_set_postfix_uart(dev_hid::type_handle h_dev);

	bool df_set_global_prepostfix_send_condition(dev_hid::type_handle h_dev);

	bool df_set_enable_track( dev_hid::type_handle h_dev, type_msr_track_number track );
	bool df_set_private_prefix( dev_hid::type_handle h_dev, type_msr_track_number track );
	bool df_set_private_postfix( dev_hid::type_handle h_dev, type_msr_track_number track );

	bool df_mmd100_raw( dev_hid::type_handle h_dev, const shared::type_buffer & v_tx, shared::type_buffer & v_rx );

	//setter
	dev_lpu237 & set_global_prepostfix_send_condition( bool b_all_track_good )
	{
		m_parameter.set_global_preposfix_send_condition(b_all_track_good);
		return *this;
	}
	dev_lpu237 & set_interface( type_interface inf )
	{
		m_parameter.set_interface( inf );
		return *this;
	}
	dev_lpu237 & set_language( type_language_map_index lang_index )
	{
		m_parameter.set_language_index( lang_index );
		return *this;
	}
	dev_lpu237 & set_buzzer_frequency( int n_freq = const_default_buzzer_frequency )
	{
		m_parameter.set_buzzer_frequency( n_freq );
		return *this;
	}
	dev_lpu237 & set_enable_track( type_msr_track_number track, bool b_enable = true )
	{
		m_parameter.set_enable_track( track, b_enable );
		return *this;
	}
	dev_lpu237 & set_global_prefix( const type_tag & tag )
	{
		m_parameter.set_global_prefix( tag );
		return *this;
	}
	dev_lpu237 & set_global_postfix( const type_tag & tag )
	{
		m_parameter.set_global_postfix( tag );
		return *this;
	}
	dev_lpu237 & set_private_prefix( type_msr_track_number track, const type_tag & tag )
	{
		m_parameter.set_private_prefix( track, tag );
		return *this;
	}
	dev_lpu237 & set_private_postfix( type_msr_track_number track, const type_tag & tag )
	{
		m_parameter.set_private_postfix( track, tag );
		return *this;
	}
	dev_lpu237 & set_ibutton_prefix( const type_tag & tag )
	{
		m_parameter.set_prefix_ibutton( tag );
		return *this;
	}
	dev_lpu237 & set_ibutton_postfix( const type_tag & tag )
	{
		m_parameter.set_postfix_ibutton( tag );
		return *this;
	}
	dev_lpu237 & set_uart_prefix( const type_tag & tag )
	{
		m_parameter.set_prefix_uart( tag );
		return *this;
	}
	dev_lpu237 & set_uart_postfix( const type_tag & tag )
	{
		m_parameter.set_postfix_uart( tag );
		return *this;
	}
	dev_lpu237 & set_parameters( const config & c )
	{
		m_parameter = c;
		return *this;
	}

	//getter
	bool get_global_prepostfix_send_condition(){	return m_parameter.get_global_preposfix_send_condition();	}
	type_name get_name(){	return m_name;	}
	type_uid get_uid(){	return m_uid;		}
	inner_version get_version(){	return m_version;	}
	inner_version get_structure_version(){	return m_structure_version;	}
	type_dword get_system_type(){	return m_dw_system_type;	}
	type_bl get_bootloader_type(){	return m_bootloader_type;	}
	type_interface get_interface(){	return m_parameter.get_interface();	}
	type_language_map_index get_language_map_index(){	return m_parameter.get_language_map_index();	}
	int get_buzzer_frequency(){	return m_parameter.get_buzzer_frequency();	}
	int get_msd_bootloader_runtime(){	return m_n_msd_run_time;	}
	bool get_enable_track( type_msr_track_number track ){	return m_parameter.get_enable_tack(track);	}

	type_tag get_global_prefix(){	return m_parameter.get_global_prefix();	}
	type_tag get_global_postfix(){	return m_parameter.get_global_postfix();	}
	type_tag get_private_prefix( type_msr_track_number track ){	return m_parameter.get_private_prefix(track);	}
	type_tag get_private_postfix( type_msr_track_number track ){	return m_parameter.get_private_postfix(track);	}

	type_tag get_prefix_ibutton(){	return m_parameter.get_prefix_ibutton();	}
	type_tag get_postfix_ibutton(){	return m_parameter.get_postfix_ibutton();	}

	type_tag get_prefix_uart(){	return m_parameter.get_prefix_uart();	}
	type_tag get_postfix_uart(){	return m_parameter.get_postfix_uart();	}

	bool df_get_global_prepostfix_send_condition( dev_hid::type_handle h_dev );
	bool df_get_name( dev_hid::type_handle h_dev );
	bool df_get_uid( dev_hid::type_handle h_dev );
	bool df_get_version_and_system_type( dev_hid::type_handle h_dev );
	bool df_get_version( dev_hid::type_handle h_dev );
	bool df_get_structure_version( dev_hid::type_handle h_dev );
	bool df_get_interface( dev_hid::type_handle h_dev );
	bool df_get_language( dev_hid::type_handle h_dev );
	bool df_get_buzzer( dev_hid::type_handle h_dev );
	bool df_get_msd_runtime( dev_hid::type_handle h_dev );
	bool df_get_enable_track( dev_hid::type_handle h_dev );
	bool df_get_global_prefix( dev_hid::type_handle h_dev );
	bool df_get_global_postfix( dev_hid::type_handle h_dev );
	bool df_get_private_prefix( dev_hid::type_handle h_dev );
	bool df_get_private_postfix( dev_hid::type_handle h_dev );

	bool df_get_prefix_ibutton( dev_hid::type_handle h_dev );
	bool df_get_postfix_ibutton( dev_hid::type_handle h_dev );
	bool df_get_prefix_uart( dev_hid::type_handle h_dev );
	bool df_get_postfix_uart( dev_hid::type_handle h_dev );

	bool df_get_enable_track( dev_hid::type_handle h_dev, type_msr_track_number track );
	bool df_get_private_prefix( dev_hid::type_handle h_dev, type_msr_track_number track );
	bool df_get_private_postfix( dev_hid::type_handle h_dev, type_msr_track_number track );

private:

	bool _df_get(
			dev_hid::type_handle h_dev
			, type_msr_host_packet & rsp
			, type_dword dw_offset
			, type_dword dw_size );


	bool _send_request(
			dev_hid::type_handle h_dev
			, type_msr_cmd cmd
			, unsigned char c_sub
			, int n_data
			, const unsigned char *s_data
			, unsigned char *s_response = NULL
			);

	bool _send_request(
			dev_hid::type_handle h_dev
			, const type_msr_host_packet & req
			, type_msr_host_packet *p_rsp = NULL
			);

	bool _df_set(
			dev_hid::type_handle h_dev
			, type_msr_host_packet & rsp
			, type_dword dw_offset
			, type_dword dw_size
			, unsigned char *ps_data );

	bool _df_set_key_map_table( dev_hid::type_handle h_dev );

	bool _get_tag_from_tag_type( MSR_TAG & out_tag, const type_tag & v_in_tag );
	bool _get_tag_type_from_tag( type_tag & v_out_tag, PMSR_TAG p_in_tag );

private:
	type_uid m_uid;
	type_name m_name;
	inner_version m_version;
	inner_version m_structure_version;
	type_dword m_dw_system_type;
	type_bl m_bootloader_type;
	bool m_b_embedded_language_map;

	int m_n_msd_run_time;

	config m_parameter;

	type_dword m_dw_scr_transaction_counter;

private://don't call these methods
	dev_lpu237(const dev_lpu237 &);
	dev_lpu237 & operator=(const dev_lpu237 &);
};



#endif /* INC_DEV_LPU237_H_ */
