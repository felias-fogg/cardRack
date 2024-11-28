# IoTRack: The Firmware for cardRack

This is an Arduino sketch that implements the IoT Card Rack. It has been designed to run on an AVR-IoT Cellular Mini board, which contains among other things an AVR128DB48 MCU and a LTE-M cellular modem.   You need to install the DxCore in order to compile the software. Before you do so, you may want to get familiar with the board and how it communicates with the environment and the Internet. I recommend to read my [blog post](https://arduino-craft-corner.de/index.php/2024/01/03/connecting-the-avr-iot-cellular-mini-board-to-the-internet/) for this purpose. 

Before you upload the software to your board, you should make sure that you have defined the right location, have run the `genlocation.sh` script in the `web/config` folder, and have set a few other compile time constants in the sketch. Make also sure that in the Arduino IDE the right board with the right parameters are selected (see top of Arduino sketch). By setting the compile time constant `CALIB_SENSORS` to 1, you will get a steady stream of sensor measurements on the serial line, which may be helpful when setting the two thresholds `PROX_INT_LOW` and `PROX_INT_HIGH`.

A last addition was the compile-time constant `LEDSINK`. It should be set in the location header file and describes whether the wiring is similar to the one in the Fritzing sketch (when set to 1) or similar to the wiring as given in the KiCAD schema and PCB V1.0 (=0).

