/*
 * dev_lpu237.cpp
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#include <unistd.h>

#include "info_sys.h"
#include "dev_lpu237.h"

const string & get_class_name()
{
	static string s_name("dev_lpu237");
	return s_name;
}

int get_vid()
{
	return 0x134b;
}

int get_pid()
{
	return 0x0206;
}

int get_inf()
{
	return 1;
}

dev_lpu237::dev_lpu237() :	dev_hid()
,m_name(SYSTEM_SIZE_NAME,0)
,m_dw_system_type(ft_none)
,m_bootloader_type(bl_none)
,m_b_embedded_language_map(true)
,m_n_msd_run_time(15000)
,m_parameter()
,m_dw_scr_transaction_counter(0)
{
	m_map_in_report_size[0] = 220;
}

dev_lpu237::~dev_lpu237()
{

}

string dev_lpu237::get_system_type_string()
{
	string s_type;

	if( is_ibutton_reader(m_dw_system_type))
		s_type = "ibutton reader";
	else if( is_standard_reader(m_dw_system_type))
		s_type = "standard MSR";
	else if( is_compact_reader(m_dw_system_type) )
		s_type = "compact MSR";
	else
		s_type = "unknown system type";
	return s_type;
}
string dev_lpu237::get_bootloader_type_string()
{
	string s_type;

	switch(m_bootloader_type ){
	case bl_drive:
		s_type = "bootloader MSD type";
		break;
	case bl_hid:
		s_type = "bootloader hid type";
		break;
	default:
		s_type = "bootloader unknown type";
		break;
	}//end switch
	return s_type;
}
string dev_lpu237::get_interface_string()
{
	string s_interface;
	inner_version v3604(3,6,0,4);

	switch( m_parameter.get_interface()){
	case inf_usb_kb:
		s_interface = "usb keyboard";
		break;
	case inf_usb_msr:
		s_interface = "usb hid vendor";
		break;
	case inf_uart:
		s_interface = "uart";
		break;
	case inf_ps2_stand_alone:
		if( m_version < v3604 )
			s_interface = "ps2 keyboard emulation";
		else
			s_interface = "ps2 keyboard";
		break;
	case inf_ps2_bypass:
		if( m_version < v3604 )
			s_interface = "ps2 keyboard bypass";
		else
			s_interface = "ps2 keyboard";
		break;
	case inf_by_hw_setting:
		s_interface = "determination by hw setting";
		break;
	default:
		break;
	}//end switch

	return s_interface;
}
string dev_lpu237::get_language_string()
{
	string s_lang;

	switch( m_parameter.get_language_map_index() ){
	case lang_map_index_english:
		s_lang = "english";
		break;
	case lang_map_index_spanish:
		s_lang = "spanish";
		break;
	case lang_map_index_danish:
		s_lang = "danish";
		break;
	case lang_map_index_french:
		s_lang = "french";
		break;
	case lang_map_index_german:
		s_lang = "german";
		break;
	case lang_map_index_italian:
		s_lang = "italian";
		break;
	case lang_map_index_norwegian:
		s_lang = "norwegian";
		break;
	case lang_map_index_swedish:
		s_lang = "swedish";
		break;
	case lang_map_index_uk_english:
		s_lang = "uk english";
		break;
	case lang_map_index_israel:
		s_lang = "israel";
		break;
	case lang_map_index_turkey:
		s_lang = "turkey";
		break;
	default:
		s_lang = "";
		break;
	}//end switch
	return s_lang;
}
string dev_lpu237::get_name_string()
{
	string s_name;

	for_each(begin(m_name),end(m_name),[&](unsigned char c){
		s_name.push_back((char)c);
	});

	return s_name;
}
string dev_lpu237::get_global_prepostfix_send_condition_string()
{
	string s_condition("anu track, no error");

	if( m_parameter.get_global_preposfix_send_condition())
		s_condition = "all track no error";
	//
	return s_condition;
}

bool dev_lpu237::send_request(
		dev_hid::type_handle h_dev
		, type_msr_cmd cmd
		, unsigned char c_sub
		, int n_data
		, const unsigned char *s_data
		, unsigned char *s_response /*= NULL*/
		)
{
	bool b_result(false);

	do{
		shared::type_result_fun result_fun(shared::result_fun_ing);

		if(n_data<0)
			n_data = 0;

		shared::type_buffer v_tx(MSR_SIZE_HOST_PACKET_HEADER+n_data);

		type_msr_host_packet *p_req=(type_msr_host_packet *)&v_tx[0];
		p_req->c_cmd = cmd;
		p_req->c_sub = c_sub;
		p_req->c_len = (unsigned char)n_data;
		memcpy( p_req->s_data, s_data, n_data );

		do{
			result_fun =  write( h_dev, v_tx );
		}while( result_fun != shared::result_fun_ing );
		//
		if( result_fun != shared::result_fun_success )
			continue;
		//
		shared::type_buffer v_rx(0);
		do{
			result_fun = read( h_dev, v_rx );
			usleep(1000*3);
		}while( result_fun != shared::result_fun_ing );

		if( result_fun != shared::result_fun_success )
			continue;

		if( s_response )
			memcpy( s_response, &v_rx[0], v_rx.size() );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::send_request(
		dev_hid::type_handle h_dev
		, const type_msr_host_packet & req
		, type_msr_host_packet *p_rsp/* = NULL*/
		)
{
	return send_request( h_dev, (type_msr_cmd)req.c_cmd, req.c_sub, (int)req.c_len, req.s_data , p_rsp );
}

bool dev_lpu237::df_bypass_uart( dev_hid::type_handle h_dev, const shared::type_buffer & v_tx_data, shared::type_buffer & v_rx_data )
{
	bool b_result(false);

	do{
		int n_out_report  = 0;
		dev_hid::type_map_report_size::iterator it = m_map_out_report_size.find(0);
		if( it == end(m_map_out_report_size) )
			continue;

		n_out_report = it->second;

		shared::type_buffer v_tx(n_out_report-MSR_SIZE_HOST_PACKET_HEADER,0);
		unsigned char c_chain(0);
		unsigned long dw_total(v_tx_data.size());
		int n_remainder = (int)dw_total;
		unsigned long dw_offset(0);
		int n_data(0);
		int n_tx(0);

		memcpy(&v_tx[0],&m_dw_scr_transaction_counter,sizeof(unsigned long));
		memcpy(&v_tx[sizeof(unsigned long)],&dw_total,sizeof(unsigned long));

		n_data = v_tx.size()-(sizeof(unsigned long)*2);
		if( n_data > v_tx_data.size() )
			n_data = v_tx_data.size();
		memcpy(&v_tx[sizeof(unsigned long)*2],&v_tx_data[dw_offset],n_data);
		n_tx = n_data + sizeof(unsigned long)*2;

		do{

		}while( n_remainder > 0 );


		b_result = true;
	}while(0);
	return b_result;
}
bool dev_lpu237::df_debug_interface()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_goto_bootloader()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_enter_config()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_leave_config()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_apply_config()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_enter_opos()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_leave_opos()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_get_system_parameters()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_system_parameters()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_interface()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_language()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_buzzer()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_enable_track()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_global_prefix()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_global_postfix()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_prefix()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_postfix()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_prefix_ibutton()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_postfix_ibutton()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_prefix_uart()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_postfix_uart()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_global_prepostfix_send_condition()
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_enable_track( type_msr_track_number track )
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_prefix( type_msr_track_number track )
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_postfix( type_msr_track_number track )
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_mmd100_raw( type_msr_track_number track )
{
	bool b_result(false);

	do{

		b_result = true;
	}while(0);
	return b_result;
}



