
/* This program reads the array of gas sensors connected to the arduino and loads it into a 
 *  JSON variable to transmit serially to NodeMCU.
 * 
 */
#include <ArduinoJson.h>


#include <SoftwareSerial.h>

#define   MQ_SENSOR_ANALOG_PIN         A0  //define which analog input channel you are going to use

float Ro = 41763.0;    // this has to be tuned 10K Ohm
#define         RL_VALUE                     5300     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          9.83  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
//#define         RO_CLEAN_AIR_FACTOR          9.83
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     50    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  500   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         50    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            5     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
#define         GAS_LPG                      0
#define         GAS_CO                       1
#define         GAS_PM10                     2
#define         GAS_CH4                      3
#define         GAS_NH4                      4
String         GAS_ENUM[5]=                {"LPG", "CO", "PM10", "CH4", "NH4"};

unsigned long SLEEP_TIME = 50000; // Sleep time between reads (in milliseconds)

int val = 0;           // variable to store the value coming from the sensor
float valMQ =0.0;
float lastMQ =0.0;

/* Values derived from exponential regression of respective gas datapoints from the datasheet. 
 *  The values represent the a & b values in the a*x^b
 */
float           LPGCurve[2]  =  {632.8357,-2.025202};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 

float           COCurve[2] = {116.6020682, -2.769034857};                                                    
float           PM10Curve[2] ={3896.4,-1.886306};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2:(lg10000,-0.22)          

float           CH4Curve[2]    = {4084.538, -2.410099};

float           NH4Curve[2]    = {102.2348, -2.554241};
                                                                                            

#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);
SoftwareSerial sw(2, 3);

void setup() {

  Serial.begin(9600);
  //pinMode(output,OUTPUT);
  Serial.println("Start Detecting");
  //digitalWrite(output,LOW);
  // put your setup code here, to run once:
  delay(5000);
   Ro = MQCalibration(MQ_SENSOR_ANALOG_PIN);         //Calibrating the sensor. Please make sure the sensor is in clean air 
   Serial.print(Ro);                                                 //when you perform the calibration  
   lcd.begin(16, 2);
   lcd.setCursor(0,1);
   sw.begin(9600);
}

void loop() {
    // put your main code here, to run repeatedly:
  /* Create a json object */
  DynamicJsonBuffer  jsonBuffer(200);
  JsonObject& root = jsonBuffer.createObject();
  //root["location"] = "IISC";
  //JsonArray& AQParam = root.createNestedArray("AQ");
  
  Serial.print("Ro value is");
  Serial.print(Ro);
   uint16_t valMQ = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_CO);
   uint16_t ppmVal = 0;
    //Serial.println(val);
  
     ppmVal = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_LPG);
     printSerialMon(ppmVal, 0);
     printLCD(ppmVal, 0);
  
    // JsonObject& AQLPG = AQParam.createNestedObject();
    JsonObject& AQLPG = root.createNestedObject(GAS_ENUM[0]);
  
   AQLPG["Value"] = ppmVal;
   AQLPG["Unit"] = "ppm";
   
   Serial.print("    ");   
   lcd.print(" ");

   ppmVal = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_CO);
   printSerialMon(ppmVal, 1);
   printLCD(ppmVal, 1);
   JsonObject& AQCO = root.createNestedObject(GAS_ENUM[1]);
   AQCO["Value"] = ppmVal;
   AQCO["Unit"] = "ppm";
   Serial.print("    ");   
   lcd.print(" ");
   
   ppmVal = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_PM10);
   printSerialMon(ppmVal, 2);
   printLCD(ppmVal, 2);
   JsonObject& AQPM10 = root.createNestedObject(GAS_ENUM[2]);
   AQPM10["Value"] = ppmVal;
   AQPM10["Unit"] = "ppm";
   Serial.print("\n");

   ppmVal = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_CH4);
   printSerialMon(ppmVal, 3);
   printLCD(ppmVal, 3);
   JsonObject& AQCH4 = root.createNestedObject(GAS_ENUM[3]);
   AQCH4["Value"] = ppmVal;
   AQCH4["Unit"] = "ppm";
   Serial.print("\n");

   ppmVal = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_NH4);
   printSerialMon(ppmVal, 4);
   printLCD(ppmVal, 4);
   JsonObject& AQNH4 = root.createNestedObject(GAS_ENUM[4]);
   AQNH4["Value"] = ppmVal;
   AQNH4["Unit"] = "ppm";
   Serial.print("\n");
     
  if (valMQ != lastMQ) {
      //gw.send(msg.set((int)ceil(valMQ)));
      //Serial.print((int)ceil(valMQ));
      lastMQ = ceil(valMQ);
  }
  root.printTo(sw);
  root.prettyPrintTo(Serial);
  delay(SLEEP_TIME); //sleep for: sleepTime 
}

/* Print Gas reading on Serial Monitor */
void printSerialMon(uint16_t ppmVal, int param)
{
   Serial.print(GAS_ENUM[param]); 
   Serial.print(":");
   Serial.print(ppmVal );
   Serial.print( "ppm" );
}

/* Show reading on LCD display */
void printLCD(uint16_t ppmVal, int param)
{
   lcd.print(GAS_ENUM[param]); 
   lcd.print(ppmVal );
   lcd.print( "ppm" );
}


/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  Serial.print("\n RS value is ");
  Serial.print(raw_adc);
  return ( ((float)RL_VALUE*(1024-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
  Serial.println("Calibrated Value is ");
  Serial.print(val);
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
  
  return val; 
}

/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    //Serial.print(analogRead(mq_pin));
    //Serial.print("");
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;

  
  return rs;  
}
/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_PM10 ) {
     return MQGetPercentage(rs_ro_ratio,PM10Curve);
  } else if ( gas_id == GAS_CH4 ) {
     return MQGetPercentage(rs_ro_ratio,CH4Curve); 
  } else if ( gas_id == GAS_NH4 ) {
     return MQGetPercentage(rs_ro_ratio,NH4Curve);
  }
     
 
  return 0;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: USing the a * b coeffients in the equation a*x^b the ppm value is derived
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  Serial.println("RS_RO Ratio is");
  Serial.println(rs_ro_ratio);
  return pcurve[0] * pow(rs_ro_ratio, pcurve[1]);
 // return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

