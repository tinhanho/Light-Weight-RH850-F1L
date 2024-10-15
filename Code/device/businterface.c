#include "businterface.h"
#include "state.h"
#include "nm_dll.h"
#include "nmdll_Cfg.h"
#include "ipdu.h"
#include "nm_alarm_Cfg.h"
#include "coding.h"
#include "os.h"
#include "nm.h"
#include "status.h"
#include "dr7f701057.dvf.h"

IntHandlerFuncPtr IntHandlerFuncPtrArray[8];
uint32 count = 0;
#if defined(CONFIG_USE_COM_EXTERNAL) || defined(CONFIG_USE_OSEK_NM)
static inline NetIdType DDL_SW_Acceptance(uint32 CANID)
{
#ifdef RS_CAN0
    int i = 0;

    for (i = 0; i < WINDOW_NUM; i++)
    {
        if ((CANID & (uint32)(windowTable[i]->windowMask)) == (uint32)(windowTable[i]->Idbase))
        {
            return busTable[windowTable[i]->busTableIndex]->netLink->netId;
        }
    }
#endif
    /* No matched ID, this should not happen if the network is well configured */
    return SW_REJECTED;
}

void InitIntHandlerFuncPtr()
{
    IntHandlerFuncPtrArray[0] = RSCAN0_TransmittedMessage;
    IntHandlerFuncPtrArray[1] = RSCAN0_ReceiveMessage;
    IntHandlerFuncPtrArray[2] = RSCAN0_BusOccurFailures;
    IntHandlerFuncPtrArray[3] = RLIN3_TransmittedMessage;
    IntHandlerFuncPtrArray[4] = RLIN3_ReceiveIpduByteFromUL;
    IntHandlerFuncPtrArray[5] = RLIN3_ErrorHandler;
    IntHandlerFuncPtrArray[6] = RCSIH_TransmittedMessage;
    IntHandlerFuncPtrArray[7] = RCSIH_ReceiveMessage;
}

/* functions for global usage */
void TriggerTransmit(PDUType targetPduType, uint8 protocolUsed, uint8 busUsed,
                     SendIpduRefType targetIPDURef, NMPDURefType targetNMPDURef, uint8 nmpduDataLengthTx)
{
    uint32 size = (targetIPDURef->sizeInBits >> 3) + ((targetIPDURef->sizeInBits & 7) != 0);
    int i;
    switch (targetPduType)
    {
#if defined(CONFIG_USE_COM_EXTERNAL) || defined(CONFIG_USE_OSEK_NM)
    case IPDU:
        /* Store IPDU header */
        memset(THIS_IPDU->transBuf, 0, DOUBLE_BUF_SIZE_TX);
        memmove(THIS_IPDU->transBuf, targetIPDURef->IpduId, 12);
        EncodeFixed32(&(THIS_IPDU->transBuf[12]), size);
        EncodeFixed32(&(THIS_IPDU->transBuf[16]), targetIPDURef->dynamicLength);
        THIS_IPDU->transBuf[20] = targetIPDURef->zeroMessagCount;

        /* Clear the zero message count and the dynamic length */
        targetIPDURef->zeroMessagCount = 0;

        /* Store IPDU data */
        memmove(THIS_IPDU->transBuf + 21,
                targetIPDURef->IPDUSendContent,
                size);

        THIS_IPDU->status.bits.TxError = 0;
        THIS_IPDU->status.bits.TxTimeout = 0;
        THIS_IPDU->transCnt = 0;
        THIS_IPDU->transDataPtr = THIS_IPDU->transBuf;
        THIS_IPDU->status.bits.startNew = 1;

        switch (protocolUsed)
        {
#ifdef R_RLIN3
        case R_RLIN3:
            /* trigger RLIN3 bus transmission for IPDU */
            THIS_IPDU->targetCnt = size + 21;
            RLIN3_TriggerTransmit(busUsed, 0);
            break;
#endif
#ifdef RS_CAN0
        case RS_CAN0:
            /* trigger CAN bus transmission for IPDU */
            THIS_IPDU->targetCnt = ((size + 22) >> 3) + (((size + 22) & 7) != 0);
            RSCAN0_GenCOM_CANFrame(THIS_IPDU->transBuf, busUsed, (size + 22));
            RSCAN0_TriggerTransmit(busUsed, 0);
            break;
#endif
#ifdef R_CSIH
        case R_CSIH:
            /* trigger CSIH bus transmission for IPDU */
            THIS_IPDU->targetCnt = size + 21;
            RCSIH_TriggerTransmit(busUsed, 0);
            break;
#endif
        default:
            /* We implement R_LIN3, RSCAN0 and R_CSIH protocol for IPDU transmission */
            break;
        }
        targetIPDURef->zeroMessagCount = 0;
        break;
#endif
#ifdef CONFIG_USE_OSEK_NM
    case NMPDU:
        THIS_WINDOW->canFrame.ID = (uint32)(THIS_WINDOW->sourceId) + THIS_WINDOW->Idbase;
        THIS_WINDOW->canFrame.DLC = nmpduDataLengthTx;
        /* Copy des ID, opcode, and the ring data into CAN data field */
        THIS_WINDOW->canFrame.DB[0] = targetNMPDURef->desId;
        THIS_WINDOW->canFrame.DB[1] = targetNMPDURef->opcode.byte;

        if (THIS_NMPDU->networkStatusInfo.status.bits.NMRingDatAllowed)
        {
            for (i = 0; i < THIS_WINDOW->dataLengthTx - 2; i++)
            {
                THIS_WINDOW->canFrame.DB[2 + i] = targetNMPDURef->ringData.dat[i];
            }
        }
        switch (busUsed)
        {
#ifdef RS_CAN0_C3
        case RS_CAN0_C3:
            SendFrame[0][0] = THIS_WINDOW->canFrame;
            /* trigger CAN bus transmission for NMPDU */
            RSCAN0_TriggerTransmit(busUsed, 0);
            break;
#endif
#ifdef RS_CAN0_C4
        case RS_CAN0_C4:
            SendFrame[1][0] = THIS_WINDOW->canFrame;
            /* trigger CAN bus transmission for NMPDU */
            RSCAN0_TriggerTransmit(busUsed, 0);
            break;
#endif
        default:
            /* We only implement CAN bus for NMPDU transmission */
            break;
        }
        break;
#endif
    default:
        break;
    }
}
void Osek_Com_Nm_IntHandler(IntHandlerType intIndex, uint8 busUsed)
{
    IntHandlerFuncPtrArray[intIndex](busUsed);
}

