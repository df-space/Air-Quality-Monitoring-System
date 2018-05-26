#ifndef MyWifi_h
#define MyWifi_h

#include "Arduino.h"

class MyWifi{
  public:
    MyWifi();
    ~MyWifi();
    char* ssid();
    char* passcode();
  private:
    char* _ssid;
    char* _pwd;
};

#endif