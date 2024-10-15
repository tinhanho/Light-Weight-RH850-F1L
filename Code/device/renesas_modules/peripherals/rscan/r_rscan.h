/*===========================================================================*/
/* Project:  F1x StarterKit V3 Sample Software                               */
/* Module :  r_rscan.h                                                       */
/* Version:  V1.00                                                           */
/*===========================================================================*/
/*                                  COPYRIGHT                                */
/*===========================================================================*/
/* Copyright (c) 2016 by Renesas Electronics Europe GmbH,                    */
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
#ifndef R_RSCAN_H
#define R_RSCAN_H


#include "com_type.h"
//#include "ipdu_Cfg.h"
/*===========================================================================*/
/* Define DLL Status indications */
/*===========================================================================*/
#define CAN_COM_MESSAGE        ((1U)<<11)
/*===========================================================================*/
/* Define DLL Status indications */
/*===========================================================================*/
/* wake-up signal indication */
#define CAN_WAKEUP_SIGNAL       ((1U)<<0)
/*===========================================================================*/
/* Define Error Mask */
/*===========================================================================*/
#define CAN_BUSERROR            ((1U)<<0)
#define CAN_ERRORWARNING        ((1U)<<1)
#define CAN_ERRORPASSIVE        ((1U)<<2)
#define CAN_BUSOFFENTRY         ((1U)<<3)
#define CAN_BUSOFFRECOVER       ((1U)<<4)
#define CAN_OVERLOAD            ((1U)<<5)
#define CAN_BUSLOCK             ((1U)<<6)
#define CAN_ARBITRATIONLOST     ((1U)<<7)
#define CAN_STUFFERROR          ((1U)<<8)
#define CAN_FORMERROR           ((1U)<<9)
#define CAN_ACKERROR            ((1U)<<10)
#define CAN_CRCERROR            ((1U)<<11)
#define CAN_RECESSIVEBITERROR   ((1U)<<12)
#define CAN_DOMINENTBITERROR    ((1U)<<13)
#define CAN_ACKDELIMITERERROR   ((1U)<<14)

/*
*******************************************************************************
**  Register bit define
*******************************************************************************
*/

/* ---- CAN frame ----- */
typedef struct
{
    uint32 ID :29;
    uint32 THLEN :1;
    uint32 RTR :1;
    uint32 IDE :1;

    uint32 TS :16;
    uint32 LBL :12;
    uint32 DLC :4;
    uint8 DB[8];
} r_CANFrame_t;
/* ---- structure for CAN register information ----- */
/*---- use this structure to get rx information --------------------*/
typedef struct
{
    uint32 rxId;
    uint8 rxDlc;
    volatile uint8* rxData;
} RSCAN0RxInfoType;
typedef RSCAN0RxInfoType* RSCAN0RxInfoRefType;
/*---- use this structure to get Dll Status --------------------*/
typedef struct
{
    uint8 txIntStatus;
    uint8 rxIntStatus;
} RSCAN0IntStatusType;
typedef RSCAN0IntStatusType* RSCAN0IntStatusRefType;
/*---- structure for Tx Frame information --------------------*/
/* ---- CAN function return ---- */
typedef uint8 r_CANRtn_t;

/* ---- function return value ---- */
#define CAN_RTN_OK                          0U
#define CAN_RTN_DISABLE_by_SW               1U
#define CAN_RTN_BUFFER_EMPTY                2U
#define CAN_RTN_ERR                         255U


/*
*******************************************************************************
**  Macro define
*******************************************************************************
*/
#ifndef CAN_ENABLE
#define CAN_ENABLE                          1U
#endif
#ifndef CAN_DISABLE
#define CAN_DISABLE                         0U
#endif

/* ---- bit value ---- */
#define CAN_SET                             1U
#define CAN_CLR                             0U

/* ---- bit mask ---- */
#define CAN_1_BIT_MASK                      1U
#define CAN_2_BIT_MASK                      3U
#define CAN_3_BIT_MASK                      7U
#define CAN_4_BIT_MASK                      0xfU
#define CAN_5_BIT_MASK                      0x1fU

/* ---- bit position ---- */
#define CAN_B0_BIT_POS                      0U
#define CAN_B1_BIT_POS                      1U
#define CAN_B2_BIT_POS                      2U
#define CAN_B3_BIT_POS                      3U
#define CAN_B4_BIT_POS                      4U
#define CAN_B5_BIT_POS                      5U
#define CAN_B6_BIT_POS                      6U
#define CAN_B7_BIT_POS                      7U
#define CAN_B8_BIT_POS                      8U
#define CAN_B9_BIT_POS                      9U
#define CAN_B10_BIT_POS                     10U
#define CAN_B11_BIT_POS                     11U
#define CAN_B12_BIT_POS                     12U
#define CAN_B13_BIT_POS                     13U
#define CAN_B14_BIT_POS                     14U
#define CAN_B15_BIT_POS                     15U

/* **** CAN channel usage **** */
#define RS_CAN_CH3                 CAN_ENABLE  //CAN_DISABLE

#define RS_CAN_CH4                 CAN_ENABLE

