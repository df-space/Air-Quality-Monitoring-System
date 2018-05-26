/* NodeMCU program that receives the JSON gas readings from Arduino and then 
 *  stores it on Firebase database on the cloud
 */

#include <Firebase.h>
#include <FirebaseArduino.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseError.h>
#include <FirebaseHttpClient.h>
#include <FirebaseObject.h>

#include <MyWifi.h>
#include <Declare.h>

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>


MyWifi MyWifi;
WiFiClient client;

static const int RXPin = 4, TXPin = 5;   // GPIO 4=D2(conneect Tx of Uno) and GPIO 5=D1(Connect Rx of Uno)
static const uint32_t GPSBaud = 9600; //if Baud rate 9600 didn't work in your case then use 4800

int wificnt = 0;
const int capacity = JSON_OBJECT_SIZE(5);
const unsigned long HTTP_TIMEOUT = 10000;   // max respone time from server
 

boolean debug=true;

/* Serrial communication to to NodeMCU */

SoftwareSerial ss(RXPin, TXPin);
/* Get date time from header of an http request */
/* Use the data and time returned in a http response from any url. Google.co.in used */

const char* server = "www.google.co.in";   // an example URL
const int serverPort = 80;      // a port number
const char* resource = "/";         // http resource

/* code from https://thisisyetanotherblog.wordpress.com/2016/08/19/esp8266-get-date-and-time-from-http-header/ */

/* Declaration for getting the data and time */
bool findDateTimeInResponseHeadersByLine();
String extractDayFromDateTimeString(String dateTime);
String extractMonthFromDateTimeString(String dateTime);
String extractYearFromDateTimeString(String dateTime);
String extractHourFromDateTimeString(String dateTime);
String extractMinuteFromDateTimeString(String dateTime);
String translateMonth(String monthStr);
String dateAndTime; // stores date and time from HTTP response header
 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println();
  Serial.println();
  Serial.print("Your are connecting to;");
  Serial.println(MyWifi.ssid());
      
  WiFi.begin(MyWifi.ssid(), MyWifi.passcode());

 /* Wait for 25 before checking connections */
      
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        wificnt++;
        if (wificnt>=25) break;
      }

  if(WiFi.status() == WL_CONNECTED){
         Serial.println("");
         Serial.println("Your ESP is connected!");  
         Serial.println("Your IP address is: ");
         Serial.println(WiFi.localIP());  
      }
      else{
        Serial.println("");
        Serial.println("WiFi not connected");
      }    

/* Connect to Firebase Database */
  Serial.print(FIREBASE_HOST);
  Serial.print(FIREBASE_AUTH);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);    

  /* Begin Serial Communication. Loop till communication enabled */
  Serial.write("In ESP");
  ss.begin(GPSBaud);
  while (!ss){
    ;
  
  }

  if( debug ) Serial.println("loop: get date and time");

 /* Get the date and time info by making a http request to google.co.in 
  *  Look for the date line in the reponse object
  *  Store and display the date & time */
      if( connect(server, serverPort) ) {
        if( sendRequest(server, resource) ) {
          if( findDateTimeInResponseHeadersByLine() ) {
            if( debug ) {
              Serial.print( "Date and Time from HTTP response header: " );
              Serial.println( dateAndTime.c_str() );
            }
            Serial.println(showDateAndTime());
          }
        }
        disconnect();
      }
}

