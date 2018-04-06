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
const char MQTT_user[10] = "drive2ru";      // api.cloudmqtt.com > Details > User  
const char MQTT_pass[15] = "martinhol221";  // api.cloudmqtt.com > Details > Password
const char MQTT_type[15] = "MQIsdp";        // тип протокола
const char MQTT_CID[15] = "Citroen";        // уникальное имя устройства в сети MQTT
String     MQTT_SERVER = "m23.cloudmqtt.com";   // api.cloudmqtt.com > Details > Server  сервер MQTT брокера
String     PORT = "10077";                      // api.cloudmqtt.com > Details > Port    порт MQTT брокера
/*  ----------------------------------------- ИНДИВИДУАЛЬНЫЕ НАСТРОЙКИ SIM-карты !!!-----------------------------------------   */
String call_phone= "+375290000000";         // телефон входящего вызова  
String SMS_phone = "+375290000000";         // телефон куда отправляем СМС 
String APN = "internet.mts.by";             // тчка доступа выхода в интернет вашего сотового оператора
String USER = "mts";                        // имя выхода в интернет вашего сотового оператора
String PASS = "mts";                        // пароль доступа выхода в интернет вашего сотового оператора
bool  sms_report = false;                   // отправка SMS отчета по умолчанию овключена (true), отключена (false)
/*  ----------------------------------------- ДАЛЕЕ НЕ ТРОГАЕМ ---------------------------------------------------------------   */
float Vstart = 13.50;                       // порог распознавания момента запуска по напряжению
String pin = "";                            // строковая переменная набираемого пинкода 
float TempDS[11];                           // массив хранения температуры c рахных датчиков 
float Vbat, VbatStart, V_min ;              // переменная хранящая напряжение бортовой сети
float m = 69.91;                            // делитель для перевода АЦП в вольты для резистров 39/11kOm
unsigned long Time1, StarterTimeON = 0;
int Timer, inDS, count = 0;
int interval = 5;                           // интервал отправки данных на народмон сразу после загрузки 
//int Timer2 = 1080;                        // часовой таймер (60 sec. x 60 min. / 10 = 360 )
bool heating = false;                       // переменная состояния режим прогрева двигателя
bool SMS_send = false;                      // флаг разовой отправки СМС
bool ring = false;                          // флаг момента снятия трубки
bool  broker = false;                       // статус подклюлючения к брокеру
bool Security = false;                      // состояние охраны после подачи питания

void setup() {
 // pinMode(RESET_Pin, OUTPUT);             // указываем пин на выход для перезагрузки модема
  pinMode(FIRST_P_Pin, OUTPUT);             // указываем пин на выход для доп реле первого положения замка зажигания
  pinMode(SECOND_P,    OUTPUT);             // указываем пин на выход доп реле зажигания
  pinMode(STARTER_Pin, OUTPUT);             // указываем пин на выход доп реле стартера
  pinMode(Lock_Pin,    OUTPUT);             // указываем пин на выход для реле на кнопку "заблокировать дверь"
  pinMode(Unlock_Pin,  OUTPUT);             // указываем пин на выход для реле на кнопку "раззаблокировать дверь"
  pinMode(LED_Pin,     OUTPUT);             // указываем пин на выход (светодиод)
  pinMode(3, INPUT_PULLUP);                 // указываем пин на вход для внешних прерываний всевозможных датчиков
  pinMode(2, INPUT_PULLUP);                 // указываем пин на вход для внешних прерываний всевозможных датчиков
  delay(100); 
  Serial.begin(9600);                       //скорость порта
//  Serial.setTimeout(50);
  
  SIM800.begin(9600);                       //скорость связи с модемом
 // SIM800.setTimeout(500);                 // тайм аут ожидания ответа
  
  Serial.println("MQTT |06/04/2018"); 
  delay (1000);
  SIM800_reset();
 
 // attachInterrupt(0, callback, FALLING);  // включаем прерывание при переходе 1 -> 0 на D2, или 0 -> 1 на ножке оптопары
 // attachInterrupt(1, callback, FALLING);  // включаем прерывание при переходе 1 -> 0 на D3, или 0 -> 1 на ножке оптопары
              }



