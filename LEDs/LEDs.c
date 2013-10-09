#define F_CPU 16000000
#define BAUD  9600

#define MYUBRR (F_CPU/16/BAUD-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define _4015_d     0 /* marrom branco     */
#define _4015_clk   1 /* verde  branco     */
#define _4015_mr    4 /* azul              */

#define _4094_clk   2 /* marrom            */
#define _4094_str   3 /* azul   branco     */
#define _4094_d     0 /* marrom branco [2] */
#define _4094_oe    5 /* verde             */

extern PROGMEM uint8_t fonte[0x100][7];

#define ligar_bit(bit) do {         \
    PORTC |=  (1<<bit);             \
} while (0)
#define desligar_bit(bit) do {      \
    PORTC &= ~(1<<bit);             \
} while (0)
#define clk_d(CI, val) do {         \
    desligar_bit(_##CI##_clk);      \
    if (val) ligar_bit(_##CI##_d);  \
    else  desligar_bit(_##CI##_d);  \
    ligar_bit(_##CI##_clk);         \
} while (0)

static uint8_t text_buf[10] = {0};
static uint8_t text_idx     =  0 ;
static uint8_t out_buf[7][7] = {{0}}; /* [linha][50 bits] */

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

void loop();

void main(void) __attribute__((noreturn));
void main()
{
    init_serial();
    init_timer();
    sei();  /* enable interrupts */
    DDRC = 0x3F;
    desligar_bit(_4015_mr);
    ligar_bit(_4094_oe);
    while (1)
        loop();
}

static uint8_t new_char = 0;
static uint8_t red   = 1;
static uint8_t green = 0;

void loop()
{
    if (new_char) {
        new_char = 0;
        for (uint8_t i = 0; i < 10; i++)
            print_char(text_buf[i], i*5);
    }
}

/* main timer interrupt */
ISR(TIMER0_OVF_vect)
{
    static uint8_t i = 0;
    desligar_bit(_4094_str);
    for (uint8_t j = 0; j < 50; j++)
        clk_d(4094, ((out_buf[i][j>>3]>>(j&7))&1));
    desligar_bit(_4094_oe);
    ligar_bit(_4094_str);
    for (uint8_t j = 0; j < 7; j++) {
        clk_d(4015, (j==i)&red  );
        clk_d(4015, (j==i)&green);
    }
    clk_d(4015, 0);
    ligar_bit(_4094_oe);
    if (++i == 7)
        i = 0;
}

/* usart rx interrupt */
ISR(USART_RX_vect)
{
    text_buf[text_idx++] = UDR0;
    if (text_idx == 10)
        text_idx = 0;
    new_char = 1;
}
