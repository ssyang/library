/*
 * dev_lpu237.cpp
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#include <unistd.h>
#include <stddef.h>

#include "info_sys.h"
#include "dev_lpu237.h"
#include "KBConst.h"

#ifndef	sizeofstructmember
#define sizeofstructmember(s,m)		sizeof(((s *)0)->m)
#endif	//sizeofstructmember

extern unsigned char gASCToHIDKeyMap[HIDKEY_MAP_NUMBER][FOR_CVT_MAX_ASCII_CODE][2];
extern unsigned char gASCToPS2KeyMap[PS2KEY_MAP_NUMBER][FOR_CVT_MAX_ASCII_CODE][2];

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

bool dev_lpu237::_send_request(
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
		//
		type_msr_host_packet *p_rsp = (type_msr_host_packet*)&v_rx[0];
		if( p_rsp->c_rsp != MSR_RSP_PREFIX )
			continue;
		if( p_rsp->c_sub != msr_rsp_good && p_rsp->c_sub != msr_rsp_good_negative )
			continue;

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::_send_request(
		dev_hid::type_handle h_dev
		, const type_msr_host_packet & req
		, type_msr_host_packet *p_rsp/* = NULL*/
		)
{
	return _send_request( h_dev, (type_msr_cmd)req.c_cmd, req.c_sub, (int)req.c_len, req.s_data ,(unsigned char *)p_rsp );
}

bool dev_lpu237::df_bypass_uart(
		dev_hid::type_handle h_dev
		, const shared::type_buffer & v_tx_data
		, shared::type_buffer & v_rx_data
		)
{
	bool b_result(false);

	do{
		v_rx_data.resize(0);


		int n_out_report  = 0;
		dev_hid::type_map_report_size::iterator it = m_map_out_report_size.find(0);
		if( it == end(m_map_out_report_size) )
			continue;

		n_out_report = it->second;

		shared::type_buffer v_tx(n_out_report-MSR_SIZE_HOST_PACKET_HEADER,0);
		unsigned char c_chain(0);
		type_dword dw_total(v_tx_data.size());
		int n_remainder = (int)dw_total;
		type_dword dw_offset(0);
		int n_data(0);
		int n_tx(0);
		shared::type_result_fun result_fun(shared::result_fun_ing);

		memcpy(&v_tx[0],&m_dw_scr_transaction_counter,sizeof(type_dword));
		memcpy(&v_tx[sizeof(type_dword)],&dw_total,sizeof(type_dword));

		n_data = v_tx.size()-(sizeof(type_dword)*2);
		if( n_data > (int)v_tx_data.size() )
			n_data = v_tx_data.size();
		memcpy(&v_tx[sizeof(type_dword)*2],&v_tx_data[dw_offset],n_data);
		n_tx = n_data + sizeof(type_dword)*2;

		shared::type_buffer v_report(n_out_report,0);
		type_msr_host_packet *p_packet = (type_msr_host_packet *)&v_report[0];
		p_packet->c_cmd = msr_cmd_uart_bypass;

		do{
			p_packet->c_sub = c_chain;
			p_packet->c_len = n_tx;
			if( n_tx > 0 ){
				memcpy(p_packet->s_data,&v_tx[0],n_tx);
			}

			do{
				result_fun =  write( h_dev, v_tx );
			}while( result_fun != shared::result_fun_ing );
			//
			if( result_fun != shared::result_fun_success )
				break;

			dw_offset += n_data;
			n_remainder -= n_data;
			c_chain++;

			if(n_remainder <= 0 )
				continue;
			//
			n_data = v_tx.size()-sizeof(type_dword);
			if( n_data > n_remainder )
				n_data = n_remainder;
			n_tx = n_data + sizeof(type_dword);
			fill( begin(v_tx),end(v_tx),0);
			memcpy(&v_tx[0],&m_dw_scr_transaction_counter,sizeof(type_dword));
			memcpy(&v_tx[sizeof(type_dword)],&v_tx_data[dw_offset],n_data);

		}while( n_remainder > 0 );

		if( result_fun != shared::result_fun_success )
			continue;

		// Receiving ack of tx.
		v_report.resize(0);
		do{
			usleep(1000*2);
			result_fun =  read( h_dev, v_report );
		}while( result_fun != shared::result_fun_ing );
		//
		if( result_fun != shared::result_fun_success )
			continue;
		// Receiving part.
		dw_total = 0;
		type_dword dw_transaction_counter(0);
		n_remainder = 0;
		dw_offset = 0;
		type_dword c_rx = 0;
		v_report.resize(0);
		shared::type_buffer v_rx(0);

		do{
			usleep(1000*2);
			result_fun =  read( h_dev, v_report );
		}while( result_fun != shared::result_fun_ing );
		//
		if( result_fun != shared::result_fun_success )
			continue;

		c_rx = v_report[2];
		memcpy(&dw_transaction_counter,&v_report[3],sizeof(type_dword));
		c_chain = v_report[3+sizeof(type_dword)];
		if( dw_transaction_counter != m_dw_scr_transaction_counter ){
			continue;
		}
		//v_rx_data
		memcpy( &dw_total,&v_report[3+sizeof(type_dword)+1],sizeof(type_dword));

		v_rx.resize(dw_total,0);
		int n_rx = ((int)c_rx) - (sizeof(type_dword)+1+sizeof(type_dword));
		memcpy( &v_rx[0],&v_report[3+sizeof(type_dword)+1+sizeof(type_dword)],n_rx );

		n_remainder = dw_total - n_rx;
		dw_offset = n_rx;

		while(n_remainder>0){
			v_report.resize(0);
			do{
				usleep(1000*2);
				result_fun =  read( h_dev, v_report );
			}while( result_fun != shared::result_fun_ing );
			//
			if( result_fun != shared::result_fun_success )
				break;
			//
			c_rx = v_report[2];
			memcpy(&dw_transaction_counter,&v_report[3],sizeof(type_dword));

			if( (c_chain+1) != v_report[3+sizeof(type_dword)] )
				break;
			//
			c_chain = v_report[3+sizeof(type_dword)];
			n_rx = (int)c_rx - (sizeof(type_dword)+1);
			memcpy( &v_rx[dw_offset], &v_report[3+sizeof(type_dword)+1],n_rx );
			n_remainder -= n_rx;
			dw_offset += n_rx;
		}//end while
		if( n_remainder > 0){
			v_rx_data.resize(0);
			continue;//error
		}

		v_rx_data.resize(v_rx.size());
		copy( begin(v_rx),end(v_rx),begin(v_rx_data));
		b_result = true;
	}while(0);

	return b_result;
}
bool dev_lpu237::df_debug_interface(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_deb_interface, 0,0,NULL);
}