void loop() {

if (SIM800.available())  resp_modem();                            // если что-то пришло от SIM800 в Ардуино отправляем для разбора
//if (Serial.available())  resp_serial();                           // если что-то пришло от Ардуино отправляем в SIM800
if (millis()> Time1 + 10000) Time1 = millis(), detection();       // выполняем функцию detection () каждые 10 сек 
if (heating == true && digitalRead(STOP_Pin)==1) heatingstop();   // если нажали на педаль тормоза в режиме прогрева
            }

/*  --------------------------------------------------- Перезагрузка МОДЕМА SIM800L ------------------------------------------------ */ 
void SIM800_reset() {  
SIM800.println("AT+CFUN=1,1");   // програмная перезагрузка модема 
// delay(2000); SIM800.println("AT+CMGDA=\"DEL ALL\"");  // Удаляем все СМС
//SIM800.println("AT+IPR=9600;E1+CMGF=1;+CSCS=\"gsm\";+CNMI=2,1,0,0,0;+VTD=1;+CMEE=1;&W"); 
//digitalWrite(RESET_Pin, LOW),delay(2000), digitalWrite(RESET_Pin, HIGH), delay(5000);
}

void callback(){                                                  // обратный звонок при появлении напряжения на входе IN1
    SIM800.println("ATD"+call_phone+";"),    delay(5000);} 

