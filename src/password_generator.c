/*
 * ============================================================================
 *  Random Password Generator using 8051 Microcontroller with LCD Display
 * ============================================================================
 *
 *  Target      : AT89S52 (8051 family), 40-pin DIP
 *  Clock       : 11.0592 MHz crystal
 *  Display     : 16x2 alphanumeric LCD (HD44780 compatible), 8-bit mode
 *  Toolchain   : Keil uVision / C51
 *
 *  ---------------------------------------------------------------------------
 *  CONNECTIONS
 *  ---------------------------------------------------------------------------
 *  The sbit / port names below are SFR bit references, not pin numbers. The
 *  physical DIP-40 pin numbers are given here for wiring reference only; they
 *  have no effect on compilation.
 *
 *    Function        Port bit        MCU pin (DIP-40)     LCD pin
 *    --------        --------        ----------------     -------
 *    LCD D0          P1.0            1                    7
 *    LCD D1          P1.1            2                    8
 *    LCD D2          P1.2            3                    9
 *    LCD D3          P1.3            4                    10
 *    LCD D4          P1.4            5                    11
 *    LCD D5          P1.5            6                    12
 *    LCD D6          P1.6            7                    13
 *    LCD D7          P1.7            8                    14
 *    LCD RS          P2.4            25                   4
 *    LCD RW          P2.5            26                   5
 *    LCD EN          P2.6            27                   6
 *    Button          P0.0            39                   -
 *
 *    Support pins:  VCC = 40 (+5 V)    GND = 20
 *                   EA  = 31 (tie to +5 V, else code will not execute)
 *                   RST = 9  (10 uF to +5 V, 10 k to GND)
 *                   XTAL1 = 19, XTAL2 = 18 (crystal + 2 x 33 pF to GND)
 *
 *  NOTE ON PORT 0 NUMBERING: on the DIP-40 package Port 0 is numbered in
 *  reverse -- P0.0 is physical pin 39 and P0.7 is pin 32. The button belongs
 *  on pin 39.
 *
 *  NOTE ON THE BUTTON PULL-UP: Port 0 is open-drain and has NO internal
 *  pull-ups. Writing button = 1 below only releases the pin; an external
 *  10k pull-up from P0.0 to +5 V is required for the idle state to read 1.
 *  Ports 1 and 2 do have internal pull-ups, which is why the LCD bus needs
 *  no external components.
 *
 *  ---------------------------------------------------------------------------
 *  OPERATION
 *  ---------------------------------------------------------------------------
 *  The LCD idles with the prompt "Press Key". On a debounced button press,
 *  eight characters are drawn from a mixed alphabet using the live Timer 0
 *  register values as the randomness source and printed to the LCD.
 *
 *  Authors     : M Mohit Srinivasa (160123735040)
 *                Muppidi Varun     (160123735047)
 *  Supervisor  : N. Jagan Mohan Reddy, Assistant Professor, Dept. of ECE
 *  Institution : Chaitanya Bharathi Institute of Technology (A), Hyderabad
 *  Year        : 2025-2026
 *
 *  CHARSET_SIZE is 70 as submitted and tested, while the charset string below
 *  holds 68 characters. Kept as-is; see "Known Issues" in README.md.
 * ============================================================================
 */

#include <reg51.h>

#define LCD          P1      /* LCD data bus on Port 1, pins 1-8 */
#define PASS_LEN     8       /* Number of characters generated    */
#define CHARSET_SIZE 70      /* Modulus used by get_random()       */

sbit RS     = P2 ^ 4;        /* MCU pin 25 -> LCD pin 4: 0 = cmd, 1 = data */
sbit RW     = P2 ^ 5;        /* MCU pin 26 -> LCD pin 5: held low (write)  */
sbit EN     = P2 ^ 6;        /* MCU pin 27 -> LCD pin 6: enable strobe     */
sbit button = P0 ^ 0;        /* MCU pin 39: trigger button, active low     */

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

    button = 1;                     /* release P0.0 (needs external pull-up) */

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
