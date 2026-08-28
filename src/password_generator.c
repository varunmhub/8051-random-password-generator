/*
 * ============================================================================
 *  Random Password Generator using 8051 Microcontroller with LCD Display
 * ============================================================================
 *
 *  Target      : AT89S52 (8051 family)
 *  Clock       : 11.0592 MHz crystal
 *  Display     : 16x2 alphanumeric LCD (HD44780 compatible), 8-bit mode
 *  Toolchain   : Keil uVision / C51
 *
 *  Connections :
 *      LCD D0-D7  -> Port 1 (P1.0 - P1.7)
 *      LCD RS     -> P2.4
 *      LCD RW     -> P2.5
 *      LCD EN     -> P2.6
 *      Button     -> P0.0 (active low)
 *
 *  Operation   : The LCD idles with the prompt "Press Key". On a debounced
 *                button press, eight characters are drawn from a mixed
 *                alphabet using the live Timer 0 register values as the
 *                randomness source and printed to the LCD.
 *
 *  Authors     : M Mohit Srinivasa (160123735040)
 *                Muppidi Varun     (160123735047)
 *  Supervisor  : N. Jagan Mohan Reddy, Assistant Professor, Dept. of ECE
 *  Institution : Chaitanya Bharathi Institute of Technology (A), Hyderabad
 *  Year        : 2025-2026
 *
 *  NOTE: CHARSET_SIZE is 70 as submitted, while the charset string below
 *        holds 68 characters. See the "Known Issues" section of README.md.
 * ============================================================================
 */

#include <reg51.h>

#define LCD          P1      /* LCD data bus on Port 1        */
#define PASS_LEN     8       /* Number of characters generated */
#define CHARSET_SIZE 70      /* Modulus used by get_random()   */

sbit RS     = P2 ^ 4;        /* Register select: 0 = cmd, 1 = data */
sbit RW     = P2 ^ 5;        /* Read/Write: held low (write only)  */
sbit EN     = P2 ^ 6;        /* Enable strobe                      */
sbit button = P0 ^ 0;        /* Trigger button, active low         */

/* Character pool: 26 uppercase + 26 lowercase + 10 digits + 6 symbols */
unsigned char code charset[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#$%&*";

/* --------------------------------------------------------------------------
 * Crude blocking delay. Approximately 1 ms per unit at 11.0592 MHz.
 * -------------------------------------------------------------------------- */
void delay(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 1275; j++)
            ;
}

/* --------------------------------------------------------------------------
 * Send a command byte to the LCD controller.
 * -------------------------------------------------------------------------- */
void lcd_cmd(unsigned char cmd)
{
    LCD = cmd;
    RS  = 0;
    RW  = 0;
    EN  = 1;
    delay(1);
    EN = 0;
    delay(2);
}

/* --------------------------------------------------------------------------
 * Send a single data byte (character) to the LCD.
 * -------------------------------------------------------------------------- */
void lcd_data(unsigned char dat)
{
    LCD = dat;
    RS  = 1;
    RW  = 0;
    EN  = 1;
    delay(1);
    EN = 0;
    delay(2);
}

/* --------------------------------------------------------------------------
 * Write a null-terminated string to the LCD at the current cursor position.
 * -------------------------------------------------------------------------- */
void lcd_string(unsigned char *str)
{
    while (*str)
        lcd_data(*str++);
}

/* --------------------------------------------------------------------------
 * LCD initialisation sequence.
 *   0x38 -> 8-bit interface, 2 lines, 5x7 dot matrix
 *   0x0C -> display on, cursor off, no blink
 *   0x01 -> clear display
 *   0x06 -> entry mode: increment cursor, no display shift
 * -------------------------------------------------------------------------- */
void lcd_init(void)
{
    lcd_cmd(0x38);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    lcd_cmd(0x06);
}

/* --------------------------------------------------------------------------
 * Start Timer 0 in Mode 1 (16-bit) and let it free-run. It is never read
 * for timing, only sampled for entropy.
 * -------------------------------------------------------------------------- */
void timer0_init(void)
{
    TMOD = 0x01;
    TR0  = 1;
}

/* --------------------------------------------------------------------------
 * Return a pseudo-random index into charset[] from the live timer registers.
 * -------------------------------------------------------------------------- */
unsigned char get_random(void)
{
    return (TH0 + TL0) % CHARSET_SIZE;
}

/* --------------------------------------------------------------------------
 * Main loop: idle prompt, debounce, generate, wait for release.
 * -------------------------------------------------------------------------- */
void main(void)
{
    unsigned char i, idx;

    lcd_init();
    timer0_init();

    button = 1;                     /* configure P0.0 as input (weak high) */

    lcd_cmd(0x80);                  /* cursor to line 1, position 0 */
    lcd_string("Press Key");

    while (1)
    {
        if (button == 0)            /* possible press detected */
        {
            delay(50);              /* debounce window */

            if (button == 0)        /* confirm it is a real press */
            {
                lcd_cmd(0x01);      /* clear display */
                lcd_cmd(0x80);      /* home cursor   */

                for (i = 0; i < PASS_LEN; i++)
                {
                    idx = get_random();
                    lcd_data(charset[idx]);
                    delay(50);      /* visible per-character cadence */
                }

                while (button == 0) /* wait for release */
                    ;
                delay(200);         /* settle before rearming */
            }
        }
    }
}