void enginestart(int Attempts ) {                                      // программа запуска двигателя
 /*  ----------------------------------------- ПРЕДНАСТРОЙКА ПЕРЕД ЗАПУСКОМ ---------------------------------------------------------*/
//Serial.println("Предпусковая настройка");
  MQTT_PUB ("C5/status", "Запуск");
    detachInterrupt(1);                                    // отключаем аппаратное прерывание, что бы не мешало запуску
    VbatStart = VoltRead();

int StTime  = map(TempDS[0], 20, -15, 1000, 5000);         // при -15 крутим стартером 5 сек, при +20 крутим 1 сек
    StTime  = constrain(StTime, 1000, 6000);               // ограничиваем диапазон работы стартера от 1 до 6 сек

int z = map(TempDS[0], 0, -25, 0, 5);                     // задаем количество раз прогрева свечей пропорциолально температуре
    z = constrain(z, 0, 5);                                // огрничиваем попытки от 0 до 5 попыток

    Timer   =  map(TempDS[0], 30, -25, 30, 150);           // при -25 крутим прогреваем 30 мин, при +50 - 5 мин
    Timer   =  constrain(Timer, 30, 180);                  // ограничиваем таймер в зачениях от 5 до 30 минут

    count = 0;                                             // переменная хранящая число совершенных попыток запуска
    V_min = 14;                                            // переменная хранящая минимальные напряжения в ммент старта

// если напряжение АКБ больше 10 вольт, зажигание выключено, температуры выше -25 град, счетчик числа попыток  меньше 5
 while (Vbat > 10.00 && digitalRead(Feedback_Pin) == LOW && TempDS[0] > -30 && count < Attempts){ 
 
 count++, Serial.print ("Попытка №"), Serial.println(count); 
    
 digitalWrite(SECOND_P,     LOW),   delay (2000);        // выключаем зажигание на 2 сек. на всякий случай   
 digitalWrite(FIRST_P_Pin, HIGH),   delay (1000);        // включаем реле первого положения замка зажигания 
// Voice(3);
 digitalWrite(SECOND_P,    HIGH),   delay (4000);        // включаем зажигание, и выжидаем 4 сек.
 
// прогреваем свечи несколько раз пропорционально понижению температуры, греем по 8 сек. с паузой 2 сек.
while (z > 0) /* Voice(3),*/ digitalWrite(SECOND_P, LOW), delay(2000), digitalWrite(SECOND_P, HIGH), delay(8000);
 
// если не нажата педаль тормоза или КПП в нейтрали то включаем реле стартера на время StTime
 if (digitalRead(STOP_Pin) == LOW) {         // в нейтрали на минус
// if (digitalRead(STOP_Pin) == HIGH) {      // в нейтрали на плюс +12
                                   Serial.println("Стартер включил");
                               //    Voice(4);
                                   MQTT_PUB ("C5/status", "Стартер Вкл");
                                   StarterTimeON = millis();
                                   digitalWrite(STARTER_Pin, HIGH);  // включаем реле стартера
                                   } else {
                               //    Voice(7); 
                                   heatingstop();
                                   Timer = 0;
                                   count = -1;  
                                   break; 
                                   } 
 delay (100);
//float V_stON = VoltRead();                              // временно так
 while ((millis() < (StarterTimeON + StTime)) /* && ((VoltRead() + V_stOFF) < V_stON)*/)VoltRead(), delay (50);
 digitalWrite(STARTER_Pin, LOW), delay (1000);
 MQTT_PUB ("C5/status", "Стартер Откл");
 //digitalWrite(FIRST_P_Pin, HIGH),           // включаем реле первого положения замка зажигания   
 Serial.println("Стартер выключил, ожидаем 6 сек.");
 
// Voice(12);
 delay (6000);       

// if (digitalRead(DDM_Pin) != LOW) {                // если детектировать по датчику давления масла 

 if (VoltRead() > Vstart) {                          // если детектировать по напряжению зарядки     
                          Serial.println ("Есть запуск!"); 
                       //   Voice(5);
                          heating = true;
                          digitalWrite(LED_Pin, HIGH);
                          break; }                   // считаем старт успешным, выхдим из цикла запуска двигателя
                          
//  Voice(6);              
  StTime = StTime + 200;                             // увеличиваем время следующего старта на 0.2 сек.
  heatingstop();   }                                 // уменьшаем на еденицу число оставшихся потыток запуска
                  
Serial.println ("Выход из запуска");
 if (count != 1) SMS_send = true;                   // отправляем смс СРАЗУ только в случае незапуска c первой попытки
 if (heating == false){ Timer = 0/*,  Voice(10)*/;                  // обнуляем таймер если запуска не произошло
           } else digitalWrite(Unlock_Pin, HIGH);  // включаем подогрев седений если все ОК


           
//delay(3000), SIM800.println("ATH0");                            // вешаем трубку (для SIM800L) 
attachInterrupt(1, callback, FALLING);          // включаем прерывание на обратный звонок
 }


float VoltRead()    {                               // замеряем напряжение на батарее и переводим значения в вольты
              float ADCC = analogRead(BAT_Pin);
                    ADCC = ADCC / m ;
                    Serial.print("Напряжение: "), Serial.print(ADCC), Serial.println("V");    
                    if (ADCC < V_min) V_min = ADCC;                   
                    return(ADCC); }                  // переводим попугаи в вольты
                    


void heatingstop() {                                // программа остановки прогрева двигателя
    digitalWrite(SECOND_P,    LOW), delay (100);
    digitalWrite(LED_Pin,     LOW), delay (100);
    digitalWrite(FIRST_P_Pin, LOW), delay (100);
    heating= false,                 delay(2000); 
    Serial.println ("Выключить все"); }
/*
void Voice(int Track){
    SIM800.print("AT+CREC=4,\"C:\\User\\"), SIM800.print(Track), SIM800.println(".amr\",0,95");}             

*/

