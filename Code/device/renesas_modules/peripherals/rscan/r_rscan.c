/*===========================================================================*/
/* Project:  F1x StarterKit V3 Sample Software                               */
/* Module :  r_rscan.c                                                       */
/* Version:  V1.00                                                           */
/*===========================================================================*/
/*                                  COPYRIGHT                                */
/*===========================================================================*/
/* Copyright (c) 2015 by Renesas Electronics Europe GmbH,                    */
/*               a company of the Renesas Electronics Corporation            */
/*===========================================================================*/
/* In case of any question please do not hesitate to contact:                */
/*                                                                           */
/*        ABG Software Tool Support                                          */
/*                                                                           */
/*        Renesas Electronics Europe GmbH                                    */
/*        Arcadiastrasse 10                                                  */
/*        D-40472 Duesseldorf, Germany                                       */
/*                                                                           */
/*        e-mail: software_support-eu@lm.renesas.com                         */
/*        FAX:   +49 - (0)211 / 65 03 - 11 31                                */
/*                                                                           */
/*===========================================================================*/
/* Warranty Disclaimer                                                       */
/*                                                                           */
/* Because the Product(s) is licensed free of charge, there is no warranty   */
/* of any kind whatsoever and expressly disclaimed and excluded by Renesas,  */
/* either expressed or implied, including but not limited to those for       */
/* non-infringement of intellectual property, merchantability and/or         */
/* fitness for the particular purpose.                                       */
/* Renesas shall not have any obligation to maintain, service or provide bug */
/* fixes for the supplied Product(s) and/or the Application.                 */
/*                                                                           */
/* Each User is solely responsible for determining the appropriateness of    */
/* using the Product(s) and assumes all risks associated with its exercise   */
/* of rights under this Agreement, including, but not limited to the risks   */
/* and costs of program errors, compliance with applicable laws, damage to   */
/* or loss of data, programs or equipment, and unavailability or             */
/* interruption of operations.                                               */
/*                                                                           */
/* Limitation of Liability                                                   */
/*                                                                           */
/* In no event shall Renesas be liable to the User for any incidental,       */
/* consequential, indirect, or punitive damage (including but not limited    */
/* to lost profits) regardless of whether such liability is based on breach  */
/* of contract, tort, strict liability, breach of warranties, failure of     */
/* essential purpose or otherwise and even if advised of the possibility of  */
/* such damages. Renesas shall not be liable for any services or products    */
/* provided by third party vendors, developers or consultants identified or  */
/* referred to the User by Renesas in connection with the Product(s) and/or  */
/* the Application.                                                          */
/*                                                                           */
/*===========================================================================*/
/* History:                                                                  */
/*              V1.00: Initial version                                       */
/*                                                                           */
/*===========================================================================*/
/*                                                                           */
/* Source code for the RS CAN Functions.                                     */
/*                                                                           */
/*===========================================================================*/

/*===========================================================================*/
/* Includes */
/*===========================================================================*/
#include "r_device.h"
#include "dr7f701057.dvf.h"
#include "businterface.h"
#include "r_rscan.h"
#include "r_can_table.h"
#include "r_port.h"
#include "r_intc.h"
#include "r_rlin.h"
#include "ipdu_Cfg.h"
#include "r_uart.h"
#include "requestlist.h"
#include "com_dll.h"
#include "state.h"
#include "nm_dll.h"
#include "bus_Cfg.h"
#include "nm_alarm.h"
#include "nm_alarm_Cfg.h"

/*===========================================================================*/
/* Functions */
/*===========================================================================*/
static void R_RSCAN0_SetRxRule(void);
/*===========================================================================*/
/* Global Variable */
/*===========================================================================*/
r_CANFrame_t SendFrame[2][((SEND_IPDU_MAXSIZE >> 3) + 29) >> 3];

/*===========================================================================*/
/* RSCAN Global Functions */
/*===========================================================================*/

