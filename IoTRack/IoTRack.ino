// This is an Arduino sketch to implement an  IoT card rack
// using the AVR-IoT Cellular Mini board.

// Configure the Arduino IDE as follows:
// "AVR-DB series (no bootloader)" under DxCore
// Chip: AVR128DB48
// Wire Library mode: 2x Wire, Master or Slave
// Programmer: Curiosity Nano
// printf: Full, 2.6k, print floats
// attachInterrupt: only on attached ports

// Pin mapping:
// A0 = PIN_PD6
// A1 = PIN_PD1
// A2 = PIN_PD3
// A3 = PIN_PD4
// A4 = PIN_PD5
// A5 = PIN_PD7
// D5 = PIN_PE2
// D6 = PIN_PE1
// D9 = PIN_PD0
// D10 = PIN_PB5
// D11 = PIN_PD2 (SW0, DBG2)
// D12 = PIN_PA7
// D13 = PIN_PB2 (LED4, DBG1)

#define VERSION "0.2.0" 

#define CALIB_SENSORS 0
#define DOMAIN "traeumt-gerade.de"
#define MAXSENSOR 2
#define PROX_INT_LOW 200
#define PROX_INT_HIGH 500


#include <Arduino.h>
#include <http_client.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include "src/FlexWire.h"
#include "src/myAPDS9930.h"

FlexWire flex;
APDS9930 sensor;

volatile bool isrflag = false;
uint16_t proximity_data = 0;

const uint8_t sdapin[MAXSENSOR] = { PIN_PD1, PIN_PD3 }; //A1, A2
const uint8_t sclpin = PIN_PD6; //A0
const uint8_t irqpin = PIN_PE2; //D5 (note: this is a fully asynchronous pin!)
const uint8_t ledpin = PIN_PB2; //D13 = LED4

void setup() {
  LedCtrl.begin();
  LedCtrl.startupCycle();

  Log.begin(115200);
  Log.setLogLevel(LogLevel::DEBUG);
  Log.info(F("Initializing sensors"));
  for (uint8_t i=0; i < MAXSENSOR; i++) {
    flex.setPins(sdapin[i], sclpin);
    if (!sensor.init()) {
      Log.errorf(F("Error while initializing sensor %d\n\r"),i);
    } else {
      Log.infof(F("Sensor %d has been initialized\n\r"),i);
    }
    // Start running the APDS-9930 proximity sensor 
    if ( sensor.enableProximitySensor(!CALIB_SENSORS) ) {
      Log.infof(F("Proximity sensor %d is now enabled\n\r"),i);
    } else {
      Log.errorf(F("Something went wrong while enabling sensor %d\n\r"),i);
    }
    
#if !CALIB_SENSORS
    // Set proximity interrupt thresholds
    if ( !sensor.setProximityIntLowThreshold(PROX_INT_LOW) ) {
      Log.error(F("Error writing low threshold"));
    }
    if ( !sensor.setProximityIntHighThreshold(PROX_INT_HIGH) ) {
      Log.error(F("Error writing high threshold"));
    }
    
    // Enable wait timer
    //   if ( !sensor.setMode(WAIT, 1) ) {
    //  Log.error(F("Something went wrong trying to set WEN=1"));
    // }

    // Set wait time to long
    if( !sensor.wireWriteDataByte(APDS9930_WTIME, 0) ) {
      Log.error(F("Something went wrong trying to set WTIME=0"));
    }

    // set time to 8.4 sec
    if( !sensor.wireWriteDataByte(APDS9930_CONFIG, 0) ) {
      Log.error(F("Something went wrong trying to set WLONG=0"));
    }
#endif
    // Adjust the Proximity sensor gain
    if ( !sensor.setProximityGain(PGAIN_2X) ) {
      Log.error(F("Something went wrong trying to set PGAIN"));
    }
    
    // Adjust the Proximity LED drive
    if ( !sensor.setLEDDrive(LED_DRIVE_12_5MA) ) {
      Log.error(F("Something went wrong trying to set LED Drive"));
    }
  }
#if !CALIB_SENSORS  
  pinMode(PIN_PE2, INPUT_PULLUP);
  PORTE.PIN2CTRL = 0b00001101; // no pullup, interrupt on LOW
#endif
}

#if !CALIB_SENSORS // run sketch in ordinary mode
void loop() {
  if (isrflag) {
    // Read the proximity value
    for (uint8_t i=0; i < MAXSENSOR; i++) {
      flex.setPins(sdapin[i], sclpin);
      if ( !sensor.readProximity(proximity_data) ) {
	Log.error(F("Error reading proximity value"));
      } else {
	Log.infof(F("Proximity (sensor %d): %d\n\r"), i, proximity_data);
      }
      if (proximity_data < PROX_INT_LOW) {
	sensor.setProximityIntLowThreshold(0);
	sensor.setProximityIntHighThreshold(PROX_INT_HIGH);
      } else if (proximity_data > PROX_INT_HIGH) {
	sensor.setProximityIntLowThreshold(PROX_INT_LOW);
	sensor.setProximityIntHighThreshold(1023);
      }	
      if ( !sensor.clearProximityInt() ) {
	Log.error(F("Error clearing interrupt"));
      }
    }
    Log.info("");
    digitalWrite(ledpin,LOW);
    delay(1000);
    digitalWrite(ledpin,HIGH);
    isrflag = false;
    PORTE.PIN2CTRL = 0b00001101;
  }
}

void sendStatus(byte slots)
{
  // Start LTE modem and connect to the operator
  if (!Lte.begin()) {
    Log.error(F("Failed to connect to the operator"));
    return;
  }

  Log.infof(F("Connected to operator: %s\r\n"), Lte.getOperator().c_str());

  if (!HttpClient.configure(DOMAIN, 443, true)) {
    Log.info(F("Failed to configure https client\r\n"));
    return;
  }

  Log.info(F("Configured to HTTPS"));

  HttpResponse response = HttpClient.post("/cgi-bin/storedata.cgi", "KEY=123&TICKETS=3");
  Log.infof(F("POST - HTTP status code: %u, data size: %u\r\n"),
	    response.status_code,
	    response.data_size);

  // Add some extra bytes for termination
  // String body =
  HttpClient.readBody(response.data_size + 16);

  //  if (body != "") {
  //  Log.infof(F("Body: %s\r\n"), body.c_str());
  //}
}

// This is ISR for the IRQ line of the sensors
ISR(PORTE_PORT_vect) {
  PORTE.INTFLAGS = 4; //clear flags
  PORTE.PIN2CTRL = 0b00001000;
  isrflag = 1;
}

#else // run sketch to gather sensor values

void loop() {
   // Read the proximity value
   for (uint8_t i=0; i < MAXSENSOR; i++) {
     flex.setPins(sdapin[i], sclpin);
     if ( !sensor.readProximity(proximity_data) ) {
       Log.error(F("Error reading proximity value"));
     } else {
       Log.infof(F("Proximity (sensor %d): %d\n\r"), i, proximity_data);
     }
   }
   // Wait 250 ms before next reading
   delay(1000);
   Log.info("");
}
#endif

