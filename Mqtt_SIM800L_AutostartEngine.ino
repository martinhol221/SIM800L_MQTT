#include <SoftwareSerial.h>
SoftwareSerial SIM800(7, 6);        // для новых плат начиная с версии RX,TX
#include <DallasTemperature.h>      // подключаем библиотеку чтения датчиков температуры
OneWire oneWire(4);                 // и настраиваем  пин 4 как шину подключения датчиков DS18B20
DallasTemperature sensors(&oneWire);
/*  ----------------------------------------- НАЗНАЧАЕМ ВЫВОДЫ АРДУИНО НА РАЗЛИЧНЫЕ НАПРАВЛЕНИЯ------------------------------   */
#define FIRST_P_Pin  8                      // на реле первого положения замка зажигания с 8-го пина ардуино
#define SECOND_P     9                      // на реле зажигания, через транзистор с 9-го пина ардуино
#define STARTER_Pin  12                     // на реле стартера, через транзистор с 12-го пина ардуино
#define Lock_Pin     10                     // реле на кнопку "заблокировать дверь"
#define Unlock_Pin   11                     // реле на кнопку "разаблокировать дверь"
#define LED_Pin      13                     // на светодиод (моргалку)
#define BAT_Pin      A0                     // на батарею, через делитель напряжения 39кОм / 11 кОм
#define Feedback_Pin A1                     // на провод от замка зажигания для обратной связи по проводу ON
#define STOP_Pin     A2                     // на концевик педали тормоза для отключения режима прогрева
#define PSO_Pin      A3                     // на прочие датчики через делитель 39 kOhm / 11 kΩ
#define RESET_Pin    5
/*  ----------------------------------------- НАСТРОЙКИ MQTT брокера---------------------------------------------------------   */
const char MQTT_user[10] = "qojokqry";      // api.cloudmqtt.com > Details > User  
const char MQTT_pass[15] = "lY1j3Yg60nes";  // api.cloudmqtt.com > Details > Password
const char MQTT_type[15] = "MQIsdp";        // тип протокола
const char MQTT_CID[15] = "CITROEN";        // уникальное имя устройства в сети MQTT
String MQTT_SERVER = "m23.cloudmqtt.com";   // api.cloudmqtt.com > Details > Server  сервер MQTT брокера
String PORT = "10077";                      // api.cloudmqtt.com > Details > Port    порт MQTT брокера
/*  ----------------------------------------- ИНДИВИДУАЛЬНЫЕ НАСТРОЙКИ !!!---------------------------------------------------------   */
String call_phone= "+375295912507";         // телефон входящего вызова  
String SMS_phone = "+375291853337";         // телефон куда отправляем СМС 
String APN = "internet.mts.by";             // тчка доступа выхода в интернет вашего сотового оператора
String USER = "mts";                        // имя выхода в интернет вашего сотового оператора
String PASS = "mts";                        // пароль доступа выхода в интернет вашего сотового оператора

/*  ----------------------------------------- ДАЛЕЕ НЕ ТРОГАЕМ ---------------------------------------------------------------   */
float Vstart = 12.80;                        // порог распознавания момента запуска по напряжению
String pin = "";                            // строковая переменная набираемого пинкода 
float TempDS[11];                           // массив хранения температуры c рахных датчиков 
float Vbat,V_min;                                 // переменная хранящая напряжение бортовой сети
float m = 69.91;                            // делитель для перевода АЦП в вольты для резистров 39/11kOm
unsigned long Time1, Time2 = 0;
int Timer, inDS, count = 0;
int interval = 3;                           // интервал тправки данных на сервер после загрузки ардуино
bool heating = false;                       // переменная состояния режим прогрева двигателя
bool ring = false;                          // флаг момента снятия трубки
bool broker = false;                        // статус подклюлючения к брокеру
bool Security = false;                      // состояние охраны после подачи питания

