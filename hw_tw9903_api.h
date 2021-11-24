/* set tabstop=4 */
/********************************************************************************
 *                                                                              *
 * Copyright(C) 2004  Penta-Micro                                               *
 *                                                                              *
 * ALL RIGHT RESERVED                                                           *
 *                                                                              *
 * This software is the property of Penta-Micro and is furnished under          *
 * license by Penta-Micro. This software may be used only in accordance         *	
 * with the terms of said license. This copyright notice may not be             *
 * removed, modified or obliterated without the prior written permission        *
 * of Penta-Micro.                                                              *
 *                                                                              *
 * This software may not be copyed, transmitted, provided to or otherwise       *
 * made available to any other person, company, corporation	or other entity     *
 * except as specified in the terms of said license.                            *
 *                                                                              *
 * No right, title, ownership or other interest in the software is hereby       *
 * granted or transferred.                                                      *
 *                                                                              *
 * The information contained herein is subject to change without notice and     *
 * should not be construed as a commitment by Penta-Micro.                      *
 *                                                                              *
 ********************************************************************************
 
  MODULE NAME:  HW_TW9903_API.H
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
 06/30/2004 2.0 CheulBeck(whitefe)       Created 
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains definition for TW9903 function.
  
 ...............................................................................
*/    
 
#ifndef _HW_TW9903_API_H
#define _HW_TW9903_API_H


/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include "typedef.h"

 
/** ************************************************************************* ** 
 ** function prototypes
 ** ************************************************************************* **/
#ifdef __cplusplus
extern "C" {
#endif

int tw99_open(); 
void tw99_close();
int tw99_write_data(int slave_adr, int sub_adr, char *data, int length);
int tw99_read_data(int slave_adr, int sub_adr, unsigned char *data, int length);

#ifdef __cplusplus
}
#endif


#endif /* _HW_TW9903_API_H */

