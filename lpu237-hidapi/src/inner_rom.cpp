/*
 * inner_rom.cpp
 *
 *  Created on: 2018. 7. 6.
 *      Author: totoro
 */

#include "inner_rom.h"
#include <string.h>
#include <fstream>
#include <KISA_SHA256.h>

inner_rom::inner_rom()
{
}

inner_rom::~inner_rom()
{
}


inner_rom::type_result inner_rom::load_header( const string & s_rom_file, PROMFILE_HEAD pHeader )
{
	inner_rom::type_result result(inner_rom::result_error);

	do{
		if( s_rom_file.empty() || pHeader == NULL ){
			continue;
			result = inner_rom::result_error_invalid_parameter;
		}
		//
		ifstream Firmware;

		Firmware.open( s_rom_file, ios::binary | ios::in  );

		if( !Firmware ){
			continue;
			result = inner_rom::result_error_not_found;
		}

		// check length
		Firmware.seekg( 0, Firmware.end );
		unsigned long nFileSize = (unsigned long) Firmware.tellg();
		if(  nFileSize < (unsigned long)sizeof( inner_rom::ROMFILE_HEAD) ){
			continue;
			result = inner_rom::result_error_shorter_then_expected;
		}

		Firmware.seekg(0);
		Firmware.read( reinterpret_cast< char* >(pHeader), (streamsize)sizeof( inner_rom::ROMFILE_HEAD) );
		Firmware.close();
		m_s_rom_file_name = s_rom_file;
		result = inner_rom::result_success;
	}while(0);
	return result;
}

int inner_rom::get_updateble_item_index(
		const inner_rom::PROMFILE_HEAD p_header
		, const type_byte *s_model
		, type_byte c_major
		, type_byte c_minor
		, type_byte c_bug_fix
		, type_byte c_build )
{
	int n_index(-1);

	do{
		if( p_header == NULL || s_model == NULL  )
			continue;
		if( p_header->dwItem == 0 || p_header->dwItem > inner_rom::MAX_ROMFILE_HEAD_ITEAM )
			continue;
		//check Model name
		type_dword i, j = 0;
		bool bEqualModel = true;

		for( i =0; i<p_header->dwItem; i++ ){
			bEqualModel = true;
			j = 0;
			while( s_model[j] != 0x00 && p_header->Item[i].sModel[j] != 0x00 ){

				if( p_header->Item[i].sModel[j] != s_model[j] ){
					bEqualModel = false;
					break;
				}

				j++;
			}//end while

			if( bEqualModel ){
				n_index = i;
				break;	//exit for
			}
		}//end for

		if( n_index < 0)
			continue;
		//
		bool bConditionOK = false;
		type_dword dwCondition = p_header->Item[n_index].dwUpdateCondition;

		// found name is equal.......
		if( p_header->Item[n_index].sVersion[0] > c_major ){
			//rom file version  greater then given version.
			if( (dwCondition & inner_rom::condition_gt) || (dwCondition & inner_rom::condition_neq) )
				bConditionOK = true;
		}
		else if( p_header->Item[n_index].sVersion[0] < c_major ){
			//rom file version  less then given version.
			if( (dwCondition & inner_rom::condition_lt) || (dwCondition & inner_rom::condition_neq) )
				bConditionOK = true;
		}
		else{
			//rom file version  equal given version.
			if( dwCondition & inner_rom::condition_eq ){

				if( p_header->Item[n_index].sVersion[1] == c_minor &&
					p_header->Item[n_index].sVersion[2] == c_bug_fix &&
					p_header->Item[n_index].sVersion[3] == c_build
					){
						bConditionOK = true;
				}
			}
		}
		///
		BYTE sRomVer[] = { p_header->Item[n_index].sVersion[1], p_header->Item[n_index].sVersion[2], p_header->Item[n_index].sVersion[3] };
		BYTE sGivenVer[] = {  c_minor, c_bug_fix, c_build };
		int nVer = 0;
		while( !bConditionOK && nVer < 3 ){

			if( sRomVer[nVer] > sGivenVer[nVer] ){
				//rom file version  greater then given version.
				if( (dwCondition & inner_rom::condition_gt) || (dwCondition & inner_rom::condition_neq) )
					bConditionOK = true;
			}
			else if( sRomVer[nVer] < sGivenVer[nVer] ){
				//rom file version  less then given version.
				if( (dwCondition & inner_rom::condition_lt) || (dwCondition & inner_rom::condition_neq) )
					bConditionOK = true;
			}

			nVer++;
		}//end while

		if( !bConditionOK )
			n_index = -1;

	}while(0);

	return n_index;
}