void loop() {
  // put your main code here, to run repeatedly:

  
  if (ss.available() > 0) 
  {
    /* Ideally an RTC will give the exact time. But we are using a workaround
     *  here by getting the date & time when the device is started and looping
     *  to manually count the seconds and update the time. Will work for 300 days 
     *  before a reset is required */
     String dateTimeNow = updateDateTime();

    /* Create a JSON dynamic object to store the gas readings from the sensor */
    
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject&  root= jsonBuffer.parseObject(ss);
    const char* d = root["GasType"];
    Serial.print(d);
    if (root == JsonObject::invalid())
    {
      Serial.println("Invalid Object");
        return;
    }
       

  Serial.println("JSON received");
  uint8_t i = 0;
  for (JsonObject::iterator it = root.begin(); it != root.end() ; ++it, i++)
  {
    
    Serial.println(it->value.asString());
  }
  Serial.println();
   String FireBasePath = FIREBASE_BASE_PATH;
   FireBasePath += dateTimeNow;
   Firebase.set(FireBasePath, root);
        // handle error
        if (Firebase.failed()) {
            Serial.print("Firebase Pushing "+ FireBasePath + " failed:");
            Serial.println(Firebase.error()); 
        }
        else {
          Serial.print("Firebase Pushed /sensor/ ");
        }
   String FireBasePathCurr = FIREBASE_BASE_PATH;
   FireBasePathCurr += "Current";
   Firebase.set(FireBasePathCurr, root);
        // handle error
        if (Firebase.failed()) {
            Serial.print("Firebase Pushing "+ FireBasePath + " failed:");
            Serial.println(Firebase.error()); 
        }
        else {
          Serial.print("Firebase Pushed /sensor/ ");
        }     
  }
//Serial.write("In loop");
}

// Open connection to the HTTP server
bool connect(const char* hostName, const int port) {
  if( debug ) {
    Serial.print("Connect to ");
    Serial.println(hostName);
  }
  bool ok = client.connect(hostName, port);
  if( debug ) Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to any url. The header info contains the date and time

bool sendRequest(const char* host, const char* resource) {
      if( debug ) {
         Serial.print("GET ");
         Serial.println(resource);
     }
     client.print("GET ");
     client.print(resource);
     client.println(" HTTP/1.1\r\n");
     client.print("Host: ");
     client.print(host);
     client.print("\r\n");
    // client.println("Accept: */*");
     client.print("Connection: close\r\n");
 //    client.println();
     return true;
}


/* Get the date and time line in the response from the url */

bool findDateTimeInResponseHeadersByLine(){
    Serial.println("[Response:]");
    while (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        int ds = line.indexOf("Date: ");
       // Serial.print(ds);
        if (ds > -1)
        {
          int de = line.indexOf("GMT");
          dateAndTime = line.substring(ds+6, de);
          Serial.println(  );
          Serial.print("date and time: ");
          Serial.println(dateAndTime.c_str());
          break;
        }
    
       }
      }
     client.stop();
    Serial.println("\n[Disconnected]"); 
    return dateAndTime.length()>15;
  }
    
    




// Close the connection with the HTTP server
void disconnect() {
  if( debug ) Serial.println("Disconnect from HTTP server");
  client.stop();
}
 
//-------- time+date code ----------
// example: Sun, 29 May 2018 10:00:14
String extractDayFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ');
  String dayStr = dateTime.substring(firstSpace+1, firstSpace+3);
  if( debug ) {
    Serial.print("Day: ");
    Serial.println(dayStr.c_str());
  }
  return dayStr;
}
 
String extractMonthFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ', 7);
  String monthStr = dateTime.substring(firstSpace+1, firstSpace+4);
  if( debug ) {
    Serial.print("Month: ");
    Serial.println(monthStr.c_str());
  }
  return monthStr;
}
 
String extractYearFromDateTimeString(String dateTime) {
  uint8_t firstSpace = dateTime.indexOf(' ', 10);
  String yearStr = dateTime.substring(firstSpace+1, firstSpace+5);
  if( debug ) {
    Serial.print("Year: ");
    Serial.println(yearStr.c_str());
  }
  return yearStr;
}
 
String extractHourFromDateTimeString(String dateTime) {
  uint8_t firstColon = dateTime.indexOf(':');
  String hourStr = dateTime.substring(firstColon, firstColon-2);
  if( debug ) {
    Serial.print("Hour (GMT): ");
    Serial.println(hourStr.c_str());
  }
  // adjust GMT time
  int h = hourStr.toInt();
  
  //h += 2; // summertime
  //h += 1; // wintertime
  if( debug ) {
    Serial.print("Hour (adjusted for summertime): ");
    Serial.println(h);
  }
  return String(h);
}
 
