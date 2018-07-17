/*
 * el_lpu237-hidapi.h
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#ifndef INC_EL_LPU237_HIDAPI_H_
#define INC_EL_LPU237_HIDAPI_H_

#include <wchar.h>
#include "shared_type.h"

#ifdef _WIN32
	#include <windows>
	#define LPU237_HIDAPI_EXPORT __declspec(dllexport)
	#define LPU237_HIDAPI_CALL	WINAPI
	#define LPU237_HANDLE	HANDLE
	typedef	void(LPU237_HIDAPI_CALL *LPU237_type_callback)(void*);
#else
	#define LPU237_HIDAPI_EXPORT /**< API export macro */
	#define LPU237_HIDAPI_CALL /**< API call macro */
	#define LPU237_HANDLE	void*
	typedef	void(LPU237_HIDAPI_CALL *LPU237_type_callback)(void*);
#endif

#define LPU237_HIDAPI_EXPORT_CALL LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL /**< API export and call macro*/

	/*!
	 * constants
	 */
#define LPU237_DLL_SIZE_ID			16
#define LPU237_DLL_SIZE_NAME		16
#define LPU237_DLL_SIZE_VERSION		4

	/*
	 * for updating firmware
	 */
#define	LPU237_DLL_WPARAM_COMPLETE		0	//firmware update complete.
#define	LPU237_DLL_WPARAM_SUCCESS			0	//firmware update complete.
#define	LPU237_DLL_WPARAM_FOUND_BL		1	//found bootloader.
#define	LPU237_DLL_WPARAM_SECTOR_ERASE	2
#define	LPU237_DLL_WPARAM_SECTOR_WRITE	3
#define	LPU237_DLL_WPARAM_ERROR			0xFFFF
	/*!
	*	the callback function type.
	*	this type will be used in LPU237_fw_msr_update()
	*
	*	parameters
	*		1'st - user defined data.
	*		2'nd - current processing result : LPU237_DLL_RESULT_x
	*		3'th - LPU237_DLL_WPARAM_x.
	*/
	typedef	type_dword (LPU237_HIDAPI_CALL *type_lpu237_update_callback)(void*,type_dword,type_dword);

	/*!
	 *	return value definition.
	 */
#define LPU237_DLL_RESULT_SUCCESS				0
#define LPU237_DLL_RESULT_ERROR				0xFFFFFFFF
#define LPU237_DLL_RESULT_CANCEL			0xFFFFFFFE
#define LPU237_DLL_RESULT_ERROR_MSR		0xFFFFFFFD	//-3

#define LPU237_DLL_RESULT_ICC_INSERTED		0xFFFFFFFC	//-4
#define LPU237_DLL_RESULT_ICC_REMOVED		0xFFFFFFFB	//-5

#ifdef __cplusplus
extern "C" {
#endif

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_dll_on)();
typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_dll_off)();

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_list_w)(wchar_t *ss_dev_path);
typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_list_a)(char *ss_dev_path);

typedef	LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_open_w)( const wchar_t *s_dev_path );
typedef	LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_open_a)( const char *s_dev_path );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_close)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_id)( LPU237_HANDLE h_dev, unsigned char *s_id );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_enable)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_disable)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_cancel_wait)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_waits)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_key_with_waits)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_or_key_with_waits)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_callback)( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_key_with_callback)( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_or_key_with_callback)( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

#ifdef _WIN32
typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_message)( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_key_with_message)( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_or_key_with_message)( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
#endif

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_data)( type_dword dw_buffer_index, type_dword dw_iso_track, unsigned char *s_track_data );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_sys_save_setting)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_sys_recover_setting)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_sys_get_name)(
		LPU237_HANDLE h_dev
		, unsigned char *s_name
		);

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_version)( LPU237_HANDLE h_dev, unsigned char *s_version );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_version_major)( const unsigned char *s_version );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_version_minor)( const unsigned char *s_version );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_cancel_update)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_update_callback_w)( const unsigned char *s_id, type_lpu237_update_callback cb_update, void *p_user, const wchar_t *s_rom_file_name, type_dword dw_index );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_update_callback_a)(	const unsigned char *s_id, type_lpu237_update_callback cb_update, void *p_user, const char *s_rom_file_name, type_dword dw_index	);

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_rom_load_w)( const wchar_t *s_rom_file_name );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_rom_load_a)( const char *s_rom_file_name );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_rom_get_index_w)(const wchar_t *s_rom_file_name, const unsigned char *s_name, const unsigned char *s_version);

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_rom_get_index_a)(const char *s_rom_file_name, const unsigned char *s_name, const unsigned char *s_version);


/*
 * system functions .......
 */
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_enter_config)( LPU237_HANDLE h_dev );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_leave_config)( LPU237_HANDLE h_dev );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_apply_config)( LPU237_HANDLE h_dev );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_goto_boot)( LPU237_HANDLE h_dev );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_bypass_uart)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tx\
		, type_dword dw_tx\
		, unsigned char *ps_rx\
		, type_dword dw_rx\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_enter_opos)( LPU237_HANDLE h_dev );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_leave_opos)( LPU237_HANDLE h_dev );

typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_interface)( LPU237_HANDLE h_dev, type_dword dw_interface  );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_language)( LPU237_HANDLE h_dev, type_dword dw_language );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_buzzer)( LPU237_HANDLE h_dev, type_dword dw_frequency );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_track_status)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, type_dword b_enable\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_global_prefix)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_global_postfix)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_private_prefix)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_private_postfix)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_prefix_ibutton)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_postfix_ibutton)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_prefix_uart)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_postfix_uart)(\
		LPU237_HANDLE h_dev\
		, const unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_set_global_prepostfix_send_condition)(\
		LPU237_HANDLE h_dev\
		, type_dword b_all_track_good\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_global_prepostfix_send_condition)(\
		LPU237_HANDLE h_dev\
		,type_dword *pb_all_track_no_error\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_uid)( LPU237_HANDLE h_dev, unsigned char  *ps_uid );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_version_and_system_type)(\
		LPU237_HANDLE h_dev\
		, unsigned char *s_version\
		, type_dword *pdw_system_type\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_structure_version)( LPU237_HANDLE h_dev, unsigned char *s_version );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_interface)( LPU237_HANDLE h_dev,type_dword *pdw_interface );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_language)( LPU237_HANDLE h_dev,type_dword *pdw_language );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_buzzer)( LPU237_HANDLE h_dev,type_dword *pdw_buzzer );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_msd_run_time)( LPU237_HANDLE h_dev,type_dword *pdw_run_time );
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_track_status)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, type_dword *pb_enable\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_global_prefix)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_global_postfix)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_private_prefix)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_private_postfix)(\
		LPU237_HANDLE h_dev\
		, type_dword dw_track\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_prefix_ibutton)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_postfix_ibutton)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_prefix_uart)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
typedef	type_dword LPU237_HIDAPI_EXPORT (*type_LPU237_sys_get_postfix_uart)(\
		LPU237_HANDLE h_dev\
		, unsigned char *ps_tag\
		, type_dword dw_tag\
		);
#ifdef __cplusplus
}
#endif





#endif /* INC_EL_LPU237_HIDAPI_H_ */
