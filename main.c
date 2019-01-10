#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

//http://wiringpi.com/download-and-install/
//compile with -lwiringPi
#include <wiringPi.h>


//sudo apt install libcurl3-dev
//compile with -lcurl
#include <curl/curl.h>

#include "lcdScreen.h"

#include "mjson.h"

#define LIGHT_RED 26
#define LIGHT_YELLOW 1
#define LIGHT_GREEN 27

#define IP_ADDR_LENGTH 15
#define PUBLC_IP_URL "http://checkip.amazonaws.com"

#define TRANSISTOR 21

 static const struct lcd4bit lcd = 
{
    6, //rs
    5, //e
    4, //d4
    0, //d5
    29, //d6
    3, //d7
    digitalWrite //gioWrite function
};

#define BUTTON 25

//neccesary becuase of to emf: the button input picks up phantom inputs on that long wire,
//the 'click threshold' is min time ellapsed of an actual click (as opposed to a phantom one)
#define CLICK_THRESHOLD 50000

#define USECS(seconds)  seconds * 1000000

static void lcdOn()
{
    pinMode(lcd.rs, OUTPUT);
    pinMode(lcd.e, OUTPUT);
    pinMode(lcd.d4, OUTPUT);
    pinMode(lcd.d5, OUTPUT);
    pinMode(lcd.d6, OUTPUT);
    pinMode(lcd.d7, OUTPUT);

    lcd4bitInit(&lcd);
    digitalWrite(TRANSISTOR, 1);
}

static void lcdOff()
{
    digitalWrite(TRANSISTOR, 0);

    pinMode(lcd.rs, INPUT);
    pinMode(lcd.e, INPUT);
    pinMode(lcd.d4, INPUT);
    pinMode(lcd.d5, INPUT);
    pinMode(lcd.d6, INPUT);
    pinMode(lcd.d7, INPUT);
}



//normally we would do a realloc here, but since we know the webpage is only 15 chars max,
//we can just alloce a 15 char array beforehand
static size_t webIpWriteCb(char *ptr, size_t size, size_t nmemb, char *ipaddr)
{
    size_t writeSize = size * nmemb;

    char * startIndex;
    for(startIndex = ipaddr;
        *startIndex != '\0';
        ++startIndex);


        
    //TODO delete newline character from the ptr string
    memcpy(startIndex, ptr, writeSize);

    startIndex[writeSize] = '\0';

    return writeSize;
}


static void getPublicIp(char * buffer)
{
    CURL * curl;

    buffer[0] = '\0';

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, PUBLC_IP_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, webIpWriteCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",  
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }

}

static void buttonCallback()
{
    digitalWrite(LIGHT_RED, 1);

    lcdOn();

    usleep(500);

    lcd4bitText(&lcd, "hello world", LCD_LINE_1);

    char ip [IP_ADDR_LENGTH + 1];
    getPublicIp(ip);
    lcd4bitText(&lcd, ip, LCD_LINE_2);

    printf("\"%s\"", ip);

    //mjson_get_string()

    usleep(USECS(4.5));
    
    //clearDisplay(&lcd);    

    digitalWrite(LIGHT_RED, 0);


    lcdOff();
}

static volatile int running = 1;
void endProgram(int dummy)
{
    running = 0;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, endProgram);

    wiringPiSetup();

    pinMode(LIGHT_RED, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(TRANSISTOR, OUTPUT);
    

    int prevState = 0;
    int currentState;
    clock_t t;
    while(running)
    {
       currentState = digitalRead(BUTTON);
       if(prevState == 0 && currentState == 1)
       {
           t = clock();
           prevState = 1;
       }
       else if(prevState == 1 && currentState == 0)
       {
           prevState = 0;
           if(clock() - t >= CLICK_THRESHOLD)
                buttonCallback(); //click registered
       }
    }


    lcdOff();


    return 0;
}