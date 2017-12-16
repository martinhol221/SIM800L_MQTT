#include <SoftwareSerial.h>
SoftwareSerial SIM800(7, 6);        // для новых плат начиная с версии RX,TX
#include <DallasTemperature.h>      // подключаем библиотеку чтения датчиков температуры
OneWire oneWire(4);                 // и настраиваем  пин 4 как шину подключения датчиков DS18B20
DallasTemperature sensors(&oneWire);
#define BAT_Pin      A0             // на батарею, через делитель напряжения 39кОм / 11 кОм
#define REL1 8                      // распиновка от  платы автозапуска
#define REL2 9
#define REL3 12
#define REL4 11
#define REL5 10
#define REL6 13
/*  ----------------------------------------- ИНДИВИДУАЛЬНЫЕ НАСТРОЙКИ !!!---------------------------------------------------------   */
String APN = "internet.mts.by";     // тчка доступа выхода в интернет вашего сотового оператора
String USER = "mts";                // имя выхода в интернет вашего сотового оператора
String PASS = "mts";                // пароль доступа выхода в интернет вашего сотового оператора
bool  broker = false;         // статус подклюлючения к брокеру
/*  ----------------------------------------- НАСТРОЙКИ MQTT брокера---------------------------------------------------------   */
const char MQTT_user[10] = "11111111";      // api.cloudmqtt.com > Details > User  
const char MQTT_pass[15] = "22222222222";  // api.cloudmqtt.com > Details > Password
const char MQTT_type[15] = "MQIsdp";        // тип протокола
const char MQTT_CID[15] = "AAAAAA";        // уникальное имя устройства в сети MQTT
String MQTT_SERVER = "m23.cloudmqtt.com";   // api.cloudmqtt.com > Details > Server  сервер MQTT брокера
String PORT = "10077";                      // api.cloudmqtt.com > Details > Port    порт MQTT брокера
/*  ----------------------------------------- ДАЛЕЕ НЕ ТРОГАЕМ --------------------------------------------------------------------   */
String at = "";                    // строка хранения ответов модема в ASCII
String str = "";                   // строка хранения ответов сервера в HEX
float TempDS[11];                  // массив хранения температуры c рахных датчиков 
float Vbat;                        // переменная хранящая напряжение бортовой сети
float m = 69.91;                   // делитель для перевода АЦП в вольты для резистров 39/11kOm
unsigned long Time1 = 0; 
int k = 0;
int interval = 2;                  // интервал отправки данных на народмон сразу после загрузки 
int inDS;

void setup() {
  pinMode(REL1, OUTPUT);
  pinMode(REL2, OUTPUT);
  pinMode(REL3, OUTPUT);
  pinMode(REL4, OUTPUT);
  pinMode(REL5, OUTPUT);
  pinMode(REL6, OUTPUT);
  Serial.begin(9600);              //скорость порта
  SIM800.begin(9600);              //скорость связи с модемом
  Serial.println("MQTT |16/12/2017"); 
  delay (1000);
  SIM800.println("AT+CFUN=1,1");   // перезагрузка модема
             }


