#include "iodefine.h"
#include "timer.h"
#include "alarm.h"
//#include "nm_alarm.h"
//#include "com_alarm.h"
//#include "nm_Cfg.h"
//#include "com_Cfg.h"

#pragma interrupt OSTM0_interrupt(enable = false, channel = 76, callt = false, fpu = false)

/******************************************************************************
** Function:    OSTM_init
** Description: This function set OSTM trigger interrupt per millisecond.
**              Use table reference method for interrupt vector.
** Parameter:   None
** Return:      None
******************************************************************************/

void OSTM0_init(void)
{
    // Set interrupt flags
    MKOSTM0 = 1U;    // Disables interrupt processing
    RFOSTM0 = 0U;    // Clear interrupt request flag
    TBOSTM0 = 1U;    // Table reference for interrupt vector
    ICOSTM0 &= 0xf8; // Set interrupt priority(the highest)
    MKOSTM0 = 0U;    // Enables interrupt processing

    OSTM0.CMP = 0x9c40; // 40000 / 40MHz = 1ms
    OSTM0.CTL = 0x01;   //interval timer mode, interrupt when counting start
    OSTM0.TS = 0x01;    //start
}

void OSTM0_interrupt(void)
{
    CounterTick();
#ifdef CONFIG_USE_COM_EXTERNAL
    CounterTick_COM();
#endif
#ifdef CONFIG_USE_OSEK_NM
    NMCounterTick();
#endif
}
