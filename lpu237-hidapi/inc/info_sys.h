
/*
 * this file have to be included in "ONLY" share.h and share.c.
 * Don't include this file in the other.
 *
 * system configuration header
 *
 */

#ifndef _INFO_SYSTEM_201007026001H_
#define _INFO_SYSTEM_201007026001H_

#include "compiler.h"
#include "info_msr.h"

// NOTICE
// enum size must be byte.  -fshort-enums in gcc

#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32

#define	SYSTEM_STATUS_NUMBER		4	//the number of system status

enum SYSTEM_STATUS		//Declare system status( SystemStatusNoChange isn't invalid event )
{
	SystemStatusNoChange	= -1,	//don't change system status
	SystemStatusNotReady	= 0,	//system is waiting the end of setup.
	SystemStatusNormal		= 1,	//system normal(idle - setup ok & waits request )
	SystemStatusConfig		= 2,	//system is configuration mode.
	SystemStatusProcessing	= 3		//system is processing a card data.
};

#define	SYSTEM_EVENT_NUMER			6	//the number of event( None isn't invalid event )

enum SYSTEM_EVENT		//Declare system events
{
	SystemEventNone				= -1,	//none event
	SystemEventSetupComplete	= 0,	//setup complete
	SystemEventBusReset			= 1,				//bus reseted
	SystemEventEnterConfig		= 2,				//enter configuration mode
	SystemEventLeaveConfig		= 3,				//leave configuration mode
	SystemEventStartProcessing	= 4,			//start card data processing
	SystemEventCompleteProcessing	= 5		//complete card processing
};
/*
 * system mode parameter must be unsigned char type &
 * the high nibble is application mode.
 * the low nibble is boot-loader mode.
 */

enum SYSTEM_MODE	// Declare system mode
{
	SystemModeEmpty		= 0xff,	//dummy : No program
	SystemModeFirst 	= 0,	//the first execution program
	SystemModeNormal	= 0x01,	//normal operation
	SystemModeConfig	= 0x02,	//configuration mode
	SystemModeTest		= 0x0c,	//Individual test mode
	SystemModeDebug		= 0x0d	//debugging mode
};


enum SYSTEM_INTERFACE		//declare system interface.
{
	SystemInterfaceUsbKB	= 0,	//system interface is USB keyboard.
	SystemInterfaceUsbMsr	= 1,	//system interface is USB MSR(generic HID interface).
	SystemInterfaceUart		= 10	//system interface is uart.
};

/*
 * this structure define Uart setting
 */
