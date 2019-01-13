#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

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

#define WEATHER_URL_FORM "https://api.openweathermap.org/data/2.5/weather?zip=%s&appid=e1554207a4c8d53ce7e5825ffe374e50&units=metric"
#define WEATHER_URL_LEN  112

#define ZIPCODE_FILE "ZIPCODE"
#define ZIPCODE_LEN 5



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

    digitalWrite(TRANSISTOR, 1);

    usleep(500);

    lcd4bitInit(&lcd);
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

struct stringData
{
    size_t usedSize;
    size_t allocSize;
    char * string;
};

//normally we would do a realloc here, but since we know the webpage is only 15 chars max,
//we can just alloce a 15 char array beforehand
static size_t webWriteCb(char *ptr, size_t size, size_t nmemb, struct stringData * pageData)
{
    size_t writeSize = size * nmemb;
    size_t newSize = pageData->usedSize + writeSize;

    if(newSize > pageData->allocSize)
    {
        pageData->string = realloc(pageData->string, newSize);
        pageData->allocSize = newSize; //if this loop is ever entered, the two variables become redundant
    }

    char * startIndex = pageData->string + pageData->usedSize - 1; // -1 for the null char at the end
    memcpy(startIndex, ptr, writeSize);
    startIndex[writeSize] = '\0';
    pageData->usedSize = newSize;

    return writeSize;
}


static char *getWebpageAlloc(size_t initSize, size_t * sizeBuf, const char * url)
{
    CURL * curl;

    struct stringData pageString;
    pageString.string = malloc(sizeof(char) * initSize);
    pageString.string[0] = '\0';
    pageString.usedSize = 1;
    pageString.allocSize = initSize;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, webWriteCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pageString);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",  
                    curl_easy_strerror(res));
            if(sizeBuf)
                *sizeBuf = 0;
            return NULL;
        }
        curl_easy_cleanup(curl);
        if(sizeBuf)
            *sizeBuf = pageString.usedSize - 1; //-1 to not include the null terminator
        return pageString.string;
    }
    else
    {
        if(sizeBuf)
            *sizeBuf = 0;
        return NULL;
    }

}

static void buttonCallback(const char * url)
{
    digitalWrite(LIGHT_RED, 1);

    size_t jsonLen;
    char * weatherJSON = getWebpageAlloc(500, &jsonLen, url);


    double temp = mjson_get_number(weatherJSON, jsonLen, "$.main.temp", 100);
    double humidity = mjson_get_number(weatherJSON, jsonLen, "$.main.humidity", 101);

    char numBuf [LCD_LINELENGTH];
    snprintf(numBuf, LCD_LINELENGTH, "%4.1fC, %3.0f%%H", temp, humidity);

    char descrBuf [LCD_LINELENGTH * 2];
    int descrLen = mjson_get_string(weatherJSON, jsonLen, "$.weather[0].description", descrBuf, sizeof(descrBuf));

    free(weatherJSON);

    digitalWrite(LIGHT_RED, 0);
    lcdOn();
    lcd4bitText(&lcd, numBuf, LCD_LINE_1);

    for(int i =0, unshown = descrLen - LCD_LINELENGTH; i <= unshown || i == 0; ++i)
    {
        lcd4bitText(&lcd, descrBuf + i, LCD_LINE_2);
        usleep(USECS(0.5));
    }

    usleep(USECS(6));


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

    //get zip code
    int f_id = open(ZIPCODE_FILE, O_RDONLY);
    char zip[ZIPCODE_LEN];
    read(f_id, zip, ZIPCODE_LEN);
    close(f_id);
    //create the url
    char urlBuf[WEATHER_URL_LEN];
    sprintf(urlBuf, WEATHER_URL_FORM, zip);

    //set up gpio
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
                buttonCallback(urlBuf); //click registered
       }
    }


    lcdOff();


    return 0;
}