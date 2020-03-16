/*
Modules :
* Sensors
* Device settings  --> wifimanager and deep sleep
                        * Wifimanager takes extra values
* DataSender  --> send via an api
* Datasaver
*/

#include "cactus_io_BME280_SPI.h"
#include <Wire.h>
#include <RtcDS3231.h>
#include <SparkFunTSL2561.h> // library for luminosity sensor
#include <FS.h>   
#include <ArduinoJson.h> 
#include <ESP8266WiFi.h> // for wifiManager
#include <DNSServer.h> // for wifiManager
#include <ESP8266WebServer.h> // for wifiManager
#include <WiFiManager.h>   
#include <ESP8266WiFiMulti.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>


class Sensors {
private:
    int moistReading;
    int humidity, pressure, ambientTemperature;
    long int BMEvalues[3];
    bool  state;
    double lux_value;
    unsigned int ms;
    boolean sensitivity;
    unsigned int data1, data2;
    int moisture_pin;
    int BME_SCK, BME_MISO, BME_MOSI, BME_CS;
    int month, day, year, hour, minute, second;
    int TIME[6];
    char datestring[20];
    BME280_SPI* bme;
    SFE_TSL2561 LUX;
    RtcDS3231<TwoWire>* Rtc;

public:
    //Creating 2 Constructors, one with soil moisture and another without it
    Sensors(int MoisturePin, int bME_SCK, int bME_MISO, int bME_MOSI, int bME_CS) {

        moisture_pin = MoisturePin;
        BME_SCK = bME_SCK;
        BME_MISO = bME_MISO;
        BME_MOSI = bME_MOSI;
        BME_CS = bME_CS;

    }


    Sensors(int bME_SCK, int bME_MISO, int bME_MOSI, int bME_CS) {

        BME_SCK = bME_SCK;
        BME_MISO = bME_MISO;
        BME_MOSI = bME_MOSI;
        BME_CS = bME_CS;

    }


    unsigned char timer = 2;

    int SoilMoisture() {


        pinMode(moisture_pin, INPUT);

        moistReading = analogRead(moisture_pin);
        moistReading = map(moistReading, 1024, 0, 0, 100);

        Serial.print("\nMoisture : ");
        Serial.print(moistReading);

        return moistReading;
    }
    long int* BME() {

        BME280_SPI bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

        state = bme.begin();
        if (!state) {
            Serial.println("Could not find a valid BME280 sensor, check wiring!");
        }

        bme.setTempCal(-1);

        bme.readSensor();
        humidity = bme.getHumidity();
        pressure = bme.getPressure_MB();
        ambientTemperature = bme.getTemperature_C();

        BMEvalues[0] = humidity;
        BMEvalues[1] = pressure;
        BMEvalues[2] = ambientTemperature;

        Serial.println("\nhere are the values");
        Serial.println(humidity);
        Serial.println(pressure);
        Serial.println(ambientTemperature);

        return BMEvalues;
    }

    int Luminosity() {


        LUX.begin();
        LUX.setTiming(sensitivity, timer, ms);
        LUX.setPowerUp();

        LUX.getData(data1, data2);

        //data1 and data2 corresponds to data of lingt and infrared rays,
        //passing them to following function, unaltered, gives luminosity

        LUX.getLux(sensitivity, ms, data1, data2, lux_value);

        Serial.print("\nLuminosity = ");
        Serial.print(lux_value);

        return lux_value;
    }

    int* RealTimeClock() {


        RtcDS3231<TwoWire> Rtc(Wire);
        const RtcDateTime& dt = Rtc.GetDateTime();
        month = dt.Month();
        day = dt.Day();
        year = dt.Year();
        hour = dt.Hour();
        minute = dt.Minute();
        second = dt.Second();

        TIME[0] = month;
        TIME[1] = day;
        TIME[2] = year;
        TIME[3] = hour;
        TIME[4] = minute;
        TIME[5] = second;

        Serial.print("\nmonth day year hour minute second\n");
        int k = 0;

        while (k < 6) {
            Serial.print(TIME[k]);
            Serial.print("  ");
            k++;
        }

        return TIME;

    }

};

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

