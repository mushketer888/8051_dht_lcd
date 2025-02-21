#include <at89x51.h>

#include <stdio.h>

#define F_CPU 16000000 UL

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#define VERSION "1.4"

#define DHT11 P1_7 /* Connect DHT11 output Pin to P2.1 Pin */

#define LCD_Port P2 /* P1 port as data port */
#define rs P2_0     /* Register select pin */
#define rw P2_1     /* Read/Write pin */
#define en P2_2     /* Enable pin */

// delay functions
inline void timer_delay20ms16mhz() /* Timer0 delay function */
{
    TMOD = 0x01;

    // 16mhz
    TH0 = 0x97; // Load higher 8-bit in TH0
    TL0 = 0xF4; // Load lower 8-bit in TL0

    TR0 = 1; /* Start timer0 */
    while (TF0 == 0)
        ;    /* Wait until timer0 flag set */
    TR0 = 0; /* Stop timer0 */
    TF0 = 0; /* Clear timer0 flag */
}

inline void timer_delay30us16mhz() /* Timer0 delay function */
{
    TMOD = 0x01; /* Timer0 mode1 (16-bit timer mode) */

    // 16mhz
    TH0 = 0xFF; /* Load higher 8-bit in TH0 */
    TL0 = 0xDB; /* Load lower 8-bit in TL0 bilo D7 */

    TR0 = 1; /* Start timer0 */
    while (TF0 == 0)
        ;    /* Wait until timer0 flag set */
    TR0 = 0; /* Stop timer0 */
    TF0 = 0; /* Clear timer0 flag */
}

inline void timer_delay20ms8mhz()
{
    TMOD = 0x01; // Set Timer 0 in Mode 1 (16-bit timer)
    TH0 = 0xCB;  // Load higher 8-bit in TH0
    TL0 = 0xEA;  // Load lower 8-bit in TL0
    TR0 = 1;     // Start Timer 0
    while (TF0 == 0)
        ;    // Wait until Timer 0 overflows (TF0 = 1)
    TR0 = 0; // Stop Timer 0
    TF0 = 0; // Clear Timer 0 overflow flag
}

inline void timer_delay30us8mhz()
{
    TMOD = 0x01; // Set Timer 0 in Mode 1 (16-bit timer)
    TH0 = 0xFF;  // Load higher 8-bit in TH0
    TL0 = 0xEB;  // Load lower 8-bit in TL0
    TR0 = 1;     // Start Timer 0
    while (TF0 == 0)
        ;    // Wait until Timer 0 overflows (TF0 = 1)
    TR0 = 0; // Stop Timer 0
    TF0 = 0; // Clear Timer 0 overflow flag
}

void delay(unsigned int count)
{
    int i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 112; j++)
        {
        };
}

// LCD functions
void LCD_Command(char cmnd) /* LCD16x2 command funtion */
{
    LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
    rs = 0;                                       /* command reg. */
    rw = 0;                                       /* Write operation */
    en = 1;
    delay(1);
    en = 0;
    delay(2);

    LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4); /* sending lower nibble */
    en = 1;                                     /* enable pulse */
    delay(1);
    en = 0;
    delay(5);
}

void LCD_Char(char char_data) /* LCD data write function */
{
    LCD_Port = (LCD_Port & 0x0F) | (char_data & 0xF0); /* sending upper nibble */
    rs = 1;                                            /*Data reg.*/
    rw = 0;                                            /*Write operation*/
    en = 1;
    delay(1);
    en = 0;
    delay(2);

    LCD_Port = (LCD_Port & 0x0F) | (char_data << 4); /* sending lower nibble */
    en = 1;                                          /* enable pulse */
    delay(1);
    en = 0;
    delay(5);
}

void LCD_String(char *str) /* Send string to LCD function */
{
    int i;
    for (i = 0; str[i] != 0; i++) /* Send each char of string till the NULL */
    {
        LCD_Char(str[i]); /* Call LCD data write */
    }
}

void LCD_String_xy(char row, char pos, char *str) /* Send string to LCD function */
{
    if (row == 0)
        LCD_Command((pos & 0x0F) | 0x80); /* Command of first row and required position<16 */
    else if (row == 1)
        LCD_Command((pos & 0x0F) | 0xC0); /* Command of first row and required position<16 */
    LCD_String(str);                      /* Call LCD string function */
}

void LCD_Init(void) /* LCD Initialize function */
{
    delay(20); /* LCD Power ON Initialization time >15ms */

    LCD_Command(0x02); /* 4bit mode */
    LCD_Command(0x28); /* Initialization of 16X2 LCD in 4bit mode */
    LCD_Command(0x0C); /* Display ON Cursor OFF */
    LCD_Command(0x06); /* Auto Increment cursor */
    LCD_Command(0x01); /* clear display */
    LCD_Command(0x80); /* cursor at home position */
}

//DHT code
inline void Request() /* Microcontroller send request */
{
    DHT11 = 0;              /* set to low pin */
    timer_delay20ms16mhz(); /* wait for 20ms */
    DHT11 = 1;              /* set to high pin */
}

