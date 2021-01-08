# SmartCoffeeBar
Firmware for an ESP8266 to control a coffee machine for integration with the MyBar.io system

In late 2020, a MyBar.io system was purchased and successfully installed. It's inspired a bigger vision of a wholly automated beverage ordering, generation and delivery system.

Staying safe at home during the Covid-19 pandemic, automated services that reduce human contact with food/beverage items are all the more important. 2020 is also a year of increased access to IoT technology. Therefore, the aim of this project and it's associated code/repositories is to use low-cost, easily sourced IoT components and integrate them with both the MyBar system and other home beverage appliances which are priced well. 

The SmartCoffeeBar repository will focus on the coffee generation and dispensing component to the Automatic Drink System (ADS), as well as potentially other code that drives a linear stage that will move a generated coffee beverage from one linear position on a table to another for dispensing of additional components from the MyBar.io. All of the code for this repo will run on an ESP32866 WiFi board with Web server based API endpoints that return JSON formatted data.

The MyBar.io uses BLE for making API calls to it's board, so I may in time get a BLE add on for the ESP8266 or move to an Arduino based platform that has both WiFi and BLE. The overall system logic for calling out to different components may be either at the app level (and this project will be iOS Swift based app) or a main controller board that delegates out various tasks to the other controller boards in the overall system. I'm thinking the overall system logic will probably be on a main controller board, but this is a system design parameter I may experiment with to see which arrangement has the least latency (iOS app based system controller or main controller board controller).

------ Some info on the MyBar.io Board API, which I've also included in the Readme for the MyBar firmware repo fork that is on the same project as this repo -------
The ESP32 gets a command over Bluetooth with number of pump, direction and duration on milliseconds: "3-f-3000"
Directions are: f - forward, b - backwards and s -  stop
1 oz of beverage is served by specifying a duration value of 10000 ms

To prime the pumps, send 6000 forward to every pump

To clean the pumps, it is a 3-cycle process:
1 - return all liquid to the bottles 10000 backwards for each pump
2 and 3 run forward 10000 for each pump
There is a pause with a dialogue to explain that you need to remove the tubes and probably empty the glass

----- Hopper Idea -----
Would need to be redesigned for the Vertuo pod form factor

https://www.yoctopuce.com/EN/article/an-automatic-nespresso-capsule-dispenser

https://www.makexyz.com/3d-models/order/a9308e6ae05433c38fa09dafb04d1d30
