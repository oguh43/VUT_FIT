/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*                     Mikroprocesorové a vestavěné systémy                     *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*                  Měření vzdálenosti/rychlosti pomocí ultrazvuku              *
*                                                                              *
*******************************************************************************/

#include "MK60D10.h"

#define GPIO_SET(port, mask)        ((port)->PSOR = (mask))
#define GPIO_CLEAR(port, mask)      ((port)->PCOR = (mask))
#define GPIO_TOGGLE(port, mask)     ((port)->PTOR = (mask))
#define GPIO_READ(port, mask)       ((port)->PDIR & (mask))
#define GPIO_SET_OUTPUT(port, mask) ((port)->PDDR |= (mask))
#define GPIO_SET_INPUT(port, mask)  ((port)->PDDR &= ~(mask))

#define PORT_GPIO_CONFIG(port, pin) \
    ((port)->PCR[(pin)] = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK)

#define PORT_GPIO_CONFIG_SRE(port, pin) \
    ((port)->PCR[(pin)] = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK | PORT_PCR_SRE_MASK)

#define PORT_GPIO_INPUT_IRQ(port, pin, irqc) \
    ((port)->PCR[(pin)] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(irqc))

#define PORT_BUTTON_CONFIG(port, pin) \
    ((port)->PCR[(pin)] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0b1010) | \
                          PORT_PCR_PE_MASK | PORT_PCR_PS_MASK)

#define PORT_CLEAR_FLAG(port, mask) ((port)->ISFR = (mask))

#define PIT_ENABLE()                (PIT->MCR = 0)
#define PIT_LOAD(ch, val)           (PIT->CHANNEL[(ch)].LDVAL = (val))
#define PIT_CLEAR_FLAG(ch)          (PIT->CHANNEL[(ch)].TFLG = 1)
#define PIT_ENABLE_IRQ(ch)          (PIT->CHANNEL[(ch)].TCTRL = PIT_TCTRL_TIE_MASK)
#define PIT_START(ch)               (PIT->CHANNEL[(ch)].TCTRL |= PIT_TCTRL_TEN_MASK)
#define PIT_STOP(ch)                (PIT->CHANNEL[(ch)].TCTRL &= ~PIT_TCTRL_TEN_MASK)
#define PIT_IS_RUNNING(ch)          (PIT->CHANNEL[(ch)].TCTRL & PIT_TCTRL_TEN_MASK)
#define PIT_GET_CURRENT(ch)         (PIT->CHANNEL[(ch)].CVAL)
#define PIT_GET_ELAPSED(ch)         (PIT->CHANNEL[(ch)].LDVAL - PIT->CHANNEL[(ch)].CVAL)

#define PIT_SETUP_TIMER(ch, val, start) do { \
    PIT_LOAD(ch, val); \
    PIT_CLEAR_FLAG(ch); \
    PIT_ENABLE_IRQ(ch); \
    if (start) PIT_START(ch); \
} while(0)

#define NVIC_SETUP_IRQ(irq) do { \
    NVIC_ClearPendingIRQ(irq); \
    NVIC_EnableIRQ(irq); \
} while(0)


// Ultrasonic sensor
#define PIN_TRIG        (1 << 27)   // PTA27
#define PIN_ECHO        (1 << 26)   // PTA26
#define PIN_TRIG_NUM    27
#define PIN_ECHO_NUM    26

// 7-segment display digits
#define DIG_1           (1 << 9)    // PTD9
#define DIG_2           (1 << 10)   // PTA10
#define DIG_3           (1 << 11)   // PTA11
#define DIG_4           (1 << 7)    // PTA7
#define DIG_PTA_MASK    (DIG_2 | DIG_3 | DIG_4)
#define DIG_PTD_MASK    (DIG_1)

// 7-segment display segments
#define SEG_A           (1 << 13)   // PTD13
#define SEG_B           (1 << 9)    // PTA9
#define SEG_C           (1 << 8)    // PTA8
#define SEG_D           (1 << 12)   // PTD12
#define SEG_E           (1 << 8)    // PTD8
#define SEG_F           (1 << 14)   // PTD14
#define SEG_G           (1 << 6)    // PTA6
#define SEG_DP          (1 << 15)   // PTD15
#define SEG_PTA_MASK    (SEG_B | SEG_C | SEG_G)
#define SEG_PTD_MASK    (SEG_A | SEG_D | SEG_E | SEG_F | SEG_DP)

// Buttons
#define BTN_MODE        (1 << 10)   // SW2  PTE10
#define BTN_DOWN        (1 << 12)   // SW3  PTE12
#define BTN_UP          (1 << 26)   // SW5  PTE26
#define BTN_BUZZ        (1 << 27)   // SW4  PTE27
#define BTN_MASK        (BTN_BUZZ | BTN_MODE | BTN_UP | BTN_DOWN)

