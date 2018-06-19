/*
 * el_lpu237-hidapi.h
 *
 *  Created on: 2018. 6. 12.
 *      Author: totoro
 */

#ifndef INC_EL_LPU237_HIDAPI_H_
#define INC_EL_LPU237_HIDAPI_H_

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

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_dll_on)();
typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_dll_off)();

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_list_w)(wchar_t *ss_dev_path);
typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_list_a)(char *ss_dev_path);

typedef	LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_open_w)( const wchar_t *s_dev_path );
typedef	LPU237_HANDLE LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_open_a)( const char *s_dev_path );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_close)( LPU237_HANDLE h_dev );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_id)( LPU237_HANDLE h_dev, unsigned char *s_id );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_enable)( LPU237_HANDLE h_dev );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_disable)( LPU237_HANDLE h_dev );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_cancel_wait_swipe)( LPU237_HANDLE h_dev );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_waits)( LPU237_HANDLE h_dev );

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_callback)( LPU237_HANDLE h_dev, LPU237_type_callback p_fun, void *p_parameter );

#ifdef _WIN32
typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_wait_swipe_with_message)( LPU237_HANDLE h_dev,HWND h_wnd, UINT n_msg );
#endif

typedef	unsigned long LPU237_HIDAPI_EXPORT LPU237_HIDAPI_CALL (*type_LPU237_get_data)( unsigned long dw_buffer_index, unsigned long dw_iso_track, unsigned char *s_track_data );

#ifdef __cplusplus
}
#endif





#endif /* INC_EL_LPU237_HIDAPI_H_ */