void setup() {
 // pinMode(RESET_Pin, OUTPUT);             // указываем пин на выход для перезагрузки модема
  pinMode(FIRST_P_Pin, OUTPUT);             // указываем пин на выход для доп реле первого положения замка зажигания
  pinMode(SECOND_P,    OUTPUT);             // указываем пин на выход доп реле зажигания
  pinMode(STARTER_Pin, OUTPUT);             // указываем пин на выход доп реле стартера
  pinMode(Lock_Pin,    OUTPUT);             // указываем пин на выход для реле на кнопку "заблокировать дверь"
  pinMode(Unlock_Pin,  OUTPUT);             // указываем пин на выход для реле на кнопку "раззаблокировать дверь"
  pinMode(LED_Pin,     OUTPUT);             // указываем пин на выход (светодиод)
 // pinMode(3, INPUT_PULLUP);                 // указываем пин на вход для внешних прерываний всевозможных датчиков
 // pinMode(2, INPUT_PULLUP);                 // указываем пин на вход для внешних прерываний всевозможных датчиков
  delay(100); 
  Serial.begin(9600);                       //скорость порта
//  Serial.setTimeout(50);
  
  SIM800.begin(9600);                       //скорость связи с модемом
 // SIM800.setTimeout(500);                 // тайм аут ожидания ответа
  
  Serial.println("MQTT |07/04/2018"); 
  delay (1000);
  SIM800_reset();
 
 // attachInterrupt(0, callback, FALLING);  // включаем прерывание при переходе 1 -> 0 на D2, или 0 -> 1 на ножке оптопары
 // attachInterrupt(1, callback, FALLING);  // включаем прерывание при переходе 1 -> 0 на D3, или 0 -> 1 на ножке оптопары
              }



void loop() {

if (SIM800.available())  resp_modem();                                    // если что-то пришло от SIM800 в Ардуино отправляем для разбора
//if (Serial.available())  resp_serial();                                 // если что-то пришло от Ардуино отправляем в SIM800
if (millis()> Time2 + 60000) {Time2 = millis(); 
    if (Timer > 0 ) Timer--, Serial.print("Таймер прогрева:"), Serial.println (Timer);}
                                               
if (millis()> Time1 + 10000) Time1 = millis(), detection();               // выполняем функцию detection () каждые 10 сек 
if (heating == true && digitalRead(STOP_Pin)==1) heatingstop();           // если нажали на педаль тормоза в режиме прогрева
            }

/*  --------------------------------------------------- Перезагрузка МОДЕМА SIM800L ------------------------------------------------ */ 
void SIM800_reset() {  
    SIM800.println("AT+CFUN=1,1");}   // програмная перезагрузка модема 

void callback(){                                                  // обратный звонок при появлении напряжения на входе IN1
    SIM800.println("ATD"+call_phone+";"),    delay(5000);} 