// Buzzer
#define BUZZER          (1 << 4)    // PTA4

#define BUS_CLK_HZ      50000000
#define US_TO_TICKS(us) ((BUS_CLK_HZ / 1000000) * (us))

#define TRIG_PERIOD     60000
#define TRIG_PULSE      10
#define ECHO_TIMEOUT    30000
#define DISPLAY_DELAY   4000

#define DEFAULT_THRESHOLD   20
#define MIN_THRESHOLD       5
#define MAX_THRESHOLD       200
#define STEP_THRESHOLD      5
#define SW_DEBOUNCE         150

#define SEG_PATTERN_0   { .a = SEG_B | SEG_C,              .d = SEG_A | SEG_D | SEG_E | SEG_F }
#define SEG_PATTERN_1   { .a = SEG_B | SEG_C,              .d = 0 }
#define SEG_PATTERN_2   { .a = SEG_B | SEG_G,              .d = SEG_A | SEG_D | SEG_E }
#define SEG_PATTERN_3   { .a = SEG_B | SEG_C | SEG_G,      .d = SEG_A | SEG_D }
#define SEG_PATTERN_4   { .a = SEG_B | SEG_C | SEG_G,      .d = SEG_F }
#define SEG_PATTERN_5   { .a = SEG_C | SEG_G,              .d = SEG_A | SEG_D | SEG_F }
#define SEG_PATTERN_6   { .a = SEG_C | SEG_G,              .d = SEG_A | SEG_D | SEG_E | SEG_F }
#define SEG_PATTERN_7   { .a = SEG_B | SEG_C,              .d = SEG_A }
#define SEG_PATTERN_8   { .a = SEG_B | SEG_C | SEG_G,      .d = SEG_A | SEG_D | SEG_E | SEG_F }
#define SEG_PATTERN_9   { .a = SEG_B | SEG_C | SEG_G,      .d = SEG_A | SEG_D | SEG_F }

typedef enum {
    S_NORMAL = 0,
    S_CFG   = 1
} state_t;

typedef struct {
    uint32_t a;
    uint32_t d;
} seg_pattern_t;

volatile state_t ui_state = S_NORMAL;
volatile int threshold = DEFAULT_THRESHOLD;
volatile int ticks = 0;

volatile int echo_ticks = 0;
volatile uint8_t echo_valid = 0;
volatile int distance = 0;

volatile uint8_t disp_digits[4] = {0, 0, 0, 0};
volatile uint8_t disp_pos = 0;

volatile int last_btn = 0;

volatile uint8_t buzzer_enabled = 1;
volatile int beep_period = 0;
volatile int beep_on = 0;

static const seg_pattern_t seg_patterns[10] = {
    SEG_PATTERN_0, SEG_PATTERN_1, SEG_PATTERN_2, SEG_PATTERN_3, SEG_PATTERN_4,
    SEG_PATTERN_5, SEG_PATTERN_6, SEG_PATTERN_7, SEG_PATTERN_8, SEG_PATTERN_9
};

/**
 * @brief Precise delay using assembly
 * @param cycles Number of cycles to delay (approximately)
 */
static inline void delay_asm(uint32_t cycles) {
    __asm volatile (
        "   subs %[cycles], #1  \n"
        "   bne  .-4            \n"
        : [cycles] "+r" (cycles)
        :
        : "cc"
    );
}

/**
 * @brief Delay in microseconds (approximate)
 * @param us Microseconds to delay
 */
#define DELAY_US(us) delay_asm((BUS_CLK_HZ / 1000000) * (us) / 4)
#define DELAY_MS(ms) delay_asm((BUS_CLK_HZ / 1000) * (ms) / 4)

#define DIGITS_ALL_OFF() do { \
    GPIO_CLEAR(GPIOA, DIG_PTA_MASK); \
    GPIO_CLEAR(GPIOD, DIG_PTD_MASK); \
} while(0)

#define SEGMENTS_ALL_OFF() do { \
    GPIO_SET(GPIOA, SEG_PTA_MASK); \
    GPIO_SET(GPIOD, SEG_PTD_MASK); \
} while(0)

/**
 * @brief Enable one digit at given position
 * @param pos 1..4 digit index
 */
static inline void digit_on(int pos) {
    DIGITS_ALL_OFF();
    switch (pos) {
        case 1: GPIO_SET(GPIOD, DIG_1); break;
        case 2: GPIO_SET(GPIOA, DIG_2); break;
        case 3: GPIO_SET(GPIOA, DIG_3); break;
        case 4: GPIO_SET(GPIOA, DIG_4); break;
        default: break;
    }
}