inline void Response() /* Receive response from DHT11 */
{
    unsigned char try_count = 0;

    while (DHT11 == 1)
    {
        if (++try_count == 0)
        { // Overflow check (255 + 1 = 0)
            // LCD_String_xy(0,0,"DHT NO RESP1"); //debug
            return;
        }
    }

    // Wait for DHT11 to pull the line high
    try_count = 0;
    while (DHT11 == 0)
    {
        if (++try_count == 0)
        { // Overflow check (255 + 1 = 0)
            // LCD_String_xy(0,0,"DHT NO RESP2"); //debug
            return;
        }
    }

    // Wait for DHT11 to pull the line low again
    try_count = 0;
    while (DHT11 == 1)
    {
        if (++try_count == 0)
        { // Overflow check (255 + 1 = 0)
            // LCD_String_xy(0,0,"DHT NO RESP3"); //debug
            return;
        }
    }
}

inline unsigned char Receive_data()
{
    unsigned char i, try_count = 0, data = 0;

    for (i = 0; i < 8; i++)
    {
        TL0 = 0;
        // Wait for DHT11 to pull the line high
        while (DHT11 == 0)
        {
            if (++try_count == 0)
            { // Overflow check (255 + 1 = 0)
                // LCD_String_xy(0,0,"DHT NO RESP"); //debug
                return 0;
            }
        }

        TR0 = 1; // Start Timer 0 !
        try_count = 0;
        // Wait for DHT11 to pull the line low
        while (DHT11 == 1)
        {
            if (++try_count == 0) // Overflow check (255 + 1 = 0)
            { 
                // LCD_String_xy(0,0,"DHT NO RESP"); //debug
                return 0;
            }
        }

        TR0 = 0; // stop timer

        // return TL0; //!!! debug low and high pulse width
        
        if (TL0 >= 50) // 50uS at 16mhz ~30 at 8mhz
        { 
            data = (data << 1) | (0x01);
        }
        else
        {
            data = (data << 1);
        }
    }

    return data;
}

//helper functions
void hexToStr(char *buf, unsigned int value)
{
    const char hexDigits[] = "0123456789ABCDEF"; // Hexadecimal digits (0-9, A-F)

    // Handle zero case
    if (value == 0)
    {
        buf[0] = '0';
        buf[1] = '\0'; // Null terminator
        return;
    }

    // Convert the value to hex and store it in buf
    int i = 0;
    while (value > 0)
    {
        buf[i] = hexDigits[value % 16]; // Get the hex digit
        value /= 16;                    // Shift the value by 16 (divide by 16)
        i++;
    }

    // Reverse the string to get the correct order
    int len = i;
    for (int j = 0; j < len / 2; j++)
    {
        char temp = buf[j];
        buf[j] = buf[len - j - 1];
        buf[len - j - 1] = temp;
    }

    buf[i] = '\0'; // Null terminator at the end
}

void intToStr(char *buf, int value)
{
    int i = 0;

    // Handle zero case
    if (value == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        // Convert the integer to string in reverse order
        while (value > 0)
        {
            buf[i++] = '0' + (value % 10); // Get the current digit
            value /= 10;                   // Shift the number by 10
        }
    }

    buf[i] = '\0'; // Null terminator

    // Reverse the string to get the correct order
    int len = i;
    for (int j = 0; j < len / 2; j++)
    {
        char temp = buf[j];
        buf[j] = buf[len - j - 1];
        buf[len - j - 1] = temp;
    }
}

int I_RH, D_RH, I_Temp, D_Temp, CheckSum;

void main()
{
    // AUXR = 0b00001001;//for at89s51

    unsigned char dat[17];
    char intPart[3], decPart[3];

    LCD_Init(); /* initialize LCD */
    LCD_String_xy(0, 0, "LCD Test");

    strcpy(dat, "Ver = ");
    strcat(dat, VERSION);
    LCD_String_xy(1, 0, dat);

    delay(2 * 1000);

    while (1)
    {
        memset(dat, 0, 20);

        // LCD_Command (0x01);
        // LCD_String_xy(0,0,"Read DHT");

        TMOD = 0x01; // Set Timer 0 in Mode 1 (16-bit timer)
        TH0 = 0;     // Load higher 8-bit in TH0
        TL0 = 0;     // Load lower 8-bit in TL0

        Request();  /* send start pulse */
        Response(); /* receive response */

        I_RH = Receive_data(); /* store first eight bit in I_RH */
        D_RH = Receive_data(); /* store next eight bit in D_RH */

        I_Temp = Receive_data(); /* store next eight bit in I_Temp */
        D_Temp = Receive_data(); /* store next eight bit in D_Temp */

        CheckSum = Receive_data(); /* store next eight bit in CheckSum */

        // while(1);
        /*if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
        {
            LCD_Command (0x01);
            LCD_String_xy(0,0,"CRC Err");
        }

        else
        {*/
        // Convert Humidity

        intToStr(intPart, I_RH);
        intToStr(decPart, D_RH);
        strcpy(dat, "Hum = ");
        strcat(dat, intPart);
        strcat(dat, ".");
        strcat(dat, decPart);
        LCD_String_xy(0, 0, dat);

        // Convert Temperature
        intToStr(intPart, I_Temp);
        intToStr(decPart, D_Temp);
        strcpy(dat, "Tem = ");
        strcat(dat, intPart);
        strcat(dat, ".");
        strcat(dat, decPart);
        LCD_String_xy(1, 0, dat);

        intToStr(dat, CheckSum);
        if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
        {
            strcat(dat, "!");
        }
        LCD_String_xy(1, 12, dat);

        //}
        DHT11 = 1;
        delay(15 * 1000);
        LCD_Command(0x01);
    }
}