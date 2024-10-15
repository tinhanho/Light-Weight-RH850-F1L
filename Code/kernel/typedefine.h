#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H

#define NULL (void *)(0U)

/*********** used for configuration file *****************************/
#define ENABLE 0
#define DISABLE 1

/*********** basic type definition ***********************************/
typedef unsigned char uint8;
typedef unsigned long uint32;
typedef unsigned short int uint16;
typedef unsigned long long uint64;

/* This type is used to represent the return value of API services */
typedef unsigned char StatusType;

/*********** task configuration type definition **********************/
typedef struct
{
    unsigned int extended : 1;
    unsigned int preemptive : 1;
    unsigned int isrTask : 1;
} TaskFlagsType;

typedef struct
{
    uint32 sp;
    uint32 lp;
    uint32 eipc;
    uint32 eipsw;
    uint32 gpr[29];
} TaskContextType;

typedef unsigned char TaskPriorityType;

typedef unsigned char TaskStateType;

typedef TaskStateType *TaskStateRefType;

typedef uint8 TaskActivationsType;

typedef uint64 EventMaskType;

typedef uint32 ResourcesMaskType;

typedef uint8 *StackPtrType;

typedef uint16 StackSizeType;

typedef TaskContextType *TaskContextRefType;

typedef void (*EntryPointType)(void);

typedef void (*CallbackType)(void);

typedef uint8 TaskTotalType;

typedef uint8 TaskType;

typedef TaskType *TaskRefType;

/*
    Brief Task Constant type definition

    This structure defines all constants and constant pointers
    needed to manage a task.
*/
typedef struct
{
    EntryPointType entry;              /* point to the entry of this task */
    TaskContextRefType Context;        /* point to the space used to save context */
    StackPtrType stackZone;            /* point to the stack space */
    StackSizeType stackSize;           /* the size of stack */
    TaskType id;                       /* task id */
    TaskFlagsType Flags;               /* the flags to indicate the status of this task */
    TaskPriorityType staticPriority;   /* the static priority of this task */
    TaskActivationsType maxActivation; /* the maximal number of activations for this task */
    EventMaskType eventsMask;          /* the events mask of this task */
    ResourcesMaskType resourcesMask;   /* the resources mask of this task */
} TaskConstType;

/*
    Brief Task Variable type definition

    This structure defines all variables needed to manage a task
*/
typedef struct TASKVARTYPE
{
    const TaskConstType *StaticPtr;                   /* point to the static task structure */
    struct RESOURCEVARTYPE *Resources;                /* point to the list of resources */
    struct TASKVARTYPE *NextTask;                     /* point to the next variable task structure in ready list */
    TaskActivationsType activateCount;                /* the unmber of actual activations */
    TaskPriorityType priority;                        /* the actual priority of this task */
    TaskStateType state;                              /* the flags to indicate the status of this task */
    struct INTERNALRESOURCEVARTYPE *InternalResource; /* point to the internal resource */
} TaskVarType;

/* Brief Auto Start Structure Type */
typedef struct
{
    TaskTotalType TotalTasks; /* the unmber of total tasks on this app mode */
    TaskRefType TasksRef;     /* point to the array of autostart tasks on this app mode */
} AutoStartType;

/*********** resource configuration type definition **********************/
typedef uint8 ResourceType;

typedef struct RESOURCEVARTYPE
{
    struct RESOURCEVARTYPE *NextResource;
    const TaskPriorityType *ceilingPriority;
    TaskPriorityType previousPriority;
    TaskType owner;
    ResourceType id;
} ResourceVarType;

typedef struct INTERNALRESOURCEVARTYPE
{
    const TaskPriorityType *ceilingPriority;
    TaskPriorityType previousPriority;
    TaskType owner;
    ResourceType id;
} InternalResourceVarType;

/************ alarm configuration type definition ***********************/
typedef uint32 TickType;
typedef uint32 HardwareTickType;
typedef uint8 AlarmType;
typedef uint8 CounterType;
typedef uint32 AppModeType;

typedef struct ALARMBASETYPE
{
    TickType maxallowedvalue;
    HardwareTickType ticksperbase;
    TickType mincycle;
} AlarmBaseType;

typedef AlarmBaseType *AlarmBaseRefType;
typedef TickType *TickRefType;

/*
    It is used to indicate if the alarm is active
    This type defines the possibly states of one alarm which are:
    0 inactive
    1 active
 */
typedef uint8 AlarmActiveType;

typedef enum
{
    ALARMCALLBACK = 0,
    SETEVENT = 1,
    ACTIVATETASK = 2,
} AlarmActionType;

/*
    brief Alarm Action Info Type
    This type has extra information of the Alarm action
*/
typedef struct
{
    CallbackType callbackFunction;
    TaskType taskID;
    EventMaskType event;
} AlarmActionInfoType;

typedef struct COUNTERVARTYPE
{
    struct ALARMVARTYPE *FirstAlarm;
    struct ALARMVARTYPE *NextAlarm;
    HardwareTickType currentTicks;
    TickType currentCounts;
} CounterVarType;

typedef struct COUNTERCONSTTYPE
{
    TickType maxAllowedValue;
    TickType minCycle;
    HardwareTickType ticksPerBase;
} CounterConstType;

typedef struct ALARMVARTYPE
{
    struct ALARMVARTYPE *NextAlarm;
    struct ALARMVARTYPE *PrevAlarm;
    AlarmType ID;
    AlarmActiveType active;
    TickType expireCount;
    TickType cycle;
} AlarmVarType;

typedef struct ALARMCONSTTYPE
{
    CounterType counter;
    AlarmActionType alarmAction;
    AlarmActionInfoType AlarmActionInfo;
} AlarmConstType;

typedef struct
{
    AppModeType Mode;
    AlarmType Alarm;
    TickType AlarmTime;
    TickType AlarmCycleTime;
} AutoStartAlarmType;

/************ event configuration type definition ***********************/
typedef EventMaskType *EventMaskRefType;
typedef struct
{
    EventMaskType eventsSet;
    EventMaskType eventsWait;
} EventsVarType;

#endif /* #ifndef TYPEDEFINE_H */