/**
 * @brief Display single digit using lookup table
 * @param digit 0..9, or 10+ = blank
 * @param dot   0/1 decimal point
 */
static inline void display_digit(int digit, int dot) {
    uint32_t a = 0;
    uint32_t d = 0;

    if (digit >= 0 && digit <= 9) {
        a = seg_patterns[digit].a;
        d = seg_patterns[digit].d;
    }

    if (dot) d |= SEG_DP;

    SEGMENTS_ALL_OFF();
    GPIO_CLEAR(GPIOA, a);
    GPIO_CLEAR(GPIOD, d);
}

/**
 * @brief Convert integer to 4 display digits
 * @param value Number to display (0-9999)
 */
#define DISPLAY_NUMBER(value) do { \
    int _val = (value); \
    if (_val < 0) _val = 0; \
    if (_val > 9999) _val = 9999; \
    disp_digits[0] = (_val / 1000) % 10; \
    disp_digits[1] = (_val / 100)  % 10; \
    disp_digits[2] = (_val / 10)   % 10; \
    disp_digits[3] = (_val / 1)    % 10; \
} while(0)

/**
 * @brief Send trigger signal to ultrasonic sensor
 */
static inline void send_trig(void) {
    echo_valid = 0;

    GPIO_SET(GPIOA, PIN_TRIG);

    PIT_STOP(0);
    PIT_CLEAR_FLAG(0);
    PIT_START(0);
}

/**
 * @brief Process received echo and calculate distance
 */
static inline void receive_echo(void) {
    if (!echo_valid) return;

    uint32_t ticks_local = echo_ticks;
    echo_valid = 0;

    uint32_t pulse_us = (uint32_t)(((uint64_t)ticks_local * 1000000ULL) / BUS_CLK_HZ);
    distance = pulse_us / 58;
}

/**
 * @brief Update buzzer parameters based on distance
 * @param dist Current distance
 * @param thresh Threshold distance
 */
static inline void buzzer_update(int dist, int thresh) {
    if (!buzzer_enabled || dist == 0 || dist > 250 || dist > thresh) {
        beep_period = 0;
        beep_on = 0;
        return;
    }

    if (dist > (thresh * 2) / 3) {
        beep_period = 300;
        beep_on = 80;
    } else if (dist > thresh / 3) {
        beep_period = 160;
        beep_on = 80;
    } else {
        beep_period = 80;
        beep_on = 80;
    }
}

void MCUInit(void) {
    MCG->C4 |= (MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01));
    SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);

    WDOG->STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

void PortsInit(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

    PORT_GPIO_CONFIG(PORTD, 13);
    PORT_GPIO_CONFIG(PORTA, 9);
    PORT_GPIO_CONFIG(PORTA, 8);
    PORT_GPIO_CONFIG_SRE(PORTD, 12);
    PORT_GPIO_CONFIG(PORTD, 8);
    PORT_GPIO_CONFIG_SRE(PORTD, 14);
    PORT_GPIO_CONFIG(PORTA, 6);
    PORT_GPIO_CONFIG(PORTD, 15);

    PORT_GPIO_CONFIG(PORTD, 9);
    PORT_GPIO_CONFIG(PORTA, 10);
    PORT_GPIO_CONFIG(PORTA, 11);
    PORT_GPIO_CONFIG(PORTA, 7);

    GPIO_SET_OUTPUT(GPIOA, DIG_2 | DIG_3 | DIG_4 | SEG_B | SEG_C | SEG_G);
    GPIO_SET_OUTPUT(GPIOD, DIG_1 | SEG_A | SEG_D | SEG_E | SEG_F | SEG_DP);

    PORT_GPIO_INPUT_IRQ(PORTA, PIN_ECHO_NUM, 0b1011);
    PORT_CLEAR_FLAG(PORTA, PIN_ECHO);
    GPIO_SET_INPUT(GPIOA, PIN_ECHO);

    PORT_GPIO_CONFIG(PORTA, PIN_TRIG_NUM);
    GPIO_SET_OUTPUT(GPIOA, PIN_TRIG);

    PORT_GPIO_CONFIG(PORTA, 4);
    GPIO_SET_OUTPUT(GPIOA, BUZZER);
    GPIO_CLEAR(GPIOA, BUZZER);

    PORT_BUTTON_CONFIG(PORTE, 27);
    PORT_BUTTON_CONFIG(PORTE, 10);
    PORT_BUTTON_CONFIG(PORTE, 12);
    PORT_BUTTON_CONFIG(PORTE, 26);
    PORT_CLEAR_FLAG(PORTE, BTN_MASK);

    NVIC_SETUP_IRQ(PORTE_IRQn);
    NVIC_SETUP_IRQ(PORTA_IRQn);
}

