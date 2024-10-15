/*******************************************************************************
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
* (c) 2014 Renesas Electronics Corporation All rights reserved.
*******************************************************************************/


/******************************************************************************
* File Name     : r_csih.c
* Version       : 1.0
* Device(s)     : R7F701035xAFP RH850/F1L
* Tool-Chain    : CubeSuite+(V2.01.00)
* Description   : This file is a sample of the CSIH communication.
* Operation     : -
*******************************************************************************
*******************************************************************************
* History       : DD.MM.YYYY Version Description
*               : 20.03.2014 1.00    First Release
******************************************************************************/
#define __R_CSIH_C_

/******************************************************************************
Includes <System Includes> , �gProject Includes�h
******************************************************************************/
#include	"r_csih.h"
#include 	"r_port.h"
#include    "write_protect.h"


#include    "com_dll.h"
#include    "businterface.h"
/******************************************************************************
Macro definitions
******************************************************************************/
/* define value for interrupt */
#define INTP_RF_BIT           (0x1000)
#define INTP_MK_BIT           (0x0080)
#define INTP_TB_BIT           (0x0040)
#define INTP_PRIORITY_VALUE   (0x0000)

#define INTP_ENABLE_VALUE     (              INTP_TB_BIT | INTP_PRIORITY_VALUE  ) /*0x0047*/
#define INTP_DISABLE_VALUE    (INTP_MK_BIT | INTP_TB_BIT | INTP_PRIORITY_VALUE  ) /*0x00C7*/

/******************************************************************************
Private global variables and functions
******************************************************************************/
volatile RCSIHStatusType RCSIHStatus = R_CSIH_SHUTDOWN;
uint8 shiftBits;

void R_CSIH_ClkInit(void)
{
    /* CSIH clock */
    protected_write(PROTCMD1,PROTS1,CKSC_ICSIS_CTL,0x02u);
    //while(CKSC_ICSIS_ACT!=0x02u);
}

void R_CSIH_Init(void)
{
#ifdef R_CSIH_C1_SLAVE
    /* port initilization */
    R_PORT_SetAltFunc(Port10, 1, Alt5, Input); //clk
    R_PORT_SetAltFunc(Port10, 2, Alt5, Output); //slave output
    R_PORT_SetAltFunc(Port10, 0, Alt5, Input);//slave input

    __DI();
    /* register initilization */
    R_csih1_slave_init();

    RCSIHStatus = R_CSIH_READY;
    /* receive start */
    R_csih1_slave_reception_enable();
    __EI();
#else
    /* port initilization */
    R_PORT_SetAltFunc(Port10, 1, Alt5, Output); //clk
    /* clock initialization */
    R_CSIH_ClkInit();
    R_PORT_SetAltFunc(Port10, 2, Alt5, Output); //master output
    R_PORT_SetAltFunc(Port10, 0, Alt5, Input);//master input
    PIPC10	= 0x0006U;

    __DI();
    /* register initilization */
    R_csih1_master_init();

    RCSIHStatus = R_CSIH_READY;
    R_csih1_master_transmission_enable();
    __EI();

    /* Transmit handshake message to synchronize with slave */
    R_csih1_master_transmit(0xff << 4);

#endif
}

void R_csih1_send_string(uint16 transferByte)
{
#ifdef R_CSIH_C1_MASTER
    /* If rcsih is not in SYNC state, which means slave is not synchronized to master
     * In this case, stop transmission and return error. */
    if(RCSIHStatus != R_CSIH_SYNC)
        return;
#ifdef R_CSIH_C1
    R_csih1_master_transmit(transferByte);
#endif

#endif
}

void R_csih1_GetDllInfo(RCSIHInfoRefType infoRef)
{
    infoRef->rs_csih_status = CSIH1.STR0 & 0xffff;
    infoRef->rs_csih_status_clear = &(CSIH1.STCR0);
    infoRef->rs_csih_rx0h = &(CSIH1.RX0H);
}

