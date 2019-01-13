#ifndef LCDSCREEN_H_
#define LCDSCREEN_H_

#define LCD_LINE_1  0x80
#define LCD_LINE_2  0xC0

#define LCD_LINELENGTH 16

#define LCD_CHR 1
#define LCD_CMD 0
#define LCD_LINE_1 0x80
#define LCD_LINE_2 0xC0

#define lcd4bitClearScreen(lcd_ptr)  _lcd4bitWrite(lcd_ptr, 0x01, LCD_CMD);

struct lcd4bit
{
    int rs, e, d4, d5, d6, d7;
    void (*gpioWrite) (int pin, int val);
};

void _lcd4bitWrite(const struct lcd4bit *, int, int);

void lcd4bitInit(const struct lcd4bit * lcd);
void lcd4bitText(const struct lcd4bit * lcd, const char mesg[], int line);

#endif