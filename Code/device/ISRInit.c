#include "iodefine.h"
#include "ISRInit.h"

/******************************************************************************
** Function:    ISRInit
** Description: This function is used to initialize the interrupt flags
**              of the user defined interrupts.
**              User should write the initial code of his interrupt
**              in this function.
**              The step of configuring a interrupt is:
**                  1. Disables interrupt processing
**                  2. Clear interrupt request flag
**                  3. Table reference for interrupt vector
**                  4. Set interrupt priority(0 as the highest and 7 as the lowest)
**                     Category1 ISR should use priority 0 or 1
**                     Category2 ISR should use priority 2 ~ 7
**                  5. Enables interrupt processing
** Parameter:   None
** Return:      None
******************************************************************************/
void ISRInit(void)
{
    /* INTP4 */
    MKP4 = 1U;
    RFP4 = 0U;
    TBP4 = 1U;
    ICP4 &= 0xff;
    MKP4 = 0U;

    /* INTP5 */
    MKP5 = 1U;
    RFP5 = 0U;
    TBP5 = 1U;
    ICP5 &= 0xff;
    MKP5 = 0U;

    /* INTP10 */
    MKP10 = 1U;
    RFP10 = 0U;
    TBP10 = 1U;
    ICP10 &= 0xff;
    MKP10 = 0U;

    /* INTP11 */
    MKP11 = 1U;
    RFP11 = 0U;
    TBP11 = 1U;
    ICP11 &= 0xff;
    MKP11 = 0U;
}

/******************************************************************************
** Function:    InitSchedulerIRQ
** Description: This function is used to initialize the interrupt flags
**              of the scheduler interrupt.
**              Users should not modify this function unless you know what you
**              are doing.
** Parameter:   None
** Return:      None
******************************************************************************/
void InitSchedulerIRQ(void)
{
    ICP3 |= 0x80;    /* Disables interrupt processing */
    ICP3 &= ~0x1000; /* Clear interrupt request flag */
    ICP3 |= 0x40;    /* Table reference for interrupt vector */
    ICP3 &= 0xf9;    /* Set interrupt priority(1) */
    ICP3 &= ~0x80;   /* Enables interrupt processing */
}