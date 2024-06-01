// This is an Arduino sketch to implement an  IoT card rack
// using the AVR-IoT Cellular Mini board.

// Configure the Arduino IDE as follows:
// "AVR-DB series (no bootloader)" under DxCore
// Chip: AVR128DB48
// Wire Library mode: 2x Wire, Master or Slave
// Programmer: Curiosity Nano
// printf: Full, 2.6k, print floats

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

#define VERSION "0.3.1" 

#define CALIB_SENSORS 0
#define DOMAIN "traeumt-gerade.de"
#define MAXSENSOR 2
#define PROX_INT_LOW 300
#define PROX_INT_HIGH 500
#define SLEEPTIME 480 // sleep time in seconds

#if CALIB_SENSORS
#define WEN 0    // no wait between two proximity tests
#define PPERS 0  // no persistence for proximity readings
#else
#define WEN 1
#define PPERS 5
#endif

#include <Arduino.h>
#include <mcp9808.h>
#include <veml3328.h>
#include <http_client.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <low_power.h>
#include "src/FlexWire.h"
#include "src/myAPDS9930.h"

FlexWire flex;
APDS9930 sensor;

volatile bool isrflag = false;
volatile bool ledoff = true;
volatile int cnt = 0;
uint16_t proximity_data = 0;
uint8_t tickets;
enum class Error { NONE, CONNECTION, SENSOR };
volatile Error globalerror = Error::NONE;


const uint8_t sdapin[MAXSENSOR] = { PIN_PD1, PIN_PD3 }; //A1, A2
const uint8_t sclpin = PIN_PD6; //A0
const uint8_t irqpin = PIN_PE2; //D5 (note: this is a fully asynchronous pin!)
const uint8_t ledpin = PIN_PB2; //D13 = LED4

void setup() {
  LedCtrl.begin();
  LedCtrl.startupCycle();

  Log.begin(115200);
  Log.setLogLevel(LogLevel::DEBUG);
  Log.infof(F("IoTRack Version %s\n\r"), VERSION);

  // Make sure sensors are turned off
  Veml3328.begin();
  Mcp9808.begin();
  Veml3328.shutdown();
  Mcp9808.shutdown();

  // init sensors
  initializeSensors();
  
  
#if !CALIB_SENSORS
  // Now we configure the low power module for power down configuration, where
  // the cellular modem and the CPU will be powered down
  LowPower.configurePowerDown();

  // init HTTP client
  initializeHTTP();

  // setup interrupt handling
  pinMode(irqpin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irqpin), sensorIRS_callback, FALLING);
#endif

  // setup extra timer for blinking
  TCB3.CCMP = 30000;
  TCB3.CTRLB = 0;
  TCB3.CTRLA = 5;
  TCB3.INTCTRL = 1;
  
  Log.info(F("Setup completed"));
}

void loop() {
  tickets = readSensors();
  Log.infof(F("#tickets: %d\n\r"),tickets);
  delay(1000);
#if !CALIB_SENSORS
  sendData(tickets);
  delay(1000);
  if (globalerror != Error::NONE) while (1);
  LowPower.powerDown(SLEEPTIME);
#endif
}

// periodic interrupt to signal data transfer and error conditions
ISR(TCB3_INT_vect) {
  if (cnt-- <= 0) {
    ledoff = !ledoff;
    if (globalerror == Error::NONE) cnt = 6;
    else cnt = 1;
  }
  digitalWrite(ledpin, ledoff);
  TCB3.INTFLAGS = 1;
}

// This is the IRS for the IRQ line of the sensors
void sensorIRS_callback(void) {
  isrflag = 1;
}