/*****************************************************************************
** Function:    RSCAN0_Global_Launch
** Description: Initial Settings for Global CAN
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_Global_Launch(void)
{
    /*
    protected_write(PROTCMD1, PROTS1, CKSC_ICANOSCD_CTL, 0x01u); // set clk_xincan (CKSCLK_ICANOSC) = MainOSC/1 (16MHz)
    while (0x01u != CKSC_ICANOSCD_ACT)
    {
    }
    */
    /* Wait while CAN RAM initialization is ongoing */
    while ((RSCAN0GSTS & 0x00000008u))
    {
    }

    /* Switch to global/channel reset mode */
    /* Global stop mode -> Global reset mode */
    RSCAN0GCTR &= 0xfffffffbu; // GSLPR bit : 0 -> set as Other than global stop mode
    // GMDC[1:0] : 01 -> Global reset mode
    while ((RSCAN0GSTS & 0x04) != 0)
    {
    } // GSLPSTS : check status for global stop mode/*

    /* Global configuration */
    RSCAN0GCFG = 0x00000012u; // DCS bit : CAN Clock Source = clk_xincan
    // MME bit : Mirror function is disabled
    // DRE bit : DLC replacement is disabled
    // DCE bit : DLC check is enabled
    // TPRI bit : Transmit Priority = ID priority

    /* ==== Rx rule setting ==== */
    R_RSCAN0_SetRxRule();

    /* ==== Buffer setting ==== */
    RSCAN0RMNB = 0x00u; /* Can_SetGlobalBuffer, set 0 receive buffers */
    // TODO: Receive FIFO probably is not needed, test this
    RSCAN0RFCC0 = 0x00001101u;   // An interrupt occurs each time a message has been received, Depth: 4 messages, Timing Select: FIFO 1/8 full
    RSCAN0CFSTS9 = 0x00000000u;  //initialization for Transmit/receive FIFO buffer 9
    RSCAN0CFSTS12 = 0x00000000u; //initialization for Transmit/receive FIFO buffer 12
    // Transmit/receive FIFO buffer 9 are used for CAN3
    // Link to Buffer 0 CFTML[23:20],
    // Select Receive mode CFM[17:16],
    // Enable Transmit/Receive FIFO interrupt CFRXIE[1]
    RSCAN0CFCC9 = 0x00001103u;
    // Transmit/receive FIFO buffer 12 are used for CAN4
    // Link to Buffer 0 CFTML[23:20],
    // Select Receive mode CFM[17:16],
    // Enable Transmit/Receive FIFO interrupt CFRXIE[1]
    RSCAN0CFCC12 = 0x00001103u;
    /* Set THLEIE disabled, MEIE(FIFO Message Lost Interrupt disabled) */

    RSCAN0GCTR &= 0xfffff8ffu; /* TODO: for global interrupt */
    /* If GlobalChannel in halt or reset mode */
    if (RSCAN0GSTS & 0x03u)
    {
        RSCAN0GCTR &= 0xfffffffcu; /* Switch to communication mode */
        while ((RSCAN0GSTS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
        while ((RSCAN0GSTS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
    }
    // TODO: These codes are probably not needed, test these
    /* Transmit/receive FIFO buffers are used. */
    RSCAN0CFCC9 |= 0x00000001u;
    RSCAN0CFCC12 |= 0x00000001u;
}
/*****************************************************************************
** Function:    Can_SetRxRule
** Description: Set all Rx rules
** Parameter:   None
** Return:      None
******************************************************************************/
static void R_RSCAN0_SetRxRule(void)
{
    uint16_t RxRuleIdx;
    uint8_t PageRxRuleIdx;
    volatile CAN_RX_RULE_TYPE *pCRE;

    /* Set Rx rule number per channel */
    RSCAN0GAFLCFG0 |= 0x00000006u; /* Channel3 rule number is 6 */

    RSCAN0GAFLCFG1 |= 0x06000000u; /* Channel4 rule number is 6 */

    /* Get access base address of Rx rule */
    pCRE = (volatile CAN_RX_RULE_TYPE *)&(RSCAN0GAFLID0);

    /* Receive Rule Table Write Enable */
    RSCAN0GAFLECTR |= 0x00000100u;

    /* Copy Rx rule one by one */
    for (RxRuleIdx = 0U; RxRuleIdx < CAN_RX_RULE_NUM; RxRuleIdx++)
    {
        PageRxRuleIdx = (uint8_t)(RxRuleIdx & CAN_PAGE_RX_RULE_IDX_MASK);
        /* Update target Rx rule page if necessary */
        /* Receive Rule Table Page Number Configuration */
        if (PageRxRuleIdx == 0U)
        {
            /* set page number, range from 0 to 23 */
            RSCAN0GAFLECTR |= RxRuleIdx >> CAN_RX_RULE_PAGE_IDX_BIT_POS;
        }

        /* Set a single Rx rule.*/
        pCRE[PageRxRuleIdx] = CAN_RX_RULE_TABLE[RxRuleIdx];
    }

    /* Rx rule write disable */
    RSCAN0GAFLECTR &= 0xfffffeffu;
}
/*****************************************************************************
** Function:    RSCAN0_Global_Port_Deinit
** Description: Configures used pins back to default.
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_Global_Port_Deinit(void)
{
    R_PORT_SetGpioHighZ(Port1, 2);
    R_PORT_SetGpioHighZ(Port1, 3);
    R_PORT_SetGpioHighZ(Port1, 1);
    R_PORT_SetGpioHighZ(Port1, 12);
    R_PORT_SetGpioHighZ(Port1, 13);
    R_PORT_SetGpioHighZ(Port2, 6);
}
/*****************************************************************************
** Function:    RSCAN0_C3_Launch
** Description: This function init RSCAN_C3 service.
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_Launch(void)
{
    RSCAN0_C3_PORT_Init();
    RSCAN0_C3_Stop_to_Reset();
    RSCAN0_C3_Init();
    RSCAN0_C3_Reset_to_Com();
}
/*****************************************************************************
** Function:    RSCAN0_C4_Launch
** Description: This function init RSCAN_C4 service.
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_Launch(void)
{
    RSCAN0_C4_PORT_Init();
    RSCAN0_C4_Stop_to_Reset();
    RSCAN0_C4_Init();
    RSCAN0_C4_Reset_to_Com();
}
/*===========================================================================*/
/* RSCAN Channel3 Functions */
/*===========================================================================*/

/*****************************************************************************
** Function:    RSCAN0_C3_PORT_Init
** Description: Set Channel 3 TX as P1_3, RX as P1_2
                Enable CAN Transeiver through P1_1
                Enable CAN3 TX/RX/Error interrupts
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_PORT_Init(void)
{

    /* Set GPIO */
    R_PORT_SetAltFunc(Port1, 3, Alt1, Output); // CAN3 TX
    R_PORT_SetAltFunc(Port1, 2, Alt1, Input);  // CAN3 RX

    /* Enable Transceiver CAN0 */
    R_PORT_SetGpioOutput(Port1, 1, High);

    /* Enable Table Interrupt */
    R_INTC_SetPriority((uint16_t *)R_ICRCAN3ERR, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN3ERR);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN3ERR);

    R_INTC_SetPriority((uint16_t *)R_ICRCAN3REC, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN3REC);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN3REC);

    R_INTC_SetPriority((uint16_t *)R_ICRCAN3TRX, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN3TRX);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN3TRX);
}
/*****************************************************************************
** Function:    RSCAN0_C3_PORT_DeInit
** Description: Configures used pins back to default  for Channel3.
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_PORT_DeInit(void)
{
    R_PORT_SetGpioHighZ(Port1, 2);
    R_PORT_SetGpioHighZ(Port1, 3);
    R_PORT_SetGpioHighZ(Port1, 1);
}
/*****************************************************************************
** Function:    RSCAN0_C3_Init
** Description: Configures the CAN0 macro for channel 3
** Parameter:   channel Id
** Return:      None
******************************************************************************/
void RSCAN0_C3_Init(void)
{
    /* Set clk_xincan Rate */
    /* When fCAN is 16MHz, */
    /* Bitrate = fCAN/(BRP+1)/(1+TSEG1+TSEG2) = 16/1/16 = 1 Mbps */
    RSCAN0C3CFG = 0x023a0000u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=0 */

    /* Bitrate = fCAN/(BRP+1)/(1+TSEG1+TSEG2) = 16/8/16 = 0.125 Mbps */
    //RSCAN0C3CFG = 0x023a0007u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=7 */

    /* ==== Buffer setting ==== */
    RSCAN0THLCC3 = 0x00000500u;

    RSCAN0TMIEC1 = 0x00010000; /* Enable Channel 3 Buffer 0 Transmit Interrupt */
    /* enable Bus-off, error passive, error warning and bus error interrupts */
    /* entry to channel halt mode automatically at bus off entry (BOM[1:0] = b01)*/
    RSCAN0C3CTR = 0x00006f01;
}
/*****************************************************************************
** Function:    RSCAN0_C3_Stop_to_Reset
** Description: Configures the CAN0 macro for channel 3
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_Stop_to_Reset(void)
{
    /* Channel stop mode -> Channel reset mode (Channel 3) */
    // CSLPR bit : 0 -> set as Other than channel stop mode
    // CHMDC[1:0] : 01 -> Channel reset mode
    if (RSCAN0C3STSLL & 0x04u)
    {
        RSCAN0C3CTRLL &= 0x01u;

        while ((RSCAN0C3STS & 0x01) == 0)
        {
        } // CRSTSTS : check status for channel reset mode/*
    }
}