class DeviceSettings {
    char  deviceName[100], API_KEY[1000], customText[];
    char sleepTime[10];

public:
    char* WIFIMANAGER() {

      strcpy(sleepTime,"30");
      strcpy(deviceName,"Buol AgriTech");

        Serial.println("mounting FS...");
        if (SPIFFS.begin()) {
            Serial.println("mounted file system");
            if (SPIFFS.exists("/config.json")) {
                Serial.println("reading config file");
                File configFile = SPIFFS.open("/config.json", "r");
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
                        strcpy(API_KEY, json["API_KEY"]);
                        strcpy(sleepTime, json["sleepTime"]);
                    }
                    else {
                        Serial.println("failed to load json config");
                    }
                    configFile.close();
                }
            }
        }
        else {
            Serial.println("failed to mount FS");
        }


        //end read



        // The extra parameters to be configured (can be either global or just in the setup)
        // After connecting, parameter.getValue() will get you the configured value
        // id/name placeholder/prompt default length
        WiFiManagerParameter custom_deviceName("deviceName", "Device name", deviceName, 100);
        WiFiManagerParameter custom_API_KEY(" API_KEY", " API", API_KEY, 1000);
        WiFiManagerParameter custom_sleepTime("sleepTime", "sleepTime", sleepTime, 15);

        //WiFiManager
        //Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;
        wifiManager.setConfigPortalTimeout(180);

        //set config save notify callback
        wifiManager.setSaveConfigCallback(saveConfigCallback);


        //add all your parameters here
        wifiManager.addParameter(&custom_deviceName);
        wifiManager.addParameter(&custom_API_KEY);
        wifiManager.addParameter(&custom_sleepTime);

        if (!wifiManager.autoConnect(deviceName, "password")) {
            Serial.println("failed to connect and hit timeout");
            delay(30);
        }

        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");

        //read updated parameters
        strcpy(deviceName, custom_deviceName.getValue());
        strcpy(API_KEY, custom_API_KEY.getValue());
        strcpy(sleepTime, custom_sleepTime.getValue());

        //save the custom parameters to FS
        if (shouldSaveConfig) {
            Serial.println("saving config\n");
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.createObject();
            json["deviceName"] = deviceName;
            json["API_KEY"] = API_KEY;
            json["sleepTime"] = sleepTime;

            File configFile = SPIFFS.open("/config.json", "w");
            if (!configFile) {
                Serial.println("failed to open config file for writing");
            }

            json.printTo(Serial);
            json.printTo(configFile);
            configFile.close();
            Serial.println("Saved\n");
        }

        if (!wifiManager.autoConnect(deviceName, "password")) {
            Serial.println("failed to connect and hit timeout");
            delay(30);
        }

        return API_KEY;

    }
    void DeepSleep() {
        Serial.println("Going to deep sleep for ");
        Serial.print(atoi(sleepTime));
        Serial.print(" minutes\n");

        ESP.deepSleep(atoi(sleepTime) * 60 * 1000000);

    }

};

class DataSender {

private:

    int i = 0;
    int* time;
    double lux_value;
    long int* BMEvalues;
    int moistReading;
    int humidity;
    int pressure;
    int ambientTemperature;
    String API;
    String API_to_Send;

    String Replacer(String str) {

        Serial.println("API from Replacer Funciton");
        Serial.println(str);

        str.replace("%humidity%", String(humidity));
        str.replace("%lux%", String(lux_value));
        str.replace("%ambient_temperature%", String(ambientTemperature));
        str.replace("%pressure%", String(pressure));
        str.replace("%moisture%", String(moistReading));
        str.replace("%month%", String(time[0]));
        str.replace("%year%", String(time[2]));
        str.replace("%day%", String(time[1]));
        str.replace("%hour%", String(time[3]));
        str.replace("%minute%", String(time[4]));
        str.replace("%second%", String(time[5]));

        Serial.println("Replacement finished --------->>>>>>>>>>");

        return str;

    }

public:

    DataSender(int* Time,
        double Lux_value,
        long int* bMEvalues,
        int MoistReading,
        String aPI
    ) {

        time = Time;
        BMEvalues = bMEvalues;

        moistReading = MoistReading;
        humidity = BMEvalues[0];
        pressure = BMEvalues[1];
        ambientTemperature = BMEvalues[2];
        lux_value = Lux_value;
        API = aPI;
        Serial.println("API in the first constructor of class DataSender");
        Serial.println(API);

    }

    DataSender(String api) {

        API = api;
        
        Serial.println("API in the second constructor of dataSender");
        Serial.println(API);

    }

    int SendAPI() {

        API_to_Send = Replacer(API);

        HTTPClient http;
        Serial.println("api to be sent inside sendApi() after replacement");
        Serial.println(API_to_Send);

        http.begin(API_to_Send);

        int httpCode = http.GET();

        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
            }
        }

        else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            //                if (atoi(http.errorToString(httpCode).c_str()) != 200) 
        }
        http.end();

        return httpCode;
    }
        int SendAPI_Second_saved() {

        API_to_Send = API;

        HTTPClient http;
        Serial.println("api to be sent inside SendAPI_Second_saved() after replacement <<<<<<<<<<<<<<<");
        Serial.println(API_to_Send);

        http.begin(API_to_Send);

        int httpCode = http.GET();

        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
            }
        }

        else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            //                if (atoi(http.errorToString(httpCode).c_str()) != 200) 
        }
        http.end();

        return httpCode;
    }

    String needAPI() {

        return Replacer(API);
    }

};

