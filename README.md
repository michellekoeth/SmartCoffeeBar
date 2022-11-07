# SmartCoffeeBar
Firmware for an ESP32 to control a coffee machine for integration with the MyBar.io system

YouTube Video of February 2021 version: https://youtu.be/5aynyWq_ZiU

Link to presentation given at Code for NoVA on June 16, 2022: https://drive.google.com/file/d/1XB2YGCqpj9WETCT1fo36XuTN03_8xNKM/view?usp=sharing

Project Vision and Inspiration:
*******************************
In late 2020, a MyBar.io system was purchased and successfully installed. It's inspired a bigger vision of a wholly automated beverage ordering, generation and delivery system.

Staying safe at home during the Covid-19 pandemic, automated services that reduce human contact with food/beverage items are all the more important. 2020 is also a year of increased access to IoT technology. Therefore, the aim of this project and it's associated code/repositories is to use low-cost, easily sourced IoT components and integrate them with both the MyBar system and other home beverage appliances which are priced well. 

The SmartCoffeeBar repository will focus on the coffee generation and dispensing component to the Automatic Drink System (ADS), as well as potentially other code that drives a linear stage that will move a generated coffee beverage from one linear position on a table to another for dispensing of additional components from the MyBar.io. All of the code for this repo will run on an ESP32 WiFi board with Web server based API endpoints that return JSON formatted data.

The dispensing and pod loading structure is all being/has been built with MakerBeam XL, a 15mmx15mm prototyping aluminum profile that is super cool, easy to work with, and has origins in open source design (love it!). https://www.makerbeam.com/makerbeamxl/ I have some other profile called "Makeblock" which was donated for the project, but it uses M4 sized screws and holes which is not really compatible with the M3 format that MakerBeam XL uses. Makeblock is also prohibitively expensive compared to MakerBeam. Nonetheless, free stuff is always good no matter what, and I made a linear stage with the Makeblock to convey a coffeecup beneath different "stations" of the whole system. Additionally, there is a "dispenserator" powder dispenser that will have its control code added in here once I have the ESP32 based ESP-NOW communication code ported over.

Stepper and servo motor sourcing has been from Amazon.com which has amateur robotics supplies at a good price. 

The MyBar.io uses BLE for making API calls to it's board, so I may in time get a BLE add on for the ESP8266 or move to an Arduino based platform that has both WiFi and BLE. The overall system logic for calling out to different components may be either at the app level (and this project will be iOS Swift based app) or a main controller board that delegates out various tasks to the other controller boards in the overall system. I'm thinking the overall system logic will probably be on a main controller board, but this is a system design parameter I may experiment with to see which arrangement has the least latency (iOS app based system controller or main controller board controller).

UPDATES:
********
June 15, 2022: The project has moved to an ESP32 based platform now, and a streamlined script that just interfaces with the Vertuo Plus coffeemachine and the pod loading mechanism has been added to the repository. This code needs only one ESP32 to run on, rather than a mesh network of ESP devices talking to each other. The mesh/network version is currently being ported over to an entirely ESP32 based system. I had to ditch using ESP8266s as there was incompatibility in the ESP-NOW based communications between the ESP8266s and the ESP32s, and it seems keeping all the microprocessors on the same ESP32 architecture makes the ESP-NOW commmunications much easier to implement.

November 2022: After updating the coffee machine's circuit board to be a all-in-one 4-ch relay and ESP32 board (https://www.amazon.com/dp/B09XWWYK4V) I have once again made the system an ESP32 mesh based system, where there are dedicated processors as follows, that all communicate to each other over ESP-NOW: 1) the coffee machine itself; 2) the pod loading mechanism; 3) the linear stage that moves the cup to different "dispenser" areas; and 4) the MyBar.io machine which is one of the dispenser areas serviced by the linear stage, and where you can add extra liquids to the brewed coffee. The special MyBar.io processor code is in a different repo: michellekoeth/firmwareESP_NOWAPI

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
