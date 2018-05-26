#include "Arduino.h"
#include "MyWifi.h"
#include "Declare.h"

MyWifi::MyWifi(){
    _ssid=WIFI_SSID;
    _pwd=WIFI_PASSWORD;
}
MyWifi::~MyWifi(){
  _ssid="";
  _pwd="";
}
char* MyWifi::ssid(){
  return _ssid;
}

char* MyWifi::passcode(){
  return _pwd;
}