void enginestart() {                                              // программа запуска двигателя
 /*  ----------------------------------------- ПРЕДНАСТРОЙКА ПЕРЕД ЗАПУСКОМ ---------------------------------------------------------*/
// detachInterrupt(1);                                            // отключаем аппаратное прерывание, что бы не мешало запуску
   Serial.println("Предпусковая настройка");
   Timer = 5;                                                     // устанавливаем таймер на 5 минут 
 /*------- Если напряжение АКБ меньше напряжения работающего двигателя, зажигание выключено то  ----------*/  
if (/*VoltRead() < Vstart &&*/ digitalRead(Feedback_Pin) == LOW)
           {
           int StTime  = map(TempDS[0], 20, -15, 700, 5000);     // Задаем время работы стартера
           StTime  = constrain(StTime, 700, 6000);               // ограничиваем диапазон работы стартера от 0,7 до 6 сек

           digitalWrite(FIRST_P_Pin, HIGH),   delay (1000);       // включаем реле первого положения замка зажигания 
           digitalWrite(SECOND_P,    HIGH),   delay (4000);       // включаем зажигание, и выжидаем 4 сек.

/*         int z = map(TempDS[0], 0, -25, 0, 5);                  // задаем количество раз прогрева свечей для дизелей
           z = constrain(z, 0, 5);                                // огрничиваем попытки от 0 до 5 попыток
           while (z > 0) digitalWrite(SECOND_P, LOW), delay(2000), digitalWrite(SECOND_P, HIGH), delay(8000); */

/*  --------------------крутим стартером и ждем если на STOP_Pin (А2) низкий уровень ------------------------------   */
            if (digitalRead(STOP_Pin) == LOW) {Serial.println("Стартер ВКЛ."), digitalWrite(STARTER_Pin, HIGH),  delay(StTime);
                                    digitalWrite(STARTER_Pin, LOW), Serial.println("Стартер Откл. ждем 6 сек."), delay (6000);}
            }
/*  --------------------проверяем по пороговому напряжению запустился ли двигатель ------------------------------   */
if (VoltRead() > Vstart){Serial.println ("Двигатель запущен"), heating = true;} else heatingstop(); 

Serial.println ("Выход из запуска"), interval = 1;
        
//delay(3000), SIM800.println("ATH0");                           // вешаем трубку (для SIM800L) 
//attachInterrupt(1, callback, FALLING);                         // включаем прерывание на обратный звонок
 }


float VoltRead()    {                               // замеряем напряжение на батарее и переводим значения в вольты
              float ADCC = analogRead(BAT_Pin);
                    ADCC = ADCC / m ;
                    Serial.print("Напряжение: "), Serial.print(ADCC), Serial.println("V");    
                    if (ADCC < V_min) V_min = ADCC;                   
                    return(ADCC); }                  // переводим попугаи в вольты
                    


void heatingstop() {                                // программа остановки прогрева двигателя
    digitalWrite(SECOND_P,    LOW), delay (100);
    digitalWrite(FIRST_P_Pin, LOW), delay (100);
    heating= false, Timer = 0;
    Serial.println ("Выключить все реле"); }

void detection(){                                                 // условия проверяемые каждые 10 сек  
    
    Vbat = VoltRead();                                            // замеряем напряжение на батарее
    Serial.print("Интервал: "), Serial.println(interval);
    inDS = 0;
    sensors.requestTemperatures();                                // читаем температуру с трех датчиков
    while (inDS < 10){
          TempDS[inDS] = sensors.getTempCByIndex(inDS);           // читаем температуру
      if (TempDS[inDS] == -127.00){TempDS[inDS]= 80;
                                   break; }                       // пока не доберемся до неподключенного датчика
              inDS++;} 
              
    for (int i=0; i < inDS; i++) Serial.print("Temp"), Serial.print(i), Serial.print("= "), Serial.println(TempDS[i]); 
    Serial.println ("");
    
if (heating == true && Timer <1)    heatingstop();      // остановка прогрева если закончился отсчет таймера

    interval--;
    if (interval <1) { interval = 6; 
        if (broker == true) { SIM800.println("AT+CIPSEND"), delay (200);  
                              MQTT_FloatPub ("C5/ds0",      TempDS[0],2);
                              MQTT_FloatPub ("C5/ds1",      TempDS[1],2);
                              MQTT_FloatPub ("C5/vbat",     Vbat,2);
                              MQTT_FloatPub ("C5/timer",    Timer,0);
                              MQTT_PUB      ("C5/security", Security ? "lock1" : "lock0");
                              MQTT_PUB      ("C5/engine",   heating ? "start" : "stop");
                              MQTT_FloatPub ("C5/engine",   heating,0);
                              MQTT_FloatPub ("C5/uptime",   millis()/60000,0); 
                              SIM800.write(0x1A); 
                              
    } else  SIM800.println ("AT+SAPBR=3,1, \"Contype\",\"GPRS\""), delay (200);    // подключаемся к GPRS 
                     }  
}  