#ifdef CONFIG_USE_OSEK_NM
void NMHWRoutine(NMHWRoutineType routineIndex, uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    NMHWRoutineFuncPtrArray[routineIndex](busUsed, busInfoOut);
}
#endif

BusRtn_t CheckDllStatus(uint8 protocolUsed, uint8 busUsed)
{
    BusRtn_t BusReturnStatus = BUS_RTN_OK;
    switch (protocolUsed)
    {
#ifdef RS_CAN0
    case RS_CAN0:
        if (RSCAN0_CheckDllStatus(busUsed) == CAN_RTN_ERR)
        {
            BusReturnStatus = BUS_RTN_BUSY;
        }
        break;
#endif
    default:
        break;
    }
    return BusReturnStatus;
}

#ifdef CONFIG_USE_OSEK_NM
BusRtn_t SWPollingTransmit(uint8 protocolUsed, uint8 busUsed, uint8 nmcounterIndex)
{
    HardwareTickType currentTicks = getNMCurrentCount(nmcounterIndex);
    HardwareTickType tolerenceTick = (currentTicks + SWPOLLINGTICKTIME) % (NMCounterList[nmcounterIndex].maxAllowedValue);

    for (; currentTicks != tolerenceTick; currentTicks = getNMCurrentCount(nmcounterIndex))
    {
        if (CheckDllStatus(protocolUsed, busUsed) == BUS_RTN_OK)
        {
            return BUS_RTN_OK;
        }
    }
    return BUS_RTN_BUSY;
}
#endif

/* The API can be used to transmit an I-PDU byte using RS UART */
void RLIN3_TriggerTransmit(uint8 busUsed, uint8 nextByte)
{
    RLIN3InfoType info;
    RLIN3_GetDllInfo(busUsed, &info);
    switch (busUsed)
    {
#ifdef R_RLIN3_C0
    case R_RLIN3_C0:
        while (R_RLIN30_GetStatus() == RLIN_BUSY)
            ;

        if (R_RLIN30_GetStatus() == RLIN_READY)
        {
            /* trigger first byte */
            if (THIS_IPDU->status.bits.startNew)
            {
                THIS_IPDU->status.bits.startNew = 0;
                //*rs_uart_txdr = *(THIS_CHANNEL->transDataPtr);
                THIS_IPDU->transCnt = -1;
                *(info.rs_uart_txdr) = '$';
            }
            else
            {
                *(info.rs_uart_txdr) = nextByte;
            }
        }
        break;
#endif
    default:
        /* we only implement RLIN30 for RLIN3 */
        /* this section should never be reached */
        break;
    }
}

