#include <Homie.h>
#include <NDIRZ16.h>
#include <SoftwareSerial.h>

#define FW_NAME "gas-monitor"
#define FW_VERSION "1.0.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

SoftwareSerial mySerial(D5,D6);
NDIRZ16 mySensor = NDIRZ16(&mySerial);

const int waitFirstMeasure = 10000;
const int delayMeasure= 1000;
const int delaySend= 30000;
unsigned long lastMeasure = 0;
unsigned long lastGasSent = 0;

int measurements[255];

HomieNode gasNode("co2", "gas");

void setupHandler() {
  Homie.setNodeProperty(gasNode, "unit", "ppm", true);
}

void loopHandler() {
  static uint8_t i;
  if(millis() > (lastMeasure + delayMeasure)){
    if (mySensor.measure()) {
      Serial.print(i);
      Serial.print(" CO2 Concentration is ");
      Serial.print(mySensor.ppm);
      Serial.println("ppm ");
      measurements[i] = mySensor.ppm;
      i++;
      if(millis() > (lastGasSent + delaySend)){
        // calculate average of last ~30 measurements
        unsigned long avg_val = 0;
        for(uint8_t c = 0; c < i; c++) {
          avg_val += measurements[c];
        }
        avg_val = avg_val/i;

        if (Homie.setNodeProperty(gasNode, "value", String(avg_val), true)) {
          Serial.print("Sent average: ");Serial.println(avg_val);
          lastGasSent = millis();
        } else {
          Serial.println("Sending Gas failed");
        }
        i = 0; // reset counter
      }      
    }
    lastMeasure = millis();
  }
}

void setup() {
  Serial.begin(115200); Serial.println();
  mySerial.begin(9600);
  Serial.println("Wait 10 seconds for the sensor to starup");
  lastMeasure = millis() + waitFirstMeasure;   // fake future measurement
  Homie.setFirmware(FW_NAME, FW_VERSION);
  Homie.registerNode(gasNode);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