void detection(){                                                 // условия проверяемые каждые 10 сек  
    
    Vbat = VoltRead();                                            // замеряем напряжение на батарее
    Serial.print("|int: "), Serial.print(interval);
    Serial.print("||Timer:"), Serial.println (Timer);
    
    inDS = 0;
    sensors.requestTemperatures();                                // читаем температуру с трех датчиков
    while (inDS < 10){
          TempDS[inDS] = sensors.getTempCByIndex(inDS);           // читаем температуру
      if (TempDS[inDS] == -127.00){TempDS[inDS]= 80;
                                   break; }                       // пока не доберемся до неподключенного датчика
              inDS++;} 
              
    for (int i=0; i < inDS; i++) Serial.print("Temp"), Serial.print(i), Serial.print("= "), Serial.println(TempDS[i]); 
    
    Serial.println ("");

  
    if (SMS_send == true && sms_report == true) { SMS_send = false;          // если фаг SMS_send равен 1 высылаем отчет по СМС
        SIM800.println("AT+CMGS=\""+SMS_phone+"\""), delay(100);
        SIM800.print("Privet !");
        for (int i=0; i < inDS; i++)          SIM800.print("\n Temp"), SIM800.print(i), SIM800.print(": "), SIM800.print(TempDS[i]);
        SIM800.print("\n Voltage Now: "), SIM800.print(Vbat),               SIM800.print("V.");
        SIM800.print("\n Voltage Min: "), SIM800.print(V_min),              SIM800.print("V.");
        if (digitalRead(Feedback_Pin) == HIGH)  SIM800.print("\n Voltage for start: "), SIM800.print(VbatStart), SIM800.print("V.");
        if (heating == true)             SIM800.print("\n Timer "),             SIM800.print(Timer/6),   SIM800.print("min.");
        SIM800.print("\n Attempts:"), SIM800.print(count);
        SIM800.print("\n Uptime: "),     SIM800.print(millis()/3600000),        SIM800.print("H.");
      //  SIM800.print("\n https://www.google.com/maps/place/"), SIM800.print(LAT), SIM800.print(","), SIM800.print(LNG);
        SIM800.print((char)26);  }
   
    if (Timer == 12 ) SMS_send = true; 
    if (Timer > 0 ) Timer--;                                // если таймер больше ноля  SMS_send = true;
    if (heating == true && Timer <1)    heatingstop();      // остановка прогрева если закончился отсчет таймера
    if (heating == true && Vbat < 11.0 ) heatingstop();     // остановка прогрева если напряжение просело ниже 11 вольт 
    if (heating == true && TempDS[0] > 86) heatingstop();   // остановка прогрева если температура достигла 86 град 
    if (heating == false) digitalWrite(LED_Pin, HIGH), delay (50), digitalWrite(LED_Pin, LOW);  // моргнем светодиодом  
    //  Автозапуск при понижении температуры ниже -18 градусов, при -25 смс оповещение каждых 3 часа
//  if (Timer2 == 2 && TempDS[0] < -18) Timer2 = 1080, Timer = 120, enginestart(3);  
//     Timer2--;                                            // вычитаем еденицу
//  if (Timer2 < 0) Timer2 = 1080;                          // продлеваем таймер на 3 часа (60x60x3/10 = 1080)

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
   } else if (at.indexOf("C5/comandstart",4) > -1 )      {enginestart(2);    // команда запуска прогрева
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

       if (pin.indexOf("123") > -1 ){ pin= "", /* Voice(2),*/ enginestart(3);  
} else if (pin.indexOf("789") > -1 ){ pin= "", /* Voice(10),*/ delay(1500), SIM800.println("ATH0"),  Timer=0, heatingstop();  
} else if (pin.indexOf("#")   > -1 ){ pin= "", SIM800.println("ATH0"), SMS_send = true;    }
/*if (ring == true) { ring = false, delay (2000), pin= ""; // обнуляем пин
                    if (heating == false){ Voice(1);
                                    }else Voice(8); } */    
                               
 } 
// функция дергания реле блокировки/разблокировки дверей с паузой "удержания кнопки" в 0,5 сек.
void blocking (bool st) {digitalWrite(st ? Lock_Pin : Unlock_Pin, HIGH), delay(500), digitalWrite(st ? Lock_Pin : Unlock_Pin, LOW), Security = st, Serial.println(st ? "На охране":"Открыто");}