/* The API can be used to transmit an I-PDU byte using RS UART */
void RLIN3_TransmittedMessage(uint8 busUsed)
{
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    uint8 nextByte = 0;
    RLIN3InfoType info;
    RLIN3_GetDllInfo(busUsed, &info);

    if (THIS_IPDU->status.bits.TxTimeout)
    {
        return;
    }

    if (THIS_IPDU->transCnt == THIS_IPDU->targetCnt) /* Keep on next byte transmission or complete the string transmission */
    {
        ConfirmSuccess(THIS_IPDU->currentIPDU, THIS_IPDU->transBuf[20]);
        return;
    }

    /* If next data byte is the flag byte, we add an escape char */
    if ((*(THIS_IPDU->transDataPtr) == '$' || *(THIS_IPDU->transDataPtr) == '^') &&
            THIS_IPDU->status.bits.TxLastEsc != 1)
    {
        /* Transmit an escape char */
        THIS_IPDU->status.bits.TxLastEsc = 1;
        nextByte = '^';
    }
    else
    {
        THIS_IPDU->status.bits.TxLastEsc = 0;
        nextByte = *((THIS_IPDU->transDataPtr)++);
        THIS_IPDU->transCnt++;
    }
    RLIN3_TriggerTransmit(busUsed, nextByte);
#endif
}
/* The API can be used to receive a byte from the UART PDU to I-PDU */
void RLIN3_ReceiveIpduByteFromUL(uint8 busUsed)
{
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
#if RECEIVE_IPDU_COUNT > 0
    int i = 0;
#endif
    RLIN3InfoType info;
    RLIN3_GetDllInfo(busUsed, &info);
    /* Escape byte received */
    if (*info.rs_uart_rxdr == '^')
    {
        if (THIS_IPDU->status.bits.RxLastEsc == 1)
        {
            THIS_IPDU->status.bits.RxLastEsc = 0;
            goto store_data;
        }
        else
        {
            THIS_IPDU->status.bits.RxLastEsc = 1;
            return;
        }
    }

    /* Flag byte received */
    if (*info.rs_uart_rxdr == '$')
    {
        if (THIS_IPDU->status.bits.RxLastEsc == 1)
        {
            THIS_IPDU->status.bits.RxLastEsc = 0;
            goto store_data;
        }
        /* This flag byte is the 'real' flag (not in the data) */
        /* In this case, we initialize for a new reception     */
        THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
        THIS_IPDU->status.bits.RxStartNew = 1;
        THIS_IPDU->status.bits.RxError = 0;
        THIS_IPDU->status.bits.RxLastEsc = 0;
        memset(THIS_IPDU->recvBuf, 0, DOUBLE_BUF_SIZE_RX);
        THIS_IPDU->recvCnt = 0;
        return;
    }

store_data:
    /* 1. An error has occured from the underlying layer, abort incoming reception */
    /* 2. The channel is not currently receiving an IPDU */
    if (THIS_IPDU->status.bits.RxError || THIS_IPDU->status.bits.RxStartNew == 0)
    {
        return;
    }

    /* Store the data from rxdr to the data buffer */
    THIS_IPDU->recvCnt++;
    *(THIS_IPDU->recvDataPtr++) = *info.rs_uart_rxdr;
    /* IPDU ID received withour error, start the matching process */
    if (THIS_IPDU->recvCnt < 16)
    {
        return;
    }
    /* IPDU ID received, start the ID matching process */
    if (THIS_IPDU->recvCnt == 16)
    {
#if RECEIVE_IPDU_COUNT > 0
        for (i = 0; i < RECEIVE_IPDU_COUNT; i++)
        {
            if (!memcmp(THIS_IPDU->recvBuf, rIPDUTable[i]->IpduId, 12))
            {
                THIS_IPDU->currentRxIPDU = rIPDUTable[i];
                THIS_IPDU->targetCntRx = GetSize(THIS_IPDU->recvBuf);
                /* find matched IPDU ID, keep receive the IPDU data field */
                return;
            }
        }
        /* No matched IPDU ID, cease the current IPDU receiving process */
        THIS_IPDU->status.bits.RxStartNew = 0;
        return;
#endif
    }
    /* Receive Complete, start the do the indication */
    if (THIS_IPDU->recvCnt - 21 == THIS_IPDU->targetCntRx)
    {
        /* Initialization for next reception */
        THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
        THIS_IPDU->status.bits.RxError = 0;
        THIS_IPDU->status.bits.RxLastEsc = 0;
        THIS_IPDU->status.bits.RxStartNew = 0;
        THIS_IPDU->recvCnt = 0;
        IndicateSuccess(THIS_IPDU->currentRxIPDU);
    }
#endif
}

void RLIN3_GetDllInfo(uint8 busUsed, RLIN3InfoRefType infoRef)
{
    switch (busUsed)
    {
#ifdef R_RLIN3_C0
    case R_RLIN3_C0:
        R_RLIN30_GetDllInfo(infoRef);
        break;
#endif
    /* we only implement RLIN30 for RLIN3*/
    default:
        break;
    }
}

void RLIN3_ErrorHandler(uint8 busUsed)
{
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    RLIN3InfoType info;
    RLIN3_GetDllInfo(busUsed, &info);
    /* An error has occured */
    if (*(info.rs_uart_lst) & 8u)
    {
        /* Stop the receiving process */
        THIS_IPDU->status.bits.RxError = 1;
        THIS_IPDU->status.bits.RxStartNew = 0;
        if (THIS_IPDU->recvCnt < 12)
        {
            /* havent recieved the full IPDU ID */
        }
        else
        {
            /* Those masks are made basedon RLIN30LEST register, please checkout the spec */
            THIS_IPDU->uLRxErrorStatus = 0;
            THIS_IPDU->uLRxErrorStatus |= (((*(info.rs_uart_lest) & UART_BIT) != 0) << UL_R_RLIN_ERROR_BIT1) |
                                          (((*(info.rs_uart_lest) & UART_OVERRUN) != 0) << UL_R_RLIN_ERROR_BIT2) |
                                          (((*(info.rs_uart_lest) & UART_FRAME) != 0) << UL_R_RLIN_ERROR_BIT3) |
                                          (((*(info.rs_uart_lest) & UART_PARITY) != 0) << UL_R_RLIN_ERROR_BIT4);
            IndicateFailure(THIS_IPDU->currentRxIPDU);
        }
        /* Restart the corresponding channel */
        switch (busUsed)
        {
#ifdef R_RLIN3_C0
        case R_RLIN3_C0 :
            R_RLIN30_UartInit();
            break;
#endif
        default:
            break;
        }
    }
#endif
}

/* The API can be used to transmit transfers 2 I-PDU bytes at a time using RS CSIH(SPI) */
void RCSIH_TriggerTransmit(uint8 busUsed, uint8 nextByte)
{
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    RCSIHInfoType info;
    RCSIH_GetDllInfo(busUsed, &info);

    /* A new transmission request
     * In this case, we should transfer start flag($) first */
#ifdef R_CSIH
    if (THIS_IPDU->status.bits.startNew)
    {
        THIS_IPDU->status.bits.startNew = 0;
        nextByte = '$';
    }
#endif
    switch (busUsed)
    {
#ifdef R_CSIH
    case R_CSIH_C1:
        /* SPI transfers 2 bytes at a time */
        R_csih1_send_string(nextByte << 4);
        break;
#endif
    default:
        break;
    }
#endif
}