void loop() {
/*  --------------------------------------------------- АНАЛИЗИРУЕМ БУФЕР ВИРТУАЛЬНОГО ПОРТА МОДЕМА---------------------------------- */ 
  if (SIM800.available()) {                                               // если что-то пришло от SIM800 в SoftwareSerial Ардуино
    while (SIM800.available()) k = SIM800.read(),at += char(k),str += String(k,HEX),delay(1);  // набиваем в переменную at
   
  /* Serial.print("Ответ в ACII "), */ Serial.println(at);               // Возвращаем ответ можема в монитор порта в ACSII
   
   /*  -------------------------------------- проверяем соеденеиние с ИНТЕРНЕТ, конектимся к серверу------------------------------------------------------- */
          if (at.indexOf("AT+SAPBR=3,1, \"Contype\",\"GPRS\"\r\r\nOK") > -1 ) {SIM800.println("AT+SAPBR=3,1, \"APN\",\""+APN+"\""), delay (500); 
   } else if (at.indexOf("AT+SAPBR=3,1, \"APN\",\""+APN+"\"\r\r\nOK") > -1 )  {SIM800.println("AT+SAPBR=1,1"), delay (1000); // устанавливаем соеденение   
   } else if (at.indexOf("AT+SAPBR=1,1\r\r\nOK") > -1 )                       {SIM800.println("AT+SAPBR=2,1"), delay (1000); // проверяем статус соединения  
   } else if (at.indexOf("+SAPBR: 1,1") > -1 )    {delay (200),  SIM800.println("AT+CIPSTART=\"TCP\",\""+MQTT_SERVER+"\",\""+PORT+"\""), delay (1000);
   } else if (at.indexOf("CONNECT OK\r\n") > -1 ) MQTT_CONNECT (); // после соединения с сервером отправляем пакет авторизации, публикации и пдписки у брокера

 //   Serial.print("Ответ в HEX  "),               Serial.println(str); // Возвращаем ответ брокера в монитор порта в HEX
    
         if (str.indexOf("52656672657368") > -1 )  {Serial.println ("Команда Refresh приянята "), Publisch(), interval = 6; // швырнуть данные на сервер
   }else if (str.indexOf("3152454c31") > -1 )  {Serial.println ("Команда 1REL1 принята "), digitalWrite(REL1, HIGH); // включить реле  1      
   }else if (str.indexOf("3152454c30") > -1 )  {Serial.println ("Команда 1REL0 принята "), digitalWrite(REL1, LOW);  // выключить реле 1  
   }else if (str.indexOf("3252454c31") > -1 )  {Serial.println ("Команда 2REL1 принята "), digitalWrite(REL2, HIGH);       
   }else if (str.indexOf("3252454c30") > -1 )  {Serial.println ("Команда 2REL0 принята "), digitalWrite(REL2, LOW);
/*   }else if (str.indexOf("3352454c31") > -1 )  {Serial.println ("Команда 3REL1 принята "), digitalWrite(REL3, HIGH);       
   }else if (str.indexOf("3352454c30") > -1 )  {Serial.println ("Команда 3REL0 принята "), digitalWrite(REL3, LOW);
   }else if (str.indexOf("3452454c31") > -1 )  {Serial.println ("Команда 4REL1 принята "), digitalWrite(REL4, HIGH);       
   }else if (str.indexOf("3452454c30") > -1 )  {Serial.println ("Команда 4REL0 принята "), digitalWrite(REL4, LOW);
   }else if (str.indexOf("3552454c31") > -1 )  {Serial.println ("Команда 5REL1 принята "), digitalWrite(REL5, HIGH);       
   }else if (str.indexOf("3552454c30") > -1 )  {Serial.println ("Команда 5REL0 принята "), digitalWrite(REL5, LOW); */
   }else if (str.indexOf("3652454c31") > -1 )  {Serial.println ("Команда 6REL1 принята "), digitalWrite(REL6, HIGH);       
   }else if (str.indexOf("3652454c30") > -1 )   Serial.println ("Команда 6REL0 принята "), digitalWrite(REL6, LOW);      
     at = "", str = "";   }                                               // очищаем переменные
   
if (millis()> Time1 + 10000) Time1 = millis(), detection();               // выполняем функцию detection () каждые 10 сек 

             }

