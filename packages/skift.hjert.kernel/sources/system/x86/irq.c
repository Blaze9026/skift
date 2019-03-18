/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <skift/atomic.h>
#include <skift/logger.h>

#include "kernel/cpu/irq.h"
#include "kernel/cpu/idt.h"

extern u32 irq_vector[];
extern bool console_bypass_lock;
irq_handler_t irq_handlers[16];

void irq_setup()
{
    for (u32 i = 0; i < 16; i++)
    {
        idt_entry(32 + i, irq_vector[i], 0x08, INTGATE);
    }
}

irq_handler_t irq_register(int index, irq_handler_t handler)
{
    if (index < 16)
    {
        irq_handlers[index] = handler;
        return handler;
    }

    return NULL;
}

reg32_t irq_handler(reg32_t esp, processor_context_t context)
{
    sk_atomic_disable();

    if (irq_handlers[context.int_no] != NULL)
    {
        esp = irq_handlers[context.int_no](esp, &context);
    }
    else
    {
        sk_log(LOG_WARNING,  "Unhandeled IRQ %d!", context.int_no);
    }

    if (context.int_no >= 8)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);

    sk_atomic_enable();

    return esp;
}