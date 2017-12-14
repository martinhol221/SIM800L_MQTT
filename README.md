# SIM800L_MQTT Management of devices on the GSM modem SIM800L and Arduino using the MQTT protocol

Тестовая передача показаний с датчиков в топики

TOPIC `C5/ds0` - Температура 1

TOPIC `C5/ds1` - Температура 2

TOPIC `C5/vbat`- Напряжение

TOPIC `C5/uptime` - Время работы

с интервалом в 60 секунд.

Регистрация у броккера cloudmqtt.com и получение настроек

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/mqtt-3.jpg)

настройки брокера в скетче

`const char MQTT_user[10] = "qXXXXXXy";`      User

`const char MQTT_pass[15] = "lXXXXXXXXXXs";`  Password   

`const char MQTT_CID[15] = "AAAAAA";`        имя устройства, придумать

`String MQTT_SERVER = "m23.cloudmqtt.com";`  Server

`String PORT = "10077";`                  

Консоль для отладки и наглядного просмотра трафика идущего от устройства

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/mqtt-4.jpg)

Загружаем приложение [MQTT Dash (IoT, Smart Home)](https://play.google.com/store/apps/details?id=net.routix.mqttdash&hl=ru)

Вводим сервер, порт, логин и пароль полученный выше. Добовляем элемент типа "Текст":

*  Имя "любое"

* Топик `C5/ds0`

* шрифт крупный

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/mqtt-6.jpg)

и т д, получится так.

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/mqtt-5.jpg)

Расход трафика с постоянно открытой сессией, 10.5 кб/час х 24 часа х 30 дней = 7.6 Мб в месяц  

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/mqtttrafic.JPG)


# Порядок передачи данных


## Подключение к броккеру

`AT+SAPBR=3,1, "Contype","GPRS"` - настройка соединения, ответ `OK`

`AT+SAPBR=3,1, "APN","internet.mts.by"` - установка точки доступа сотового оператора, ответ `OK` 

`AT+SAPBR=1,1` - установка GPRS соединения, ответ `OK` 

`AT+SAPBR=2,1` - запрос состояния подключения, ответ `+SAPBR: 1,1,"100.78.6.XXX"`

`AT+CIPSTART="TCP","m23.cloudmqtt.com",\"10077"` - полключение к броккеру оквет `CONNECT OK`


## Состав пакета авторизации и подписки 

`AT+CIPSEND` - команда начала передачи данных в ASCII, это же в HEX `41 54 2B 43 49 50 53 45 4E 44    0D 0A` 

`10 2D 00 06 4D 51 49 73 64 70 03 C2 00 3C 00 07 43 49 54 52 4F 45 4E 00 08 64 72 69 76 65 32 72 75 00 0C 6D 61 72 74 69 6E 68 6F 6C 32 32 31 `

где все в HEX поюайтно

`10` маркер пакета авторизации

`2D` - общая длинна всего пакета авторизации 2D (HEX)  = 42 (DEC) байта 

`00` - постоянно

`06` - длинна типа протоколоа 06 (HEX)  = 6 (DEC) байт

`4D 51 49 73 64 70` - сам тип протокола, он же в ASCII  `MQIsdp`

`03 C2 00 3C 00` - постоянные

`07` - длина имени устройства 07 (HEX)  = 7 (DEC) байт

`43 49 54 52 4F 45 4E` - само имя устройства , он же в ASCII  `CITROEN`

`00` - постоянно

`08` - длинна логина 08 (HEX)  = 8 (DEC) байт

`64 72 69 76 65 32 72 75` - сам логин, он же в ASCII  `drive2ru`

`00` - постоянно 

`0C` - длинна пароля 0C (HEX)  = 12 (DEC) байт

`6D 61 72 74 69 6E 68 6F 6C 32 32 31`  - сам пароль, он же в ASCII  `martinhol221`



`1A` - байт окончания передачи данных
