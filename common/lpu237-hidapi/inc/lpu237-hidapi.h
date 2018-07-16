/*
 * lpu237-hidapi.h
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#ifndef INC_LPU237_HIDAPI_H_
#define INC_LPU237_HIDAPI_H_

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

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_on();

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_dll_off();

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_w(wchar_t *ss_dev_path);
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_list_a(char *ss_dev_path);

LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_w( const wchar_t *s_dev_path );
LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_open_a( const char *s_dev_path );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_close( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_id( LPU237_HANDLE h_dev, unsigned char *s_id );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_enable( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_disable( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_cancel_wait( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_waits( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_waits( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_waits( LPU237_HANDLE h_dev );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_callback( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

#ifdef _WIN32
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_key_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_wait_swipe_or_key_with_message( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
#endif

/*
 * dw_iso_track = 0 : ibutton data.
 * dw_iso_track = 1 : MSR ISO1 track data
 * dw_iso_track = 2 : MSR ISO2 track data
 * dw_iso_track = 3 : MSR ISO3 track data
 */
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_get_data( type_dword dw_buffer_index, type_dword dw_iso_track, unsigned char *s_track_data );

/*********************************************************************************************
 * system interface.
 */
/*!
* function
*	save the current lpu237 device setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*
* return
*	if success, return LPU237_DLL_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_save_setting( LPU237_HANDLE h_dev );

/*!
* function
*	resetting  lpu237 device with saved setting.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*
* return
*	if success, return LPU237_DLLW_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_recover_setting( LPU237_HANDLE h_dev );

/*!
* function
*	get device internal name.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*	sName : [in/out] A pointer to the buffer that save the device name.
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else the size of internal name.[unit byte]
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL LPU237_sys_get_name(
		LPU237_HANDLE h_dev
		, unsigned char *s_name
		);


/*!
* function
*	get device firmware version.
*
* parameters
*	hDev : [in] device handle( return value of LPU237_open() )
*	sName : [in/out] A pointer to the buffer that save the device firmware version.( version 4 bytes )
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else the size of version.[unit byte]
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version( LPU237_HANDLE h_dev, unsigned char *s_version );

/*!
* function
*	get major number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_sys_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else major version number.
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version_major( const unsigned char *s_version );

/*!
* function
*	get minor number from firmware version.
*
* parameters
*	sVersion : [in] device firmware version( return value of LPU237_sys_get_version() ).
*			this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else minor version number.
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_get_version_minor( const unsigned char *s_version );

/*!
* function
*	stop operation of LPU237_sys_update_x.
*
* parameters
*
* return
*	if success, return LPU237_DLL_RESULT_SUCCESS
*	else return LPU237_DLL_RESULT_ERROR
*
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_cancel_update( LPU237_HANDLE h_dev );


/*!
* function
*	start firmware update.( unicode version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cbUpdate : [in] callback function for serving updating information.
*	pUser : [in] user data pointer for calling cbUpdate().
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR.
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_update_callback_w
		(
		const unsigned char *s_id
		, type_lpu237_update_callback cb_update
		, void *p_user
		, const wchar_t *s_rom_file_name
		, type_dword dw_index
		);

/*!
* function
*	start firmware update..( Multi Byte Code Set version )
*
* parameters
*	sId : [in] the device ID.( ID is 16 bytes )
*	cbUpdate : [in] callback function for serving updating information.
*	pUser : [in] user data pointer for calling cbUpdate().
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*	dwIndex : [in] the index of firmware in rom file. greater then equal -1
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_update_callback_a
		(
		const unsigned char *s_id
		, type_lpu237_update_callback cb_update
		, void *p_user
		, const char *s_rom_file_name
		, type_dword dw_index
		);

/*!
* function
*	load rom file.( unicode version )
*
* parameters
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else LPU237_DLL_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_load_w( const wchar_t *s_rom_file_name );

/*!
* function
*	load rom file.( Multi Byte Code Set version )
*
* parameters
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*
* return
* 	if error, return LPU237_FW_RESULT_ERROR
*	else LPU237_FW_RESULT_SUCCESS
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_load_a( const char *s_rom_file_name );

/*!
* function
*	get firmware index value( greater then equal 0 ) in rom file.( unicode version ).
*
* parameters
*	sRomFileName : [in] rom file name. unicode, zero-string type. this value can be NULL(0).
*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
*
* return
* 	if error, return LPU237_DLL_RESULT_ERROR
*	else firmware index value( greater then equal 0 )
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_get_index_w(
		const wchar_t *s_rom_file_name
		, const unsigned char *s_name
		, const unsigned char *s_version
		);

/*!
* function
*	get firmware index value( greater then equal 0 ) in rom file.( multi-byte-code-set version ).
*
* parameters
*	sRomFileName : [in] rom file name. multi-byte-code-set, zero-string type. this value can be NULL(0).
*	sName : [in] device internal name. LPU237_fw_msr_get_name' return value.
*	sVersion : [in] device firmware version. LPU237_fw_msr_get_version' return value.
*
* return
* 	if error, return LPU237_FW_RESULT_ERROR
*	else firmware index value( greater then equal 0 )
*/
type_dword LPU237_HIDAPI_EXPORT LPU237_sys_rom_get_index_a(
		const char *s_rom_file_name
		, const unsigned char *s_name
		, const unsigned char *s_version
		);

