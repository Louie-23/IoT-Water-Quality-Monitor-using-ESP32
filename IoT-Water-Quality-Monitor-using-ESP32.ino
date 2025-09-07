#define BLYNK_TEMPLATE_ID "Your TempID" //Enter Your ID
#define BLYNK_TEMPLATE_NAME "Your TempName" // Enter Your TempName
#define BLYNK_AUTH_TOKEN "Your AuthToken" // Enter Your Auth Token

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5 // GPIO pin for the DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Your SSID"; // Enter Your SSID
char pass[] = "Your Pass"; // Enter Your Pass

BlynkTimer timer;

namespace pin {
    const byte tds_sensor = 32;       // Pin for TDS sensor
    const byte turbidity_sensor = 35; // Pin for Turbidity sensor
    const byte ph_sensor = 34;        // Pin for pH sensor
}

namespace device {
    float aref = 3.3; // Reference voltage for ESP32 ADC
}

namespace sensor {
    float ec = 0;
    unsigned int tds = 0;
    float ecCalibration = 1;
    float temperature = 0; // Placeholder for temperature value
    int turbidity = 0;     // Placeholder for turbidity value
    float ph = 0;          // Placeholder for pH value
}

BLYNK_WRITE(V4) {
  digitalWrite(4, param.asInt()); // Change '4' to the actual GPIO pin you're using
}

void setup() {
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);

    // Initialize temperature sensor
    sensors.begin();
    pinMode(4, OUTPUT);
    pinMode(pin::turbidity_sensor, INPUT);
    pinMode(pin::ph_sensor, INPUT);

    // Set up timers for periodic tasks
    timer.setInterval(10000L, sendTemperatureToBlynk); // Every 10 seconds
    timer.setInterval(10000L, readTdsQuick);          // Every 10 seconds
    timer.setInterval(10000L, readTurbidity);         // Every 10 seconds
    timer.setInterval(10000L, readPH);                // Every 10 seconds
}

void loop() {
    Blynk.run();
    timer.run(); // Run the timers
}

// Function to read and send temperature to Blynk
void sendTemperatureToBlynk() {
    sensors.requestTemperatures(); // Request temperature data
    sensor::temperature = sensors.getTempCByIndex(0); // Get temperature in Celsius

    // Debug output
    Serial.print("Temperature (°C): ");
    Serial.println(sensor::temperature);

    // Send temperature to Blynk
    Blynk.virtualWrite(V5, sensor::temperature);
}

// Function to read TDS and EC values and send to Blynk
void readTdsQuick() {
    float rawEc = analogRead(pin::tds_sensor) * device::aref / 4095.0; // Adjusted for 12-bit ADC

    float offset = 0.14; // Set this to the observed raw analog value in air
    sensor::ec = (rawEc * sensor::ecCalibration) - offset;
    if (sensor::ec < 0) sensor::ec = 0;
    
    Serial.print(F("Raw Analog Value: "));
    Serial.println(rawEc);

    sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec);

    Serial.print(F("TDS: "));
    Serial.println(sensor::tds);
    Serial.print(F("EC: "));
    Serial.println(sensor::ec, 2);

    Blynk.virtualWrite(V6, sensor::tds);
    Blynk.virtualWrite(V7, sensor::ec);
}

// Function to read turbidity and send to Blynk
void readTurbidity() {
    int sensorValue = analogRead(pin::turbidity_sensor);
    sensor::turbidity = map(sensorValue, 0, 2800, 50, 1);

    Serial.print("Turbidity (NTU): ");
    Serial.println(sensor::turbidity);

    Blynk.virtualWrite(V8, sensor::turbidity);
}

// Function to read pH value and send to Blynk
void readPH() {
    float measure = analogRead(pin::ph_sensor); 
    double voltage = measure*3.3/4095.0; //Analog-to-Digital Conversion
    sensor::ph = 7+((2.5 - voltage)/0.1841);
    Serial.print("pH: ");
    Serial.println(sensor::ph);

    Blynk.virtualWrite(V9, sensor::ph);
}
