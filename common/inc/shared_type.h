
/*
 * share_type.h
 *
 *  Created on: 2018. 6. 29.
 *      Author: totoro
 *      This code is released under the terms of the MIT License
 */

#ifndef INC_SHARE_TYPE_H_
#define INC_SHARE_TYPE_H_

#if __x86_64__
// 64-bit
#define type_dword		unsigned int
#define type_byte			unsigned char

#else
// 32-bit
#define type_dword		unsigned long
#define type_byte			unsigned char

#endif


#endif /* INC_SHARE_TYPE_H_ */
