#ifndef COM_TYPE_H
#define COM_TYPE_H

#include "C:\Users\hotin\Desktop\MilkshopOSEK-main\MyProj\MilkshopOSEK\kernel\typedefine.h"

/* COM-303 specification data type definitions */
typedef uint8 MessageIdentifier;
typedef uint8 DataType;
typedef DataType *DataRefType;
typedef DataRefType ApplicationDataRef;
typedef uint32 COMLengthType;
typedef COMLengthType *LengthRefType;

/* Internal data type definitions */
typedef uint8 *string;
typedef uint16 IpduTableIndexType;
typedef uint64 InitialValueType;
typedef uint8 ByteOrderBufferType;
typedef void (*NMCallbackType)(uint8);

typedef enum
{
    COM_FALSE = 0,
    COM_TRUE = 1
} COMBoolType;
typedef COMBoolType FlagValue;
typedef uint32 COMApplicationModeType;
typedef uint32 COMShutdownModeType;
typedef COMBoolType CalloutReturnType;

typedef enum
{
    COMServiceId_None = 0,
    COMServiceId_StartCOM = 1,
    COMServiceId_StopCOM = 2,
    COMServiceId_InitMessage = 3,
    COMServiceId_StartPeriodic = 4,
    COMServiceId_StopPeriodic = 5,
    COMServiceId_SendMessage = 6,
    COMServiceId_ReceiveMessage = 7,
    COMServiceId_SendDynamicMessage = 8,
    COMServiceId_ReceiveDynamicMessage = 9,
    COMServiceId_SendZeroMessage = 10,
    COMServiceId_GetMessageStatus = 11
} COMServiceIdType;

typedef union
{
    COMApplicationModeType COMApplicationMode;
    COMShutdownModeType COMShutdownMode;
} COMModeType;

/* Store the COM paramters provided to the hook function */
typedef struct
{
    COMServiceIdType ServiceId;
    COMModeType COMMode;
    MessageIdentifier Message;
    ApplicationDataRef DataRef;
    LengthRefType LengthRef;
} COMParameterType;

/* notification mechanism related data structures */
typedef FlagValue (*ReadFlagType)(void);
/* @SFD101-Notification-012 */
/* @SFD101-Notification-013 */
typedef void (*SetFlagType)(void);
typedef void (*ResetFlagType)(void);

typedef struct
{
    FlagValue flag;
    ReadFlagType read;
    SetFlagType set;
    ResetFlagType reset;
} COMFlagType;
typedef COMFlagType *COMFlagRefType;
typedef struct
{
    uint16 monitoredIpdu;
    NMCallbackType nmcallbackFunction;
} NMCallbackInfoType;
typedef NMCallbackInfoType* NMCallbackInfoRefType;
typedef enum
{
    NOTIFICATION_NONE = 0,
    NOTIFICATION_ACTIVATETASK,
    NOTIFICATION_SETEVENT,
    NOTIFICATION_COMCALLBACK,
    NOTIFICATION_FLAG,
    NOTIFICATION_INMCALLBACK
} NotificationKindType;

typedef struct
{
    NMCallbackInfoRefType nmcallbackInfo;
    CallbackType callbackFunction;
    TaskType taskID;
    EventMaskType event;
    COMFlagRefType Flag;
} NotificationInfoType;

typedef struct
{
    NotificationKindType kind;
    NotificationInfoType Info;
} NotificationType;
typedef NotificationType *NotificationRefType;

typedef enum
{
    SEND_STATIC_INTERNAL = 0,
    SEND_ZERO_INTERNAL,
    RECEIVE_UNQUEUED_INTERNAL,
    RECEIVE_QUEUED_INTERNAL,
    RECEIVE_ZERO_INTERNAL,
    SEND_STATIC_EXTERNAL,
    SEND_DYNAMIC_EXTERNAL,
    SEND_ZERO_EXTERNAL,
    RECEIVE_ZERO_EXTERNAL,
    RECEIVE_UNQUEUED_EXTERNAL,
    RECEIVE_QUEUED_EXTERNAL,
    RECEIVE_DYNAMIC_EXTERNAL,
    RECEIVE_ZERO_SENDERS
} MessagePropertyType;

/* base receiving message object configuration type definition */
typedef struct BaseReceivingMoType *BaseReceivingMoRefType;
typedef uint32 SizeInBitsType;
typedef StatusType (*ReceiverType)(BaseReceivingMoRefType, DataRefType, SizeInBitsType);
typedef StatusType (*CopierType)(BaseReceivingMoRefType, DataRefType, LengthRefType);
typedef struct BaseReceivingMoType
{
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    /* The Receiver function is reponsible to extract
       message from an IPDU and copy it to the message
       object */
    ReceiverType Receiver;
    /* The Copier function is responsible for copying data from
       message object to the application data buffer */
    CopierType Copier;
    /* This is a pointer to the next message obejct receving the same
       message from a single Sender */
    BaseReceivingMoRefType NextReceivingMoRef;
    MessagePropertyType messageProperty;
} BaseReceivingMoType;

