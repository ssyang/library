/*
 * lpu237-hidapi.h
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#ifndef INC_LPU237_HIDAPI_H_
#define INC_LPU237_HIDAPI_H_

#include <wchar.h>

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

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on();

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off();

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_w(wchar_t *ss_dev_path);
unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_a(char *ss_dev_path);

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_w( const wchar_t *s_dev_path );
LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_a( const char *s_dev_path );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_close( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_waits( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_waits( LPU237_HANDLE h_dev );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

#ifdef _WIN32
unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
#endif

/*
 * dw_iso_track = 0 : ibutton data.
 * dw_iso_track = 1 : MSR ISO1 track data
 * dw_iso_track = 2 : MSR ISO2 track data
 * dw_iso_track = 3 : MSR ISO3 track data
 */
unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_data( unsigned long dw_buffer_index, unsigned long dw_iso_track, unsigned char *s_track_data );

/*
 * system interface.
 */
/*
 * LPU237_sys_save_setting
 * LPU237_sys_recover_setting
 * LPU237_sys_get_name
 * LPU237_sys_get_version
 * LPU237_sys_get_version_major
 * LPU237_sys_get_version_minor
 * LPU237_sys_update_w
 * LPU237_sys_update_a
 * LPU237_sys_update_callback_w
 * LPU237_sys_update_callback_a
 * LPU237_sys_rom_load_w
 * LPU237_sys_rom_load_a
 * LPU237_sys_rom_get_index_w
 * LPU237_sys_rom_get_index_a
 * LPU237_sys_enter_config
 * LPU237_sys_leave_config
 * LPU237_sys_apply_config
 * LPU237_sys_goto_boot
 * LPU237_sys_bypass_uart
 * LPU237_sys_enter_opos
 * LPU237_sys_leave_opos
 *
 * LPU237_sys_set_interface
 * LPU237_sys_set_language
 * LPU237_sys_set_buzzer
 * LPU237_sys_set_track_status - each and sum
 * LPU237_sys_set_global_prefix
 * LPU237_sys_set_global_postfix
 * LPU237_sys_set_private_prefix - each and sum
 * LPU237_sys_set_private_postfix - each and sum
 * LPU237_sys_set_prefix_ibutton
 * LPU237_sys_set_postfix_ibutton
 * LPU237_sys_set_prefix_uart
 * LPU237_sys_set_postfix_uart
 * LPU237_sys_set_global_prepostfix_send_condition
 * LPU237_sys_set_keymap_table
 *
 * LPU237_sys_mmd_raw
 *
 * LPU237_sys_get_global_prepostfix_send_condition
 * LPU237_sys_get_uid
 * LPU237_sys_get_version_and_system_type
 * LPU237_sys_get_structure_version
 * LPU237_sys_get_interface
 * LPU237_sys_get_language
 * LPU237_sys_get_buzzer
 * LPU237_sys_get_msd_run_time
 * LPU237_sys_get_track_status - each and sum
 * LPU237_sys_get_global_prefix
 * LPU237_sys_get_global_postfix
 * LPU237_sys_get_private_prefix - each and sum
 * LPU237_sys_get_private_postfix - each and sum
 * LPU237_sys_get_prefix_ibutton
 * LPU237_sys_get_postfix_ibutton
 * LPU237_sys_get_prefix_uart
 * LPU237_sys_get_postfix_uart
 * LPU237_sys_get_language_map_table
 *
 *
 *
*/

#ifdef __cplusplus
}
#endif




#endif /* INC_LPU237_HIDAPI_H_ */