/* The API can be used to transmit transfers 2 I-PDU bytes at a time using RS CSIH(SPI) */
void RCSIH_TransmittedMessage(uint8 busUsed)
{
#ifdef R_CSIH
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    RCSIHInfoType info;
    RCSIH_GetDllInfo(busUsed, &info);

    if (THIS_IPDU->status.bits.TxTimeout)
    {
        return;
    }

    /* Transmission error occurrs */
    if (info.rs_csih_status & R_CSIH_DC_ERROR)
    {
        THIS_IPDU->status.bits.TxError = 1;
        THIS_IPDU->status.bits.TxTimeout = 0;
        THIS_IPDU->uLTxErrorStatus = 0;
        THIS_IPDU->uLTxErrorStatus |= ((info.rs_csih_status & R_CSIH_DC_ERROR) != 0) << UL_R_CSIH_ERROR_BIT;
        /* clear error bit in register */
        *info.rs_csih_status_clear |= R_CSIH_DC_ERROR;
        ConfirmFailure(THIS_IPDU->currentIPDU);
        return;
    }

    if (THIS_IPDU->transCnt == THIS_IPDU->targetCnt)
    {
        ConfirmSuccess(THIS_IPDU->currentIPDU, THIS_IPDU->transBuf[20]);
        return;
    }
    RCSIH_TriggerTransmit(R_CSIH_C1, GetNextByte(busUsed));
#endif
#endif
}

/* The API can be used to receive 2 I-PDU bytes from the SPI PDU to I-PDU */
void RCSIH_ReceiveMessage(uint8 busUsed)
{
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    uint8 rxByte = 0;
    RCSIHInfoType info;
#if RECEIVE_IPDU_COUNT > 0
    int i;
#endif
    RCSIH_GetDllInfo(busUsed, &info);

    rxByte = ((*info.rs_csih_rx0h) >> shiftBits);
    if (rxByte == '$')
    {
        if (THIS_IPDU->status.bits.RxLastEsc == 1)
        {
            /* if received escape byte before, this '$' represents data, not start flag */
            THIS_IPDU->status.bits.RxLastEsc = 0;
            goto store_data;
        }
        /* No escaped byte is received before, this '$' represents start flag */
        /* In this case, we should initialize for a new reception */
        THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
        THIS_IPDU->status.bits.RxError = 0;
        THIS_IPDU->status.bits.RxStartNew = 1;
        THIS_IPDU->status.bits.RxLastEsc = 0;
        memset(THIS_IPDU->recvBuf, 0, DOUBLE_BUF_SIZE_RX);
        THIS_IPDU->recvCnt = 0;
        return;
    }
    /* An error has occured, or the channel is not currently receiving an IPDU */
    if (THIS_IPDU->status.bits.RxError || !THIS_IPDU->status.bits.RxStartNew)
    {
        return;
    }

    if (rxByte == '^')
    {
        if (THIS_IPDU->status.bits.RxLastEsc == 1)
        {
            /* if received escape byte before, this '^' represents data */
            THIS_IPDU->status.bits.RxLastEsc = 0;
            goto store_data;
        }
        else
        {
            /* this '$' represents escape byte */
            THIS_IPDU->status.bits.RxLastEsc = 1;
            return;
        }
    }

store_data:
    THIS_IPDU->recvCnt++;

    /* Reception error occurrs */
    if (info.rs_csih_status & R_CSIH_OF_ERROR)
    {
        THIS_IPDU->status.bits.RxError = 1;
        THIS_IPDU->status.bits.RxStartNew = 0;
        if (THIS_IPDU->recvCnt > 12)
        {
            THIS_IPDU->uLRxErrorStatus = 0;
            THIS_IPDU->uLRxErrorStatus |= ((info.rs_csih_status & R_CSIH_OF_ERROR) != 0) << UL_R_CSIH_ERROR_BIT;
            /* clear error bit in register */
            *info.rs_csih_status_clear |= R_CSIH_OF_ERROR;
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
            IndicateFailure(THIS_IPDU->currentRxIPDU);
#endif
            return;
        }
        else
        {
            /* ID receive fail, waiting for timeout and we CAN'T do notfications */
        }

    }

    *(THIS_IPDU->recvDataPtr++) = rxByte;

    if (THIS_IPDU->recvCnt < 12)
    {
        /* size field not received yet */
        return;
    }
    else if (THIS_IPDU->recvCnt == 12)
    {
#if RECEIVE_IPDU_COUNT > 0
        for (i = 0; i < RECEIVE_IPDU_COUNT; i++)
        {
            if (!memcmp(THIS_IPDU->recvBuf, rIPDUTable[i]->IpduId, 12))
            {
                /* find matched IPDU ID */
                THIS_IPDU->currentRxIPDU = rIPDUTable[i];
                return;
            }
        }
        /* No matched IPDU ID, cease the current IPDU receiving process */
        THIS_IPDU->status.bits.RxStartNew = 0;
        return;
#endif
    }
    else if (THIS_IPDU->recvCnt == 16)
    {
        THIS_IPDU->targetCntRx = GetSize(THIS_IPDU->recvBuf);
    }
    else if (THIS_IPDU->recvCnt - 21 ==  THIS_IPDU->targetCntRx)
    {
        /* Reception finished, initialization for next reception */
        THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
        THIS_IPDU->status.bits.RxError = 0;
        THIS_IPDU->status.bits.RxLastEsc = 0;
        THIS_IPDU->status.bits.RxStartNew = 0;
        THIS_IPDU->recvCnt = 0;
        IndicateSuccess(THIS_IPDU->currentRxIPDU);
    }
#endif
}

