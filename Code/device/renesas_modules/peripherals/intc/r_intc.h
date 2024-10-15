/*===========================================================================*/
/* Project:  F1x StarterKit V3 Sample Software                               */
/* Module :  r_intc.h                                                        */
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
#ifndef R_INTC_H
#define R_INTC_H

/*===========================================================================*/
/* Defines */
/*===========================================================================*/
#define R_ICADCA0I0     &ICADCA0I0
#define R_ICOSTM0       &ICOSTM0
#define R_ICTAUJ0I1     &ICTAUJ0I1
#define R_ICADCA0ERR    &ICADCA0ERR
#define R_ICRLIN30UR0   &ICRLIN30UR0
#define R_ICRLIN30UR1   &ICRLIN30UR1
#define R_ICRLIN30UR2   &ICRLIN30UR2
#define R_ICRLIN31UR0   &ICRLIN31UR0
#define R_ICRLIN31UR1   &ICRLIN31UR1
#define R_ICRLIN35UR0   &ICRLIN35UR0
#define R_ICRLIN35UR1   &ICRLIN35UR1
#define R_ICRCAN3ERR    &ICRCAN3ERR
#define R_ICRCAN3REC    &ICRCAN3REC
#define R_ICRCAN3TRX    &ICRCAN3TRX
#define R_ICRCAN4ERR    &ICRCAN4ERR
#define R_ICRCAN4REC    &ICRCAN4REC
#define R_ICRCAN4TRX    &ICRCAN4TRX

#define R_REQUEST_FLAG_NOT_SET  0
#define R_REQUEST_FLAG_SET      1

/*===========================================================================*/
/* Function defines */
/*===========================================================================*/
void R_INTC_MaskInterrupt(volatile uint16_t *IC_ptr);
void R_INTC_UnmaskInterrupt(volatile uint16_t *IC_ptr);
void R_INTC_SetPriority(volatile uint16_t *IC_ptr, uint8_t Priority);
void R_INTC_SetTableBit(volatile uint16_t *IC_ptr);
void R_INTC_ResetTableBit(volatile uint16_t *IC_ptr);
void R_INTC_ResetRequestFlag(volatile uint16_t *IC_ptr);
uint32_t R_INTC_GetRequestFlag(volatile uint16_t *IC_ptr);

#endif /* R_INTC_H */