class Data {

private:

    String API;
    String API_return[100];
    String filename  = "datalogger.txt";
    int readingNo = 0;


public:

    void WriteData(String api) {
        API = api;

        SPIFFS.begin();

        File myDataFile = SPIFFS.open(filename, "a+");
        if (!myDataFile)Serial.println("file open failed");
       
        Serial.println("Writing the following");
        Serial.println(API);
        
        myDataFile.println(API);
//        myDataFile.commit();
        myDataFile.close();

        Serial.println("Writing of data done \n");

    }

    String* ReadData() {

        // convert them to set of APIs and they will be read and sent next time

        SPIFFS.begin();
        File myDataFile = SPIFFS.open(filename, "r");
        if (!myDataFile) Serial.println("file open failed");

        delay(500);

        char buffer[1000];
        
        while (myDataFile.available()) {

            Serial.println("file opened");

            int l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l-1] = 0;
            API = String(buffer);

            Serial.println("API");
            Serial.println(API);

            API_return[readingNo] = API;
            readingNo++;
            Serial.println("readingNo in read data");
            Serial.println(readingNo);

        }
        myDataFile.close();                                   // Close the file

        Serial.println("File reading done\n");
        

        return API_return;
    }

    void DeleteData() {

        SPIFFS.remove(filename);//delete the file now
        Serial.println("file deleted");

    }
    int sizer(){
        Serial.println("readingNo in  sizer");
        Serial.println(readingNo);
        return readingNo;
        
    }
    void DeleteLine(int LineNumber) {


    }

};

//Sensors testing(A0, D0, D7, D6, D5);

DeviceSettings SetDevice;

char* AP;
String API;
String* ReadValues;
Sensors SensorReady(A0, D0, D7, D6, D5);
int DeepSleepTime;
int moisture;
int* TimeArray;
double lux;
long int* BmeValues;
int returnCodeOfHTTP;
int i;

char* abc;
void setup() {

    Serial.begin(115200);
    SPIFFS.begin();

    AP = SetDevice.WIFIMANAGER();
    API = String(AP);
    Serial.println("API from WifiManager in setup(); ");
    Serial.println(API);

    //  int sleepTime_string = toInt(AP[1]);
//    delay(15000);

}


void loop() {
    delay(6000);
    moisture = SensorReady.SoilMoisture();
    lux = SensorReady.Luminosity();
    TimeArray = SensorReady.RealTimeClock();
    BmeValues = SensorReady.BME();
    Serial.println("here comes the debugger");
    DataSender SendMe(TimeArray, lux, BmeValues, moisture, API);
     
    returnCodeOfHTTP = SendMe.SendAPI();

    for (i = 0; i < 5; i++) {
        if (returnCodeOfHTTP == 200)  break;
        else returnCodeOfHTTP = SendMe.SendAPI();
    }

    Data SaveMe;

    API = SendMe.needAPI();

    
    if (returnCodeOfHTTP != 200)  SaveMe.WriteData(API);
    else{
        ReadValues = SaveMe.ReadData();
        int Size = SaveMe.sizer();
        Serial.println("entering else,\n size of array =  ");
        Serial.println(Size);

        
        // Now if it is 200(OK), then send all the data you have stored in the memory
        
        returnCodeOfHTTP=12;
        for (i = 0; i < Size; ) {
            Serial.println("loop iteration number ");
            Serial.print(i);
            Serial.println("\nsaved but to be sent");
            Serial.println(ReadValues[i]);
            DataSender SendMe_savedData(ReadValues[i]);
            
            for (int k = 0; k < 2; k++) {
              Serial.print("looping to send the saved data\n");
              if (returnCodeOfHTTP == 200){ 
                
                Serial.println("breaking the loop");
                break;
              
              }
              else{
                
                returnCodeOfHTTP = SendMe_savedData.SendAPI_Second_saved();
                
                Serial.println(" sending the data and code is ");
                Serial.println(returnCodeOfHTTP);
                
              }
            
            }
            
            Serial.println("ended previous loop");
            if (returnCodeOfHTTP == 200){
              i++;  // and delete the read&sent line of API ----> to be done
              returnCodeOfHTTP=12;
            }
            else break;
            delay(1000);
        
        }
        if(i >= Size){
          SaveMe.DeleteData();
        }
    }


//    delay(1000);
      SetDevice.DeepSleep();

}
