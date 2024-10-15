#ifndef __INTERRUPT_CTL_H__
#define __INTERRUPT_CTL_H__

#define PMRregID 11
#define PMRselID 2

#define arch_disable_all_interrupts()               MaskEIPriority(0)
#define arch_disable_low_priority_interrupts()      MaskEIPriority(2)
#define arch_enable_all_interrupts()                MaskEIPriority(8)
#define arch_irq_ack()								__EI()

/******************************************************************************
** Function:    MaskEIPriority
** Description: This function is used to mask the specified interrupt priorities.
**              These bits in the PMR register mask an interrupt request with a
**              priority level that corresponds to the relevant bit position.
**              Specify the masks by setting the bits to 1 in order from the
**              lowest-priority bit.
**              For example, F0H can be set, but 10H or 0FH cannot.
** Parameter:   priority: The same or lower priority interrupts are masked.
**                        Can use 8 to unmask all interrupts.
** Return:      None
******************************************************************************/
static inline void MaskEIPriority(unsigned char priority)
{
    unsigned long value = 0xff;

    value = value << priority;
    value &= 0xff;
    __ldsr_rh(PMRregID, PMRselID, value);
}

#endif //__INTERRUPT_CTL_H__