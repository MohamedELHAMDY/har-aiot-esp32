#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

Adafruit_MPU6050 mpu;
bool sensorFound = false;

// Acquisition parameters: 50 Hz (20 ms interval)
const unsigned long sampleIntervalMs = 20;
unsigned long lastSampleTime = 0;
const int windowSize = 50; 
int sampleCount = 0;
float energySum = 0.0;

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow time for serial port to open

  Serial.println("==================================================");
  Serial.println("Starting AIoT HAR System...");
  Serial.println("==================================================");

  // Initialize I2C bus (SDA = 21, SCL = 22)
  Wire.begin(21, 22);

  if (mpu.begin()) {
    sensorFound = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    Serial.println("[OK] MPU6050 detected and initialized!");
  } else {
    Serial.println("[INFO] MPU6050 not detected: Activating automatic simulated mode.");
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSampleTime >= sampleIntervalMs) {
    lastSampleTime = currentTime;

    float ax = 0.0, ay = 0.0, az = 9.81;

    if (sensorFound) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      ax = a.acceleration.x;
      ay = a.acceleration.y;
      az = a.acceleration.z;
    } else {
      // Fallback synthetic data generator if wiring is missing
      ax = (random(-30, 30)) / 10.0;
      ay = (random(-30, 30)) / 10.0;
      az = 9.81 + (random(-20, 20)) / 10.0;
    }

    // Calculate dynamic energy (deviation from gravity 9.81 m/s²)
    float dynamicVal = abs(sqrt(ax * ax + ay * ay + az * az) - 9.81);
    energySum += dynamicVal;
    sampleCount++;

    // Every 50 readings (1 second), output edge inference results
    if (sampleCount >= windowSize) {
      float avgEnergy = energySum / windowSize;

      Serial.print("[Edge Inference] Dynamic Energy: ");
      Serial.print(avgEnergy, 4);

      if (avgEnergy > 2.0) {
        Serial.println(" --> Intense Movement / Running");
      } else if (avgEnergy > 0.4) {
        Serial.println(" --> Walking");
      } else {
        Serial.println(" --> Stationary");
      }

      // Reset window counter and accumulator
      sampleCount = 0;
      energySum = 0.0;
    }
  }
}
