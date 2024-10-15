/*===========================================================================*/
/* Project:  F1x StarterKit V3 Sample Software                               */
/* Module :  r_rlin.h                                                        */
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
#ifndef R_RLIN_H
#define R_RLIN_H

#include "C:\Users\hotin\Desktop\MilkshopOSEK-main\MyProj\MilkshopOSEK\com\com_type.h"
#include "r_typedefs.h"

/*===========================================================================*/
/* Defines */
/*===========================================================================*/
/* LIN Return Type */
#define RLIN_OK      0
#define RLIN_ERROR   1
#define RLIN_NO_MSG  2
#define RLIN_READY   3
#define RLIN_BUSY    255U

/*===========================================================================*/
/* Error Mask Defines */
/*===========================================================================*/
#define UART_BIT     ((1U)<<0)
#define UART_OVERRUN ((1U)<<2)
#define UART_FRAME   ((1U)<<3)
#define UART_PARITY  ((1U)<<6)

/*===========================================================================*/
/* Variables */
/*===========================================================================*/
extern uint8 *g_pNextData31;
extern uint8 *g_pNextData35;

/*---- use this structure to get rx information --------------------*/
typedef struct
{
    volatile uint16 *rs_uart_txdr;
    const volatile uint16 *rs_uart_rxdr;
    volatile uint8 *rs_uart_lst;
    volatile uint8 *rs_uart_lest;
} RLIN3InfoType;
typedef RLIN3InfoType* RLIN3InfoRefType;

/*===========================================================================*/
/* Function defines */
/*===========================================================================*/
void R_RLIN24_BaudrateInit(void);
void R_RLIN24_Channel1Init(void);
uint8_t R_RLIN24_Channel1RxFrame(uint8_t RxDB[]);
uint8_t R_RLIN24_Channel1TxFrame(uint8_t ID, uint8_t TxDB[]);
uint8_t R_RLIN30_GetStatus(void);
void R_RLIN30_GetDllInfo(RLIN3InfoRefType infoRef);
uint8_t R_RLIN31_GetStatus(void);
uint8_t R_RLIN35_GetStatus(void);
void R_RLIN30_UartSendString(uint8* send_string);
void R_RLIN31_UartSendString(uint8* send_string);
void R_RLIN35_UartSendString(uint8* send_string);
void R_RLIN30_UartInit(void);
void R_RLIN31_UartInit(void);
void R_RLIN35_UartInit(void);
void R_RLIN30_TxInterrupt_Enable(void);
void R_RLIN30_TxInterruptDisable(void);

#endif /* R_RLIN_H */
