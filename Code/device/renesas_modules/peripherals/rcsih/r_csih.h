/*****************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.
*****************************************************************************/
/*******************************************************************************
* File Name : r_csih.h
* Description : Definition to use by a sample program.
******************************************************************************/

#ifndef __R_CSIH_H_
#define __R_CSIH_H_
#include "com_type.h"
#include "r_typedefs.h"
/*----------------------------------------------------------------------------*/
#ifndef __R_CSIH_C_
#define CLASS_CSIH extern
#else
#define CLASS_CSIH
#endif

/******************************************************************************
Typedef definitions
******************************************************************************/
/* Those masks are made based on RCSIHSTR0 register, please checkout the spec */
#define R_CSIH_DC_ERROR     ((1U)<<3)
#define R_CSIH_OF_ERROR     ((1U)<<14)

typedef void (*CSIH_CALLBACK_POINTER)(void);
typedef enum
{
    R_CSIH_SHUTDOWN = 0,
    R_CSIH_READY,
    R_CSIH_SYNC,
    R_CSIH_TRANSMIT,
} RCSIHStatusType;

typedef struct
{
    volatile uint16 rs_csih_status;
    volatile uint16 *rs_csih_status_clear;
    volatile const uint16 *rs_csih_rx0h;
} RCSIHInfoType;
typedef RCSIHInfoType* RCSIHInfoRefType;
extern uint8 shiftBits;
extern volatile RCSIHStatusType RCSIHStatus;
/******************************************************************************
Functions for bus interface
******************************************************************************/
void R_csih1_send_string(uint16 transferByte);
void R_csih1_GetDllInfo(RCSIHInfoRefType infoRef);

/******************************************************************************
Global functions
******************************************************************************/
void R_CSIH_ClkInit(void);
void R_CSIH_Init(void);
CLASS_CSIH void R_csih1_master_init( void );
CLASS_CSIH void R_csih1_master_transmission_enable( void );
CLASS_CSIH void R_csih1_master_transmit( uint16_t uint16_tx );
CLASS_CSIH void R_csih1_master_transmission_disable( void );

CLASS_CSIH void R_csih1_slave_init( void );
CLASS_CSIH void R_csih1_slave_reception_enable( void );
CLASS_CSIH void R_csih1_slave_reception_disable( void );
CLASS_CSIH void R_csih1_slave_receive( void );
#endif
