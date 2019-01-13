#include <unistd.h>
#include "lcdScreen.h"

#include <stdio.h>

#define LCD_CHR 1
#define LCD_CMD 0

#define BIT_AT(num, pos)  (num & ( 1 << pos)) == (1 << pos)

#define sleepForProcessing() usleep(500)

static void lcd4bitFlushEnablePin(const struct lcd4bit *lcd)
{
    sleepForProcessing();
    lcd->gpioWrite(lcd->e, 1);
    sleepForProcessing();
    lcd->gpioWrite(lcd->e, 0);
    sleepForProcessing();
}

void _lcd4bitWrite(const struct lcd4bit *lcd, int bits, int mode)
{
    lcd->gpioWrite(lcd->rs, mode);

    //high bits
    lcd->gpioWrite(lcd->d4, 0);
    lcd->gpioWrite(lcd->d5, 0);
    lcd->gpioWrite(lcd->d6, 0);
    lcd->gpioWrite(lcd->d7, 0);

    if(BIT_AT(bits, 4))
        lcd->gpioWrite(lcd->d4, 1);
    if(BIT_AT(bits, 5))
        lcd->gpioWrite(lcd->d5, 1);
    if(BIT_AT(bits, 6))
        lcd->gpioWrite(lcd->d6, 1);
    if(BIT_AT(bits, 7))
        lcd->gpioWrite(lcd->d7, 1);


    lcd4bitFlushEnablePin(lcd);

    //low bits
    lcd->gpioWrite(lcd->d4, 0);
    lcd->gpioWrite(lcd->d5, 0);
    lcd->gpioWrite(lcd->d6, 0);
    lcd->gpioWrite(lcd->d7, 0);

    if(BIT_AT(bits, 0))
        lcd->gpioWrite(lcd->d4, 1);
    if(BIT_AT(bits, 1))
        lcd->gpioWrite(lcd->d5, 1);
    if(BIT_AT(bits, 2))
        lcd->gpioWrite(lcd->d6, 1);
    if(BIT_AT(bits, 3))
        lcd->gpioWrite(lcd->d7, 1);

    lcd4bitFlushEnablePin(lcd);
}


void lcd4bitInit(const struct lcd4bit *lcd)
{
    _lcd4bitWrite(lcd, 0x33, LCD_CMD); //initialize
    _lcd4bitWrite(lcd, 0x32, LCD_CMD); //set to 4 bit -mode
    _lcd4bitWrite(lcd, 0x06, LCD_CMD); //move cursor
    _lcd4bitWrite(lcd, 0x0C, LCD_CMD); // Turn cursor off
    _lcd4bitWrite(lcd, 0x28, LCD_CMD); //2 line display
    lcd4bitClearScreen(lcd);
    sleepForProcessing();

}

void lcd4bitText(const struct lcd4bit *lcd, const char mesg[], int line)
{
    _lcd4bitWrite(lcd, line, LCD_CMD);

    char c;
    for(int i = 0, nullChrHit = 0; i < LCD_LINELENGTH; ++i)
    {
        c = mesg[i];
        if(!nullChrHit && c == '\0')
            nullChrHit = 1;
        _lcd4bitWrite(lcd, nullChrHit ? ' ' : c, LCD_CHR);
    }
}