typedef	struct tagUARTINFO{

	UINT32 nCom;	//the com-port number
	UINT32 nBaud;	//uart baud rate

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT UARTINFO, *PUARTINFO, *LPUARTINFO;


/*
 * this container structure of i-Button & uart
 */
//
typedef struct tagINFO_PRE_POST_OBJ{
	/////////////////////////////////////////////////////////////////////
	//TagPre & Post is displayed always.

	//if all MsrObject occured error, display.
	MSR_TAG TagPre;
	MSR_TAG TagPost;

	/////////////////////////////////////////////////////////////////////
	//GlobalPre & Post is displayed when a MsrObject have been processed.
	//if all MsrObject occured error, not display.
	MSR_TAG GlobalPrefix;
	MSR_TAG GlobalPostfix;

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT INFO_PRE_POST_OBJ, *PINFO_PRE_POST_OBJ,*LPINFO_PRE_POST_OBJ;

/////////////////////////////////////////////////////////

#define	SYSTEM_SIZE_BLANK			4
#define	SYSTEM_SIZE_STRUCTVER		4
#define	SYSTEM_SIZE_NAME			16
#define	SYSTEM_SIZE_SYSVER			4
#define	SYSTEM_SIZE_SN				8
/*
 * this structure is saved to flash
 */
typedef struct tagSYSINFO_STD{

	unsigned char cBlank[SYSTEM_SIZE_BLANK];	//Don't use this member

	UINT32 dwSize;		//the size of this structure = sizeof(SYSINFO)

	unsigned char sStrucVer[SYSTEM_SIZE_STRUCTVER];	//	 	: the version of structure 3.0.0.0
								//    	  sVersion[0]-marjor version (2)
								//		  sVersion[1]-minor version  (0)
								//		  sVersion[2]-fix mistake version  (0)
								// 		  sVersion[3]-build version (0)
								//default: sStrucVer[]={2,0,0,0}

	unsigned char sName[SYSTEM_SIZE_NAME];	//16: the name of system( "tylenol" fix value)
											//default: sName[]={"tylenol"}

	unsigned char sSysVer[SYSTEM_SIZE_SYSVER];	//4(32) : the version of system
								//    	  sVersion[0]-marjor version
								//		  sVersion[1]-minor version
								//		  sVersion[2]-fix mistake version
								// 		  sVersion[3]-build version
								//default: sSysVer[]={1,0,0,0}
#ifdef	WIN32
	unsigned char ModeBL;	//the current boot loader system mode
	unsigned char ModeAP;	//the current application system mode
#else
	enum SYSTEM_MODE ModeBL;	//the current boot loader system mode
	enum SYSTEM_MODE ModeAP;	//the current application system mode
#endif	//WIN32

	unsigned char sSN[SYSTEM_SIZE_SN];		// :serial number : use only sSN[0]~[5], [6]~[7]: RFU
											//default: sSN[]={'0','0','0','0','0','0','0','0'}

#ifdef	WIN32
	unsigned char Interface;	//the current active interface.
#else
	enum SYSTEM_INTERFACE Interface;	//the current active interface.
#endif	//WIN32

	UINT32 nBuzzerFrequency;		//buzzer frequency(Hz)

	UINT32 nNormalWDT;			//the watch-dog timeout value unit : 10msec
	UINT32 nBootRunTime;			//the bootload running time by bootload run command

	UARTINFO Uart;				//the current uart set

	CONTAINER_INFO_MSR_OBJ ContainerInfoMsrObj;	//msr info container

	INFO_MSR_OBJ InfoMsr[MSROBJ_INFO_NUM];		//msr info

	INFO_PRE_POST_OBJ InfoIButton;	//i-button pre/postfix information.	- 3.0.0.0 new member.

	INFO_PRE_POST_OBJ InfoUart;		// uart pre/postfix information.	- 3.0.0.0 new member.

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT SYSINFO_STD, *PSYSINFO_STD, *LPSYSINFO_STD;

typedef struct tagSYSINFO{

	unsigned char cBlank[SYSTEM_SIZE_BLANK];	//Don't use this member

	UINT32 dwSize;		//the size of this structure = sizeof(SYSINFO)

	unsigned char sStrucVer[SYSTEM_SIZE_STRUCTVER];	//	 	: the version of structure 2.0.0.0
								//    	  sVersion[0]-marjor version (2)
								//		  sVersion[1]-minor version  (0)
								//		  sVersion[2]-fix mistake version  (0)
								// 		  sVersion[3]-build version (0)
								//default: sStrucVer[]={2,0,0,0}

	unsigned char sName[SYSTEM_SIZE_NAME];	//16: the name of system( "tylenol" fix value)
											//default: sName[]={"tylenol"}

	unsigned char sSysVer[SYSTEM_SIZE_SYSVER];	//4(32) : the version of system
								//    	  sVersion[0]-marjor version
								//		  sVersion[1]-minor version
								//		  sVersion[2]-fix mistake version
								// 		  sVersion[3]-build version
								//default: sSysVer[]={1,0,0,0}
#ifdef	WIN32
	unsigned char ModeBL;	//the current boot loader system mode
	unsigned char ModeAP;	//the current application system mode
#else
	enum SYSTEM_MODE ModeBL;	//the current boot loader system mode
	enum SYSTEM_MODE ModeAP;	//the current application system mode
#endif	//WIN32

	unsigned char sSN[SYSTEM_SIZE_SN];		// :serial number : use only sSN[0]~[5], [6]~[7]: RFU
											//default: sSN[]={'0','0','0','0','0','0','0','0'}

#ifdef	WIN32
	unsigned char Interface;	//the current active interface.
#else
	enum SYSTEM_INTERFACE Interface;	//the current active interface.
#endif	//WIN32

	UINT32 nBuzzerFrequency;		//buzzer frequency(Hz)

	UINT32 nNormalWDT;			//the watch-dog timeout value unit : 10msec
	UINT32 nBootRunTime;			//the bootload running time by bootload run command

	UARTINFO Uart;				//the current uart set

	CONTAINER_INFO_MSR_OBJ ContainerInfoMsrObj;	//msr info container

	INFO_MSR_OBJ InfoMsr[MSROBJ_INFO_NUM];		//msr info

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT SYSINFO, *PSYSINFO, *LPSYSINFO;

typedef struct tagSYSINFO_OLD{

	unsigned char cBlank[SYSTEM_SIZE_BLANK];	//Don't use this member

	UINT32 dwSize;		//the size of this structure = sizeof(SYSINFO)

	unsigned char sStrucVer[SYSTEM_SIZE_STRUCTVER];	//	 	: the version of structure 2.0.0.0
								//    	  sVersion[0]-marjor version (2)
								//		  sVersion[1]-minor version  (0)
								//		  sVersion[2]-fix mistake version  (0)
								// 		  sVersion[3]-build version (0)
								//default: sStrucVer[]={2,0,0,0}

	unsigned char sName[SYSTEM_SIZE_NAME];	//16: the name of system( "tylenol" fix value)
											//default: sName[]={"tylenol"}

	unsigned char sSysVer[SYSTEM_SIZE_SYSVER];	//4(32) : the version of system
								//    	  sVersion[0]-marjor version
								//		  sVersion[1]-minor version
								//		  sVersion[2]-fix mistake version
								// 		  sVersion[3]-build version
								//default: sSysVer[]={1,0,0,0}
#ifdef	WIN32
	unsigned char ModeBL;	//the current boot loader system mode
	unsigned char ModeAP;	//the current application system mode
#else
	enum SYSTEM_MODE ModeBL;	//the current boot loader system mode
	enum SYSTEM_MODE ModeAP;	//the current application system mode
#endif	//WIN32

	unsigned char sSN[SYSTEM_SIZE_SN];		// :serial number : use only sSN[0]~[5], [6]~[7]: RFU
											//default: sSN[]={'0','0','0','0','0','0','0','0'}

#ifdef	WIN32
	unsigned char Interface;	//the current active interface.
#else
	enum SYSTEM_INTERFACE Interface;	//the current active interface.
#endif	//WIN32

	UINT32 nBuzzerFrequency;		//buzzer frequency(Hz)

	UINT32 nNormalWDT;			//the watch-dog timeout value unit : 10msec
	UINT32 nBootRunTime;			//the bootload running time by bootload run command

	UARTINFO Uart;				//the current uart set

	CONTAINER_INFO_MSR_OBJ_OLD ContainerInfoMsrObj;	//msr info container

	INFO_MSR_OBJ_OLD InfoMsr[MSROBJ_INFO_NUM];		//msr info

}COMPILER_ATTRIBUTE_BYTE_ALIGNMENT SYSINFO_OLD, *PSYSINFO_OLD, *LPSYSINFO_OLD;

#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32

#endif	//_INFO_SYSTEM_201007026001H_
