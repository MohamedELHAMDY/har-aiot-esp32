#import <Adafruit_MPU6050.h>
#import <Adafruit_Sensor.h>
#import <Wire.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Include your exported quantized model array header
#include "model.h" 

Adafruit_MPU6050 mpu;

// TFLite Global Variables
const int kArenaSize = 4096;
uint8_t tensor_arena[kArenaSize];
tflite::AllOpsResolver resolver;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Sampling configuration (50 Hz -> 20ms interval)
const unsigned long sampleIntervalMs = 20;
unsigned long lastSampleTime = 0;
const int windowSize = 50; // 50 samples per frame
int sampleCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize MPU6050 sensor over I2C
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Load TFLite Model
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    return;
  }

  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kArenaSize);
  interpreter = &static_interpreter;

  interpreter->AllocateTensors();
  input = interpreter->input(0);
  output = interpreter->output(0);
  
  Serial.println("AIoT HAR System Initialized. Starting inference loop...");
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastSampleTime >= sampleIntervalMs) {
    lastSampleTime = currentTime;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Populate input tensor buffer (example: normalizing raw acceleration data)
    if (sampleCount < windowSize) {
      int inputIndex = sampleCount * 3;
      // Quantize float to int8 based on model scaling parameters if applicable
      input->data.int8[inputIndex + 0] = (int8_t)(a.acceleration.x * 10);
      input->data.int8[inputIndex + 1] = (int8_t)(a.acceleration.y * 10);
      input->data.int8[inputIndex + 2] = (int8_t)(a.acceleration.z * 10);
      sampleCount++;
    }

    // Once window is full, run local edge inference
    if (sampleCount >= windowSize) {
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        Serial.println("Inference failed!");
        return;
      }

      // Read classification output (e.g., 0: Stationary, 1: Walking)
      int8_t prediction = output->data.int8[0];
      Serial.print("Activity Classified - Output Class Index: ");
      Serial.println(prediction);

      // Reset window counter
      sampleCount = 0;
    }
  }
}
