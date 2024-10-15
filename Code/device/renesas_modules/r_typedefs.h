/*
****************************************************************************
PROJECT : VLIB
FILE    : $Id: r_typedefs.h 3854 2014-09-15 14:29:17Z florian.zimmermann $
============================================================================
DESCRIPTION
Predefined data types
============================================================================
                            C O P Y R I G H T
============================================================================
                           Copyright (c) 2012
                                  by
                       Renesas Electronics (Europe) GmbH.
                           Arcadiastrasse 10
                          D-40472 Duesseldorf
                               Germany
                          All rights reserved.
============================================================================
Purpose: only for testing

DISCLAIMER
This software is supplied by Renesas Electronics Corporation and is only
intended for use with Renesas products. No other uses are authorized. This
software is owned by Renesas Electronics Corporation and is protected under
all applicable laws, including copyright laws.
THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
Renesas reserves the right, without notice, to make changes to this software
and to discontinue the availability of this software. By using this software,
you agree to the additional terms and conditions found by accessing the
following link:
http://www.renesas.com/disclaimer *
Copyright (C) 2011 Renesas Electronics Corporation. All rights reserved.

****************************************************************************
*/

#ifndef R_TYPEDEF_H
#define R_TYPEDEF_H

/*******************************************************************************
*
*                   Defines
*
************************************************************
*/
#ifdef __GHS__
#pragma ghs startnomisra
#endif
// #include "stddef.h"

#define R_STR(s)           # s
#define R_XSTR(s)          R_STR(s)
#define R_DOT(a,b)         a ## . ## b
#define R_XDOT(a,b)        R_DOT(a,b)

#define R_SPACE            " "
#define R_VERSIONPREFIX    "@(#)Version "
#define R_VERSIONPOSTFIX   " - "

#define R_VERSION(macro, hi, lo)        { "@(#)" # macro " driver version "  R_XSTR(hi) "."  R_XSTR(lo)}

#if defined (R_DBG_PRINT_MSG)
#define R_HALT_ON_ERROR(txt)    {R_DBG_PRINT(R_DBG_MSG_ERR, txt); for ( ; ; ){}}
#define R_DISPLAY_ERROR(txt)     R_DBG_PRINT(R_DBG_MSG_ERR, txt)
#define R_LOG(...)              printf(__VA_ARGS__)

#else
#define R_HALT_ON_ERROR(txt)    {for ( ; ; ){}}
#define R_DISPLAY_ERROR(txt)
#define R_LOG(...)
#endif

#define R_INLINE inline

#ifdef __GHS__
#pragma ghs endnomisra
#endif


/*******************************************************************************
*
*                   Enumerators
*
************************************************************
*/

typedef enum
{
    R_OK = 0u,
    R_ERROR,
    R_ERROR_NOTIFY,
    R_PARAMETER_MISSING,
    R_PARAMETER_RANGE,
    R_PARAMETER_INCORRECT,
    R_DATA_MISMATCH
} r_Error_t;


/*******************************************************************************
*
*                   Types
*
************************************************************
*/

/* Define ISO conform exact types, if the compiler does not
   support them,
*/

#ifndef uint8_t
typedef char               char_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed long        int32_t;
typedef signed long long   int64_t;
//typedef unsigned char      uint8;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned long      uint32_t;
typedef unsigned long long uint64_t;
// typedef unsigned int size_t;
typedef unsigned int FlagType;
//typedef uint8 ConfirmFailureType;
//typedef uint8 IndicationFailureType;
#endif /* uint8_t */


typedef void* handle_t;

/* Version info */
typedef struct
{
    uint16_t major;
    uint16_t minor;
} r_version_t;


#ifndef R_NULL
#define R_NULL ((void*)0)
#endif /* R_NULL */

#ifndef R_TRUE
#define R_TRUE 1
#endif /* R_TRUE */

#ifndef R_FALSE
#define R_FALSE 0
#endif /* R_FALSE */

#define R_EXTERN extern


#endif /* R_TYPEDEF_H_ */




