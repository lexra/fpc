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
 
  MODULE NAME:  PTHREAD_INPUT.H
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
 06/30/2004 2.0 CheulBeck(whitefe)       Created 
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains definition for Input function.
  
 ...............................................................................
*/    
 
#ifndef __PTHREAD_INPUT_H
#define __PTHREAD_INPUT_H


/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include "typedef.h"

 
/** ************************************************************************* ** 
 ** typedefs
 ** ************************************************************************* **/
typedef enum {
	IR_VALUE
} IR_IOCTL_T;


  
/** ************************************************************************* ** 
 ** function prototypes
 ** ************************************************************************* **/
#ifdef __cplusplus
extern "C" {
#endif

void *pthread_input(void *args);

#ifdef __cplusplus
}
#endif


#endif /* __PTHREAD_INPUT_H */

