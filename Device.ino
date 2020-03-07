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

/*

class DeviceSettings{

    // include wifimanager and deepsleep

    #include <ESP8266WiFiMulti.h>
    #include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
    #include <ESP8266WebServer.h> //not sure if it is necessary for wifi manager
    #include <ESP8266HTTPClient.h>//not sure if it is necessary for wifi manager
    #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
    #include <WiFiClient.h>

    WiFiManager wifiManager;

    private:

        String  deviceName, API_KEY_ch, data4;
        int sleepTime;



        DeviceSettings(){
        }
    public:

        void WIFIMANAGER() {

            WiFiManagerParameter custom_deviceName("device name", "device name", deviceName, 20);
            WiFiManagerParameter custom_API_KEY(" API_KEY", " API_KEY", API_KEY_ch, 2000);
            WiFiManagerParameter custom_sleepTime("sleepTime", "sleepTime", sleepTime, 15);
            WiFiManagerParameter custom_text(data4);

            WiFiManager wifiManager;

            wifiManager.setSaveConfigCallback(saveConfigCallback);

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

            Serial.println("connected...yeey :)");

            //read updated parameters
            strcpy(deviceName, custom_deviceName.getValue());
            strcpy(API_KEY_ch, custom_API_KEY.getValue());
            strcpy(sleepTime, custom_sleepTime.getValue());

        }

        void DeepSleep(int Time_minutes) {
            Serial.println("Going to deep sleep for ");
            Serial.print(Time_minutes);
            Serial.print(" minutes\n");

            ESP.deepSleep(Time_minutes * 60 * 1000000);

        }


};

class DataSender {

    private:

        int i = 0;
        int time[6];
        double lux_value;
        long int BMEvalues[3];
        int moistReading;
        int humidity;
        int pressure;
        int ambientTemperature;
        String API;

        DataSender(int Time[6],
            double Lux_value,
            long int bMEvalues[3],
            int MoistReading,
            int Humidity,
            int Pressure,
            int AmbientTemperature,
            String aPI;
            ) {


            while (i < 7) {
                time[i] = Time[i];
                i++;
            }

            i = 0;

            while (i < 4) {
                BMEvalues[i] = bMEvalues[i];
            }

            moistReading = MoistReading;
            humidity = Humidity;
            pressure = Pressure;
            ambientTemperature = AmbientTemperature;
            API = aPI;

        }

    public:

        void SendToAPI() {


        }



};

*/


class Data {

private:
    int moisture_reading, pressure, humidity, ambient_temperature, lux_value, month, day, year, hour, minute, second;
    String filename;
    int Return_values[11];
    int readingNo = 0;
    struct ReadValues {

        int Return_values[11];

    };
    ReadValues ReadValues1[];

public:
    Data(int Moisture, int Pressure, int Humidity, int Month, int Day, int Year, int Hour, int Minute, int Second, int lux) {

        moisture_reading = Moisture;
        pressure = Pressure;
        humidity = Humidity;
        month = Month;
        day = Day;
        year = Year;
        hour = Hour;
        minute = Minute;
        second = Second;
        lux_value = lux;
    }

    void WriteData() {
        filename = "datalog.txt";

        File myDataFile = SPIFFS.open(filename, "W");
        if (!myDataFile)Serial.println("file open failed");

        myDataFile.println(moisture_reading);
        myDataFile.println(pressure);
        myDataFile.println(humidity);
        myDataFile.println(ambient_temperature);
        myDataFile.println(lux_value);
        myDataFile.println(month);
        myDataFile.println(day);
        myDataFile.println(year);
        myDataFile.println(hour);
        myDataFile.println(minute);
        myDataFile.println(second);

        myDataFile.close();


        Serial.println("Writing of data done\n");

    }

    ReadValues* ReadData() {

        File myDataFile = SPIFFS.open(filename, "r");
        if (!myDataFile) Serial.println("file open failed");

        delay(500);

        char buffer[64];
        readingNo = 0;
        while (myDataFile.available()) {

            Serial.println("file opened");
            int l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            lux_value = atoi(buffer);

            Serial.println("lux_value");
            Serial.println(lux_value);


            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            moisture_reading = atoi(buffer);

            Serial.println("moisture_reading");
            Serial.println(moisture_reading);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            ambient_temperature = atoi(buffer);

            Serial.println("ambient_temperature");
            Serial.println(ambient_temperature);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            pressure = atoi(buffer);

            Serial.println("pressure");
            Serial.println(pressure);


            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            humidity = atoi(buffer);

            Serial.println("humidity");
            Serial.println(humidity);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            month = atoi(buffer);

            Serial.println("month");
            Serial.println(month);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            day = atoi(buffer);

            Serial.println("day");
            Serial.println(day);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            year = atoi(buffer);

            Serial.println("year");
            Serial.println(year);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            hour = atoi(buffer);

            Serial.println("hour");
            Serial.println(hour);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            minute = atoi(buffer);

            Serial.println("minute");
            Serial.println(minute);

            l = myDataFile.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[l] = 0;
            second = atoi(buffer);

            Serial.println("second");
            Serial.println(second);

            //            Struct ReadValues ReadValues1[];

            ReadValues1[readingNo].Return_values[0] = moisture_reading;
            ReadValues1[readingNo].Return_values[1] = pressure;
            ReadValues1[readingNo].Return_values[2] = humidity;
            ReadValues1[readingNo].Return_values[3] = ambient_temperature;
            ReadValues1[readingNo].Return_values[4] = lux_value;
            ReadValues1[readingNo].Return_values[5] = month;
            ReadValues1[readingNo].Return_values[6] = day;
            ReadValues1[readingNo].Return_values[7] = year;
            ReadValues1[readingNo].Return_values[8] = hour;
            ReadValues1[readingNo].Return_values[9] = minute;
            ReadValues1[readingNo].Return_values[10] = second;

            readingNo++;
        }
        myDataFile.close();                                   // Close the file

        Serial.println("File reading done\n");

        return ReadValues1;
    }

    void DeleteData() {

        SPIFFS.remove(filename);//delete the file now
        Serial.println("file deleted");

    }


};


//Sensors testing(D0, D7, D6, D5);

Data TestData(12, 22, 33, 4, 3, 2, 5, 6, 4, 7);
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

}
void loop() {
    // put your main code here, to run repeatedly:
    Serial.print("\nhere i came");
    TestData.WriteData();
    TestData.ReadData();
    delay(1500);


    /* testing.SoilMoisture();
    //    delay(1000000);
    testing.BME();
    testing.Luminosity();
    testing.RealTimeClock();*/






}