inner_rom::type_result inner_rom::get_item(
		const inner_rom::PROMFILE_HEAD p_header
		, int n_index
		,  PROMFILE_HEAD_ITEAM p_item )
{
	inner_rom::type_result result(inner_rom::result_error_invalid_parameter);

	do{
		if( p_header == NULL || n_index < 0 || n_index > (inner_rom::MAX_ROMFILE_HEAD_ITEAM-1) || p_item == NULL )
			continue;
		//
		if( (type_dword)n_index > (p_header->dwItem-1) )
			continue;

		memcpy( p_item, &(p_header->Item[n_index]), sizeof(inner_rom::ROMFILE_HEAD_ITEAM) );

		result = inner_rom::result_success;
	}while(0);
	return result;
}

unsigned int inner_rom::read_binary_of_item(
		type_byte *s_read
		, unsigned int dw_read
		, unsigned int dw_offset
		, const inner_rom::PROMFILE_HEAD_ITEAM p_item )
{
	int n_read(0);

	do{
		if( p_item == NULL || m_s_rom_file_name.empty() || s_read==NULL || dw_read == 0 )
			continue;

		ifstream rom;
		rom.open(  m_s_rom_file_name, ios::binary | ios::in );
		if( !rom )
			continue;
		//
		rom.seekg( p_item->dwOffset, rom.beg );
		rom.seekg( dw_offset, rom.cur );

		rom.read( reinterpret_cast<char*>(s_read), dw_read );

		n_read = rom.gcount();

		rom.close();

	}while(0);
	return n_read;
}

inner_rom::type_result inner_rom::CreateHeader( const string & s_rom_file )
{
	inner_rom::type_result result(inner_rom::result_error_not_open_file);

	do{
		ofstream romfile;

		romfile.open( s_rom_file,ofstream::binary | ios::out | ios::trunc );
		if( !romfile.is_open() )
			continue;

		romfile.seekp(0);

		inner_rom::ROMFILE_HEAD header;

		::memset( &header, 0, sizeof( header ) );

		header.dwHeaderSize = sizeof( inner_rom::ROMFILE_HEAD );
		header.sFormatVersion[0] = 1;
		header.sFormatVersion[1] = 1;//2016/6/10
		header.sFormatVersion[2] = 0;
		header.sFormatVersion[3] = 0;
		//
		romfile.write( reinterpret_cast<const char*>(&header), (streamsize)sizeof( header ) );
		romfile.flush();

		romfile.close();

		m_s_rom_file_name = s_rom_file;
		result = inner_rom::result_success;
	}while(0);
	return result;
}

