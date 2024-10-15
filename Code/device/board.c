#include "board.h"
#include "dr7f701057.dvf.h"
#include "timer.h"
#include "GPIO.h"
#include "r_uart.h"
#include "r_clkc.h"
#include "write_protect.h"
//#include "com_Cfg.h"
//#include "nm_Cfg.h"
#include "r_csih.h"
//#include "nm_dll.h"

void WDTAInit(void)
{
    // Set interrupt flags
    MKWDTA0 = 1U;    // Disables interrupt processing
    RFWDTA0 = 0U;    // Clear interrupt request flag
    TBWDTA0 = 1U;    // Table reference for interrupt vector
    ICWDTA0 &= 0xf8; // Set interrupt priority(the highest)
    MKWDTA0 = 0U;    // Enables interrupt processing

    WDTA0MD = 0x0b; // NMI error mode, enable 75% EI. The overflow interval time is set by 6 to 4 bits, check hardware spec.

    // 512 / 240kHz * 128 = 0.2731 sec
    WDTA0WDTE = 0xAC;
}
void SoftwareResetWDTA0(void) {}

#pragma interrupt INTWDTA0(enable = false, channel = 32, callt = false, fpu = false)
void INTWDTA0(void)
{
    WDTA0WDTE = 0xAC; // Restart the WDTA counter
}

void BoardInit(void)
{
    WDTAInit();
    //OSTM0_init();
    GPIO_init();
    R_CLKC_PllInit(); /* CPUCLK = 120MHz, PPLLCLK = 80MHz */
    /*  Module Clock = PPLLCLK, Communication Clock = MainOSC */
    R_CLKC_SetRscanClockDomain(R_CLKC_RSCAN_PPLLCLK,
                               R_CLKC_RSCAN_DIV_MOSC1);
    //RSCAN0_Global_Launch();
    R_UART_Init();
    //RSCAN0_C3_Launch();
    //RSCAN0_C4_Launch();
#if defined(CONFIG_USE_COM_EXTERNAL)
    R_CSIH_Init();
#endif

#if defined(CONFIG_USE_COM_EXTERNAL) || defined(CONFIG_USE_OSEK_NM)
    InitIntHandlerFuncPtr();
#endif

#if defined(CONFIG_USE_COM_EXTERNAL) && defined(CONFIG_USE_OSEK_NM)
    Init_IndirectNM_Window();
#endif
}

/******************************************************************************
** Function:    SWReset
** Description: This function is used to do the software reset.
** Parameter:   None
** Return:      None
******************************************************************************/
void SWReset(void)
{
    protected_write(PROTCMD0, PROTS0, SWRESA, 0x0001u);
}