bool dev_lpu237::df_goto_bootloader(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_goto_bootloader, 0,0,NULL);
}

bool dev_lpu237::df_enter_config(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_enter_cs, 0,0,NULL);
}

bool dev_lpu237::df_leave_config(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_leave_cs, 0,0,NULL);
}

bool dev_lpu237::df_apply_config(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_apply, 0,0,NULL);
}

bool dev_lpu237::df_enter_opos(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_enter_opos, 0,0,NULL);
}

bool dev_lpu237::df_leave_opos(dev_hid::type_handle h_dev)
{
	return _send_request( h_dev, msr_cmd_leave_opos, 0,0,NULL);
}


bool dev_lpu237::df_get_system_parameters(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		if( !df_get_global_prepostfix_send_condition( h_dev ) )
			continue;
		if( !df_get_version_and_system_type( h_dev ) )
			continue;
		if( !df_get_name( h_dev ) )
			continue;
		if( !df_get_structure_version(h_dev))
			continue;
		if( !df_get_uid(h_dev))
			continue;
		if( !df_get_interface(h_dev))
			continue;
		if( !df_get_language(h_dev))
			continue;
		if( !df_get_buzzer(h_dev) )
			continue;
		if( !df_get_msd_runtime(h_dev) )
			continue;
		if( !df_get_enable_track(h_dev) )
			continue;
		if( !df_get_global_prefix(h_dev) )
			continue;
		if( !df_get_global_postfix(h_dev) )
			continue;
		if( !df_get_private_prefix(h_dev) )
			continue;
		if( !df_get_private_postfix(h_dev) )
			continue;
		//
		inner_version v3001(3,0,0,1);
		if( m_version >= v3001 ){
			if( !df_get_prefix_ibutton(h_dev) )
				continue;
			if( !df_get_postfix_ibutton(h_dev) )
				continue;
			if( !df_get_prefix_uart(h_dev) )
				continue;
			if( !df_get_postfix_uart(h_dev) )
				continue;
		}

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_system_parameters(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		if( !this->df_set_global_prepostfix_send_condition(h_dev) )
			continue;
		if( !this->df_set_interface(h_dev) )
			continue;
		if( !this->df_set_language(h_dev) )
			continue;
		if( !this->df_set_buzzer(h_dev) )
			continue;
		if( !this->df_set_enable_track(h_dev) )
			continue;
		if( !this->df_set_global_prefix(h_dev) )
			continue;
		if( !this->df_set_global_postfix(h_dev) )
			continue;
		if( !this->df_set_private_prefix(h_dev) )
			continue;
		if( !this->df_set_private_postfix(h_dev) )
			continue;
		if( !this->df_set_prefix_ibutton (h_dev) )
			continue;
		if( !this->df_set_postfix_ibutton(h_dev) )
			continue;
		if( !this->df_set_prefix_uart(h_dev) )
			continue;
		if( !this->df_set_postfix_uart(h_dev) )
			continue;
		if( !this->_df_set_key_map_table(h_dev) )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::_df_set(
		dev_hid::type_handle h_dev
		, type_msr_host_packet & rsp
		, type_dword dw_offset
		, type_dword dw_size
		, unsigned char *ps_data
		)
{
	type_msr_host_packet req;

	memset( &req, 0, sizeof(req) );
	memset( &rsp, 0, sizeof(rsp) );
	//
	req.c_cmd = msr_cmd_config;
	req.c_sub = msr_sys_req_config_set;
	req.c_len = sizeof(type_dword) + sizeof(type_dword) + dw_size;

	memcpy( &req.s_data[0], &dw_offset, sizeof(dw_offset) );
	memcpy( &req.s_data[sizeof(dw_offset)], &dw_size, sizeof(dw_size) );
	memcpy( &req.s_data[sizeof(dw_offset)+sizeof(dw_size)], ps_data, dw_size );

	return _send_request( h_dev, req, &rsp );
}

bool dev_lpu237::_df_set_key_map_table( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		if( m_version < inner_version(3,4,0,1) ){
			b_result = true;
			continue;
		}
		//
		type_dword n_map_table_index = (type_dword)m_parameter.get_language_map_index();
		type_dword dw_offset(0);
		type_dword dw_size(0);
		unsigned char *ps_data(NULL);

		//USB map
		dw_offset = _const_hid_keymap_address_offset;
		dw_size = FOR_CVT_MAX_ASCII_CODE;
		ps_data = (unsigned char*)(&gASCToHIDKeyMap[n_map_table_index][0][0]);

		type_msr_host_packet rsp;

		if( !_df_set( h_dev, rsp, dw_offset, dw_size, ps_data ) )
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		dw_offset = _const_hid_keymap_address_offset+FOR_CVT_MAX_ASCII_CODE;
		dw_size = FOR_CVT_MAX_ASCII_CODE;
		ps_data = (unsigned char*)(&gASCToHIDKeyMap[n_map_table_index][FOR_CVT_MAX_ASCII_CODE/2][0]);

		if( !_df_set( h_dev, rsp, dw_offset, dw_size, ps_data ) )
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;

		//PS2 map
		dw_offset = _const_ps2_keymap_address_offset;
		dw_size = FOR_CVT_MAX_ASCII_CODE;
		ps_data = (unsigned char*)(&gASCToPS2KeyMap[n_map_table_index][0][0]);

		if( !_df_set( h_dev, rsp, dw_offset, dw_size, ps_data ) )
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		dw_offset = _const_ps2_keymap_address_offset+FOR_CVT_MAX_ASCII_CODE;
		dw_size = FOR_CVT_MAX_ASCII_CODE;
		ps_data = (unsigned char*)(&gASCToPS2KeyMap[n_map_table_index][FOR_CVT_MAX_ASCII_CODE/2][0]);

		if( !_df_set( h_dev, rsp, dw_offset, dw_size, ps_data ) )
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_interface(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;
		unsigned char c_interface = (unsigned char)m_parameter.get_interface();
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,Interface)
				, sizeofstructmember(SYSINFO,Interface)
				, &c_interface
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_language(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;
		type_dword n_map_table_index = (type_dword)m_parameter.get_language_map_index();
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.KeyMap.nMappingTableIndex)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.KeyMap.nMappingTableIndex)
				, (unsigned char*)&n_map_table_index
				))
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,InfoMsr[0].KeyMap[0].nMappingTableIndex)
				, sizeofstructmember(SYSINFO,InfoMsr[0].KeyMap[0].nMappingTableIndex)
				, (unsigned char*)&n_map_table_index
				))
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,InfoMsr[1].KeyMap[0].nMappingTableIndex)
				, sizeofstructmember(SYSINFO,InfoMsr[1].KeyMap[0].nMappingTableIndex)
				, (unsigned char*)&n_map_table_index
				))
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,InfoMsr[2].KeyMap[0].nMappingTableIndex)
				, sizeofstructmember(SYSINFO,InfoMsr[2].KeyMap[0].nMappingTableIndex)
				, (unsigned char*)&n_map_table_index
				))
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		b_result = _df_set_key_map_table( h_dev );
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_buzzer(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;
		type_dword dw_frquency = (type_dword)m_parameter.get_buzzer_frequency();
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,nBuzzerFrequency)
				, sizeofstructmember(SYSINFO,nBuzzerFrequency)
				, (unsigned char*)&dw_frquency
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_enable_track(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{//cont_the_number_of_tracks

		int i(0);
		for( i = 0; i<cont_the_number_of_tracks; i++ ){
			if( !df_set_enable_track( h_dev, (type_msr_track_number)i ) ){
				break;
			}
		}//end for

		if( i < cont_the_number_of_tracks )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_global_prefix(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_global_prefix() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.GlobalPrefix)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.GlobalPrefix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_global_postfix(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_global_postfix() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.GlobalPostfix)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.GlobalPostfix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_prefix(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		int i(0);

		for( i=0; i<cont_the_number_of_tracks; i++ ){
			if( !df_set_private_prefix( h_dev,(type_msr_track_number)i) )
				break;
		}//end for
		if( i<cont_the_number_of_tracks )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_postfix(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		int i(0);

		for( i=0; i<cont_the_number_of_tracks; i++ ){
			if( !df_set_private_postfix( h_dev,(type_msr_track_number)i) )
				break;
		}//end for
		if( i<cont_the_number_of_tracks )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_prefix_ibutton(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_prefix_ibutton() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoIButton.GlobalPrefix)
				, sizeofstructmember(SYSINFO_STD,InfoIButton.GlobalPrefix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_postfix_ibutton(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_postfix_ibutton() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoIButton.GlobalPostfix)
				, sizeofstructmember(SYSINFO_STD,InfoIButton.GlobalPostfix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_prefix_uart(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_prefix_uart() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoUart.GlobalPrefix)
				, sizeofstructmember(SYSINFO_STD,InfoUart.GlobalPrefix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_postfix_uart(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_postfix_uart() ) )
			continue;

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoUart.GlobalPostfix)
				, sizeofstructmember(SYSINFO_STD,InfoUart.GlobalPostfix)
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_global_prepostfix_send_condition(dev_hid::type_handle h_dev)
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;
		type_dword n_condition(0);

		if( m_parameter.get_global_preposfix_send_condition() )
			n_condition = 1;
		//
		if( !_df_set(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.nGlobalTagCondition)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.nGlobalTagCondition)
				, (unsigned char*)&n_condition
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}


bool dev_lpu237::df_set_enable_track( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		type_dword dw_offset(0);
		type_dword dw_size(0);
		unsigned char c_enable(0);
		//
		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].cEnableTrack);
			if( m_parameter.get_enable_tack(track))
				c_enable = 1;
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].cEnableTrack);
			if( m_parameter.get_enable_tack(track))
				c_enable = 1;
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].cEnableTrack);
			if( m_parameter.get_enable_tack(track))
				c_enable = 1;
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				, (unsigned char*)&c_enable
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;

		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_prefix( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_private_prefix(track) ) )
			continue;

		type_dword dw_offset(0);
		type_dword dw_size(0);

		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].PrivatePrefix[0]);
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].PrivatePrefix[0]);
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].PrivatePrefix[0]);
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;
		b_result = true;
	}while(0);
	return b_result;
}