inner_rom::type_result inner_rom::AddItem(
	const string &  s_bin_file,
	type_byte cMajor,
	type_byte cMinor,
	type_byte cBugFix,
	type_byte cBuild,
	type_byte *sModel,
	type_dword dwUpdateCondition
	)
{
	inner_rom::type_result result(inner_rom::result_error_invalid_parameter);

	do{
		if( m_s_rom_file_name.empty() || s_bin_file.empty() )
			continue;

		ifstream rRom;
		rRom.open( m_s_rom_file_name, ios::binary  | ios::in );
		if( !rRom ){
			result = inner_rom::result_error_not_found;
			continue;
		}

		rRom.seekg( 0, rRom.end );
		type_dword nLen = rRom.tellg();

		if( nLen < (type_dword)sizeof( inner_rom::ROMFILE_HEAD )){
			rRom.close();
			result = inner_rom::result_error_shorter_then_expected;
			continue;
		}
		//// load header.
		inner_rom::ROMFILE_HEAD header;
		rRom.seekg( 0, rRom.beg );
		rRom.read( reinterpret_cast<char *>(&header), (streamsize)sizeof( header ) );
		rRom.close();

		if( header.dwItem == inner_rom::MAX_ROMFILE_HEAD_ITEAM ){
			result = inner_rom::result_error_over_capacity;
			continue;
		}

		//here load header OK.
		ifstream Bin;

		Bin.open( s_bin_file, ios::binary | ios::in );

		if( !Bin ){
			result = inner_rom::result_error_not_found;
			continue;
		}

		//
		Bin.seekg( 0, Bin.end );
		nLen = Bin.tellg();

		if( nLen > inner_rom::MAX_SIZE_APP ){
			Bin.close();
			result = inner_rom::result_error_greater_then_expected;
			continue;
		}

		Bin.seekg( 0, Bin.beg );

		fstream wRom;
		wRom.open( m_s_rom_file_name,ofstream::binary | ios::out | ios::in );
		if( !wRom.is_open() ){
			Bin.close();
			result = inner_rom::result_error_not_open_file;
			continue;
		}

		//
		wRom.seekp( 0,wRom.end);

		//create item header
		inner_rom::ROMFILE_HEAD_ITEAM item;

		::memset( &item, 0, sizeof( item ) );
		item.dwSize = static_cast<type_dword>(nLen);
		item.dwOffset = wRom.tellp();
		item.sVersion[0] = cMajor;
		item.sVersion[1] = cMinor;
		item.sVersion[2] = cBugFix;
		item.sVersion[3] = cBuild;
		item.dwUpdateCondition = dwUpdateCondition;

		if( sModel ){

			//get model length except terminator NULL.
			int nModel = 0;

			while( sModel[nModel] ){
				nModel++;
			};

			if( nModel > inner_rom::MAX_MODEL_NAME_SIZE ){
				Bin.close();
				wRom.close();
				result = inner_rom::result_error_greater_then_expected;
				continue;
			}
			//
			::memcpy( item.sModel, sModel, nModel );
		}

		//
		SHA256_INFO sha_info;
		SHA256_Init( &sha_info  );


		//update header
		::memcpy( &header.Item[header.dwItem], &item, sizeof( item ));
		header.dwItem++;

		//merge binary file to rom file
		// bin -> wRom
		#define	BUF_LEN	 4*1024
		int nBuffer = BUF_LEN;
		char sBuffer[BUF_LEN];
		unsigned int i = 0;
		int j;

		while( Bin && !Bin.eof() ){

			 nBuffer = BUF_LEN;
			Bin.read( sBuffer, nBuffer );

			nBuffer = Bin.gcount();
			wRom.write( sBuffer, nBuffer );
			if( i==0 ){
				for( j=0; j<nBuffer; j++ ){
					if( j%2 == 0 )
						sBuffer[j] = sBuffer[j] ^ 0xC0;
					else
						sBuffer[j] = sBuffer[j] ^ 0xFF;
				}//end ofr
			}

			SHA256_Process(  &sha_info, (const  BYTE *)sBuffer, nBuffer  );
			++i;
		}//end while


		SHA256_Close( &sha_info,header.Item[header.dwItem-1].sHash );
		//write header.
		wRom.seekp( 0, wRom.beg );
		wRom.write( reinterpret_cast<const char *>(&header), (streamsize)sizeof( header ) );
		//
		Bin.close();
		wRom.flush();
		wRom.close();
		//
		result = inner_rom::result_success;
	}while(0);
	return result;
}