/*
void resp_serial (){     // ---------------- ТРАНСЛИРУЕМ КОМАНДЫ из ПОРТА В МОДЕМ ----------------------------------
     String at = "";   
 //    while (Serial.available()) at = Serial.readString();
  int k = 0;
   while (Serial.available()) k = Serial.read(),at += char(k),delay(1);
     SIM800.println(at), at = "";   }  */





void  MQTT_FloatPub (const char topic[15], float val, int x) { // топик, переменная в float, количество знаков после точки
           char st[10];
           dtostrf(val,0, x, st), MQTT_PUB (topic, st);      }

void MQTT_CONNECT () {
  SIM800.println("AT+CIPSEND"), delay (100);
     
  SIM800.write(0x10);                                                              // маркер пакета на установку соединения
  SIM800.write(strlen(MQTT_type)+strlen(MQTT_CID)+strlen(MQTT_user)+strlen(MQTT_pass)+12);
  SIM800.write((byte)0),SIM800.write(strlen(MQTT_type)),SIM800.write(MQTT_type);   // тип протокола
  SIM800.write(0x03), SIM800.write(0xC2),SIM800.write((byte)0),SIM800.write(0x3C); // просто так нужно
  SIM800.write((byte)0), SIM800.write(strlen(MQTT_CID)),  SIM800.write(MQTT_CID);  // MQTT  идентификатор устройства
  SIM800.write((byte)0), SIM800.write(strlen(MQTT_user)), SIM800.write(MQTT_user); // MQTT логин
  SIM800.write((byte)0), SIM800.write(strlen(MQTT_pass)), SIM800.write(MQTT_pass); // MQTT пароль

  MQTT_PUB ("C5/status", "Подключено");                                            // пакет публикации
  MQTT_SUB ("C5/comand");                                                          // пакет подписки на присылаемые команды
  MQTT_SUB ("C5/settimer");                                                        // пакет подписки на присылаемые значения таймера
  SIM800.write(0x1A),  broker = true;    }                                         // маркер завершения пакета

void  MQTT_PUB (const char MQTT_topic[15], const char MQTT_messege[15]) {          // пакет на публикацию

  SIM800.write(0x30), SIM800.write(strlen(MQTT_topic)+strlen(MQTT_messege)+2);
  SIM800.write((byte)0), SIM800.write(strlen(MQTT_topic)), SIM800.write(MQTT_topic); // топик
  SIM800.write(MQTT_messege);   }                                                  // сообщение

void  MQTT_SUB (const char MQTT_topic[15]) {                                       // пакет подписки на топик
  
  SIM800.write(0x82), SIM800.write(strlen(MQTT_topic)+5);                          // сумма пакета 
  SIM800.write((byte)0), SIM800.write(0x01), SIM800.write((byte)0);                // просто так нужно
  SIM800.write(strlen(MQTT_topic)), SIM800.write(MQTT_topic);                      // топик
  SIM800.write((byte)0);  }                          

