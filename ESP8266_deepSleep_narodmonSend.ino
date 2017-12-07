#include <DallasTemperature.h>     // https://github.com/milesburton/Arduino-Temperature-Control-Library
#define ONE_WIRE_BUS 2              
OneWire oneWire(ONE_WIRE_BUS);     // https://github.com/PaulStoffregen/OneWire
DallasTemperature sensors(&oneWire);
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
WiFiClient client;
#define PIN_POWER_DS 4             // Шина питания датчика температуры
const char* ssid = "AAA";          // имя удаленной точки доступа роутера 
const char* password = "BBB";      // пароль удаленной точки доступа

void setup()   {
Serial.begin(115200);  
pinMode     (PIN_POWER_DS, OUTPUT); 
digitalWrite(PIN_POWER_DS, HIGH); 
WiFi.mode   (WIFI_AP_STA);         // запускаем смешенный режим 
WiFi.softAP ("ESP","martinhol");   // поднимаем соaт точку доступа
WiFi.begin  (ssid, password);
Serial.print("Подключаюсь к сети "),      Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED)     delay(500), Serial.print(".");
Serial.print("WiFi подключен, ChipId: "), Serial.print(ESP.getChipId());
Serial.print("| IP Адрес: "),             Serial.print(WiFi.localIP());
Serial.print("| MAC Адрес: "),            Serial.print(WiFi.macAddress()), Serial.println();
sensors.begin(); 
               }

void loop()    {

narodmonSend ();
delay (10000);
//ESP.deepSleep(5*60*1000000);          // спать на 5 минут
                }


void narodmonSend () {
int inDS = 0;
sensors.requestTemperatures(); 
String buf;
buf = "#" + WiFi.macAddress() + "#"  +  ESP.getChipId()  + "\r\n";   // Заголовок
buf = buf + "#RSSI#" + WiFi.RSSI()  + "#Уровень вайфай"  + "\r\n";   // Уровень вайфай
buf = buf + "#VBAT#" + analogRead(A0)/3 + "#Напряжение"  + "\r\n";   // Напряжение
while (1){
        float TempDS = sensors.getTempCByIndex(inDS);                 // читаем температуру
        if (TempDS == -127.00) break;                                 // пока не доберемся до неподключенного датчика
        inDS++;                                                               
        buf = buf + "#Temp"+inDS+"#"+TempDS+ "\r\n";                  // дописываем в строку
         } 
buf = buf + "#Uptime#" +millis()/1000+"\r\n";                         // время работы       
buf = buf + "##";                                                     // закрываем пакет ##
  
Serial.print(buf);  
if (!client.connect("narodmon.ru", 8283)) {
    Serial.println("connection failed");
    return;
                                          }
client.print(buf); // и отправляем данные   
                    }              