/* ---- CAN resource ---- */
/*#define CAN_CHNL_NUM                        3U
#define CAN_RFIFO_NUM                       8U
#define CAN_CHNL_TRFIFO_NUM                 3U
#define CAN_TRFIFO_NUM                      (CAN_CHNL_TRFIFO_NUM * CAN_CHNL_NUM)
#define CAN_FIFO_NUM                        (CAN_RFIFO_NUM + CAN_TRFIFO_NUM)
#define CAN_CHNL_TX_BUF_NUM                 16U
*/
#define CAN_CRBRCF0_RX_BUF_NUM              32U
#define CAN_CRBRCF1_RX_BUF_NUM              32U
#define CAN_CRBRCF2_RX_BUF_NUM              32U
#define CAN_PAGE_RX_RULE_IDX_MASK           0xfU
#define CAN_RX_RULE_PAGE_IDX_BIT_POS        4U
#define CAN_RAM_LW_SIZE                     (0x1C20U >> 2U)
#define CAN_RAM_PAGE_IDX_BIT_POS            6U
#define CAN_RAM_DATA_IDX_MASK               0x3fU

#define RX_RULE_NUM_MAX              (192U)       /* Max Rx Rule number */
#define RX_RULE_PAGE_NUM             (1U)         /* Rx Rule Table page number */


/* ==== Rx rule table (Refer to can_table.h) ==== */
#define CAN_C0RN                       RX_RULE_NUM_CH0
#define CAN_C1RN                       RX_RULE_NUM_CH1
#define CAN_C2RN                       RX_RULE_NUM_CH2
#define CAN_C3RN                       RX_RULE_NUM_CH3
#define CAN_C4RN                       RX_RULE_NUM_CH4
#define CAN_RX_RULE_NUM                (CAN_C0RN + CAN_C1RN + CAN_C2RN + CAN_C3RN + CAN_C4RN)
#define CAN_RX_RULE_TABLE              RX_RULE_TABLE_LIST
#define CAN_RX_RULE_TYPE               can_cre_type

/* ---- TX buffer, TX status flag ---- */
#define CAN_TBTST_NOT_TRANSMITTING          0U
#define CAN_TBTST_TRANSMITTING              1U

/* ---- bit operations ---- */
#define GET_BIT(reg, pos)              (((reg) >> (pos)) & 1U)
#define SET_BIT(reg, pos)              ((reg) |= 1U << (pos))
#define CLR_BIT(reg, pos)              ((reg) &= ~(1UL << (pos)))

/* ---- CiTBCRj ---- */
#define CAN_TBTR_BIT_POS                    0U
#define CAN_TBAR_BIT_POS                    1U
#define CAN_TBOE_BIT_POS                    2U

/* transmission command */
#define CAN_TBCR_TRM                        (CAN_ENABLE << CAN_TBTR_BIT_POS)

/*
*******************************************************************************
**  Function define
******************************************************************************/
/* CAN Global related functions */
void RSCAN0_Global_Launch(void);
void RSCAN0_Global_Port_Deinit(void);
/* CAN COM related functions */
void RSCAN0_C3_Launch(void);
void RSCAN0_C4_Launch(void);

/* CAN Channel 3 related functions */
void RSCAN0_C3_PORT_Init(void);
void RSCAN0_C3_PORT_DeInit(void);
void RSCAN0_C3_Init(void);
void RSCAN0_C3_Stop_to_Reset(void);
void RSCAN0_C3_Reset_to_Com(void);
void RSCAN0_C3_Com_to_Reset(void);
void RSCAN0_C3_Reset_to_Stop(void);
void RSCAN0_C3_GetRxInfo(RSCAN0RxInfoRefType rxInfoRef);
r_CANRtn_t RSCAN0_C3_GetIntStatus(RSCAN0IntStatusRefType intStatusRef);
void RSCAN0_C3_GetErrorFlag(uint16* errorFlagRef);
r_CANRtn_t RSCAN0_C3_TrmByTxBuf(uint8 TxBufIdx, const r_CANFrame_t* pFrame);
r_CANRtn_t RSCAN0_C3_CheckDllStatus(void);

/* CAN Channel 4 related functions */
void RSCAN0_C4_PORT_Init(void);
void RSCAN0_C4_PORT_DeInit(void);
void RSCAN0_C4_Init(void);
void RSCAN0_C4_Stop_to_Reset(void);
void RSCAN0_C4_Reset_to_Com(void);
void RSCAN0_C4_Com_to_Reset(void);
void RSCAN0_C4_Reset_to_Stop(void);
void RSCAN0_C4_GetRxInfo(RSCAN0RxInfoRefType rxInfoRef);
r_CANRtn_t RSCAN0_C4_GetIntStatus(RSCAN0IntStatusRefType intStatusRef);
void RSCAN0_C4_GetErrorFlag(uint16* errorFlagRef);
r_CANRtn_t RSCAN0_C4_TrmByTxBuf(uint8 TxBufIdx, const r_CANFrame_t* pFrame);
r_CANRtn_t RSCAN0_C4_CheckDllStatus(void);

/* Not used in OSEK_NM */
void RSCAN0_Init(void);
r_CANRtn_t RSCAN0_ReadRxBuffer(r_CANFrame_t* pFrame);

extern r_CANFrame_t SendFrame[2][((SEND_IPDU_MAXSIZE >> 3)+29)>>3];

#endif /* R_RSCAN_H */

