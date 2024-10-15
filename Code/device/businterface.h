#ifndef BUS_INTERFACE_H
#define BUS_INTERFACE_H

/* This provides the driver developer an interface to our OSEK COM/NM modules */


#include "com_type.h"
//#include "nm_Cfg.h"
#include "r_rscan.h"
#include "requestlist.h"
//#include "bus_Cfg.h"
#include "r_rlin.h"
#include "r_csih.h"
//#include "com_dll.h"

#define DOUBLE_BUF_SIZE_TX (21 + IPDU_MAXSIZE)
#define DOUBLE_BUF_SIZE_RX (21 + IPDU_MAXSIZE)
#define BUS_RTN_OK 0
#define BUS_RTN_BUSY 1

#define UL_R_RLIN_ERROR_BIT1 0
#define UL_R_RLIN_ERROR_BIT2 1
#define UL_R_RLIN_ERROR_BIT3 2
#define UL_R_RLIN_ERROR_BIT4 3
#define UL_R_CSIH_ERROR_BIT 4
#define UL_R_CAN_ERROR_START_BIT 5

#define SW_REJECTED 255

typedef uint8 BusRtn_t;
typedef enum
{
    IPDU = 0,
    NMPDU,
} PDUType;
typedef PDUType pduType;

typedef struct
{
    uint32 windowMask;
    uint32 Idbase;
    uint8 dataLengthTx;
    uint8 dataLengthRx;
    uint8 sourceId;
    NMNetNodeMappingInfoType netnodeMap;
    /* This variable is used when protocol mode does not fully correspond to OSEK_NM DLL requirement */
    uint8 disableTxbySW;
    /* This attribute includes CAN header and CAN data buffer */
    r_CANFrame_t canFrame;
    CANBusInfoType canInfo;
    uint8 busTableIndex;
} WindowObjectType;
typedef WindowObjectType *WindowObjectRefType;

typedef struct
{
    uint8 ProtocolUsed;
    uint8 busUsed;
    IPDUPerBusRefType IPDUPerBusLink;
    BaseNMNodeRefType netLink;
    WindowObjectRefType windowLink;
} BusType;
typedef BusType *BusRefType;

#define THIS_IPDU (busTable[busUsed]->IPDUPerBusLink)
#define THIS_SIPDU_WINDOW (busTable[IPDUPerBusTable[sIPDUTable[ipduIndex]->IPDUPerBusIndex]->busTableIndex]->windowLink)
#define THIS_RIPDU_WINDOW (busTable[IPDUPerBusTable[rIPDUTable[ipduIndex]->IPDUPerBusIndex]->busTableIndex]->windowLink)
#define THIS_NMBUS (busTable[windowTable[networkTable[NetId]->windowTableIndex]->busTableIndex])
#define THIS_NMPDU (busTable[busUsed]->netLink)
#define THIS_WINDOW (busTable[busUsed]->windowLink)

/* Assign IntHandler function pointers */
void InitIntHandlerFuncPtr(void);
/* functions for global usage */
void TriggerTransmit(pduType targetPdu, uint8 protocolUsed, uint8 busUsed,
                     SendIpduRefType targetIPDURef, NMPDURefType targetNMPDURef, uint8 dataLengthTx);
void Osek_Com_Nm_IntHandler(IntHandlerType intIndex, uint8 busUsed);
void NMHWRoutine(NMHWRoutineType routineType, uint8 busUsed, CANBusInfoRefType busInfoOut);
BusRtn_t CheckDllStatus(uint8 protocolUsed, uint8 busUsed);
BusRtn_t SWPollingTransmit(uint8 protocolUsed, uint8 busUsed, uint8 nmcounterIndex);
/* functions for specific protocol usage */
/* RSCAN0 related functions */
void RSCAN0_TriggerTransmit(uint8 busUsed, uint8 CANFrameIndex);
void RSCAN0_GenCOM_CANFrame(uint8* send_string, uint8 busUsed, uint8 targetTransCnt);
void RSCAN0_TransmittedMessage(uint8 busUsed);
void RSCAN0_ReceiveMessage(uint8 busUsed);
void RSCAN0_BusOccurFailures(uint8 busUsed);
void RSCAN0_GetTxInfo(uint8 busUsed, uint16 *errorFlagRef);
void RSCAN0_GetRxInfo(uint8 busUsed, RSCAN0RxInfoRefType rxInfo, uint16 *errorFlagRef);
r_CANRtn_t RSCAN0_CheckDllStatus(uint8 busUsed);
/* RSCAN0 NWHWRoutine function pointer */
void RSCAN0_DInit_BusInit(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_DInit_BusAwake(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_DInit_BusSleep(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_DInit_BusRestart(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_DInit_BusShutDown(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_D_Online(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_D_Offline(uint8 busUsed, CANBusInfoRefType busInfoOut);
void RSCAN0_GetDLLStatus(uint8 busUsed, CANBusInfoRefType busInfoOut);

/* RS_UART related functions */
void RLIN3_TriggerTransmit(uint8 busUsed, uint8 nextByte);
void RLIN3_TransmittedMessage(uint8 busUsed);
void RLIN3_ReceiveIpduByteFromUL(uint8 busUsed);
void RLIN3_GetDllInfo(uint8 busUsed, RLIN3InfoRefType infoRef);
void RLIN3_ErrorHandler(uint8 busUsed);

/* RS_RCSIH related functions */
void RCSIH_TriggerTransmit(uint8 busUsed, uint8 nextByte);
void RCSIH_TransmittedMessage(uint8 busUsed);
void RCSIH_ReceiveMessage(uint8 busUsed);
void RCSIH_GetDllInfo(uint8 busUsed, RCSIHInfoRefType infoRef);
uint8 GetNextByte(uint8 busUsed);

/* Bus checking for COM Startup */
StatusType COM_CheckHWStatus(void);

#endif