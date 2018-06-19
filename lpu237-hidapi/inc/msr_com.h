/*
 * msr_com.h
 *
 *  Created on: 2018. 6. 14.
 *      Author: totoro
 */

#ifndef INC_MSR_COM_H_
#define INC_MSR_COM_H_

typedef enum {
	msr_cmd_change_authkey = 'C',
	msr_cmd_change_enkey = 'K',
	msr_cmd_change_status = 'M',
	msr_cmd_change_sn = 'S',
	msr_cmd_config = 'A',
	msr_cmd_apply = 'B',

	msr_cmd_enter_cs = 'X',
	msr_cmd_leave_cs = 'Y',
	msr_cmd_goto_bootloader = 'G',
	msr_cmd_enter_opos = 'I',
	msr_cmd_leave_opos = 'J',

	msr_cmd_hw_is_standard = 'D',
	msr_cmd_hw_is_only_ibutton = 'W',
	msr_cmd_read_uid = 'U',
	msr_cmd_raw_mmd1000 = 'E',
	msr_cmd_deb_interface = 'Z',
	msr_cmd_generate_head_signal = 'E',
	msr_cmd_uart_bypass = 'T'
}type_msr_cmd;

typedef enum {
	msr_status_disable = 0,
	msr_status_enable = 1
}type_msr_status;

#define MSR_RSP_PREFIX		'R'

typedef enum {
	msr_rsp_good = 0xFF,
	msr_rsp_good_negative = 0x80,

	msr_rsp_error_crc = 0x01,
	msr_rsp_error_miss_length = 0x02,
	msr_rsp_error_miss_key = 0x03,
	msr_rsp_error_miss_check_block = 0x04,
	msr_rsp_error_invalid = 0x05,
	msr_rsp_error_verify = 0x06
}type_msr_rsp;

#define MSR_WARN_ICC_INSERTED	0xBA
#define MSR_WARN_ICC_REMOVED	0xB9

#define MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD	255
#define MSR_SIZE_HOST_PACKET_HEADER	3	//the sum of size( c_cmd, c_sub, c_len )

#define MSR_NUMBER_SUPPORT_LANGUAGE	2

typedef enum {
	msr_language_index_english = 0,
	msr_language_index_spanish = 1
}type_msr_language_index;

#define MSR_SYS_REQ_SETTER_START_NUMBER	0
#define MSR_SYS_REQ_GETTER_START_NUMBER	0

//cofig request case
typedef enum {
	msr_sys_req_config_set	= 200,
	msr_sys_req_config_get	= 201
}type_msr_sys_req_config;

#ifdef	WIN32
	#define INC_MSR_COM_H_ALIGN
#else
	#define INC_MSR_COM_H_ALIGN	__attribute__ ((packed))
#endif


#ifdef	WIN32
#pragma pack(push,1)
#endif
typedef struct tag_msr_host_packet{
	union{
		unsigned char c_cmd;//command value
		unsigned char c_rsp;//response value
	};
	unsigned char c_sub;
	unsigned char c_len;
	unsigned char s_data[MSR_MAX_SIZE_HOST_PACKET_DATA_FIELD];
}INC_MSR_COM_H_ALIGN type_msr_host_packet, *type_pmsr_host_packet;

#ifdef	WIN32
#pragma pack(pop)
#endif


#endif /* INC_MSR_COM_H_ */