/*****************************************************************************
** Function:    RSCAN0_C3_Reset_to_Com
** Description: Change channel 3 mode from reset mode to communication mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_Reset_to_Com(void)
{
    /* If Channel3 in reset mode */
    if ((RSCAN0C3STSLL & 0x01u) && !(RSCAN0C3STSLL & 0x04u))
    {
        /* Set CHMDC[1:0] to b00, switch to communication mode */
        RSCAN0C3CTRLL = 8;
        while ((RSCAN0C3STS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
        while ((RSCAN0C3STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_C3_Com_to_Reset
** Description: Change channel 3 mode from communication mode to reset mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_Com_to_Reset(void)
{
    /* If Channel3 in communication mode */
    if (RSCAN0C3STS & 0x80u)
    {
        /* Set CHMDC[1:0] to b01, switch to reset mode */
        RSCAN0C3CTRLL = 5;
        while ((RSCAN0C3STS & 0x03u) == 0u)
        {
            /* While communication mode */
        }
        while ((RSCAN0C3STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_C3_Reset_to_Stop
** Description: Change channel 3 mode from reset mode to stop mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C3_Reset_to_Stop(void)
{
    /* If Channel3 in reset mode */
    if ((RSCAN0C3STSLL & 0x01u) && !(RSCAN0C3STSLL & 0x04u))
    {
        /* Set CHMDC[1:0] to b10, switch to halt mode */
        RSCAN0C3CTRLL = 0x05;
        while ((RSCAN0C3STS & 0x05) == 0)
        {
            /* While not in stop mode */
        } // GSLPSTS : check status for global stop mode/*
    }
}

/*****************************************************************************
** Function:    RSCAN0_C3_GetRxInfo
** Description: Return RSCAN0 channel 3 rx Information.
** Parameter:   rxInfo
** Return:      None
******************************************************************************/
void RSCAN0_C3_GetRxInfo(RSCAN0RxInfoRefType rxInfoRef)
{
    rxInfoRef->rxId = RSCAN0CFID9;
    rxInfoRef->rxDlc = RSCAN0CFPTR9HH;
    rxInfoRef->rxData = &RSCAN0CFDF09LL;
}

/*****************************************************************************
** Function:    RSCAN0_C3_GetIntStatus
** Description: Return RSCAN0 channel 3 interrupt status.
** Parameter:   intStatus
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C3_GetIntStatus(RSCAN0IntStatusRefType intStatusRef)
{
    intStatusRef->txIntStatus = RSCAN0TMSTS48 & (uint8_t)0x05u;
    intStatusRef->rxIntStatus = RSCAN0CFSTS9LL;
    return CAN_RTN_OK;
}

/*****************************************************************************
** Function:    RSCAN0_C3_GetErrorFlag
** Description: Return RSCAN0 channel 3 error flag.
** Parameter:   errorflag
** Return:      void
******************************************************************************/
void RSCAN0_C3_GetErrorFlag(uint16 *errorFlagRef)
{
    *errorFlagRef = RSCAN0C3ERFLL;
}
/*****************************************************************************
** Function:    RSCAN0_C3_TrmByTxBuf
** Description: Transmit pFrame through channel 3
** Parameter:   TxBufIdx, pFrame
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C3_TrmByTxBuf(uint8_t TxBufIdx, const r_CANFrame_t *pFrame)
{
    volatile uint8_t *pTBSR;
    r_CANFrame_t *pTxBuffer;
    volatile uint8_t *pTBCR;
    pTBSR = (volatile uint8_t *)&(RSCAN0TMSTS48);
    pTBCR = (volatile uint8_t *)&(RSCAN0TMC48);

    /* ---- Return if Tx Buffer is transmitting. ---- */

    if ((pTBSR[TxBufIdx] & (uint8_t)0x01u) == CAN_TBTST_TRANSMITTING)
    {
        return CAN_RTN_ERR;
    }

    /* Clear Tx buffer status */
    do
    {
        pTBSR[TxBufIdx] = CAN_CLR;
    }
    while (pTBSR[TxBufIdx] != CAN_CLR);

    /* Store message to Tx buffer */
    pTxBuffer = (r_CANFrame_t *)&(RSCAN0TMID48);
    pTxBuffer[TxBufIdx] = *pFrame;

    /* Set transmission request */
    pTBCR[TxBufIdx] = 1;

    // need to check transmit OK or Error ?
    //while((RSCAN0.TMSTS48 & 0x07U) != 0x04U);
    return CAN_RTN_OK;
}
/*****************************************************************************
** Function:    RSCAN0_C3_CheckDllStatus
** Description: Check if channel 3 is ready to transmit
** Parameter:   void
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C3_CheckDllStatus()
{
    volatile uint8_t *pTBSR;
    pTBSR = (volatile uint8_t *)&(RSCAN0TMSTS48);
    /* ---- Return if Tx Buffer is transmitting. ---- */
    if ((pTBSR[0] & (uint8_t)0x01u) == CAN_TBTST_TRANSMITTING) // default TxBufIdx = 0
    {
        return CAN_RTN_ERR;
    }
    else
    {
        return CAN_RTN_OK;
    }
}
/*****************************************************************************
** Interrupt:    INTRCAN3TRX
** Description: TODO
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN3TRX(enable = manual)
void INTRCAN3TRX(void)
{
#ifdef RS_CAN0_C3
    Osek_Com_Nm_IntHandler(RSCAN0_TxSuccess, RS_CAN0_C3);
    RSCAN0TMSTS48 &= 0xf0; // set TMTRF[1:0] to 0
#endif
}
/*****************************************************************************
** Interrupt:    INTRCAN3REC
** Description:
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN3REC(enable = manual)
void INTRCAN3REC(void)
{
#ifdef RS_CAN0_C3
    Osek_Com_Nm_IntHandler(RSCAN0_RxSuccess, RS_CAN0_C3);
    RSCAN0CFSTS9 &= 0xFFFFFFF7U; // set CFRXIF to 0
    RSCAN0CFPCTR9 = 0xFFU;
#endif
}
/*****************************************************************************
** Interrupt:    INTRCAN3ERR
** Description:
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN3ERR(enable = manual)
void INTRCAN3ERR(void)
{
#ifdef RS_CAN0_C3
    Osek_Com_Nm_IntHandler(RSCAN0_BusFailures, RS_CAN0_C3);
    RSCAN0C3ERFL = 0;
#endif
}
/*===========================================================================*/
/* RSCAN Channel 4 Functions */
/*===========================================================================*/

/*****************************************************************************
** Function:    RSCAN0_C4_PORT_Init
** Description: Set Channel 4 TX as P1_13, RX as P1_12
                Enable CAN Transceiver through setting P2_6
                Enable TX/RX/Error interrupts
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_PORT_Init(void)
{
    /* Set GPIO */

    R_PORT_SetAltFunc(Port1, 13, Alt1, Output); // CAN4 TX, sender
    R_PORT_SetAltFunc(Port1, 12, Alt1, Input);  // CAN4 RX, Receiver

    /* Enable Transceiver CAN1 */
    R_PORT_SetGpioOutput(Port2, 6, High);

    /* Enable Table Interrupt */
    R_INTC_SetPriority((uint16_t *)R_ICRCAN4ERR, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN4ERR);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN4ERR);

    R_INTC_SetPriority((uint16_t *)R_ICRCAN4REC, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN4REC);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN4REC);

    R_INTC_SetPriority((uint16_t *)R_ICRCAN4TRX, 1);
    R_INTC_SetTableBit((uint16_t *)R_ICRCAN4TRX);
    R_INTC_UnmaskInterrupt((uint16_t *)R_ICRCAN4TRX);
}
/*****************************************************************************
** Function:    RSCAN0_C4_PORT_DeInit
** Description: Configures used pins back to default for Channel4.
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_PORT_DeInit(void)
{
    R_PORT_SetGpioHighZ(Port1, 12);
    R_PORT_SetGpioHighZ(Port1, 13);
    R_PORT_SetGpioHighZ(Port2, 6);
}
/*****************************************************************************
** Function:    RSCAN0_C4_Init
** Description: Configures the CAN1 macro for channel 4
** Parameter:   NULL
** Return:      None
******************************************************************************/
void RSCAN0_C4_Init(void)
{
    /* Set clk_xincan Rate */
    /* When fCAN is 16MHz, */
    /* Bitrate = fCAN/(BRP+1)/(1+TSEG1+TSEG2) = 16/1/16 = 1 Mbps */
    RSCAN0C4CFG = 0x023a0000u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=0 */

    /* Bitrate = fCAN/(BRP+1)/(1+TSEG1+TSEG2) = 16/8/16 = 0.125 Mbps */
    //RSCAN0C4CFG = 0x023a0007u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=7 */

    /* ==== Buffer setting ==== */
    // TODO: probably not needed, test this
    RSCAN0THLCC4 = 0x00000500u;

    RSCAN0TMIEC2 = 0x00000001; /* Enable Channel 4 Buffer 0 Transmit Interrupt */
    /* enable Arbitration lost, Bus lock, Overload, error interrupts */
    /* enable Bus-off, error passive, error warning and bus error interrupts */
    /* entry to channel halt mode automatically at bus off entry (BOM[1:0] = b01)*/
    RSCAN0C4CTR = 0x00006f01;
}
/*****************************************************************************
** Function:    RSCAN0_C4_Stop_to_Reset
** Description: Configures the CAN0 macro for channel 4
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_Stop_to_Reset(void)
{
    /* Channel stop mode -> Channel reset mode (Channel 4) */
    // CSLPR bit : 0 -> set as Other than channel stop mode
    // CHMDC[1:0] : 01 -> Channel reset mode
    if (RSCAN0C4STSLL & 0x04u)
    {
        RSCAN0C4CTRLL &= 0x01u;
        while ((RSCAN0C4STS & 0x01) == 0)
        {
        } // CRSTSTS : check status for channel reset mode/*
    }
}

/*****************************************************************************
** Function:    RSCAN0_C4_Reset_to_Com
** Description: Change channel 4 mode from reset mode to communication mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_Reset_to_Com(void)
{
    /* If Channel4 in reset mode */
    if (RSCAN0C4STS & 0x01u)
    {
        /* Set CHMDC[1:0] to b00, switch to communication mode */
        RSCAN0C4CTRLL = 8;
        while ((RSCAN0C4STS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
        while ((RSCAN0C4STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_C4_Com_to_Reset
** Description: Change channel 4 mode from communication mode to reset mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_Com_to_Reset(void)
{
    /* If Channel3 in communication mode */
    if (RSCAN0C4STS & 0x80u)
    {
        /* Set CHMDC[1:0] to b01, switch to reset mode */
        RSCAN0C4CTRLL = 5;
        while ((RSCAN0C4STS & 0x03u) == 0u)
        {
            /* While communication mode */
        }
        while ((RSCAN0C4STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_C4_Reset_to_Stop
** Description: Change channel 4 mode from reset mode to stop mode
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_C4_Reset_to_Stop(void)
{
    /* If Channel4 in reset mode */
    if (RSCAN0C4STS & 0x01u)
    {
        /* Set CHMDC[1:0] to b10, switch to halt mode */
        RSCAN0C4CTRLL = 0x04;
        while ((RSCAN0C4STS & 0x05) == 0)
        {
            /* While not in stop mode */
        } // GSLPSTS : check status for global stop mode/*
    }
}
/*****************************************************************************
** Function:    RSCAN0_C4_GetRxInfo
** Description: Return RSCAN0 channel 4 rx Information.
** Parameter:   rxInfo
** Return:      None
******************************************************************************/
void RSCAN0_C4_GetRxInfo(RSCAN0RxInfoRefType rxInfoRef)
{
    rxInfoRef->rxId = RSCAN0CFID12;
    rxInfoRef->rxDlc = RSCAN0CFPTR12HH;
    rxInfoRef->rxData = &RSCAN0CFDF012LL;
}

/*****************************************************************************
** Function:    RSCAN0_C4_GetIntStatus
** Description: Return RSCAN0 channel 4 interrupt status.
** Parameter:   intStatus
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C4_GetIntStatus(RSCAN0IntStatusRefType intStatusRef)
{
    intStatusRef->txIntStatus = RSCAN0TMSTS64 & (uint8_t)0x05u;
    intStatusRef->rxIntStatus = RSCAN0CFSTS12LL;
    return CAN_RTN_OK;
}
/*****************************************************************************
** Function:    RSCAN0_C4_GetErrorFlag
** Description: Return RSCAN0 channel 4 error flag.
** Parameter:   errorflag
** Return:      void
******************************************************************************/
void RSCAN0_C4_GetErrorFlag(uint16 *errorflagRef)
{
    *errorflagRef = RSCAN0C4ERFLL;
}
/*****************************************************************************
** Function:    RSCAN0_C4_TrmByTxBuf
** Description: Transmit pFrame through channel 4
** Parameter:   TxBufIdx, pFrame
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C4_TrmByTxBuf(uint8_t TxBufIdx, const r_CANFrame_t *pFrame)
{
    volatile uint8_t *pTBSR;
    r_CANFrame_t *pTxBuffer;
    volatile uint8_t *pTBCR;

    pTBSR = (volatile uint8_t *)&(RSCAN0TMSTS64);
    pTBCR = (volatile uint8_t *)&(RSCAN0TMC64);

    /* ---- Return if Tx Buffer is transmitting. ---- */
    if ((pTBSR[TxBufIdx] & (uint8_t)0x01u) == CAN_TBTST_TRANSMITTING)
    {
        return CAN_RTN_ERR;
    }

    /* Clear Tx buffer status */
    do
    {
        pTBSR[TxBufIdx] = CAN_CLR;
    }
    while (pTBSR[TxBufIdx] != CAN_CLR);

    /* Store message to Tx buffer */
    pTxBuffer = (r_CANFrame_t *)&(RSCAN0TMID64);
    pTxBuffer[TxBufIdx] = *pFrame;

    /* Set transmission request */
    pTBCR[TxBufIdx] = CAN_TBCR_TRM;

    return CAN_RTN_OK;
}
/*****************************************************************************
** Function:    RSCAN0_C4_CheckDllStatus
** Description: Check if channel 3 is ready to transmit
** Parameter:   void
** Return:      r_CANRtn_t
******************************************************************************/
r_CANRtn_t RSCAN0_C4_CheckDllStatus()
{
    volatile uint8_t *pTBSR;
    pTBSR = (volatile uint8_t *)&(RSCAN0TMSTS64);
    /* ---- Return if Tx Buffer is transmitting. ---- */
    if ((pTBSR[0] & (uint8_t)0x01u) == CAN_TBTST_TRANSMITTING) // default TxBufIdx = 0
    {
        return CAN_RTN_ERR;
    }
    else
    {
        return CAN_RTN_OK;
    }
}
/*****************************************************************************
** Interrupt:    INTRCAN4TRX
** Description:
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN4TRX(enable = manual)
void INTRCAN4TRX(void)
{
#ifdef RS_CAN0_C4
    Osek_Com_Nm_IntHandler(RSCAN0_TxSuccess, RS_CAN0_C4);
    RSCAN0TMSTS64 &= 0xf1; // set TMTRF[1:0] to 0
#endif
}
/*****************************************************************************
** Interrupt:    INTRCAN4REC
** Description:
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN4REC(enable = manual)
void INTRCAN4REC(void)
{
#ifdef RS_CAN0_C4
    Osek_Com_Nm_IntHandler(RSCAN0_RxSuccess, RS_CAN0_C4);
    RSCAN0CFSTS12 &= 0xFFFFFFF7U; // set CFRXIF to 0
    RSCAN0CFPCTR12 = 0xFFU;
#endif
}
/*****************************************************************************
** Interrupt:    INTRCAN4ERR
** Description:
** Parameter:   None
** Return:      None
******************************************************************************/
#pragma interrupt INTRCAN4ERR(enable = manual)
void INTRCAN4ERR(void)
{
#ifdef RS_CAN0_C4
    Osek_Com_Nm_IntHandler(RSCAN0_BusFailures, RS_CAN0_C4);
    RSCAN0C4ERFL = 0;
#endif
}

/*===========================================================================*/
/* RSCAN Functions do not use currently*/
/* TODO: Delete this section after testing */
/*===========================================================================*/

/*****************************************************************************
** Function:    RSCAN0_Init
** Description: Configures the CAN0 macro for channel 3 and channel 4 usage
** Parameter:   None
** Return:      None
******************************************************************************/
void RSCAN0_Init(void)
{
    protected_write(PROTCMD1, PROTS1, CKSC_ICANOSCD_CTL, 0x01u); // set clk_xincan (CKSCLK_ICANOSC) = MainOSC/1 (16MHz)
    while (0x01u != CKSC_ICANOSCD_ACT)
    {
    }
    /* Wait while CAN RAM initialization is ongoing */
    while ((RSCAN0GSTS & 0x00000008u))
    {
    }

    /* Switch to global/channel reset mode */
    /* Global stop mode -> Global reset mode */
    RSCAN0GCTR &= 0xfffffffbu; // GSLPR bit : 0 -> set as Other than global stop mode
    // GMDC[1:0] : 01 -> Global reset mode
    while ((RSCAN0GSTS & 0x04) != 0)
    {
    } // GSLPSTS : check status for global stop mode/*

    /* Channel stop mode -> Channel reset mode (Channel 3) */
    RSCAN0C3CTR = 0xfffffffbu; // CSLPR bit : 0 -> set as Other than channel stop mode
    // CHMDC[1:0] : 01 -> Channel reset mode
    while ((RSCAN0C3STS & 0x04) != 0)
    {
    } // GSLPSTS : check status for global stop mode/*

    RSCAN0C4CTR &= 0xfffffffbu; // CSLPR bit : 0 -> set as Other than channel stop mode
    // CHMDC[1:0] : 01 -> Channel reset mode
    while ((RSCAN0C4STS & 0x04) != 0)
    {
    } // GSLPSTS : check status for global stop mode/*

    /* Global configuration */
    RSCAN0GCFG = 0x00000011u; // DCS bit : CAN Clock Source = clk_xincan
    // MME bit : Mirror function is disabled
    // DRE bit : DLC replacement is disabled
    // DCE bit : DLC check is enabled
    // TPRI bit : Transmit Priority = ID priority

    /* TODO: Need to set clk_xincan Rate */
    /* When fCAN is 16MHz,
    Bitrate = fCAN/(BRP+1)/(1+TSEG1+TSEG2) = 16/2/16 = 0.5Mbps(500Kbps) */

    RSCAN0C3CFG = 0x023a0001u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=1 */

    RSCAN0C4CFG = 0x023a0001u; /* SJW =3TQ, TSEG1=10TQ, TSEG2=4TQ, BRP=1 */

    /* ==== Rx rule setting ==== */
    R_RSCAN0_SetRxRule();

    /* ==== Buffer setting ==== */
    RSCAN0RMNB = 0x00u;         /* Can_SetGlobalBuffer, set 16 receive buffers */
    RSCAN0RFCC0 = 0x00001101u;  // An interrupt occurs each time a message has been received, Depth: 4 messages, Timing Select: FIFO 1/8 full
    RSCAN0CFSTS9 = 0x00000000u; //initialization
    RSCAN0CFCC9 = 0x00001103u;  // Link to Buffer 0 CFTML[23:20], Select Receive mode CFM[17:16], Enable Transmit/Receive FIFO interrupt CFRXIE[1] Transmit/receive FIFO buffer are used
    /* Set THLEIE disabled, MEIE(FIFO Message Lost Interrupt disabled) */
    /* Set Transmit History Configuration, Entry from transmit buffer and enabled interrupt for Channel 3 and Channel 4 */
    RSCAN0THLCC3 = 0x00000500u;
    RSCAN0THLCC4 = 0x00000500u;

    RSCAN0TMIEC1 = 0x00010000; /* Enable Channel 3 Buffer 0 Transmit Interrupt */
    RSCAN0GCTR &= 0xfffff8ffu; /* TODO: for global interrupt */
    RSCAN0C3CTR = 0x0000ff0b;  /* enable all receive error interrupts */
    /* If GlobalChannel in halt or reset mode */
    if (RSCAN0GSTS & 0x03u)
    {
        RSCAN0GCTR &= 0xfffffffcu; /* Switch to communication mode */
        while ((RSCAN0GSTS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
        while ((RSCAN0GSTS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
    }
    RSCAN0CFCC9 |= 0x00000001u;
    /* If Channel3 in halt or reset mode */
    if (RSCAN0C3STS & 0x03u)
    {
        RSCAN0C3CTR = 0x00000000; //0xfffffffcu;    /* Switch to communication mode */
        while ((RSCAN0C3STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
        while ((RSCAN0C3STS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
    }

    /* If Channel4 in halt or reset mode */
    if (RSCAN0C4STS & 0x03u)
    {
        RSCAN0C4CTR &= 0xfffffffcu; /* Switch to communication mode */
        while ((RSCAN0C4STS & 0x02u) == 2u)
        {
            /* While halt mode */
        }
        while ((RSCAN0C4STS & 0x01u) == 1u)
        {
            /* While reset mode */
        }
    }
}

/*****************************************************************************
** Function:    Can_ReadRx_buffer
** Description: This code shows how to read message from Rx buffer
** Parameter:   pRxBufIdx - Pointer to Rx buffer that receives frame
** Return:      CAN_RTN_OK           - A frame is successfully read out
**              CAN_RTN_BUFFER_EMPTY - No frame is read out
******************************************************************************/
r_CANRtn_t RSCAN0_ReadRxBuffer(r_CANFrame_t *pFrame)
{
    uint8_t BufIdx;
    uint8_t CRBRCFiBufIdx;
    uint8_t OverwrittenFlag;
    uint32_t TempCRBRCF0;
    uint32_t TempCRBRCF1;
    uint32_t TempCRBRCF2;
    r_CANFrame_t *pRxBuffer;
    volatile uint32_t *pCRBRCF;
    r_CANRtn_t RtnValue;

    /* Judge if new messages are available */
    TempCRBRCF0 = RSCAN0RMND0;
    TempCRBRCF1 = RSCAN0RMND1;
    TempCRBRCF2 = RSCAN0RMND2;
    if ((TempCRBRCF0 == CAN_CLR) && (TempCRBRCF1 == CAN_CLR) && (TempCRBRCF2 == CAN_CLR))
    {
        RtnValue = CAN_RTN_BUFFER_EMPTY;
    }
    else
    {
        /* Get Rx buffer that has new message */
        if (TempCRBRCF0 != CAN_CLR)
        {
            pCRBRCF = (volatile uint32_t *)&(RSCAN0RMND0);
            for (BufIdx = 0U; BufIdx < CAN_CRBRCF0_RX_BUF_NUM; ++BufIdx)
            {
                if ((TempCRBRCF0 & CAN_1_BIT_MASK) == CAN_SET)
                {
                    break;
                }
                TempCRBRCF0 = TempCRBRCF0 >> CAN_B1_BIT_POS;
            }
        }
        else if (TempCRBRCF1 != CAN_CLR)
        {
            pCRBRCF = (volatile uint32_t *)&(RSCAN0RMND1);
            for (BufIdx = 0U; BufIdx < CAN_CRBRCF1_RX_BUF_NUM; ++BufIdx)
            {
                if ((TempCRBRCF1 & CAN_1_BIT_MASK) == CAN_SET)
                {
                    break;
                }
                TempCRBRCF1 = TempCRBRCF1 >> CAN_B1_BIT_POS;
            }
            BufIdx += CAN_CRBRCF0_RX_BUF_NUM;
        }
        else if (TempCRBRCF2 != CAN_CLR)
        {
            pCRBRCF = (volatile uint32_t *)&(RSCAN0RMND2);
            for (BufIdx = 0U; BufIdx < CAN_CRBRCF2_RX_BUF_NUM; ++BufIdx)
            {
                if ((TempCRBRCF2 & CAN_1_BIT_MASK) == CAN_SET)
                {
                    break;
                }
                TempCRBRCF2 = TempCRBRCF2 >> CAN_B1_BIT_POS;
            }
            BufIdx += (CAN_CRBRCF0_RX_BUF_NUM + CAN_CRBRCF1_RX_BUF_NUM);
        }
        /* Calculate index value in CRBRCFi */
        CRBRCFiBufIdx = BufIdx & CAN_5_BIT_MASK;

        do
        {
            /* Clear Rx complete flag of corresponding Rx buffer */
            do
            {
                CLR_BIT(*pCRBRCF, CRBRCFiBufIdx);
            }
            while (GET_BIT(*pCRBRCF, CRBRCFiBufIdx) == CAN_SET);

            /* Read out message from Rx buffer */
            pRxBuffer = (r_CANFrame_t *)&(RSCAN0RMID0);
            *pFrame = pRxBuffer[BufIdx];

            /* Judge if current message is overwritten */
            OverwrittenFlag = GET_BIT(*pCRBRCF, CRBRCFiBufIdx);
            /* Message is overwritten */
            if (OverwrittenFlag == CAN_SET)
            {
                /* User process for message overwritten */
            }
        }
        while (OverwrittenFlag == CAN_SET);

        RtnValue = CAN_RTN_OK;
    }
    return RtnValue;
}