void initializeSensors(void) {
  for (uint8_t i=0; i < MAXSENSOR; i++) {
    Log.debugf(F("Sensor %d initialization\n\r"),i);
    flex.setPins(sdapin[i], sclpin);
    if (!sensor.init()) {
      Log.errorf(F("Error while initializing sensor %d\n\r"),i);
      globalerror = Error::SENSOR;
    }
    // Start running the APDS-9930 proximity sensor 
    if ( !sensor.enableProximitySensor(!CALIB_SENSORS) ) {
      Log.errorf(F("Something went wrong while enabling sensor %d\n\r"),i);
      globalerror = Error::SENSOR;
    }
    
    // Set proximity interrupt thresholds
    if ( !sensor.setProximityIntLowThreshold(PROX_INT_LOW) ) {
      Log.error(F("Error writing low threshold"));
      globalerror = Error::SENSOR;
    }
    if ( !sensor.setProximityIntHighThreshold(PROX_INT_HIGH) ) {
      Log.error(F("Error writing high threshold"));
    }
    
    // Enable wait timer
    if ( !sensor.setMode(WAIT, WEN) ) {
      Log.error(F("Something went wrong trying to set WEN=1"));
      globalerror = Error::SENSOR;
    }

    // Set wait time to long (700 ms)
    if( !sensor.wireWriteDataByte(APDS9930_WTIME, 0x00) ) {
      Log.error(F("Something went wrong trying to set WTIME=0"));
      globalerror = Error::SENSOR;
    }

    // set time scale to short
    if( !sensor.wireWriteDataByte(APDS9930_CONFIG, 0) ) {
      Log.error(F("Something went wrong trying to set WLONG=0"));
      globalerror = Error::SENSOR;
    }

    // Adjust the Proximity sensor gain
    if ( !sensor.setProximityGain(PGAIN_2X) ) {
      Log.error(F("Something went wrong trying to set PGAIN"));
      globalerror = Error::SENSOR;
    }

    // Setting persistence
    if ( !sensor.wireWriteDataByte(APDS9930_PERS, (PPERS<<4)) ) { // IRQ only after 5 meas.
      Log.error(F("Something went wrong trying to set persistence"));
      globalerror = Error::SENSOR;
    }
    
    // Adjust the Proximity LED drive
    if ( !sensor.setLEDDrive(LED_DRIVE_12_5MA) ) {
      Log.error(F("Something went wrong trying to set LED Drive"));
      globalerror = Error::SENSOR;
    }

    // Clear interrupt flag
    if ( !sensor.clearProximityInt() ) {
      Log.error(F("Error clearing interrupt"));
      globalerror = Error::SENSOR;
    }
  }
  Log.debug(F("All sensors initialized"));
}

int readSensors(void) {
  int high = 0;

  // Read the proximity value
  for (uint8_t i=0; i < MAXSENSOR; i++) {
    flex.setPins(sdapin[i], sclpin);
    if ( !sensor.readProximity(proximity_data) ) {
      Log.errorf(F("Error reading proximity value from sensor %d\n\r"),i);
      globalerror = Error::SENSOR;
    } 
    Log.debugf(F("Proximity (sensor %d): %d\n\r"), i, proximity_data);
    if (proximity_data < PROX_INT_LOW) {
      if (!sensor.setProximityIntLowThreshold(0)) {
	Log.errorf(F("Error setting low proximity threshold for sensor %d\n\r"),i);
	globalerror = Error::SENSOR;
      }
      if (!sensor.setProximityIntHighThreshold(PROX_INT_HIGH)) {
	Log.errorf(F("Error setting high proximity threshold for sensor %d\n\r"),i);
	globalerror = Error::SENSOR;
      }
    } else if (proximity_data > PROX_INT_HIGH) {
      high++;
      if (!sensor.setProximityIntLowThreshold(PROX_INT_LOW)) {
	Log.errorf(F("Error setting low proximity threshold for sensor %d\n\r"),i);
	globalerror = Error::SENSOR;
      }
      if (!sensor.setProximityIntHighThreshold(1023)) {
	Log.errorf(F("Error setting high proximity threshold for sensor %d\n\r"),i);
	globalerror = Error::SENSOR;
      }
    }	
    if ( !sensor.clearProximityInt() ) {
      Log.error(F("Error clearing interrupt"));
      globalerror = Error::SENSOR;
    }
  }
  return high;
}

void initializeHTTP(void) {
  if (!HttpClient.configure(DOMAIN, 443, true)) {
    Log.error(F("Failed to configure https client\r\n"));
    globalerror = Error::CONNECTION;
    return;
  }
  Log.debug(F("Configured for HTTPS"));
}

/**
 * @brief Sends a payload with latitude, longitude and the timestamp.
 */
void sendData(int slots) {
  char data[80];

  sprintf_P(data, PSTR("KEY=123&TICKETS=%d"), slots);

  connectToNetwork();

  Log.debugf(F("Data to send: %s\n\r"), data);
  HttpResponse response = HttpClient.post("/cgi-bin/storedata.cgi", data);
  Log.debugf(F("POST - HTTP status code: %u, data size: %u\r\n"),
	    response.status_code,
	    response.data_size);
  HttpClient.readBody(response.data_size + 16);
  Log.infof(F("Data transmitted: %d\n\r"), slots);
}

/**
 * @brief Connects to the network operator. Will block until connection is
 * achieved.
 */
void connectToNetwork() {
  int retry = 100; // connecttion tries before we go into error state
  // If we already are connected, don't do anything
  if (!Lte.isConnected()) {
    while (!Lte.begin() && retry--) {}
    if (!Lte.isConnected()) {
      globalerror = Error::CONNECTION;
      while (!Lte.begin()) { delay(30000); }
    }
    globalerror = Error::NONE;
    Log.infof(F("Connected to operator: %s\r\n"),
	      Lte.getOperator().c_str());
  }
}


