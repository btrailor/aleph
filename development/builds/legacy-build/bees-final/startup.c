#include <stdint.h>

extern uint32_t _stack_top;
extern int main(void);

void Reset_Handler(void) {
    main();
    while(1);
}

void Default_Handler(void) { while(1); }

__attribute__((section(".isr_vector")))
uint32_t vectors[] = {
    (uint32_t)&_stack_top,
    (uint32_t)Reset_Handler,
    (uint32_t)Default_Handler,
    (uint32_t)Default_Handler,
};