bool dev_lpu237::df_set_private_postfix( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		MSR_TAG tag;

		if( !_get_tag_from_tag_type( tag, m_parameter.get_private_postfix(track) ) )
			continue;

		type_dword dw_offset(0);
		type_dword dw_size(0);

		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].PrivatePostfix[0]);
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].PrivatePostfix[0]);
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].PrivatePostfix[0]);
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_set(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				, (unsigned char*)&tag
				))
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;
		b_result = true;	}while(0);
	return b_result;
}


bool dev_lpu237::df_mmd100_raw( dev_hid::type_handle h_dev, const shared::type_buffer & v_tx, shared::type_buffer & v_rx )
{
	bool b_result(false);

	do{
		v_rx.resize(256,0);
		fill( begin(v_rx),end(v_rx), 0 );
		//
		if( !_send_request( h_dev, msr_cmd_raw_mmd1000, 0, v_tx.size(), &v_tx[0], &v_rx[0]) )
			continue;
		//
		type_msr_host_packet *p_rsp = (type_msr_host_packet*)&v_rx[0];
		if( p_rsp->c_rsp != MSR_RSP_PREFIX )
			continue;
		if( p_rsp->c_sub != msr_rsp_good )
			continue;

		shared::type_buffer v_rx_data(p_rsp->c_len,0);
		memcpy( &v_rx_data[0], &v_rx[3], p_rsp->c_len );

		v_rx.swap(v_rx_data);

		b_result = true;
	}while(0);

	if( !b_result )
		v_rx.resize(0);

	return b_result;
}

