// for flexible mapping
#define IN_SQ (!(PINC & (1 << 3)))
#define IN_CI (!(PINC & (1 << 1)))
#define IN_X  (!(PINC & (1 << 2)))
#define IN_TR (!(PINC & (1 << 0)))

#define IN_ST (!(PINC & (1 << 4)))
#define IN_SE (!(PINC & (1 << 5)))
#define IN_PS (!(PIND & (1 << 2)))

#define IN_R1 (!(PINB & (1 << 1)))
#define IN_R2 (!(PINB & (1 << 2)))
#define IN_L1 (!(PIND & (1 << 0)))
#define IN_L2 (!(PIND & (1 << 1)))

#define IN_UP (!(PINB & (1 << 0)))
#define IN_DN (!(PIND & (1 << 7)))
#define IN_RT (!(PIND & (1 << 6)))
#define IN_LT (!(PIND & (1 << 5)))


//these actually do get optimized to sbi and cbi
//actually not just sbi, but sbr too
#define sbi(port, bit) (port) |= (1 << bit)
#define cbi(port, bit) (port) &= ~(1 << bit)

