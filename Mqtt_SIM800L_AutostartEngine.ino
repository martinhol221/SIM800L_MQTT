//голый запуск для теста

unsigned long Time1,tms,tms2,tms3,tms4, StarterTimeON = 0;
int steps = 0;
int StTime = 3000;
int z,zh,count = 0 ; 
float Vstart = 16.50;              // поорог распознавания момента запуска по напряжению
float Vbat, VbatStart, V_min ;     // переменная хранящая напряжение бортовой сети
float m = 69.91;                   // делитель для перевода АЦП в вольты для резистров 39/11kOm
#define BAT_Pin      A0             // на батарею, через делитель напряжения 39кОм / 11 кОм
#define Feedback_Pin A1             // на провод от замка зажигания
#define STOP_Pin     A2             // на концевик педали тормоза для отключения режима прогрева
#define FIRST_P_Pin  8              // на реле первого положения замка зажигания
#define SECOND_P     9              // на реле зажигания, через транзистор с 9-го пина ардуино
#define STARTER_Pin  12             // на реле стартера, через транзистор с 12-го пина ардуино
int Timer = 0;
int Attempts = 2;                   // число задаваемых попыток запуска
float TempDS = -12; 



void setup() {
  Serial.begin(9600);
  pinMode(FIRST_P_Pin, OUTPUT);    // указываем пин на выход для доп реле первого положения замка зажигания
  pinMode(SECOND_P,    OUTPUT);    // указываем пин на выход доп реле зажигания
  pinMode(STARTER_Pin, OUTPUT);    // указываем пин на выход доп реле стартера

  SETUPSTART ();
  delay(20);
}

void loop() {
//if (steps == 1 && millis()> tms + 1000) steps = 2, ACC(1),        tms = millis();  // включаем потребители 
     if (steps == 2 && millis()> tms + 1000)   steps = 3, ACC(1),        tms = millis();  // шаг 2 включаем потребители 
else if (steps == 3 && millis()> tms + 1000)   steps = 4, ING(0),        tms = millis();  // шаг 3 выключаем зажигание      
else if (steps == 4 && millis()> tms + 4000)   steps = 5, ING(1),        tms = millis();  // шаг 4 включаем зажигание 
else if (steps == 5 && millis()> tms + 10000)  steps = 6, CANDLES (),    tms = millis();  // шаг 5 проверяем нагрев свечей 
else if (steps == 6 && millis()> tms + 2000)   steps = 7, ROTATION(),    tms = millis();  // шаг 6 крутим стартером
else if (steps == 7 && millis()> tms + 6000)   steps = 8, DETECT(),      tms = millis();  // шаг 7 детектируем 
else if (steps == 8 && millis()> tms + 2000)   steps = 3, STOP(),        tms = millis();  // шаг 8 отключаем все                   
else if (steps == 9 && millis()> tms2 + 10000)        TIMERSTEP(),      tms2 = millis();  // шаг 9 таймер прогрева   

            }

void ACC (bool st)     {digitalWrite(FIRST_P_Pin, st ? HIGH:LOW), Serial.print("Потребители "), Serial.println(st ? "ВКЛ.":"ОТКЛ.");} 
void ING (bool st)     {digitalWrite(SECOND_P, st ? HIGH:LOW),    Serial.print("Зажигание "),   Serial.println(st ? "ВКЛ.":"ОТКЛ.");} 
void STARTER (bool st) {digitalWrite(STARTER_Pin, st ? HIGH:LOW), Serial.print("Стартер "),     Serial.println(st ? "ВКЛ.":"ОТКЛ.");} 
void ROTATION ()       {if(!NEUTRAL())  {STARTER(1), delay (StTime + 500 * count), STARTER(0), count++;}    else {STOP(), steps=0;} }
void DETECT()          {if(VoltRead()>Vstart) steps=9, Timer=map(TempDS,30,-25,30,150), Timer=constrain(Timer,30,180);}
void STOP()            {Serial.println("Стоп"), ING(0), ACC(0), zh = z; ;  if (count > Attempts) {Timer=0, steps = 0;}}
void TIMERSTEP ()      {if(Timer>0) {Timer--, Serial.print("Таймер "), Serial.println(Timer);} else {STOP(), steps = 0;}} 
void CANDLES ()        {Serial.print("Свечи "), Serial.println(zh); if (zh>0){zh--, steps=3; }}   
bool NEUTRAL ()        { return(digitalRead(STOP_Pin)); }
void SETUPSTART ()     {Serial.println("Предпусковая настройка");
     count = 0;
   if (/*VoltRead() > 10.00 && digitalRead(Feedback_Pin) == LOW  && TempDS[0] > -30 && */ count < Attempts){  
  //   VbatStart = VoltRead();
     StTime  = map(TempDS, 20, -15, 1000, 5000);         // при -15 крутим стартером 5 сек, при +20 крутим 1 сек
     StTime  = constrain(StTime, 1000, 6000);            // ограничиваем диапазон работы стартера от 1 до 6 сек
     z = map(TempDS, 0, -25, 0, 5);                      // задаем количество раз прогрева свечей пропорциолально температуре
     z = constrain(z, 0, 5);                             // огрничиваем попытки от 0 до 5 попыток
     zh = z;
     V_min = 14;                                         // переменная хранящая минимальные напряжения в ммент старта
     steps = 2;} 

}



float VoltRead()    {                                       // замеряем напряжение на батарее и переводим значения в вольты
              float ADCC = analogRead(BAT_Pin); 
                    ADCC = ADCC / m ;
                    Serial.print("Напряжение: "), Serial.print(ADCC), Serial.println("V");    
                    if (ADCC < V_min) V_min = ADCC;                   
                    return(ADCC); }                         // переводим попугаи в вольты
                    
