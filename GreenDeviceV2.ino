
// this version contains thingspeak 

#include <FS.h>   
#include <ESP8266WiFiMulti.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>     
#include <EEPROM.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <ESP8266WebServer.h>
#include <SparkFunTSL2561.h> // library for luminosity sensor
#include <SPI.h>
#include "cactus_io_BME280_SPI.h"
#include <WiFiClient.h>
#include <OneWire.h>
#include <RtcDS3231.h>
#include <DallasTemperature.h>  // library for temperature sensor
RtcDS3231<TwoWire> Rtc(Wire);
//#define sleepTime 180
void Searcher();
void Replacer(String *Replacee,int length);
int month,day,year,hour,minute,second;
uint8_t addr=0;
bool shouldSaveConfig;
bool data_reading_flag();
//=================================================
char mqtt_server[106];
String Name(mqtt_server);
String tester,API_KEY;
String API_KEY2;
char deviceName[200]="device CorpH";
char API_KEY_ch[3000];
char sleepTime[105];
void saveConfigCallback () {
      Serial.println("Should save config");
      shouldSaveConfig = true;
}

String apiKey = "HG509EGZ1VR7RE18";    
WiFiClient client;
const char* server = "api.thingspeak.com";

double global_lux_value;
int global_humidity;
int global_pressure;
int global_ambientTemperature;
double global_dew;
float global_vpd;
float global_moisture;
float global_temperature;