bool dev_lpu237::df_get_global_prepostfix_send_condition( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.nGlobalTagCondition)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.nGlobalTagCondition)
				) )
			continue;
		//
		if( rsp.c_sub != msr_rsp_good )
			continue;
		type_dword dw_condition(0);
		memcpy( &dw_condition, rsp.s_data, rsp.c_len );

		if( dw_condition != 0 )
			m_parameter.set_global_preposfix_send_condition(true);
		else
			m_parameter.set_global_preposfix_send_condition(false);

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_name( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;
		m_name.assign(m_name.size(),0);

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,sName)
				, sizeofstructmember(SYSINFO,sName)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		for( int i = 0; i<(int)rsp.c_len && i<(int)m_name.size(); i++ ){

			if( rsp.s_data[i] == 0 || rsp.s_data[i] == ' ')
				break;
			else
				m_name[i] = rsp.s_data[i];
		}//end for
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_uid( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_uid.clear();

		type_msr_host_packet req,rsp;

		memset( &req,0,sizeof(req));
		memset( &rsp,0,sizeof(rsp));

		req.c_cmd = msr_cmd_read_uid;

		if( !_send_request( h_dev, req, &rsp ) )
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;

		m_uid.resize(const_size_uid);
		memcpy( &m_uid[0], rsp.s_data, const_size_uid );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_version( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_bootloader_type = bl_none;
		m_version.set(0,0,0,0);

		type_msr_host_packet req,rsp;

		memset( &req,0,sizeof(req));
		memset( &rsp,0,sizeof(rsp));
		//
		req.c_cmd = msr_cmd_config;
		req.c_sub = msr_sys_req_config_get;
		req.c_len = sizeof(type_dword)*2; //offset & size

		type_dword dw_offset = offsetof(SYSINFO,sSysVer);
		type_dword dw_size = sizeofstructmember(SYSINFO,sSysVer);
		memcpy( &req.s_data[0], &dw_offset, sizeof(dw_offset) );
		memcpy( &req.s_data[sizeof(dw_offset)], &dw_size, sizeof(dw_size) );

		if( !_send_request( h_dev, req, &rsp ))
			continue;
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		m_version = inner_version(rsp.s_data[0], rsp.s_data[1], rsp.s_data[2], rsp.s_data[3] );

		m_bootloader_type = bl_hid;
		m_b_embedded_language_map = false;
		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_version_and_system_type( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_dw_system_type = ft_none;

		if( !df_get_version(h_dev) )
			continue;
		//
		inner_version v3302(3,3,0,2);
		if( m_version < v3302 ){
			m_dw_system_type = ft_msr;
			b_result = true;
			continue;
		}

		type_msr_host_packet req,rsp;

		memset( &req,0,sizeof(req));
		memset( &rsp,0,sizeof(rsp));
		//
		req.c_cmd = msr_cmd_hw_is_standard;
		if( !_send_request( h_dev, req, &rsp ))
			continue;
		if( rsp.c_sub != msr_rsp_good && rsp.c_sub != msr_rsp_good_negative )
			continue;
		if( rsp.c_sub == msr_rsp_good )
			m_dw_system_type = ft_msr | ft_ibutton;
		else
			m_dw_system_type = ft_msr;
		//
		inner_version v3704(3,7,0,4);
		if( m_version < v3704 ){
			b_result = true;
			continue;
		}
		//
		memset( &req,0,sizeof(req));
		memset( &rsp,0,sizeof(rsp));
		//
		req.c_cmd = msr_cmd_hw_is_only_ibutton;
		if( !_send_request( h_dev, req, &rsp ))
			continue;
		if( rsp.c_sub != msr_rsp_good && rsp.c_sub != msr_rsp_good_negative )
			continue;
		if( rsp.c_sub == msr_rsp_good )
			m_dw_system_type = ft_ibutton;

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_structure_version( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_structure_version.set(0,0,0,0);
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,sStrucVer)
				, sizeofstructmember(SYSINFO,sStrucVer)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		m_structure_version.set( rsp.s_data[0], rsp.s_data[1], rsp.s_data[2], rsp.s_data[3] );
		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_interface( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,Interface)
				, sizeofstructmember(SYSINFO,Interface)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		m_parameter.set_interface( (type_interface)rsp.s_data[0] );
		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_language( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.KeyMap.nMappingTableIndex)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.KeyMap.nMappingTableIndex)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		type_dword n_lang(0);
		memcpy( &n_lang, rsp.s_data, sizeof(n_lang));

		m_parameter.set_language_index( (type_language_map_index)n_lang );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_buzzer( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,nBuzzerFrequency)
				, sizeofstructmember(SYSINFO,nBuzzerFrequency)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		type_dword n_buzzer(0);
		memcpy( &n_buzzer, rsp.s_data, sizeof(n_buzzer));

		m_parameter.set_buzzer_frequency( n_buzzer/10 );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_msd_runtime( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,nBootRunTime)
				, sizeofstructmember(SYSINFO,nBootRunTime)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		type_dword n_runtime(0);
		memcpy( &n_runtime, rsp.s_data, sizeof(n_runtime));

		m_n_msd_run_time = n_runtime * 10;

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_enable_track( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		int i(0);

		for( i=0; i<cont_the_number_of_tracks; i++ ){
			if( !df_get_enable_track( h_dev, (type_msr_track_number)i))
				break;
		}//end for

		if( i<cont_the_number_of_tracks )
			continue;
		//
		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_global_prefix( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_global_prefix();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.GlobalPrefix)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.GlobalPrefix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_global_prefix( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_global_postfix( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_global_postfix();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO,ContainerInfoMsrObj.GlobalPostfix)
				, sizeofstructmember(SYSINFO,ContainerInfoMsrObj.GlobalPostfix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_global_postfix( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_private_prefix( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		int i(0);

		for( i=0; i<cont_the_number_of_tracks; i++ ){
			if( !df_get_private_prefix( h_dev, (type_msr_track_number)i))
				break;
		}//end for

		if( i<cont_the_number_of_tracks )
			continue;

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_private_postfix( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		int i(0);

		for( i=0; i<cont_the_number_of_tracks; i++ ){
			if( !df_get_private_postfix( h_dev, (type_msr_track_number)i))
				break;
		}//end for

		if( i<cont_the_number_of_tracks )
			continue;

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_prefix_ibutton( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_prefix_ibutton();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoIButton.GlobalPrefix)
				, sizeofstructmember(SYSINFO_STD,InfoIButton.GlobalPrefix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_prefix_ibutton( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_postfix_ibutton( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_postfix_ibutton();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoIButton.GlobalPostfix)
				, sizeofstructmember(SYSINFO_STD,InfoIButton.GlobalPostfix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_postfix_ibutton( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_prefix_uart( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_prefix_uart();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoUart.GlobalPrefix)
				, sizeofstructmember(SYSINFO_STD,InfoUart.GlobalPrefix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_prefix_uart( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_postfix_uart( dev_hid::type_handle h_dev )
{
	bool b_result(false);

	do{
		m_parameter.set_postfix_uart();//clear

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, offsetof(SYSINFO_STD,InfoUart.GlobalPostfix)
				, sizeofstructmember(SYSINFO_STD,InfoUart.GlobalPostfix)
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;
		//
		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;

		m_parameter.set_postfix_uart( v_tag );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_enable_track( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		type_dword dw_offset(0);
		type_dword dw_size(0);

		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].cEnableTrack);
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].cEnableTrack);
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].cEnableTrack);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].cEnableTrack);
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		bool b_enable(false);
		if( rsp.s_data[0] )
			b_enable = true;

		m_parameter.set_enable_track( track, b_enable );

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_private_prefix( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		type_dword dw_offset(0);
		type_dword dw_size(0);

		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].PrivatePrefix[0]);
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].PrivatePrefix[0]);
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].PrivatePrefix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].PrivatePrefix[0]);
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;
		m_parameter.set_private_prefix(track, v_tag);

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::df_get_private_postfix( dev_hid::type_handle h_dev, type_msr_track_number track )
{
	bool b_result(false);

	do{
		type_dword dw_offset(0);
		type_dword dw_size(0);

		switch(track){
		case mt_iso1:
			dw_offset = offsetof(SYSINFO,InfoMsr[0].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[0].PrivatePostfix[0]);
			break;
		case mt_iso2:
			dw_offset = offsetof(SYSINFO,InfoMsr[1].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[1].PrivatePostfix[0]);
			break;
		case mt_iso3:
			dw_offset = offsetof(SYSINFO,InfoMsr[2].PrivatePostfix[0]);
			dw_size = sizeofstructmember(SYSINFO,InfoMsr[2].PrivatePostfix[0]);
			break;
		default:
			continue;
		}//end switch

		type_msr_host_packet rsp;

		if( !_df_get(
				h_dev
				, rsp
				, dw_offset
				, dw_size
				) )
			continue;
		//here start.
		if( rsp.c_sub != msr_rsp_good )
			continue;

		PMSR_TAG p_tag = (PMSR_TAG)rsp.s_data;
		type_tag v_tag;

		if( !_get_tag_type_from_tag( v_tag,p_tag ) )
			continue;
		m_parameter.set_private_postfix(track, v_tag);

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::_df_get(
		dev_hid::type_handle h_dev
		, type_msr_host_packet & rsp
		, type_dword dw_offset
		, type_dword dw_size )
{
	bool b_result(false);

	do{
		if( dw_size > (type_dword)(m_map_in_report_size[0]-MSR_SIZE_HOST_PACKET_HEADER) )
			continue;
		//
		type_msr_host_packet req;

		memset( &req, 0, sizeof(req) );
		memset( &rsp, 0, sizeof(rsp) );

		req.c_cmd = msr_cmd_config;
		req.c_sub = msr_sys_req_config_get;
		req.c_len = sizeof(type_dword) + sizeof(type_dword);//offset and size

		memcpy( &req.s_data[0], &dw_offset, sizeof(dw_offset) );
		memcpy( &req.s_data[sizeof(dw_offset)], &dw_size, sizeof(dw_size) );


		b_result= _send_request( h_dev, req, &rsp );
	}while(0);
	return b_result;
}

bool dev_lpu237::_get_tag_from_tag_type( MSR_TAG & out_tag, const type_tag & v_in_tag )
{
	bool b_result(false);

	do{
		if( v_in_tag.size() > MSROBJ_INFO_DEF_TAG_SIZE )
			continue;
		//
		memset( &out_tag, 0, sizeof(out_tag) );

		for_each( begin(v_in_tag),end(v_in_tag),[&](unsigned char c_tag){
			out_tag.sTag[out_tag.cSize++] = c_tag;
		});

		b_result = true;
	}while(0);

	return b_result;
}

bool dev_lpu237::_get_tag_type_from_tag( type_tag & v_out_tag, PMSR_TAG p_in_tag )
{
	bool b_result(false);

	do{
		v_out_tag.resize(0);

		if( p_in_tag == NULL )
			continue;
		//
		type_dword n_map_index = m_parameter.get_language_map_index();

		for( unsigned char i = 0; i<p_in_tag->cSize/2; i++ ){
			if( p_in_tag->sTag[2*i] == 0xFF ){
				//2*i+1 is ASCII code
				v_out_tag.push_back( gASCToHIDKeyMap[n_map_index][p_in_tag->sTag[2*i+1]][0] );
				v_out_tag.push_back( gASCToHIDKeyMap[n_map_index][p_in_tag->sTag[2*i+1]][1] );
			}
			else{
				//2*i mode code, 2*i+1 is key code
				v_out_tag.push_back(p_in_tag->sTag[2*i]);
				v_out_tag.push_back(p_in_tag->sTag[2*i+1]);
			}
		}//end for

		b_result= true;
	}while(0);

	return b_result;
}

