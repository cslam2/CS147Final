// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>


// This #include statement was automatically added by the Particle IDE.
#include <Grove_LCD_RGB_Backlight.h>

// This #include statement was automatically added by the Particle IDE.
#include "RHT03.h"
#include "application.h"


//pins for connections
#define THPIN D2
#define BUTTONPIN A0
#define LIGHTPIN D4

//state machine states 
#define TIMESTATE 0
#define TEMPHSTATE 1
#define LIGHTSTATE 2
#define WEATHERSTATE 3
#define SERVERSTATE 4


//initialize components and state machine
RHT03 RHT;
rgb_lcd LCD;
int state = TIMESTATE;
// variables for sensor readings
float temp;
float humidity;
float light;
int buttonState = LOW;
//variables for time
int hour;
int minute;
int second;
char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char weekday[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

//http headers
HttpClient http;// Headers currently need to be set at init, usefulfor API keys etc.
http_header_t headers[] = { // { "Content-Type", "application/json" },
                            //  { "Accept" , "application/json" },
                            { "Accept" , "*/*"},
                            { NULL, NULL } // NOTE: Always terminate headerswill NULL
                            };
http_request_t request;
http_response_t response;

void setup() {
//initialize time variables
Time.zone(-8);
Time.beginDST();
Serial.begin(9600);
//set pins
pinMode(LIGHTPIN, INPUT);
pinMode(BUTTONPIN, INPUT);
//establish ip to cloud server
request.hostname = "54.164.113.194";
request.port = 5000;
RHT.begin(THPIN);
LCD.begin(16,2);
LCD.setColor(GREEN);
LCD.print("WeatherMan");
LCD.setCursor(0,1);
LCD.print("Booting up");
delay(5000);
}

void changeState() {
//changes the state machine to the next state and loops back to the first state after last state 
    if (state == TIMESTATE)
    {
        state = TEMPHSTATE;
    }
    else if (state == TEMPHSTATE)
    {
        state = LIGHTSTATE;
    }
    else if(state == LIGHTSTATE)
    {
        state = WEATHERSTATE;
    }
    else if(state == WEATHERSTATE)
    {
        state = SERVERSTATE;
    }
    else if(state == SERVERSTATE)
    {
        state = TIMESTATE;
    }
}

void printTime() {
    //prints time to LCD
    LCD.clear();
    LCD.print("Time: ");
    if(hour < 10)
    {
        LCD.print("0");
    }
    LCD.print(hour);
    LCD.print(":");
    if(minute < 10)
    {
        LCD.print("0");
    }
    LCD.print(minute);
    LCD.print(":");
    if(second < 10)
    {
        LCD.print("0");
    }
    LCD.print(second);
    if(Time.isAM())
    {
        LCD.print("AM");
    }
    else
    {
        LCD.print("PM");
    }
    LCD.setCursor(0,1);
    LCD.print(weekday[Time.weekday()-1]);
    LCD.print(" ");
    LCD.print(months[Time.month()-1]);
    LCD.print(" ");
    LCD.print(Time.day());
    LCD.print(" ");
    LCD.print(Time.year());
}

void loop() {
int t = Time.now();
hour = Time.hourFormat12(t);
minute = Time.minute(t);
second = Time.second(t);
RHT.update();
temp = RHT.tempF();
humidity = RHT.humidity();
light = digitalRead(LIGHTPIN);
request.path = "/get-data?hum="+String(humidity)+"&temp="+String(temp);

switch(state)
{
    case(TIMESTATE):
        printTime();
        delay(1000);
        break;
    case(TEMPHSTATE):
        LCD.clear();
        LCD.print("Temp: ");
        LCD.print(int(round(temp)));
        LCD.print("F");
        LCD.setCursor(0,1);
        LCD.print("Humidity: ");
        LCD.print(int(round(humidity)));
        LCD.print("%");
        delay(1000);
        break;
    case(LIGHTSTATE):
        LCD.clear();
        LCD.print("Light: ");
        LCD.print(light);
        delay(1000);
        break;
    case(SERVERSTATE):
        LCD.clear();
        LCD.print("Sending Data...");
        http.get(request, response, headers);
        break;
}
buttonState = digitalRead(BUTTONPIN);
if(buttonState == HIGH)
    {
        Serial.println("state change");
        changeState();
    }
}