void printDateTime(const RtcDateTime& dt);
//=================================================
// assign the SPI bus to pins
#define BME_SCK D0 // Serial Clock
#define BME_MISO D7 // Serial Data Out
#define BME_MOSI D6 //Serial Data In
#define BME_CS D5 // Chip Select  
// Create the BME280 object
BME280_SPI bme(BME_CS,BME_MOSI,BME_MISO,BME_SCK);    // software SPI
long int   pressure,altitude,ambient_temperature,temperature,lux,moisture,humidity;
#define common_power 3
//temp related info
int temperature_pin = 2; // often dosent work if pin is otherthan D3 or D4..........
OneWire oneWire(temperature_pin);
DallasTemperature sensors(&oneWire);
double Celcius = 0.00;
//moisture sensor related info
int moisture_pin = A0;
int reading_no=1;
int moisture_reading;
// luminosity related info
//create object  LUX
SFE_TSL2561 LUX;
double lux_value;
//following three are parameters for function getTiming
//and have these particular datatypes with few predefined values
//which have their already meant purpose                       SDA=> D2.   SCL=> D1
unsigned char timer = 2;
unsigned int ms;
boolean sensitivity;
//username and password for net connection,which esp shall search for
//char  *username = "corph", *password = "corph@786";
//80=http
//ESP8266WebServer server(80);
char filename [] = "datalog.txt";
String page;
void cool();
void sendData(String(lux),String( Temperature),String( moisture),String(ambient_temperature),String(pressure),String(humidity),int month,int day,int year,int hour,int minute,int second) ;
File myDataFile;
void setup() {
    
  // put your setup code here, to run once:
  Serial.begin(115200);
   Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid())
    {
        if (Rtc.LastError() != 0)
        {
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            Serial.println("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  EEPROM.begin(4096);
  
  WiFiManager wifiManager;
  cool();
//  String(API_KEY);
  pinMode(moisture_pin, INPUT);
// ds18b20 must be taken in parasitic mode or pull up resistor should be defined
//observed: it works if none of the  above is done,...but some narrate it gets hot if not done so
  Serial.println("\nConnected to Wifi \nAll info shall be on the website");
  Serial.println("connected to");
//  Serial.println(username);
//  Serial.println("IP ADDRESS for changin api key");
//IP should be noted and followed in browser
//  Serial.println(WiFi.localIP());
  EEPROM.begin(512);
//  char c = EEPROM.read(0);
//       API_KEY = c ;
//  for (byte x = 1; x < EEPROM.read(100); x++ ){
//        c = EEPROM.read(x);
//       API_KEY += c ;
//   }
//   for (byte x = EEPROM.read(100)+6; x < EEPROM.read(110); x++ ){
//       char c = EEPROM.read(x);
//       Name += c ;
//   }
   bool  status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
//        while (1);
    }
  bme.setTempCal(-1);
//  for (uint8_t t = 4; t > 0; t--) {
//    Serial.printf("[SETUP] WAIT %d...\n", t);
//    Serial.flush();
//    delay(1000);
//  }
API_KEY2=API_KEY;
}

int SendAPI_to_Thingspeak(){
    
//    API_to_Send = Replacer(API);

        Serial.println("api to be sent inside sendApi() after replacement");
//        Serial.println(API_to_Send);
    
    
        if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
    {  

      String postStr = apiKey;
      postStr +="&field1=";
      postStr += String(global_ambientTemperature);
      postStr +="&field2=";
      postStr += String(global_humidity);
      postStr +="&field3=";
      postStr += String(global_temperature);
      postStr +="&field4=";
      postStr += String(global_lux_value);
      postStr +="&field5=";
      postStr += String(global_pressure);
      postStr +="&field6=";
      postStr += String(global_moisture);
      
      postStr += "\r\n\r\n";
      
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      
    }
    
    

    
  }

  
void loop(){
   Serial.println("here come the api in loop:: "+API_KEY);
  delay(20);
  // put your main code here, to run repeatedly:
// calling functions to collect data
//  data_reader();
  
  Serial.println("................before delay");
  delay(5000);
  RTc();
  Serial.println("after rtc...................");
  moisture_reading_function();
  temperature_reading_function();
  lux_reading_function();
  BME();
  for(int i =0;i<10;i++){
    Serial.println("sending to things apeak .. ");
    SendAPI_to_Thingspeak();
    delay(9000);
  }
  
//  Searcher();
//  delay(5000);
//  Serial.println("temperatuer = veeeeeey");
//  Serial.println(String(Celcius));
  delay(1000);
//  sendData( String(lux_value), String(Celcius),String(moisture_reading),String(ambient_temperature),String(pressure),String(humidity),month,day,year,hour,minute,second);
//  delay(1000);
//  digitalWrite(common_power,LOW);
// taking a nap to save battries
  
//  delay(1000);
  Serial.print("Off to sleep she goes");
  ESP.deepSleep(atoi(sleepTime)*60*1000000);
}
//functions for particular sensor
void moisture_reading_function() {
  
  delay(10);
  
  moisture_reading = analogRead(moisture_pin);
//changing voltage (data) form sensor to percentage
  moisture_reading = map(moisture_reading, 1024, 0, 0, 100);
  Serial.println("moisture");
  Serial.println(moisture_reading);
  moisture=moisture_reading;
  
  global_moisture = moisture;
}
void temperature_reading_function() {
  delay(10);
  sensors.begin();
  sensors.requestTemperatures();
  Celcius = sensors.getTempCByIndex(0);
  Serial.println("Celcius");
  Serial.println(Celcius);
  temperature=Celcius;
  
  global_temperature = temperature;
}
void BME(){
    Serial.println("reading bme sensor");
    bme.readSensor();
    Serial.println("done reading.........................");
    delay(10);
    
    Serial.print(bme.getPressure_MB()); Serial.print("\t\t");    // Pressure in millibars
    Serial.print(bme.getHumidity()); Serial.print("\t\t");
    Serial.print(bme.getTemperature_C()); Serial.print(" *C\t");
    Serial.print(bme.getTemperature_F()); Serial.println(" *F\t");
    humidity=bme.getHumidity();
    pressure=bme.getPressure_MB();
    ambient_temperature=bme.getTemperature_C();   
  
  global_humidity = humidity;
  global_pressure = pressure;
  global_ambientTemperature = ambient_temperature;
}
void lux_reading_function() {
  delay(10);
  LUX.begin();
  LUX.setTiming(sensitivity, timer, ms);
  LUX.setPowerUp();
  unsigned int data1, data2;
  LUX.getData(data1, data2);
  // data1 and data2 corresponds to data of lingt and infrared rays,
  //passing them to following function, unaltered, gives luminosity
  LUX.getLux(sensitivity, ms, data1, data2, lux_value);   // this function is of boolean type
  
  Serial.println("lux_value");
  Serial.println(lux_value);
  lux=lux_value;
  
  global_lux_value = lux;
}
void sendData(String(lux),String(Temperature),String( moisture),String(ambient_temperature),String(pressure),String(humidity),int month,int day,int year,int hour,int minute,int second) {  
        Serial.print("sending.....................\n");
        HTTPClient http;
        Searcher();
        
//       Serial.println("temperatuer eeeeeeeeeeeeeeeeeeeeeeeey");
//  Serial.println(temperature);   
//    Serial.print("lux  .................................................................................................................................................  "+lux);
    
    Serial.println(API_KEY);
//    String API_KEYy;
//    API_KEYy=API_KEY.charAt(0);
//    for(int a=1;a<API_KEY.length()-2;a++){
//      API_KEYy+=API_KEY.charAt(a);
//    }
//    API_KEY=API_KEYy;
//    Serial.print("[HTTP] begin...\n");
    Serial.println("before  http------");
//    Serial.println(API_KEYy);
    Serial.println(API_KEY);
//    Serial.println(API_KEY2);
    yoo:{   http.begin(API_KEY);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      if (atoi(http.errorToString(httpCode).c_str())!=200) goto yoo;
    }
    http.end();
   }
  }
