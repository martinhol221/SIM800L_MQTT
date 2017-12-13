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

`const char MQTT_pass[15] = "lXXXXXXXXXXs";`  Password   

`const char MQTT_CID[15] = "AAAAAA";`        имя устройства, придумать

`String MQTT_SERVER = "m23.cloudmqtt.com"; `  Server

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