String extractMinuteFromDateTimeString(String dateTime) {
  uint8_t secondColon = dateTime.lastIndexOf(':');
  String minuteStr = dateTime.substring(secondColon, secondColon-2);
  if( debug ) {
    Serial.print("Minute: ");
    Serial.println(minuteStr.c_str());
  }
  return minuteStr;
}
 
String extractDayFromCalendarDate(String date) {
  String dateStr = String(date);
  uint8_t firstDot = dateStr.indexOf('.');
  String dayStr = dateStr.substring(1, firstDot);
  if( debug ) {
    Serial.print("Day: ");
    Serial.println(dayStr.c_str());
  }
  return dayStr;
}
 
String translateMonth(String monthStr) {
  if(monthStr.equals(String("Jan"))) return String("01");
  if(monthStr.equals(String("Feb"))) return String("02");
  if(monthStr.equals(String("Mar"))) return String("03");
  if(monthStr.equals(String("Apr"))) return String("04");
  if(monthStr.equals(String("May"))) return String("05");
  if(monthStr.equals(String("Jun"))) return String("06");
  if(monthStr.equals(String("Jul"))) return String("07");
  if(monthStr.equals(String("Aug"))) return String("08");
  if(monthStr.equals(String("Sep"))) return String("09");
  if(monthStr.equals(String("Oct"))) return String("10");
  if(monthStr.equals(String("Nov"))) return String("11");
  if(monthStr.equals(String("Dec"))) return String("12");
}

String showDateAndTime() {
  
  String date = extractDayFromDateTimeString(dateAndTime);
  date += ".";
  date += translateMonth(extractMonthFromDateTimeString(dateAndTime));
  date += ".";
  date += extractYearFromDateTimeString(dateAndTime);
  
  String timeStr = extractHourFromDateTimeString(dateAndTime);
  timeStr += ":";
  timeStr += extractMinuteFromDateTimeString(dateAndTime);
  String dateTimeStr = "";
  dateTimeStr += date;
  dateTimeStr += " ";
  dateTimeStr += timeStr;
  return dateTimeStr;
}

String updateDateTime() {
  Serial.println("In UpdateTime");
    long day = 86400000; // 86400000 milliseconds in a day
    long hour = 3600000; // 3600000 milliseconds in an hour
    long minute = 60000; // 60000 milliseconds in a minute
    long second =  1000; // 1000 milliseconds in a second
    int adder = 0;
    unsigned long currentMillis = millis();
    Serial.print(currentMillis);

    int updMinutes = extractMinuteFromDateTimeString(dateAndTime).toInt() + (((currentMillis % day) % hour) / minute) ;  
    if (updMinutes >= 60 )
    {
      updMinutes -= 60;
      adder = 1;
    }

     int updHours = extractHourFromDateTimeString(dateAndTime).toInt() + adder + ((currentMillis % day) / hour);                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
     adder = 0;
     while (updHours >= 24) 
     {
       updHours -= 24;
       adder += 1;
     }
    
    
    int noofdays = currentMillis/day;
    int updDay = extractDayFromDateTimeString(dateAndTime).toInt();
    int updDate = updDay + (noofdays % 30) + adder;
    adder = 0;
    while (updDate > 30)
    {
      updDate =- 30;
      adder += 1;
    }
    
    int mnth = translateMonth(extractMonthFromDateTimeString(dateAndTime)).toInt();
    Serial.print("Mnth "+ mnth);
    int updMnth = mnth + (noofdays/30) + adder ;
    
    adder = 0;
    while (updMnth > 12)
    {
      updMnth -= 12;
      adder += 1;
    }

     int updYear = extractYearFromDateTimeString(dateAndTime).toInt() + adder;
   
   
    String currDate = String(updDay); 
    //currDate += ".";
    currDate += String(updMnth);
    //currDate += ".";
    currDate += String(updYear);;
  
    String updtimeStr = String(updHours);
    //updtimeStr += ":";
    updtimeStr += String(updMinutes);
    String updDateTimeStr = "";
    updDateTimeStr += currDate;
    updDateTimeStr += "-";
    updDateTimeStr += updtimeStr;
    return updDateTimeStr;

}