/* base sending message object configuration type definition */
typedef uint8 MessageSizeType;
typedef struct BaseSendingMoType *BaseSendingMoRefType;
typedef StatusType (*SenderType)(BaseSendingMoRefType, DataRefType, LengthRefType);
typedef struct BaseSendingMoType
{
    /* The Sender function is respondible for :
       1. for internal com, copying the data from the sending message obejct to the receiving message object
       2. for external com, copying the data from the sending message object to the IPDU, and it may trigger
       an I-PDU transmission */
    SenderType Sender;
    MessagePropertyType messageProperty;
} BaseSendingMoType;
/*************** basic type of message *****************************/
typedef enum
{
    TRIGGERED = 0,
    PENDING = 1,
    AUTO = 2,
} TransferPropertyType;


typedef StatusType (*NetworkOrderCalloutType)();
typedef StatusType (*CpuOrderCalloutType)();
typedef StatusType (*IpduCalloutType)();

typedef struct
{
    NetworkOrderCalloutType NetOrderCallout;
    CpuOrderCalloutType CpuOrderCallout;
} LinkInfoType;
typedef LinkInfoType *LinkInfoRefType;

/*************** basic type of network message *****************************/
typedef enum
{
    ZERO = 0,
    STATIC = 1,
    DYNAMIC = 2,
} NetworkMessagePropertyType;

typedef enum
{
    LITTLEENDIAN = 0,
    BIGENDIAN = 1,
} BitOrderingType;
typedef uint32 BitPositionType;
typedef enum
{
    UNSIGNEDINTEGER = 0,
    BYTEARRAY = 1,
} DataInterpretationType;

typedef uint32 MaximumSizeInBitsType;
typedef struct SendIpduType *SendIpduRefType;
typedef struct StaticNetworkMessageType *StaticNetworkMessageRefType;

/* The following defines the network messages types */
/* These types are used by both sending and receiving network messages */
typedef struct StaticNetworkMessageType
{
    SizeInBitsType sizeInBits;
    BitOrderingType bitOrdering;
    BitPositionType bitPosition;
    DataInterpretationType dataInterpretation;
    BaseReceivingMoRefType ReceiveMoListRef;
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    StaticNetworkMessageRefType NextSnm;
    StaticNetworkMessageRefType NextRnm;
    SendIpduRefType SendIpduRef;
    DataRefType NetworkMessageDataRef;
} StaticNetworkMessageType;

typedef struct DynamicNetworkMessageType *DynamicNetworkMessageRefType;
typedef struct DynamicNetworkMessageType
{
    MaximumSizeInBitsType maximumSizeInBits;
    BitPositionType bitPosition;
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    BaseReceivingMoRefType ReceiveMoListRef;
    SendIpduRefType SendIpduRef;
    DataRefType NetworkMessageDataRef;
} DynamicNetworkMessageType;

typedef struct ZeroNetworkMessageType *ZeroNetworkMessageRefType;
typedef struct ZeroNetworkMessageType
{
    BaseReceivingMoRefType ReceiveMoListRef;
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    ZeroNetworkMessageRefType NextSnm;
    ZeroNetworkMessageRefType NextRnm;
    SendIpduRefType SendIpduRef;
} ZeroNetworkMessageType;

/*************** basic type of IPDU *****************/
typedef enum IPDUTransmissionModeType
{
    PERIODIC = 0,
    DIRECT = 1,
    MIXED = 2,
} IPDUTransmissionModeType;

typedef string IPDULayerUsedType;
typedef uint8 IPDUProtocolType;

/* COM alarm module */
typedef void (*COMAlarmCallBack)(SendIpduRefType);

typedef enum
{
    MDT = 0,
    TO = 1,
    TPD = 2,
    RECVTO = 3,
} COMAlarmUsageType;

typedef struct COMALARM
{
    /* variables */
    struct COMALARM *NextAlarm;
    struct COMALARM *PrevAlarm;
    TickType expireCount;
    AlarmActiveType active;
    /* constants */
    COMAlarmUsageType alarmUsage;
    IpduTableIndexType tableIndex;
} COMAlarmType;
typedef COMAlarmType* COMAlarmRefType;

typedef struct COMCOUNTER
{
    /* variables */
    COMAlarmType *FirstAlarm;
    COMAlarmType *NextAlarm;
    TickType currentCounts;
    HardwareTickType currentTicks;
    /* constant */
    HardwareTickType ticksPerBase; /* Ticks for a COM TimeBase */
    TickType maxAllowedValue; /* This is configured based on maximim time set in OIL */
} COMCounterType;