void PITInit(void) {
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT_ENABLE();

    PIT_SETUP_TIMER(0, US_TO_TICKS(TRIG_PULSE), 0);

    PIT_SETUP_TIMER(1, US_TO_TICKS(ECHO_TIMEOUT), 0);

    PIT_SETUP_TIMER(2, US_TO_TICKS(TRIG_PERIOD), 1);

    PIT_SETUP_TIMER(3, US_TO_TICKS(DISPLAY_DELAY), 1);

    NVIC_SETUP_IRQ(PIT0_IRQn);
    NVIC_SETUP_IRQ(PIT1_IRQn);
    NVIC_SETUP_IRQ(PIT2_IRQn);
    NVIC_SETUP_IRQ(PIT3_IRQn);
}

/**
 * @brief PIT0 IRQ - End trigger pulse
 */
void PIT0_IRQHandler(void) {
    PIT_CLEAR_FLAG(0);
    GPIO_CLEAR(GPIOA, PIN_TRIG);
    PIT_STOP(0);
}

/**
 * @brief PIT1 IRQ - Echo timeout
 */
void PIT1_IRQHandler(void) {
    echo_valid = 0;
    PIT_CLEAR_FLAG(1);
    PIT_STOP(1);
}

/**
 * @brief PIT2 IRQ - Periodic trigger
 */
void PIT2_IRQHandler(void) {
    PIT_CLEAR_FLAG(2);
    send_trig();
}

/**
 * @brief PIT3 IRQ - Display refresh and buzzer control
 */
void PIT3_IRQHandler(void) {
    PIT_CLEAR_FLAG(3);
    ticks++;

    DIGITS_ALL_OFF();
    SEGMENTS_ALL_OFF();

    uint8_t pos = disp_pos;
    disp_pos = (disp_pos + 1) & 3;

    display_digit(disp_digits[pos], 0);
    digit_on(pos + 1);

    static uint16_t phase = 0;
    static uint8_t tone_toggle = 0;

    if (!buzzer_enabled || beep_period == 0 || beep_on == 0) {
        GPIO_CLEAR(GPIOA, BUZZER);
        phase = 0;
        tone_toggle = 0;
        return;
    }

    if (++phase >= beep_period) phase = 0;

    if (phase < beep_on) {
        tone_toggle ^= 1;
        if (tone_toggle) GPIO_SET(GPIOA, BUZZER);
        else GPIO_CLEAR(GPIOA, BUZZER);
    } else {
        GPIO_CLEAR(GPIOA, BUZZER);
        tone_toggle = 0;
    }
}

/**
 * @brief PORTA IRQ - Echo pulse capture
 */
void PORTA_IRQHandler(void) {
    if (!(PORTA->ISFR & PIN_ECHO)) return;
    PORT_CLEAR_FLAG(PORTA, PIN_ECHO);

    if (GPIO_READ(GPIOA, PIN_ECHO)) {
        PIT_STOP(1);
        PIT_CLEAR_FLAG(1);
        PIT_START(1);
    } else {
        if (PIT_IS_RUNNING(1)) {
            echo_ticks = PIT_GET_ELAPSED(1);
            echo_valid = 1;
            PIT_STOP(1);
        }
    }
}

/**
 * @brief PORTE IRQ - Button handling
 */
void PORTE_IRQHandler(void) {
    int flags = PORTE->ISFR & BTN_MASK;
    int now = ticks;

    PORT_CLEAR_FLAG(PORTE, flags);
    if (flags == 0) return;

    if ((now - last_btn) < SW_DEBOUNCE) return;
    last_btn = now;

    if (flags & BTN_BUZZ) {
        buzzer_enabled ^= 1;
    }

    if (flags & BTN_MODE) {
        ui_state = (ui_state == S_NORMAL) ? S_CFG : S_NORMAL;
    }

    if (ui_state == S_CFG) {
        if (flags & BTN_UP) {
            threshold = (threshold + STEP_THRESHOLD <= MAX_THRESHOLD)
                        ? threshold + STEP_THRESHOLD : MAX_THRESHOLD;
        }
        if (flags & BTN_DOWN) {
            threshold = (threshold >= MIN_THRESHOLD + STEP_THRESHOLD)
                        ? threshold - STEP_THRESHOLD : MIN_THRESHOLD;
        }
    }
}

int main(void) {
    MCUInit();
    PortsInit();
    PITInit();

    DIGITS_ALL_OFF();
    SEGMENTS_ALL_OFF();
    DISPLAY_NUMBER(0);

    while (1) {
        receive_echo();

        DISPLAY_NUMBER(ui_state==S_NORMAL?distance:threshold);

        buzzer_update((uint16_t)distance, threshold);
    }
}
