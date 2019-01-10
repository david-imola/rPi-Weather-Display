
#ifndef GPIO_H_
#define GPIO_H_


int __writeIntToFile(int, const char[]);
#define gpioExport(pin)    __writeIntToFile(pin, "/sys/class/gpio/export");
#define gpioUnexport(pin)  __writeIntToFile(pin, "/sys/class/gpio/unexport");

#define GPIO_IN 0
#define GPIO_OUT 1
int gpioDirection(int pin, int dir);

#define GPIO_LOW 0
#define GPIO_HIGH 1
int gpioRead(int pin);
int gpioWrite(int pin, int value);

#endif