void R_csih1_master_init( void )
{
#ifdef R_CSIH_C1_MASTER
    CSIH1.CTL0.UINT8 = 0x00;
    CSIH1.CTL1       = 0x00000020;
    CSIH1.CTL2       = 0x4000;
    CSIH1.BRS0       = 0x005F;
    CSIH1.CFG0       = 0x00000000;
    CSIH1.MCTL0      = 0x00;
#endif
}

void R_csih1_master_transmission_enable( void )
{
#ifdef R_CSIH_C1_MASTER
    /* interrupt setting */
    ICCSIH1IC  = INTP_ENABLE_VALUE;
    /* transmission start */
    CSIH1.CTL0.UINT8 = 0xC1;
#endif
}

void R_csih1_master_transmission_disable( void )
{
#ifdef R_CSIH_C1_MASTER
    /* interrupt setting */
    ICCSIH0IC  = INTP_DISABLE_VALUE;
    /* transmission stop */
    CSIH1.CTL0.UINT8 = 0x01;
#endif
}

void R_csih1_master_transmit( uint16_t uint16_tx )
{
#ifdef R_CSIH_C1_MASTER
    CSIH1.TX0H        = uint16_tx;
#endif
}

void R_csih1_slave_init( void )
{
#ifdef R_CSIH_C1_SLAVE
    CSIH1.CTL0.UINT8 = 0x00;
    CSIH1.CTL1       = 0x00010000;
    CSIH1.CTL2       = 0xE000;
    CSIH1.BRS0       = 0x0000;
    CSIH1.CFG0       = 0x00000000;
    CSIH1.MCTL0      = 0x00;
    CSIH1.MCTL1      = 0x7F;
#endif
}

void R_csih1_slave_reception_enable( void )
{
#ifdef R_CSIH_C1_SLAVE
    /* interrupt setting */
    ICCSIH1IR  = INTP_ENABLE_VALUE;
    /* initial */
    CSIH1.CTL0.UINT8 = 0x00;
    /* reception start */
    CSIH1.CTL0.UINT8 = 0xA0;
#endif
}

void R_csih1_slave_reception_disable( void )
{
#ifdef R_CSIH_C1_SLAVE
    /* interrupt setting */
    ICCSIH1IR  = INTP_DISABLE_VALUE;

    /* reception stop */
    CSIH1.CTL0.UINT8 = 0x00;
#endif
}

void R_csih1_slave_receive( void )
{
}


#pragma interrupt intr_csih1ic_function( enable=false , channel=108 , callt=false , fpu=false )
void intr_csih1ic_function( void )
{
#ifdef R_CSIH_C1_MASTER
    if(RCSIHStatus == R_CSIH_READY)
    {
        /* After hankshake message is transmitted, change rcsih state
         * and is ready to transmit normal message. */
        RCSIHStatus = R_CSIH_SYNC;
    }
    else if (RCSIHStatus == R_CSIH_SYNC)
    {
#ifdef R_CSIH_C1
        Osek_Com_Nm_IntHandler(RCSIH_Tx,R_CSIH_C1);
#endif
    }
#endif
}

#pragma interrupt intr_csih1ir_function( enable=false , channel=109 , callt=false , fpu=false )
void intr_csih1ir_function( void )
{
#ifdef R_CSIH_C1_SLAVE
    /* If received handshake message, restart recepjtion in order to
     * synchronize to master clock and change state. */
    CSIH1.STCR0 = 0x0100;
    if(RCSIHStatus == R_CSIH_READY && CSIH1.RX0H != 0x0000)
    {
        CSIH1.CTL0.UINT8 = 0x00;
        CSIH1.CTL0.UINT8 = 0xA0;
        RCSIHStatus = R_CSIH_SYNC;
        shiftBits = CSIH1.RX0H & 0xF;
        shiftBits = !(shiftBits & 1U) + !((shiftBits >> 1) & 1U) + !((shiftBits >> 2) & 1U) + !((shiftBits >> 3) & 1U);
    }
    else if (RCSIHStatus == R_CSIH_SYNC)
    {
#ifdef R_CSIH_C1
        Osek_Com_Nm_IntHandler(RCSIH_Rx,R_CSIH_C1);
#endif
    }
#endif
}
