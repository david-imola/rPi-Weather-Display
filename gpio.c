#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpio.h"



//since it's same function, we can just 
//static int GPIOImportExport


#define BUFFER_MAX 3
int __writeIntToFile(int val, const char filename[])
{
    int f_id = open(filename, O_WRONLY);
    if (f_id == -1)
    {
        perror("Exporting/Unexporting file failed");
        return -1;  //file open failed
    }

    char buffer[BUFFER_MAX];
    ssize_t bytesWritten = snprintf(buffer, BUFFER_MAX, "%d", val);
    write(f_id, buffer, bytesWritten);

    close(f_id);
    return 0;
}

#define VALUE_MAX  30
#define VALUE_PATH(path, pin)    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin)
int gpioRead(int pin)
{
    char path[VALUE_MAX];
    VALUE_PATH(path, pin);
    int f_id = open(path, O_RDONLY);
    if(f_id == -1)
    {
        perror("Opening file for gpio read failed");
        return -1;
    }

    char value[BUFFER_MAX];
    if( -1 == read(f_id, value, BUFFER_MAX))
    {
        perror("reading gpio value failed");
        return -1;
    }

    close(f_id);
    return atoi(value);
}


int gpioWrite(int pin, int value)
{
    char path[VALUE_MAX];
    VALUE_PATH(path, pin);
    int f_id = open(path, O_WRONLY);
    if(f_id == -1)
    {
        perror("opening file for gpio write failed");
        return -1;
    }

    

    if (1 != write(f_id, value == GPIO_LOW ? "0" : "1", 1))
    {
        perror("writing to gpio output failed");
        return -1;
    }

    close(f_id);
    return 0;
}


#define DIRECTION_MAX 35
#define DIRECTION_PATH(path, pin)    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin)
int gpioDirection(int pin, int direct)
{
    char path[DIRECTION_MAX];
    DIRECTION_PATH(path, pin);
    int f_id = open(path, O_WRONLY);
    if (f_id == -1)
    {
        perror("opening direction file failed");
        return -1;  //file open failed
    }

    size_t directions_len = direct == GPIO_IN ? 2 : 3;
    
    if(-1 == write(f_id, direct == GPIO_IN ? "in" : "out", directions_len))
    {
        perror("writing to direction file failed");
        return -1;
    }

    close(f_id);
    return 0;
}

