#define F_CPU 16000000
#define BAUD  9600

#define MYUBRR (F_CPU/16/BAUD-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

struct pino {
    volatile uint8_t *dir;
    volatile uint8_t *pin;
    volatile uint8_t *port;
    uint8_t bit;
};

static const struct pino _4015_d   = { &DDRB, &PINB, &PORTB, 0 }; /* marrom branco     */
static const struct pino _4015_clk = { &DDRD, &PIND, &PORTD, 7 }; /* verde  branco     */
static const struct pino _4015_mr  = { &DDRD, &PIND, &PORTD, 4 }; /* azul              */

static const struct pino _4094_clk = { &DDRD, &PIND, &PORTD, 6 }; /* marrom            */
static const struct pino _4094_str = { &DDRD, &PIND, &PORTD, 5 }; /* azul   branco     */
static const struct pino _4094_d   = { &DDRB, &PINB, &PORTB, 0 }; /* marrom branco [2] */
static const struct pino _4094_oe  = { &DDRD, &PIND, &PORTD, 3 }; /* verde             */

static const struct pino botao     = { &DDRD, &PIND, &PORTD, 2 }; /* botao             */
static const struct pino botao_led = { &DDRC, &PINC, &PORTC, 5 }; /* led do botao      */

static const struct pino rele_1    = { &DDRB, &PINB, &PORTB, 4 }; /* rele 1            */
static const struct pino rele_2    = { &DDRB, &PINB, &PORTB, 5 }; /* rele 2            */

static inline void set_output  (const struct pino *p) { *p->dir  |=  (1<<p->bit); }
static inline void set_input   (const struct pino *p) { *p->dir  &= ~(1<<p->bit); }
static inline void ligar_bit   (const struct pino *p) { *p->port |=  (1<<p->bit); }
static inline void desligar_bit(const struct pino *p) { *p->port &= ~(1<<p->bit); }
static inline uint8_t ler_bit  (const struct pino *p) { return *p->pin & (1<<p->bit); }
static inline void clk_d       (const struct pino *clk, const struct pino *d, uint8_t val)
{
    desligar_bit(clk);
    if (val) ligar_bit(d);
    else  desligar_bit(d);
    ligar_bit(clk);
}

static const struct pino pino_r = { &DDRB, &PINB, &PORTB, 1 }; /* led vermelho      */
static const struct pino pino_g = { &DDRB, &PINB, &PORTB, 3 }; /* led verde         */
static const struct pino pino_b = { &DDRB, &PINB, &PORTB, 2 }; /* led azul          */
static volatile uint8_t *led_r  = &OCR1AL;              /* led vermelho      */
static volatile uint8_t *led_g  = &OCR2A ;              /* led verde         */
static volatile uint8_t *led_b  = &OCR1BL;              /* led azul          */
static uint8_t cur_r = 0;
static uint8_t cur_g = 0;
static uint8_t cur_b = 0;

/* this is a cheap random number generator */
static uint8_t cheap_rand(void)
{
    static uint16_t state;
    state = state * 25173 + 13849;
    return state>>7;
}

extern PROGMEM uint8_t fonte[0x100][7];

static uint8_t out_buf[7][7] = {{0}}; /* [linha][50 bits] */
static uint8_t out_idx       =   0  ;

static void
print_char(uint8_t c, uint8_t pos)
{
    uint8_t letra[4] = { pgm_read_byte_near(&fonte[c][0]),
                         pgm_read_byte_near(&fonte[c][1]),
                         pgm_read_byte_near(&fonte[c][2]),
                         pgm_read_byte_near(&fonte[c][3]) };
    uint8_t linhas[7] = { letra[0] >> 4, letra[0] & 0x0F,
                          letra[1] >> 4, letra[1] & 0x0F,
                          letra[2] >> 4, letra[2] & 0x0F,
                          letra[3] >> 4 };
    uint8_t d[2] = { pos >> 3, (pos + 3) >> 3 };
    uint8_t q[2] = { pos  & 7, (8 - pos)  & 7 };
    uint8_t mask[2] = { 0xff & ~(0x0f << q[0]), 0xff << (q[0]-4) };

    for (uint8_t i = 0; i < 7; i++) {
        out_buf[i][d[0]] = (out_buf[i][d[0]] & mask[0]) | (linhas[i] << q[0]);
        if (d[0] != d[1])
            out_buf[i][d[1]] = (out_buf[i][d[1]] & mask[1]) | (linhas[i] >> q[1]);
    }
}

static void
init_button()
{
    EICRA = (1 << ISC01); /* falling edge */
    EIMSK = (1 << INT0);
}