void detection(){                                                        // условия проверяемые каждые 10 сек  
    Vbat = analogRead(BAT_Pin);                                          // замеряем напряжение на батарее
    Vbat = Vbat / m ;                                                    // переводим попугаи в вольты
    Serial.print("Vbat="),Serial.print(Vbat);  
    Serial.print("| int: "), Serial.print(interval);
    inDS = 0;
    sensors.requestTemperatures();          // читаем температуру с трех датчиков
    while (inDS < 10){
          TempDS[inDS] = sensors.getTempCByIndex(inDS);            // читаем температуру
      if (TempDS[inDS] == -127.00){ 
                                  TempDS[inDS]= 80;
                                  break; }                               // пока не доберемся до неподключенного датчика
              inDS++;} 
    for (int i=0; i < inDS; i++) Serial.print(" | Temp"), Serial.print(i), Serial.print("= "), Serial.print(TempDS[i]); 
    Serial.println (" ");
    interval--;
    if (interval <1) { interval = 6; 
        if (broker == true) { Publisch();
    } else  SIM800.println ("AT+SAPBR=3,1, \"Contype\",\"GPRS\""), delay (200);    // подключаемся к GPRS 
                     }
}             



void  MQTT_FloatPub (const char topic[15], float val, int x) { // топик, переменная в float, количество знаков после запятой
           char st[10];
           dtostrf(val,0, x, st), MQTT_PUB (topic, st);      }


void  Publisch() { 
             SIM800.println("AT+CIPSEND"), delay (200);  
             MQTT_FloatPub ("C5/ds0", TempDS[0],2);
             MQTT_FloatPub ("C5/ds1", TempDS[1],2);
             MQTT_FloatPub ("C5/vbat", Vbat,2);
             MQTT_FloatPub ("C5/uptime", millis()/1000,0); 
             SIM800.write(0x1A); 
             }


void MQTT_CONNECT () {
  SIM800.println("AT+CIPSEND"), delay (100);
     
  SIM800.write(0x10);                                // заголовок пакета на установку соединения
  SIM800.write(strlen(MQTT_type)+strlen(MQTT_CID)+strlen(MQTT_user)+strlen(MQTT_pass)+12);
  SIM800.write((byte)0);                             // 0x00
  SIM800.write(strlen(MQTT_type));                   // длинна типа протокола
  SIM800.write(MQTT_type);                           // тип протокола
  SIM800.write(0x03);                                // LVL
  SIM800.write(0xC2);                                // FL
  SIM800.write((byte)0);                             // 0x00
  SIM800.write(0x3C);                                // KA
  SIM800.write((byte)0);                             // 0x00
  SIM800.write(strlen(MQTT_CID));                    // длинна идентификатора устройства
  SIM800.write(MQTT_CID);                            // MQTT  идентификатор устройства
  SIM800.write((byte)0);                             // 0x00  
  SIM800.write(strlen(MQTT_user));                   // длинна логина 
  SIM800.write(MQTT_user);                           // логин
  SIM800.write((byte)0);                             // 0x00
  SIM800.write(strlen(MQTT_pass));                   // длинна пароля
  SIM800.write(MQTT_pass);                           // пароль

  MQTT_PUB ("C5/status", "Подключено");              // пакет публикации
  
  MQTT_SUB ("C5/comand");                            // пакет подписки
  
  SIM800.write(0x1A),  broker = true;          // символ завершения и маркер
  }

void  MQTT_PUB (const char MQTT_topic[15], const char MQTT_messege[15]) {
  
  SIM800.write(0x30);                                // заголовок пакета на публикацию
  SIM800.write(strlen(MQTT_topic)+strlen(MQTT_messege)+2);
  SIM800.write((byte)0);                             // 0x00
  SIM800.write(strlen(MQTT_topic));                  // длинна топика
  SIM800.write(MQTT_topic);                          // топик
  SIM800.write(MQTT_messege);                        // сообщение
                     }


void  MQTT_SUB (const char MQTT_topic[15]) {
  
  SIM800.write(0x82);                               // заголовок пакета подписки на топик
  SIM800.write(strlen(MQTT_topic)+5);               // сумма пакета 
  SIM800.write((byte)0);                            // 0x00
  SIM800.write(0x01);                               // PKTID
  SIM800.write((byte)0);                            // 0x00
  SIM800.write(strlen(MQTT_topic));                 // длинна топика
  SIM800.write(MQTT_topic);                         // топик
  SIM800.write((byte)0);                            // 0x00
                     }

