#ifndef INTERRUPT_H
#define INTERRUPT_H

extern void interrupt_wrapper_keyboard();
extern void interrupt_wrapper_PIT();
extern void interrupt_wrapper_spurious();

#endif /* INTERRUPT_H */