static void
init_serial()
{
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); /* 8-bit */
    /* enable rx, tx, and USART_RX interrupt */
    UCSR0B = (1 << RXEN0 ) | (1 << TXEN0 ) | (1 << RXCIE0);
}

static void
init_timer()
{
    /* timer0 = main timer */
    TCCR0A = 0;                         /* normal port operation */
    TCCR0B = (1 << CS01) | (1 << CS00); /* prescaler = 64 */
    TIFR0  = (1 << TOV0);               /* clear pending interrupts */
    TIMSK0 = (1 << TOIE0);              /* enable TIMER0_OVF interrupt */
}

static void
init_pwm()
{
    /* pwm on arduino pins 9 and 10 */
    /* phase-correct pwm, 8-bit, OC1A and OC1B */
    TCCR1A = (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
    TCCR1B = (1 << CS10); /* prescaler = 1 */

    /* pwm on arduino pin 11 */
    /* phase-correct pwm, 8-bit, OC2A */
    TCCR2A = (1 << WGM20) | (1 << COM2A1);
    TCCR2B = (1 << CS20); /* prescaler = 1 */
}

static void loop()
{
}

static uint8_t status = 0;

void main(void) __attribute__((noreturn));
void main()
{
    init_button();
    init_serial();
    init_timer();
    init_pwm();
    sei();  /* enable interrupts */

    set_output(&_4015_d  );
    set_output(&_4015_clk);
    set_output(&_4015_mr );
    set_output(&_4094_clk);
    set_output(&_4094_str);
    set_output(&_4094_d  );
    set_output(&_4094_oe );

    set_output(&pino_r);
    set_output(&pino_g);
    set_output(&pino_b);

    set_input (&botao    );
    set_output(&botao_led);

    set_output(&rele_1);
    set_output(&rele_2);

    desligar_bit(&_4015_mr);
    ligar_bit(&_4094_oe);
    while (1)
        loop();
}

static uint8_t red   = 1;
static uint8_t green = 0;

static uint16_t turnoff_count = 0;
static uint8_t debounce_count = 0;

/* main timer interrupt: F_CPU / (256 * 64) = 976.5625 Hz, 1.024 ms */
ISR(TIMER0_OVF_vect)
{
    if (debounce_count) {
        debounce_count--;
        if (!debounce_count && ler_bit(&botao)) {
            /* 51.2 ms debounce */
            status = !status;
            if (!status) {
                desligar_bit(&botao_led);
                *led_r = 255;
                *led_g = 255;
                *led_b = 255;
                turnoff_count = 0xffff;
            } else {
                ligar_bit(&botao_led);
                ligar_bit(&rele_1);
                ligar_bit(&rele_2);
            }
            UDR0 = status + '0';
        }
    }

    if (!status) {
        if (turnoff_count) {
            turnoff_count--;
            if (!turnoff_count) {
                /* 67.10784s delay */
                desligar_bit(&rele_1);
                desligar_bit(&rele_2);
                *led_r = 0;
                *led_g = 0;
                *led_b = 0;
            }
        }
        return;
    }

    static uint8_t led_count = 0;

    if (++led_count == 10) {
        uint8_t old_r = *led_r;
        uint8_t old_g = *led_g;
        uint8_t old_b = *led_b;
        if (old_r == cur_r && old_g == cur_g && old_b == cur_b) {
            cur_r = cheap_rand();
            cur_g = cheap_rand();
            cur_b = cheap_rand();
        }
        if (old_r > cur_r) *led_r -= 1; else if (old_r < cur_r) *led_r += 1;
        if (old_g > cur_g) *led_g -= 1; else if (old_g < cur_g) *led_g += 1;
        if (old_b > cur_b) *led_b -= 1; else if (old_b < cur_b) *led_b += 1;
        led_count = 0;
    }

    static uint8_t i = 0;
    desligar_bit(&_4094_str);
    for (uint8_t j = 0; j < 50; j++)
        clk_d(&_4094_clk, &_4094_d, ((out_buf[i][j>>3]>>(j&7))&1));
    desligar_bit(&_4094_oe);
    ligar_bit(&_4094_str);
    for (uint8_t j = 0; j < 7; j++) {
        clk_d(&_4015_clk, &_4015_d, (j==i)&red  );
        clk_d(&_4015_clk, &_4015_d, (j==i)&green);
    }
    clk_d(&_4015_clk, &_4015_d, 0);
    ligar_bit(&_4094_oe);
    if (++i == 7)
        i = 0;
}

/* usart rx interrupt */
ISR(USART_RX_vect)
{
    print_char(UDR0, out_idx++ * 5);
    if (out_idx == 10)
        out_idx = 0;
}

/* button interrupt */
ISR(INT0_vect)
{
    debounce_count = 50;
}