/* Sending I-PDU related data structures */
typedef struct
{
    uint64 timePeriod;
    uint64 timeOffset;
    uint64 minDelayTime;
    uint64 timeOut;
} COMTxIPDUTimingPar;

typedef struct
{
    uint64 timeOut;
    uint64 firstTimeOut;
} COMRxIPDUTimingPar;

typedef struct SendIpduType
{
    uint8 IpduId[12];
    uint32 sizeInBits;
    COMLengthType dynamicLength;
    uint32 zeroMessagCount;
    IPDUTransmissionModeType transmissionMode;
    COMTxIPDUTimingPar timeParas;
    IpduCalloutType IpduCallout;
    IPDULayerUsedType layerUsed;
    uint8 IPDUPerBusIndex;
    /* This is the data reference to the content in an I-PDU,
       with 12 bytes ID, 4 bytes IPDU size, 4 bytes dynamic message length, 1 byte zero message count */
    DataRefType IPDUSendContent; // ID(12) + IPDUSize(4)(byte) + DynamicLength(4) + zeroMessageNum(1) + IPDUDatasize(sizeInBits/8)
    /* The three attributes are references to all the network messages using this I-PDU */
    StaticNetworkMessageRefType StaticNetworkMessageRef;
    DynamicNetworkMessageRefType DynamicNetworkMessageRef;
    ZeroNetworkMessageRefType ZeroNetworkMessageRef;
    /* The index in the I-PDU table of this I-PDU */
    IpduTableIndexType tableIndex;
    /* This flag indicate the status of this I-PDU (e.g., this I-PDU is currently
       delayed by a MDT and need to be resent */
    uint16 statusFlag;
    /* The two references is used in the I-PDU request list */
    SendIpduRefType PrevRequest;
    SendIpduRefType NextRequest;
    /* This the reference to the COM alarm group of this I-PDU */
    COMAlarmRefType AlarmGroup;
    uint32 NMsender;
} SendIpduType;

/* Receiving I-PDU related data structures */
typedef struct ReceiveIpduType
{
    uint8 IpduId[12];
    uint32 sizeInBits;
    COMRxIPDUTimingPar timeParas;
    IpduCalloutType IpduCallout;
    IPDULayerUsedType layerUsed;
    DataRefType IPDUDataRef;
    IpduTableIndexType tableIndex;
    StaticNetworkMessageRefType StaticNetworkMessageRef;
    DynamicNetworkMessageRefType DynamicNetworkMessageRef;
    ZeroNetworkMessageRefType ZeroNetworkMessageRef;
    uint16 statusFlag;
    uint8 IPDUPerBusIndex;
    COMAlarmRefType AlarmGroup;
    uint32 NMsender;
} ReceiveIpduType;
typedef ReceiveIpduType *ReceiveIpduRefType;


/* filter related data structures */
typedef struct
{
    uint64 oldValue;
    union
    {
        struct
        {
            uint64 occurrence;
            uint64 period;
            uint64 offset;
        } oneEveryN;
        struct
        {
            uint64 mask;
            uint64 x;
        } CmpOld;
        struct
        {
            uint64 min;
            uint64 max;
        } CheckRange;
    };
} FilterInfoType;
typedef uint8 (*FilterFuncPtrType)(FilterInfoType*, uint64);

typedef struct
{
    FilterFuncPtrType FilterFunc;
    FilterInfoType info;
} FilterType;
typedef FilterType *FilterRefType;

/* unqueued receiving message object related data structures */
typedef struct
{
    DataRefType UnqueuedDataRef;
    MessageSizeType messageSizeInBytes;
} BufferType;
typedef BufferType *BufferRefType;

/* queued receiving message object related data structures */
typedef struct
{
    uint32 maxSize;
    MessageSizeType messageSizeInBytes;
    DataRefType QueuedDataRef;
    uint8 readIndex;
    uint8 writeIndex;
    uint8 isOverflow;
} QueueType;
typedef QueueType *QueueRefType;

/* internal sending message object related data structures */
typedef struct
{
    BaseSendingMoType BaseSendingMo;
    uint8 filterOrNot;
    /* The configured Receiver of this message */
    BaseReceivingMoRefType TargetRef;
} InternalStaticSendingMoType;
typedef InternalStaticSendingMoType *InternalStaticSendingMoRefType;

typedef struct
{
    BaseSendingMoType BaseSendingMo;
    /* The configured Receiver of this message */
    BaseReceivingMoRefType TargetRef;
} InternalZeroSendingMoType;
typedef InternalZeroSendingMoType *InternalZeroSendingMoRefType;

