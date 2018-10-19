# Удаленное управление запуском двигателя автомобиля через интернет с использованием протокола MQTT.

https://www.youtube.com/watch?v=HzQ7oy439Qk

[Анатомия автозапуска 5.2](https://www.drive2.ru/c/499270089304965988/)

[Само устройство останется таким же](https://github.com/martinhol221/SIM800L_DTMF_control/blob/master/README.md) изменится только прошивка и функции управления. 

![](https://github.com/martinhol221/SIM800L_DTMF_control/raw/master/img/conecting-001.JPG)

**Останется:**

* Входящий звонок, автоподъем, распознавание DTMF команд `123`,`789`,`*` и `#`

* Исходящий звонок при срабатывании датчика вибрации только при поставленной охране

* Отключение зажигания по таймеру

**Будут убраны не нужные функции:**

* Исходящее SMS сообщение

* Входящие SMS команды

* Отправка показаний датчиков на сервер narodmon.ru

* Прием команд из приложения Народмон 2017

* Автопрогрев

* Моргалка светодиодом

* Голосовое информирование о событиях в "трубку" - под вопросм


**Добавится:**

* Онлайн связь с автомобилем по интернет соединению с получением из авто данных температуры и напряжения каждых 60 сек.

* Возможность установки таймера прогрева из приложения "накруткой"

* Возможность запуска двигателя в одно касание кнопки

* Возможность снатия/постановки на охрану нажатием одной кнопки.


**Главное окно программы MQTT Dash** 
![](https://github.com/martinhol221/SIM800L_MQTT/raw/master/other/mqtt-18.jpg)


### Регистрация на https://www.cloudmqtt.com/

* зарегистрируйтесь и залогинтесь

* нажмите  **+Create New Instanct**

* Name **любое значение**

* Plan  **Cute Cat (Free)**

* Data center **EU-West-1 (Ireland)**

* Tags **любое значение**

* Нажать по **Create New Instance**

* Перейти в созданный профиль

Server

User

Password

Port

перенести значение этих параметров в  MQTT Dash, [Скачать](https://play.google.com/store/apps/details?id=net.routix.mqttdash&hl=ru), можно использовать любой другой MQTT клиент

**Настройка приложения к серверу**

![](https://github.com/martinhol221/SIM800L_MQTT/raw/master/other/mqtt-9.jpg)

**Загрузка настроек интерфейса**

Открыть в приложении ***Импорт/экспорт*** (1)

Нажать ***Подписаться и ждать метрики*** (2) 

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/setupMqtt.JPG)

В вебсокете https://api.cloudmqtt.com/console/________/websocket  заполнить  и отправить настройки в телефон

![](https://github.com/martinhol221/SIM800L_MQTT/blob/master/other/setupMqtt2.JPG)

***Topic***  metrics/exchange

***Message*** 
[{"mainTextSize":"LARGE","postfix":"°","prefix":"","textColor":-12517632,"enableIntermediateState":true,"enablePub":false,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"23.44","qos":0,"retained":false,"topic":"C5/ds0","topicPub":"","updateLastPayloadOnPub":true,"id":"2e64db41-6222-4723-90ac-929a1f16b75a","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":5,"name":"Двигатель","type":1},{"mainTextSize":"LARGE","postfix":"°","prefix":"","textColor":-16384,"enableIntermediateState":true,"enablePub":false,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"23.31","qos":0,"retained":false,"topic":"C5/ds1","topicPub":"","updateLastPayloadOnPub":true,"id":"23395be6-0d65-44c7-a661-8e50d494fc97","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":7,"name":"Салон","type":1},{"decimalPrecision":2,"displayPayloadValue":true,"maxValue":15.0,"minValue":0.0,"postfix":"V","prefix":"","progressColor":-4128769,"enableIntermediateState":true,"enablePub":false,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"13.37","qos":0,"retained":false,"topic":"C5/vbat","topicPub":"","updateLastPayloadOnPub":true,"id":"2b7ab4a9-4bfa-435e-b384-da2bf322ed75","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":11,"name":"Напряжение","type":3},{"decimalPrecision":0,"displayPayloadValue":true,"maxValue":30.0,"minValue":0.0,"postfix":"мин","prefix":"","progressColor":-192,"enableIntermediateState":true,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"2","qos":0,"retained":false,"topic":"C5/timer","topicPub":"C5/settimer","updateLastPayloadOnPub":true,"id":"03bd3e53-d473-4ab6-987f-269a25f2cce5","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":12,"name":"Таймер прогрева","type":3},{"iconOff":"ic_lock_opened_outline","iconOn":"ic_lock_closed_outline","offColor":-12517632,"onColor":-65472,"payloadOff":"lock0","payloadOn":"lock1","enableIntermediateState":true,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"lock0","qos":0,"retained":false,"topic":"C5/security","topicPub":"C5/comand","updateLastPayloadOnPub":true,"id":"6a98ecb5-c75a-47ad-bb8c-e3a83d071bdf","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":15,"name":"Охрана","type":2},{"iconOff":"ic_car","iconOn":"ic_propeller_on","offColor":-128,"onColor":-32513,"payloadOff":"stop","payloadOn":"start","enableIntermediateState":true,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"start","qos":0,"retained":false,"topic":"C5/engine","topicPub":"C5/comand","updateLastPayloadOnPub":true,"id":"d0b38071-570d-4712-9dbf-4c0dbb51459d","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":13,"name":"Двигатель","type":2},{"mainTextSize":"LARGE","postfix":"ч.","prefix":"","textColor":-64,"enableIntermediateState":true,"enablePub":false,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"0","qos":0,"retained":false,"topic":"C5/uptime","topicPub":"","updateLastPayloadOnPub":true,"id":"7700ae13-aef6-4f14-9c60-57fc1da44422","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968683,"longId":8,"name":"Время работы","type":1},{"iconOff":"ic_autorenew","iconOn":"ic_autorenew","offColor":-1,"onColor":-1,"payloadOff":"Refresh","payloadOn":"Refresh","enableIntermediateState":false,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"Refresh","qos":0,"retained":false,"topic":"C5/comand","topicPub":"","updateLastPayloadOnPub":false,"id":"f7bfd0fc-f0c1-43c9-9b5c-9621ea4e6f13","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539964863,"longId":10,"name":"Обновить данные","type":2},{"iconOff":"ic_account_balance","iconOn":"ic_account_balance","offColor":-65344,"onColor":-65344,"payloadOff":"balans","payloadOn":"balans","enableIntermediateState":false,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"balans","qos":0,"retained":false,"topic":"C5/comand","topicPub":"","updateLastPayloadOnPub":false,"id":"9fc15da1-040d-46ae-b781-26c05a329485","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968267,"longId":16,"name":"запрос баланса","type":2},{"mainTextSize":"SMALL","postfix":"","prefix":"","textColor":-1,"enableIntermediateState":true,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"start","qos":0,"retained":false,"topic":"C5/comand","topicPub":"","updateLastPayloadOnPub":true,"id":"700dbe98-03b8-42db-86e1-3211559400f8","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968488,"longId":14,"name":"Команды в машину","type":1},{"mainTextSize":"SMALL","postfix":"","prefix":"","textColor":-1,"enableIntermediateState":true,"enablePub":true,"enteredIntermediateStateAt":0,"intermediateStateTimeout":0,"jsOnReceive":"","jsonPath":"","lastPayload":"Balans\u003d1.99r ","qos":0,"retained":false,"topic":"C5/status","topicPub":"","updateLastPayloadOnPub":true,"id":"19e50ffc-1904-438a-bfbf-eb73c0c024dc","jsBlinkExpression":"","jsOnDisplay":"","jsOnTap":"","lastActivity":1539968275,"longId":17,"name":"Состояние","type":1}]