/*
 * system functions .......
 */
type_dword LPU237_sys_enter_config( LPU237_HANDLE h_dev );
type_dword LPU237_sys_leave_config( LPU237_HANDLE h_dev );
type_dword LPU237_sys_apply_config( LPU237_HANDLE h_dev );
type_dword LPU237_sys_goto_boot( LPU237_HANDLE h_dev );
type_dword LPU237_sys_bypass_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tx
		, type_dword dw_tx
		, unsigned char *ps_rx
		, type_dword dw_rx
		);
type_dword LPU237_sys_enter_opos( LPU237_HANDLE h_dev );
type_dword LPU237_sys_leave_opos( LPU237_HANDLE h_dev );

type_dword LPU237_sys_set_interface( LPU237_HANDLE h_dev, type_dword dw_interface  );
type_dword LPU237_sys_set_language( LPU237_HANDLE h_dev, type_dword dw_language );
type_dword LPU237_sys_set_buzzer( LPU237_HANDLE h_dev, type_dword dw_frequency );
type_dword LPU237_sys_set_track_status(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, type_dword b_enable
		);
type_dword LPU237_sys_set_global_prefix(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_global_postfix(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_private_prefix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_private_postfix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_prefix_ibutton(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_postfix_ibutton(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_prefix_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_postfix_uart(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_set_global_prepostfix_send_condition(
		LPU237_HANDLE h_dev
		, type_dword b_all_track_good
		);
type_dword LPU237_sys_mmd_raw(
		LPU237_HANDLE h_dev
		, const unsigned char *ps_tx
		, type_dword dw_tx
		, unsigned char *ps_rx
		, type_dword dw_rx
		);

type_dword LPU237_sys_get_global_prepostfix_send_condition( LPU237_HANDLE h_dev, type_dword *pb_all_track_no_error );
type_dword LPU237_sys_get_uid( LPU237_HANDLE h_dev, unsigned char  *ps_uid );
type_dword LPU237_sys_get_version_and_system_type(
		LPU237_HANDLE h_dev
		, unsigned char *s_version
		, type_dword *pdw_system_type
		);
type_dword LPU237_sys_get_structure_version( LPU237_HANDLE h_dev, unsigned char *s_version );
type_dword LPU237_sys_get_interface( LPU237_HANDLE h_dev,type_dword *pdw_interface );
type_dword LPU237_sys_get_language( LPU237_HANDLE h_dev );
type_dword LPU237_sys_get_buzzer( LPU237_HANDLE h_dev );
type_dword LPU237_sys_get_msd_run_time( LPU237_HANDLE h_dev );
type_dword LPU237_sys_get_track_status(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, type_dword *pb_enable
		);
type_dword LPU237_sys_get_global_prefix(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_global_postfix(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_private_prefix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_private_postfix(
		LPU237_HANDLE h_dev
		, type_dword dw_track
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_prefix_ibutton(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_postfix_ibutton(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_prefix_uart(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_postfix_uart(
		LPU237_HANDLE h_dev
		, unsigned char *ps_tag
		, type_dword dw_tag
		);
type_dword LPU237_sys_get_language_map_table(
		LPU237_HANDLE h_dev
		, unsigned char *ps_map
		, type_dword dw_map
		);

#ifdef __cplusplus
}
#endif




#endif /* INC_LPU237_HIDAPI_H_ */
