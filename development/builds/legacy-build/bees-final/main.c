extern void bounds_checking_init(void);

int main(void) {
    bounds_checking_init();
    while(1) {
        __asm__("nop");
    }
    return 0;
}
