#include <DallasTemperature.h>     // https://github.com/milesburton/Arduino-Temperature-Control-Library
#define ONE_WIRE_BUS 0              
OneWire oneWire(ONE_WIRE_BUS);     // https://github.com/PaulStoffregen/OneWire
DallasTemperature sensors(&oneWire);
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
WiFiClient client;
#define PIN_POWER_DS 13             // Шина питания датчика температуры
const char* ssid = "AAA";          // имя удаленной точки доступа роутера 
const char* password = "BBB";      // пароль удаленной точки доступа
unsigned long Time1 = 0; 

void setup()   {
Serial.begin(115200);  
pinMode     (PIN_POWER_DS, OUTPUT); 
digitalWrite(PIN_POWER_DS, HIGH); 
WiFi.mode   (WIFI_AP_STA);         // запускаем смешенный режим 
WiFi.softAP ("ESP","martinhol");   // поднимаем соaт точку доступа
WiFi.begin  (ssid, password);
Serial.println("Подключаюсь к сети "),      Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED)     delay(500), Serial.print(".");
Serial.print("WiFi подключен, ChipId: "), Serial.println(ESP.getChipId());
Serial.print("IP Адрес: "),             Serial.println(WiFi.localIP());
Serial.print("MAC Адрес: "),            Serial.println(WiFi.macAddress()), Serial.println();
sensors.begin(); 
               }

void loop()    {
//if (millis()> Time1 + 5000) Time1 = millis(), narodmonSend ();       // выполняем функцию narodmonSend каждые 10 сек для теста

narodmonSend ();
ESP.deepSleep(5*60*1000000);          // спать на 5 минут пины D16 и  RST должны быть соеденены между собой
                }


void narodmonSend () {
float vbat = analogRead(A0); 
vbat = vbat / 3; 
int inDS = 0;
sensors.requestTemperatures(); 
delay (1000);
String buf;
buf = "#" + WiFi.macAddress() + "#Dev"  +  ESP.getChipId()  + "\n";    // Заголовок с МАС адресом и ID чипа
buf = buf + "#RSSI#" + WiFi.RSSI() /* + "#Уровень вайфай" */ + "\n";   // Уровень вайфай
buf = buf + "#VBAT#" + vbat /* + "#Напряжение" */ + "\n";              // Напряжение
while (1){
        float TempDS = sensors.getTempCByIndex(inDS);                  // читаем температуру
        if (TempDS == -127.00) break;                                  // пока не доберемся до неподключенного датчика
        inDS++;                                                               
        buf = buf + "#Temp"+inDS+"#"+TempDS+ "\n" /* + "#температура" */; // дописываем в строку с температурой
        } 
buf = buf + "#Uptime#" +millis()/1000+"\n";                            // время работы       
buf = buf + "##";                                                      // закрываем пакет ##
  

if (!client.connect("narodmon.ru", 8283)) {
    Serial.println("connection failed");
    return;
                                          }
client.print(buf);                                                     // и отправляем данные   
Serial.println(buf);  
                    }              