void handleWifiSave() {
//    for (byte x = 0; x <API_KEY.length()-2; x++ ){//<<<<<<<<<<<<<<---------------------------------here it is
//        EEPROM.write(x,API_KEY.charAt(x));  // stores ascii of char , one by one
//    }
//   
//  EEPROM.write(100,API_KEY.length()-2);   // stores string len
////   Serial.println("in fxnlength=");
////   Serial.println(API_KEY.length());
//  EEPROM.commit();
//     for (byte x = API_KEY.length()+6;x < sizeof(mqtt_server)+API_KEY.length()+1; x++ ){
//        EEPROM.write(x,mqtt_server[x]);  // stores ascii of char , one by one
//    }
//  EEPROM.write(110,sizeof(mqtt_server));   // stores string len
//  EEPROM.commit();
}
void cool(){
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json","r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(deviceName, json["deviceName"]);
          strcpy(API_KEY_ch, json["API_KEY"]);
          strcpy(sleepTime, json["sleepTime"]);
//          String(API_KEY);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  String data2;
  data2="<b>Data from sensors :</b>Pressure: "+String(pressure)+"Temperature : "+String(temperature)+"Ambient Temperature : "+String(ambient_temperature)+"Lux : "+String(lux)+"Moisture : "+String(moisture)+"Humidity : "+String(humidity)+"";
  char data3[data2.length()+2];
  data3[0]=data2.charAt(0);
  
  for(int k=1;k<=data2.length();k++){
    data3[k]=data2.charAt(k);
  }
  char* data4;
  data4=data3;
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_deviceName("device name", "device name", deviceName, 20);
  WiFiManagerParameter custom_API_KEY(" API_KEY", " API_KEY", API_KEY_ch, 2000);
  WiFiManagerParameter custom_sleepTime("sleepTime", "sleepTime",sleepTime, 15);
  WiFiManagerParameter custom_text(data4);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  //set static ip
  //add all your parameters here
  wifiManager.addParameter(&custom_deviceName);
  wifiManager.addParameter(&custom_API_KEY);
  wifiManager.addParameter(&custom_sleepTime);
  wifiManager.addParameter(&custom_text);
  wifiManager.setTimeout(100);
  
  if (!wifiManager.autoConnect(deviceName, "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(30);
    //reset and try again, or maybe put it to deep sleep
    data_saver();
    delay(50);
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //read updated parameters
  strcpy(deviceName, custom_deviceName.getValue());
  strcpy(API_KEY_ch, custom_API_KEY.getValue());
  strcpy(sleepTime, custom_sleepTime.getValue());
//  sabi= sabi1.getValue();
  //save the custom parameters to FS
  if (shouldSaveConfig){
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["deviceName"] = deviceName;
    json["API_KEY"] = API_KEY_ch;
    json["sleepTime"] =sleepTime;
    
    File configFile = SPIFFS.open("/config.json","w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  API_KEY=String(API_KEY_ch);
  Serial.println("no string attached"+API_KEY);
  Searcher();
//  handleWifiSave();
Serial.println("coddddddddddddddddected...yeey :)");
}        
void Replacer(String *Replacee,int length){
  int b=0;
  String API_KEY2;
  for(int a=0;a<=length;a++){
    if(API_KEY.charAt(a)=='%'){
      a++;
      while(API_KEY.charAt(a)!='%'){
        a++;
      }
      a++;
      API_KEY2+=Replacee[b];
  Serial.println("replacee[]=  ");
  Serial.println(Replacee[b]);
      b++;
    }
    API_KEY2+=API_KEY.charAt(a);
  }
//   Serial.println("--------------------------$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$----------------------------------------");
// Serial.println(API_KEY2);
  API_KEY=API_KEY2.charAt(0);
  for(int a=1;a<=API_KEY2.length()-2;a++){
    API_KEY+=API_KEY2.charAt(a);
//    Serial.println(API_KEY);
//    delay(500);
  }
Serial.println("--------------------------------------------------------------------------------------------------------------------------------------");
Serial.println(API_KEY);
}
void Searcher(){
  Serial.print("hoishyaar here comes the nele *****************************************************");
  Serial.println(API_KEY);
//  String(API_KEY);
  int length=API_KEY.length();
//int length=22;
  char pointer;
  String to_be_replaced[50];
  String Replacee[50];
  int x=0;
  for(int a=0;a<=length;a++){
    if(API_KEY.charAt(a)=='%'){
//       Serial.println("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
      to_be_replaced[x]+=API_KEY.charAt(a);
      do{
//         Serial.print("******************************************************");
        a++;
        to_be_replaced[x]+=API_KEY.charAt(a);
//        Serial.println(to_be_replaced[x]);
//        delay(500);
      }
      while(API_KEY.charAt(a)!='%');
      x++;
    }
  }
  //for debugging
  int nele;
  Serial.println("length = ");
  Serial.println(length);
//  for(nele=0;nele<=length;nele++){
//    Serial.print(to_be_replaced[nele]);
//    Serial.print("gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg********");
//
//  }
       
Serial.println(humidity);
Serial.println(String(moisture));
  // debugging ended.............................................
for(int b=0;b<=x;b++){
//      Serial.println(moisture);
//    Serial.println(moisture);
  if(to_be_replaced[b]=="%humidity%"){
    
    Replacee[b]=String(humidity);
    Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
  }
  else if(to_be_replaced[b]=="%temperature%"){
    Replacee[b]=String(temperature);
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(String(temperature));
  }
  else if(to_be_replaced[b]=="%lux%"){
    Replacee[b]=String(lux);
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(String(lux));
  }
  else if(to_be_replaced[b]=="%ambient_temperature%"){
    Replacee[b]=String(ambient_temperature);
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(String(ambient_temperature));
  }
  else if(to_be_replaced[b]=="%pressure%"){
    Replacee[b]=String(pressure);
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(String(pressure));
  }
  else if(to_be_replaced[b]=="%moisture%"){
    Replacee[b]=String(moisture);
    Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(String(moisture));
    Serial.println(Replacee[b]);
  }
  else if(to_be_replaced[b]=="%month%"){
    Replacee[b]=month;
      Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }  
   else if(to_be_replaced[b]=="%year%"){
    Replacee[b]=year;
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }  
   else if(to_be_replaced[b]=="%day%"){
    Replacee[b]=day;
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }  
   else if(to_be_replaced[b]=="%hour%"){
    Replacee[b]=hour;
       Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }
   else if(to_be_replaced[b]=="%minute%"){
    Replacee[b]=minute;
      Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }
   else if(to_be_replaced[b]=="%second%"){
    Replacee[b]=second;
      Serial.println("in "+String(to_be_replaced[b]));
    Serial.println(Replacee[b]);
   }
          
}
// for debugging
// int tooti;
// for(tooti=0;tooti<=5;tooti++){
//  Serial.println("tootil i ba :");
//  Serial.println(Replacee[tooti]);
// }
//delay(1000);
Replacer(Replacee,length);
}
//.........................................
void data_saver(){
  //saves data in flash if wifi is not connected
//  digitalWrite(common_power,HIGH);
//  delay(20);
  
  moisture_reading_function();
  temperature_reading_function();
  lux_reading_function();
//  BME();
//  lux_value, Celcius,moisture_reading,ambient_temperature,pressure,humidity
int a=0;
while(a<3){
  File myDataFile = SPIFFS.open(filename, "a+");
  if (!myDataFile)Serial.println("file open failed");
//  myDataFile.println("reading :"+String(reading_no));
  reading_no++;
  myDataFile.println(String(reading_no));
  myDataFile.println(String(temperature));
  myDataFile.println(String(moisture));
  myDataFile.println(String(pressure));
  myDataFile.println(String(humidity));
  myDataFile.println(String(ambient_temperature));
  myDataFile.println(String(lux));
  RTc();
  myDataFile.println(String(month));
  myDataFile.println(String(day));
  myDataFile.println(String(year));
  myDataFile.println(String(hour));
  myDataFile.println(String(minute));
  myDataFile.println(String(second));
  myDataFile.close();
  Serial.println("---->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>here i am");
  a++;
}
//  data_reader();
//  ESP.deepSleep(10);
}
uint8_t beef[50];
int len;
void data_reader(){
     myDataFile = SPIFFS.open(filename, "r");              // Open the file again, this time for reading
    if (!myDataFile) Serial.println("file open failed");  // Check for errors
//    Serial.println(myDataFile.read());
//    Serial.println(".....................");
    delay(500);   
            char buffer[64];
              while (myDataFile.available()){
                  int l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(reading)=String(buffer);
                  Serial.println("reading");
                  Serial.println(reading);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(lux_value)=String(buffer);
                  Serial.println("lux_value");
                  Serial.println(lux_value);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(Celcius)=String(buffer);
                  Serial.println("Celcius");
                  Serial.println(Celcius);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(moisture_reading)=String(buffer);
                  Serial.println("moisture_reading");
                  Serial.println(moisture_reading);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(ambient_temperature)=String(buffer);
                  Serial.println("ambient_temperature");
                  Serial.println(ambient_temperature);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(pressure)=String(buffer);
                  Serial.println("pressure");
                  Serial.println(pressure);
                  l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  buffer[l] = 0;
                  String(humidity)=String(buffer);
                  Serial.println("humidity");
                  Serial.println(humidity);
                   RTc();
                  month=int(buffer);
                  Serial.println("month");
                  Serial.println(month);
                  day=int(buffer);
                  Serial.println("day");
                  Serial.println(day);
                  year=int(buffer);
                  Serial.println("year");
                  Serial.println(year);
                  hour=int(buffer);
                  Serial.println("hour");
                  Serial.println(hour);
                  minute=int(buffer);
                  Serial.println("minute");
                  Serial.println(minute);
                  second=int(buffer);
                  Serial.println("second");
                  Serial.println(second);
                  myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
                  sendData(String(lux),String( temperature),String( moisture),String(ambient_temperature),String(pressure),String(humidity),month,day,year,hour,minute,second);
                  Serial.println("123654789841258542568525856852568585here i am");
              }
    myDataFile.close();                                   // Close the file
    delay(100);
  SPIFFS.remove(filename);//delete the file now
}
void printError(byte error)
  // If there's an I2C error, this function will
  // print out an explanation.
{
  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error)
  {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}
#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            month=dt.Month(),
            day=dt.Day(),
            year=dt.Year(),
            hour=dt.Hour(),
            minute=dt.Minute(),
            second=dt.Second() );
//            Time=String(dt.Month())+"/"+String(dt.Day())+"/"+String(dt.Year())+"/"+String(dt.Hour())+"/"+String(dt.Minute())+"/"+String(dt.Second());
            Serial.println("here is the time from your new RTC =  ");
            Serial.println(month);
            Serial.println(day);
            Serial.println(year);
            Serial.println(hour);
            Serial.println(minute);
            Serial.println(second);
//    Serial.print(datestring);
}
void RTc ()
{
    if (!Rtc.IsDateTimeValid())
    {
        if (Rtc.LastError() != 0)
        {
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
            
        }
        else
        {
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();
  RtcTemperature temp = Rtc.GetTemperature();
  temp.Print(Serial);
    Serial.println("C");
    delay(10);
}