/* static sending message object related data structures */
typedef struct
{
    BaseSendingMoType BaseSendingMo;
    uint8 filterOrNot;
    TransferPropertyType transferProperty;
    /* FilterInfo */
    FilterRefType FilterInfo;
    /* callout function pointers */
    NetworkOrderCalloutType NetOrderCallout;
    CpuOrderCalloutType CpuOrderCallout;
    /* reference to its network message */
    StaticNetworkMessageRefType StaticNetworkMessageRef;
    /* notifications */
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    /* initialvalue */
    InitialValueType initialValue;
} ExternalStaticSendingMoType;
typedef ExternalStaticSendingMoType *ExternalStaticSendingMoRefType;

/* dynamic sending message object related data structures */
typedef struct
{
    BaseSendingMoType BaseSendingMo;
    /* transfer property */
    TransferPropertyType transferProperty;
    /* callout function pointers */
    NetworkOrderCalloutType NetOrderCallout;
    CpuOrderCalloutType CpuOrderCallout;
    /* reference to its network message */
    DynamicNetworkMessageRefType DynamicNetworkMessageRef;
    /* notification */
    NotificationRefType Notification;
    NotificationRefType NotificationError;
    /* initialvalue */
    InitialValueType initialValue;
} ExternalDynamicSendingMoType;
typedef ExternalDynamicSendingMoType *ExternalDynamicSendingMoRefType;

/* zero sending message object related data structures */
typedef struct
{
    BaseSendingMoType BaseSendingMo;
    /* callout function pointers */
    NetworkOrderCalloutType NetOrderCallout;
    CpuOrderCalloutType CpuOrderCallout;
    /* reference to its network message */
    ZeroNetworkMessageRefType ZeroNetworkMessageRef;
    /* notification */
    NotificationRefType Notification;
    NotificationRefType NotificationError;
} ExternalZeroSendingMoType;
typedef ExternalZeroSendingMoType *ExternalZeroSendingMoRefType;

/* zero receiving message object related data structures */
typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    /* callout function pointers */
    NetworkOrderCalloutType NetOrderCallout;
    CpuOrderCalloutType CpuOrderCallout;
} ExternalZeroReceivingMoType;
typedef ExternalZeroReceivingMoType *ExternalZeroReceivingMoRefType;

/* unqueued receiving message object related data structures */
typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    uint8 filterOrNot;
    /* FilterInfo */
    FilterRefType FilterInfo;
    /* LINK */
    LinkInfoRefType LinkInfoRef;
    /* data buffer of this unqueued message */
    BufferRefType BufferRef;
    /* initialvalue */
    InitialValueType initialValue;
} ExternalUnqueuedReceivingMoType;
typedef ExternalUnqueuedReceivingMoType *ExternalUnqueuedReceivingMoRefType;

/* queued receiving message object related data structures */
typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    uint8 filterOrNot;
    /* reference to the queue of this queued message obejct */
    /* @SFD101-MessageProperty-003 */
    QueueRefType QueueRef;
    /* FilterInfo */
    FilterRefType FilterInfo;
    /* LINK */
    LinkInfoRefType LinkInfoRef;
    /* initialvalue */
    InitialValueType initialValue;
} ExternalQueuedReceivingMoType;
typedef ExternalQueuedReceivingMoType *ExternalQueuedReceivingMoRefType;

/* dynamic receiving message object related data structures */
typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    uint32 messageSizeInBits;
    /* LINK */
    LinkInfoRefType LinkInfoRef;
    /* data buffer of this unqueued message */
    BufferRefType BufferRef;
    /* initialvalue */
    InitialValueType initialValue;
    /* dynamic message max length */
    const uint32 messageMaxSizeInBytes;
} ExternalDynamicReceivingMOType;
typedef ExternalDynamicReceivingMOType *ExternalDynamicReceivingMORefType;

/* internal RMO configuration type definition */
typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    /* FilterInfo */
    FilterRefType FilterInfo;
    /* reference to the queue of this queued message obejct */
    QueueRefType QueueRef;
    /* initialvalue */
    InitialValueType initialValue;
} InternalQueuedReceivingMoType;
typedef InternalQueuedReceivingMoType *InternalQueuedReceivingMoRefType;

typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    /* FilterInfo */
    FilterRefType FilterInfo;
    /* data buffer of this unqueued message */
    BufferRefType BufferRef;
    /* initialvalue */
    InitialValueType initialValue;
} InternalUnqueuedReceivingMoType;
typedef InternalUnqueuedReceivingMoType *InternalUnqueuedReceivingMoRefType;

typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
} InternalZeroReceivingMoType;
typedef InternalZeroReceivingMoType *InternalZeroReceivingMoRefType;

typedef struct
{
    BaseReceivingMoType BaseReceivingMo;
    InitialValueType initialValue;
} ZeroSendersMoType;
typedef ZeroSendersMoType *ZeroSendersMoRefType;


#endif /* #ifndef COMTYPE_H */