void resp_modem (){     //------------------ АНЛИЗИРУЕМ БУФЕР ВИРТУАЛЬНОГО ПОРТА МОДЕМА------------------------------
     String at = "";
 //    while (SIM800.available()) at = SIM800.readString();  // набиваем в переменную at
  int k = 0;
   while (SIM800.available()) k = SIM800.read(),at += char(k),delay(1);           
   Serial.println(at);  
 
      if (at.indexOf("+CLIP: \""+call_phone+"\",") > -1  && at.indexOf("+CMGR:") == -1 ) {delay(200), SIM800.println("ATA"), ring = true;
      } else if (at.indexOf("+DTMF: ")  > -1)        {String key = at.substring(at.indexOf("")+9, at.indexOf("")+10);
                                                     pin = pin + key;
                                                     if (pin.indexOf("*") > -1 ) pin= ""; 
      } else if (at.indexOf("SMS Ready") > -1 || at.indexOf("NO CARRIER") > -1 ) {SIM800.println("AT+CLIP=1;+DDET=1"); // Активируем АОН и декодер DTMF
/*  -------------------------------------- проверяем соеденеиние с ИНТЕРНЕТ, конектимся к серверу------------------------------------------------------- */
      } else if  (at.indexOf("AT+SAPBR=3,1, \"Contype\",\"GPRS\"\r\r\nOK") > -1 ) {SIM800.println("AT+SAPBR=3,1, \"APN\",\""+APN+"\""), delay (500); 
      } else if (at.indexOf("AT+SAPBR=3,1, \"APN\",\""+APN+"\"\r\r\nOK") > -1 )   {SIM800.println("AT+SAPBR=1,1"), delay (1000); // устанавливаем соеденение   
      } else if (at.indexOf("AT+SAPBR=1,1\r\r\nOK") > -1 )                        {SIM800.println("AT+SAPBR=2,1"), delay (1000); // проверяем статус соединения  
      } else if (at.indexOf("+SAPBR: 1,1") > -1 )    {delay (200),  SIM800.println("AT+CIPSTART=\"TCP\",\""+MQTT_SERVER+"\",\""+PORT+"\""), delay (1000);
      } else if (at.indexOf("+CME ERROR:") > -1 )    {broker = false, delay (50), SIM800.println("AT+CFUN=1,1"), delay (1000), interval = 6 ;

 
   } else if (at.indexOf("C5/comandlock1",4) > -1 )      {blocking(1);        // команда постановки на охрану       
   } else if (at.indexOf("C5/comandlock0",4) > -1 )      {blocking(0);        // команда снятия с хораны
   } else if (at.indexOf("C5/settimer",4) > -1 )         {Timer = at.substring(at.indexOf("")+15, at.indexOf("")+18).toInt();
   } else if (at.indexOf("C5/comandstop",4) > -1 )       {heatingstop();     // команда остановки прогрева
   } else if (at.indexOf("C5/comandstart",4) > -1 )      {enginestart();    // команда запуска прогрева
   } else if (at.indexOf("C5/comandRefresh",4) > -1 )    {// Serial.println ("Команда обнвления");
                                                          SIM800.println("AT+CIPSEND"), delay (200);  
                                                          MQTT_FloatPub ("C5/ds0",      TempDS[0],2);
                                                          MQTT_FloatPub ("C5/ds1",      TempDS[1],2);
                                                          MQTT_FloatPub ("C5/vbat",     Vbat,2);
                                                          MQTT_FloatPub ("C5/timer",    Timer,0);
                                                          MQTT_PUB      ("C5/security", Security ? "lock1" : "lock0");
                                                          MQTT_PUB      ("C5/engine",   heating ? "start" : "stop");
                                                          MQTT_FloatPub ("C5/uptime",   millis()/60000,0); 
                                                          SIM800.write(0x1A); 
                                                          interval = 6; // швырнуть данные на сервер и ждать 60 сек
            
   } else if (at.indexOf("CONNECT OK\r\n") > -1 ) MQTT_CONNECT (); // после соединения с сервером отправляем пакет авторизации, публикации и пдписки у брокера
   at = "";                                                        // Возвращаем ответ можема в монитор порта , очищаем переменную

       if (pin.indexOf("123") > -1 ){ pin= "", /* Voice(2),*/ enginestart();  
} else if (pin.indexOf("789") > -1 ){ pin= "", /* Voice(10),*/ delay(1500), SIM800.println("ATH0"),heatingstop();  
} else if (pin.indexOf("#")   > -1 ){ pin= "", SIM800.println("ATH0");    }
/*if (ring == true) { ring = false, delay (2000), pin= ""; // обнуляем пин
                    if (heating == false){ Voice(1);
                                    }else Voice(8); } */    
                               
 } 
// функция дергания реле блокировки/разблокировки дверей с паузой "удержания кнопки" в 0,5 сек.
void blocking (bool st) {digitalWrite(st ? Lock_Pin : Unlock_Pin, HIGH), delay(500), digitalWrite(st ? Lock_Pin : Unlock_Pin, LOW), Security = st, Serial.println(st ? "На охране":"Открыто");}