void RCSIH_GetDllInfo(uint8 busUsed, RCSIHInfoRefType infoRef)
{
    switch (busUsed)
    {
#ifdef R_CSIH
    case R_CSIH_C1:
        R_csih1_GetDllInfo(infoRef);
        break;
#endif
    default:
        break;
    }
}

uint8 GetNextByte(uint8 busUsed)
{
    if ((*(THIS_IPDU->transDataPtr) == '$' || *(THIS_IPDU->transDataPtr) == '^') &&
            THIS_IPDU->status.bits.TxLastEsc != 1)
    {
        /* Transmit an escape char */
        THIS_IPDU->status.bits.TxLastEsc = 1;
        return '^';
    }
    else
    {
        THIS_IPDU->status.bits.TxLastEsc = 0;

        /* Transmit next byte */
        THIS_IPDU->transCnt++;
        return *((THIS_IPDU->transDataPtr)++);
    }
    return 0;
}

/*****************************************************************************
** Function:    RSCAN0_TriggerTransmit
** Description: Transmitted message to specified bus.
** Parameter:   busUsed, CANFrame
** Return:      r_CANRtn_t
******************************************************************************/
void RSCAN0_TriggerTransmit(uint8 busUsed, uint8 CANFrameIndex)
{
    if (THIS_WINDOW->disableTxbySW != 1)
    {
        switch (busUsed)
        {
#ifdef RS_CAN0_C3
        case RS_CAN0_C3:
            RSCAN0_C3_TrmByTxBuf(0, &SendFrame[0][CANFrameIndex]);
            break;
#endif
#ifdef RS_CAN0_C4
        case RS_CAN0_C4:
            RSCAN0_C4_TrmByTxBuf(0, &SendFrame[1][CANFrameIndex]);
            break;
#endif
        }
    }
    else
    {
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
        THIS_IPDU->status.bits.isLocked = 0;
#endif
    }
}
/*****************************************************************************
** Function:    R_RSCAN0_Gen_CANFrame
** Description: This function gen CAN frame for OSEK_COM Message.
** Parameter:   pointer to send_string, protocolUsed
** Return:      None
******************************************************************************/
void RSCAN0_GenCOM_CANFrame(uint8 *send_string, uint8 busUsed, uint8 targetTransCnt)
{
    int i, j;
    r_CANFrame_t *CANFrame;
    switch (busTable[busUsed]->busUsed)
    {
#if defined RS_CAN0_C3
    case RS_CAN0_C3:
        CANFrame = &SendFrame[0][0];
        break;
#endif
#if defined RS_CAN0_C4
    case RS_CAN0_C4:
        CANFrame = &SendFrame[1][0];
        break;
#endif
    default:
        break;
    }
    for (i = 0; i < THIS_IPDU->targetCnt; i++)
    {
        (CANFrame+i)->IDE = 1;
        (CANFrame+i)->ID = THIS_WINDOW->canFrame.ID = (0x800) | ((uint32)(THIS_WINDOW->sourceId) + (THIS_WINDOW->Idbase)); // assign CAN frame ID
        if (i == 0)
        {
            (CANFrame+i)->DLC = 0x08u; // Transmit 8 data bytes
            (CANFrame+i)->DB[0] = '$';
            for (j = 1; j < 8; j++)
                (CANFrame+i)->DB[j] = *send_string++;
        }
        else if (i < THIS_IPDU->targetCnt - 1)
        {
            (CANFrame+i)->DLC = 0x08u; // Transmit 8 data bytes
            for (j = 0; j < 8; j++)
                (CANFrame+i)->DB[j] = *send_string++;
        }
        else
        {
            (CANFrame+i)->DLC = targetTransCnt % 8 == 0 ? 8 : targetTransCnt % 8; // Last DLC might not be 8 bytes
            for (j = 0; j < (CANFrame+i)->DLC; j++)
                (CANFrame+i)->DB[j] = *send_string++;
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_TransmittedMessage
** Description: Transmitted message to specified bus successfully.
**              Should be call from specified bus Tx completion ISR.
**              Trigger RSCAN transmission if necessary.
** Parameter:   protocolUsed, errorFlag
** Return:      None
******************************************************************************/
void RSCAN0_TransmittedMessage(uint8 busUsed)
{
#ifdef CONFIG_USE_OSEK_NM
    OpcodeType Opcode;
#endif
    uint16 errorFlag = 0;
    /* Transmit NMPDU */
#ifdef CONFIG_USE_OSEK_NM
    if (!(THIS_WINDOW->canFrame.ID >> 11))
    {
        Opcode.byte = THIS_WINDOW->canFrame.DB[1];
        D_Window_Data_confirm(THIS_NMPDU->netId, Opcode);
        return;
    }
#endif
    /* Transmit IPDU */
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
    RSCAN0_GetTxInfo(busUsed, &errorFlag);

    if (THIS_IPDU->status.bits.TxTimeout)
    {
        return;
    }

    THIS_IPDU->transCnt++;
    /* Detect ACK Delimeter, CRC, ACK, Form Error */
    if (errorFlag & 0x7f6fU)
    {
        THIS_IPDU->status.bits.TxError = 1;
        THIS_IPDU->status.bits.TxTimeout = 0;
        THIS_IPDU->uLTxErrorStatus = 0;
        THIS_IPDU->uLTxErrorStatus |= (errorFlag & 0x7fffU) << UL_R_CAN_ERROR_START_BIT;
        ConfirmFailure(THIS_IPDU->currentIPDU);
        return;
    }

    if (THIS_IPDU->status.bits.TxTimeout)
    {
        return;
    }

    if (THIS_IPDU->transCnt == THIS_IPDU->targetCnt)
    {
        ConfirmSuccess(THIS_IPDU->currentIPDU, THIS_IPDU->transBuf[20]);
        return;
    }
    else
    {
        //THIS_WINDOW->canFrame = SendFrame[THIS_IPDU->transCnt];
        RSCAN0_TriggerTransmit(busUsed, THIS_IPDU->transCnt);
    }
#endif
}

/*****************************************************************************
** Function:    RSCAN0_ReceiveMessage
** Description: Receive message to RSCAN0 successfully.
**              Should be call from RSCAN0 Rx completion ISR.
**              Detect wake-up signal in this function.
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_ReceiveMessage(uint8 busUsed)
{
    RSCAN0RxInfoType rxInfo;
    uint16 errorFlag = 0;
    NetIdType NetId = 0;
    int i = 0;

    RSCAN0_GetRxInfo(busUsed, &rxInfo, &errorFlag);
    /* Check software acceptance for both COM/NM message */
    NetId = DDL_SW_Acceptance(rxInfo.rxId);
    if (NetId != SW_REJECTED)
    {
        /* receive complete with error flag */
        if (errorFlag & 0x7f6fU)
        {
            /* receive COM message */
            if (rxInfo.rxId >> 11)
            {
                THIS_IPDU->status.bits.RxError = 1;
                THIS_IPDU->uLRxErrorStatus = 0;
                THIS_IPDU->status.bits.RxStartNew = 0;
                THIS_IPDU->uLRxErrorStatus |= (errorFlag & 0x7fffU) << UL_R_CAN_ERROR_START_BIT;
                if (THIS_IPDU->recvCnt < 1)
                {
                    /* ID receive fail, waiting for timeout and we CAN'T do notfications */
                }
                else
                {
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
                    IndicateFailure(THIS_IPDU->currentRxIPDU);
#endif
                }
                return;
            }
            /* receive NM message */
#ifdef CONFIG_USE_OSEK_NM
            else
            {
                if (!GET_STATUS(NMOff))
                {
                    D_StatusInd(NetId, CAN_RTN_ERR);
                }
            }
#endif
        }
        /* receive complete without error flag */
        else
        {
            /* receive COM message */
            if (rxInfo.rxId >> 11)
            {
                if (rxInfo.rxData[0] == '$')
                {
                    THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
                    THIS_IPDU->status.bits.RxStartNew = 1;
                    THIS_IPDU->recvCnt = 0;
                    for (i = 1; i < 8; i++)
                        *(THIS_IPDU->recvDataPtr++) = rxInfo.rxData[i];
                    THIS_IPDU->recvCnt++;
                }
                else
                {
                    /* This CAN is not currently receiving an IPDU */
                    if (THIS_IPDU->status.bits.RxStartNew == 0)
                    {
                        return;
                    }
                    /* Copy the data */
                    THIS_IPDU->recvCnt++;
                    //DLC = *RSCAN0CFPTR9 >> 6;
                    for (i = 0; i < 8; i++)
                        *(THIS_IPDU->recvDataPtr++) = rxInfo.rxData[i];
                    /* IPDU ID not received yet */
                    if (THIS_IPDU->recvCnt < 3)
                    {
                        return;
                    }

                    if (THIS_IPDU->recvCnt == 3)
                    {
#if RECEIVE_IPDU_COUNT > 0
                        for (i = 0; i < RECEIVE_IPDU_COUNT; i++)
                        {
                            if (!memcmp(THIS_IPDU->recvBuf, rIPDUTable[i]->IpduId, 12))
                            {
                                THIS_IPDU->currentRxIPDU = rIPDUTable[i];
                                THIS_IPDU->targetCntRx = (((GetSize(THIS_IPDU->recvBuf) + 21 + 1) >> 3) + (((GetSize(THIS_IPDU->recvBuf) + 21 + 1) & 7) != 0));
                                break;
                            }
                        }
                        if (i == RECEIVE_IPDU_COUNT)
                        {
                            THIS_IPDU->status.bits.RxStartNew = 0;
                            return;
                        }
#endif
                    }
                    if (THIS_IPDU->recvCnt == THIS_IPDU->targetCntRx) // '21+1' for transmitting '$' for first 1 byte and IPDU_header
                    {
                        THIS_IPDU->recvDataPtr = THIS_IPDU->recvBuf;
                        THIS_IPDU->status.bits.RxStartNew = 0;
                        THIS_IPDU->recvCnt = 0;
#ifdef CONFIG_USE_OSEK_NM
                        if (((IndirectNMNodeRefType)networkTable[NetId])->state == Indirect_NMBusSleep)
                        {
                            D_StatusInd(NetId, CAN_WAKEUP_SIGNAL);
                        }
#endif
#if defined (CONFIG_USE_OSEK_COM) && defined (CONFIG_USE_COM_EXTERNAL)
                        IndicateSuccess(THIS_IPDU->currentRxIPDU);
#endif
                    }
                }
            }
            /* receive NM message */
            else
            {
                /* receive wake-up signal while in Direct_NMBusSleep state */
#ifdef CONFIG_USE_OSEK_NM
                if ( !GET_STATUS(NMOff) )
                {
                    if (((DirectNMNodeRefType)networkTable[NetId])->state == Direct_NMBusSleep)
                    {
                        D_StatusInd(NetId, CAN_WAKEUP_SIGNAL);
                    }
                    else
                    {
                        D_Window_Data_ind(NetId, rxInfo.rxId, rxInfo.rxData[0], rxInfo.rxData[1], &rxInfo.rxData[2], (rxInfo.rxDlc >> 4));
                    }
                }
#endif
            }
        }
    }
}

/*****************************************************************************
** Function:    RSCAN0_BusOccurFailures
** Description: Detect RSCAN0 bus hardware errors.
**              Should be call from RSCAN0 Error ISR.
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_BusOccurFailures(uint8 busUsed)
{
#ifdef CONFIG_USE_OSEK_NM
    D_StatusInd(THIS_NMPDU->netId, CAN_RTN_ERR);
#endif
}

/*****************************************************************************
** Function:    RSCAN0_DInit_BusInit
** Description: 1. Init GPIO Port
**              2. Channel stop mode -> Channel reset mode
**              3. Channel initialize
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_DInit_BusInit(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_PORT_Init();
        RSCAN0_C3_Stop_to_Reset();
        RSCAN0_C3_Init();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_PORT_Init();
        RSCAN0_C4_Stop_to_Reset();
        RSCAN0_C4_Init();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}

/*****************************************************************************
** Function:    RSCAN0_DInit_BusSleep
** Description: There is no power down mode in rh850f1l CAN bus while reception ability remains available.
**              Disable Transmission ability by software and stay in communication mode.
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_DInit_BusSleep(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    THIS_WINDOW->disableTxbySW = 1;
}

/*****************************************************************************
** Function:    RSCAN0_DInit_BusAwake
** Description: 1. Channel communication mode -> Channel reset mode -> Channel stop mode
**              2. Deinit GPIO Port -> Init GPIO Port
**              3. Channel stop mode -> Channel reset mode
**              4. Channel Initialization
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_DInit_BusAwake(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_Com_to_Reset();
        RSCAN0_C3_Reset_to_Stop();
        RSCAN0_C3_PORT_DeInit();
        RSCAN0_C3_PORT_Init();
        RSCAN0_C3_Stop_to_Reset();
        RSCAN0_C3_Init();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_Com_to_Reset();
        RSCAN0_C4_Reset_to_Stop();
        RSCAN0_C4_PORT_DeInit();
        RSCAN0_C4_PORT_Init();
        RSCAN0_C4_Stop_to_Reset();
        RSCAN0_C4_Init();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}

/*****************************************************************************
** Function:    RSCAN0_DInit_BusRestart
** Description: 1. Channel communication mode -> Channel reset mode -> Channel stop mode
**              2. Deinit GPIO Port -> Init GPIO Port
**              3. Channel stop mode -> Channel reset mode
**              4. Channel Initialization
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_DInit_BusRestart(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_Com_to_Reset();
        RSCAN0_C3_Reset_to_Stop();
        RSCAN0_C3_PORT_DeInit();
        RSCAN0_C3_PORT_Init();
        RSCAN0_C3_Stop_to_Reset();
        RSCAN0_C3_Init();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_Com_to_Reset();
        RSCAN0_C4_Reset_to_Stop();
        RSCAN0_C4_PORT_DeInit();
        RSCAN0_C4_PORT_Init();
        RSCAN0_C4_Stop_to_Reset();
        RSCAN0_C4_Init();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}

/*****************************************************************************
** Function:    RSCAN0_DInit_BusShutDown
** Description: 1. any mode -> Channel stop mode
**              2. Deinit GPIO Port
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_DInit_BusShutDown(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_Com_to_Reset();
        RSCAN0_C3_Reset_to_Stop();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_Com_to_Reset();
        RSCAN0_C4_Reset_to_Stop();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}

/*****************************************************************************
** Function:    RSCAN0_D_Online
** Description: Channel reset mode  -> Channel communication mode
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_D_Online(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_Reset_to_Com();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_Reset_to_Com();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
    THIS_WINDOW->disableTxbySW = 0;
}

/*****************************************************************************
** Function:    RSCAN0_D_Offline
** Description: Use software to disable transmission.
** Parameter:   busUsed
** Return:      None
******************************************************************************/
void RSCAN0_D_Offline(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    THIS_WINDOW->disableTxbySW = 1;
}
/*****************************************************************************
** Function:    RSCAN0_GetTxInfo
** Description: Reading the status information of the RSCAN0.
** Parameter:   protocolUsed
** Return:      CANBusInfo
******************************************************************************/
void RSCAN0_GetTxInfo(uint8 busUsed, uint16 *errorFlagRef)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_GetErrorFlag(errorFlagRef);
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_GetErrorFlag(errorFlagRef);
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}

/*****************************************************************************
** Function:    RSCAN0_GetRxInfo
** Description: Reading the status information of the RSCAN0.
** Parameter:   protocolUsed
** Return:      CANBusInfo
******************************************************************************/
void RSCAN0_GetRxInfo(uint8 busUsed, RSCAN0RxInfoRefType rxInfoRef, uint16 *errorFlagRef)
{
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        RSCAN0_C3_GetRxInfo(rxInfoRef);
        RSCAN0_C3_GetErrorFlag(errorFlagRef);
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        RSCAN0_C4_GetRxInfo(rxInfoRef);
        RSCAN0_C4_GetErrorFlag(errorFlagRef);
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
}
/*****************************************************************************
** Function:    RSCAN0_GetDLLStatus
** Description: Reading the status information of the RSCAN0.
** Parameter:   protocolUsed
** Return:      CANBusInfo
******************************************************************************/
void RSCAN0_GetDLLStatus(uint8 busUsed, CANBusInfoRefType busInfoOut)
{
    RSCAN0IntStatusType intStatus;
    uint16 errorFlag = 0;
    r_CANRtn_t ifGetCANStatus = CAN_RTN_ERR;
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        ifGetCANStatus = RSCAN0_C3_GetIntStatus(&intStatus);
        RSCAN0_C3_GetErrorFlag(&errorFlag);
        RSCAN0C3ERFLL = 0;
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        ifGetCANStatus = RSCAN0_C4_GetIntStatus(&intStatus);
        RSCAN0_C4_GetErrorFlag(&errorFlag);
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
    if (ifGetCANStatus == CAN_RTN_OK)
    {

        /* get transmit related DLL status */
        if (intStatus.txIntStatus & 0x05u)
        {
            if (intStatus.txIntStatus & 0x04u)
            {
                THIS_WINDOW->canInfo.busStatus.bits.IntTx = 1;
            }
            if (intStatus.txIntStatus & 0x01u)
            {
                THIS_WINDOW->canInfo.busStatus.bits.isTransmit = 1;
            }
        }
        else
        {
            THIS_WINDOW->canInfo.busStatus.bits.IntTx = 0;
            THIS_WINDOW->canInfo.busStatus.bits.isTransmit = 0;
        }
        /* get reception related DLL status */
        if (intStatus.rxIntStatus & 0x08u)
        {
            THIS_WINDOW->canInfo.busStatus.bits.IntRx = 1;
        }
        else
        {
            THIS_WINDOW->canInfo.busStatus.bits.IntRx = 0;
        }
        /* get bus error related DLL status */
        if (errorFlag & 0x7f6fU)
        {
            THIS_WINDOW->canInfo.busStatus.bits.IntError = 1;
        }
        else
        {
            THIS_WINDOW->canInfo.busStatus.bits.IntError = 0;
        }
        THIS_WINDOW->canInfo.busErrorFlag.word = (errorFlag & CAN_BUSERROR) |
                (errorFlag & CAN_ERRORWARNING) |
                (errorFlag & CAN_ERRORPASSIVE) |
                (errorFlag & CAN_BUSOFFENTRY) |
                (errorFlag & CAN_BUSOFFRECOVER) |
                (errorFlag & CAN_OVERLOAD) |
                (errorFlag & CAN_BUSLOCK) |
                (errorFlag & CAN_ARBITRATIONLOST) |
                (errorFlag & CAN_STUFFERROR) |
                (errorFlag & CAN_FORMERROR) |
                (errorFlag & CAN_ACKERROR) |
                (errorFlag & CAN_CRCERROR) |
                (errorFlag & CAN_RECESSIVEBITERROR) |
                (errorFlag & CAN_DOMINENTBITERROR) |
                (errorFlag & CAN_ACKDELIMITERERROR);
    }
    *busInfoOut = THIS_WINDOW->canInfo;
}

r_CANRtn_t RSCAN0_CheckDllStatus(uint8 busUsed)
{
    r_CANRtn_t CANReturnStatus = CAN_RTN_OK;
    switch (busUsed)
    {
#ifdef RS_CAN0_C3
    case RS_CAN0_C3:
        CANReturnStatus = RSCAN0_C3_CheckDllStatus();
        break;
#endif
#ifdef RS_CAN0_C4
    case RS_CAN0_C4:
        CANReturnStatus = RSCAN0_C4_CheckDllStatus();
        break;
#endif
    default:
        // only implement C3 and C4
        break;
    }
    return CANReturnStatus;
}
#endif

#ifdef R_CSIH
extern volatile RCSIHStatusType RCSIHStatus;
#endif

StatusType COM_CheckHWStatus(void)
{
    /* Bus HW that are currently supported */
#ifdef R_RLIN3_C0
    if (RLN30LMD != 0x01u && RLN30LMST != 0x01u)
    {
        return E_COM_SYS_HW;
    }
#endif
#ifdef R_CSIH_C1
    if(RCSIHStatus != R_CSIH_SYNC && RCSIHStatus != R_CSIH_READY)
    {
        return E_COM_SYS_HW;
    }
#endif
#ifdef RS_CAN0_C3
    if (RSCAN0C3STS & 0x07u != 0u)
    {
        return E_COM_SYS_HW;
    }
#endif
#ifdef RS_CAN0_C4
    if (RSCAN0C4STS & 0x07u != 0u )
    {
        return E_COM_SYS_HW;
    }
#endif
    return E_OK;
}