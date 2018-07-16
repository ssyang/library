/*
 * inner_rom.h
 *
 *  Created on: 2018. 7. 6.
 *      Author: totoro
 */

#ifndef INC_INNER_ROM_H_
#define INC_INNER_ROM_H_

#include "shared_type.h"
#include <string>

using namespace std;

class inner_rom
{
public:
	enum type_result : type_dword {
		result_success = 0,
		result_error = 1,
		result_error_not_found = 2,
		result_error_invalid_parameter = 3,
		result_error_shorter_then_expected = 4,
		result_error_not_loaded_dll = 5,
		result_error_not_open_file = 6,
		result_error_greater_then_expected = 7,
		result_error_over_capacity = 8
	};

	enum mask_update_condition : type_dword {
		condition_no		= 0x00000000,
		condition_eq		= 0x00000001,	//equal
		condition_neq	= 0x00000002,	//not equal
		condition_gt		= 0x00000004,	//greater then
		condition_lt		= 0x00000008	//less then
	};


	enum{
		MAX_ROMFILE_HEAD_ITEAM	= 45,
		MAX_MODEL_NAME_SIZE		= 30,
		MAX_RFU_SIZE					= 128,
		//MAX_SIZE_APP					= 24*1024	// version 1.0
		MAX_SIZE_APP					= 95443539	//version 1.1
	};

	typedef struct tagROMFILE_HEAD_ITEAM{
		type_dword dwSize;			//the firmware data size
		type_dword dwOffset;		//the firmware data starting offset from start of file.
		type_byte sVersion[4];		// the version of firmware.
										// sVersion[0] : major version
										// sVersion[1] : minor version
										// sVersion[2] : bug fix version
										// sVersion[3] : build version
		type_byte sModel[MAX_MODEL_NAME_SIZE+1];	// device model name.

		type_dword dwUpdateCondition;	//the combination of type_update_condition

		union{
			type_byte sRFU[MAX_RFU_SIZE];//version 1.0
			type_byte sHash[32];	//version 1.1 - SHA256 hash code of this firmware
		};

	}__attribute__ ((packed)) ROMFILE_HEAD_ITEAM, *PROMFILE_HEAD_ITEAM, *LPROMFILE_HEAD_ITEAM;

	// rom file head.
	typedef struct tagROMFILE_HEAD{
		type_dword dwHeaderSize;	//the size of Header : dwHeaderSize = sizeof(ROMFILE_HEAD)
		type_byte sFormatVersion[4];	// the version of header format. Now 1.0.0.0
										// sFormatVersion[0] : major version
										// sFormatVersion[1] : minor version
										// sFormatVersion[2] : bug fix version
										// sFormatVersion[3] : build version
		//
		type_byte sRFU[MAX_RFU_SIZE];

		type_dword dwItem;			// the number of item.(max MAX_ROMFILE_HEAD_ITEAM )
		ROMFILE_HEAD_ITEAM Item[MAX_ROMFILE_HEAD_ITEAM];

	}__attribute__ ((packed)) ROMFILE_HEAD, *PROMFILE_HEAD, *LPROMFILE_HEAD;

public:
	static inner_rom & get_instance()
	{
		static inner_rom obj;
		return obj;
	}

	~inner_rom();

	type_result load_header( const string & s_rom_file, PROMFILE_HEAD p_header );

	int get_updateble_item_index(
			const inner_rom::PROMFILE_HEAD p_header
			, const type_byte *s_model
			, type_byte c_major
			, type_byte c_minor
			, type_byte c_bug_fix
			, type_byte c_build );

	type_result get_item( const inner_rom::PROMFILE_HEAD p_header, int nIndex,  PROMFILE_HEAD_ITEAM pItem );

	unsigned int read_binary_of_item(
			type_byte *s_read
			, unsigned int dw_read
			, unsigned int dw_offset
			, const inner_rom::PROMFILE_HEAD_ITEAM p_item );

	type_result CreateHeader( const string & s_rom_file );

	type_result AddItem(
		const string & s_bin_file,
		type_byte c_major,
		type_byte c_minor,
		type_byte c_bug_fix,
		type_byte c_build,
		type_byte *s_model,
		type_dword dw_update_condition
		);


private:
	inner_rom();

private:
	string m_s_rom_file_name;

private://don't call these method
	inner_rom( const inner_rom &);
	inner_rom & operator = ( const inner_rom & );
};


#endif /* INC_INNER_ROM_H_ */
