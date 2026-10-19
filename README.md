# Human Activity Recognition (HAR) on ESP32 (AIoT Project)

This project implements an edge-optimized Human Activity Recognition (HAR) system using an ESP32 microcontroller and an MPU6050 6-axis accelerometer/gyroscope in Wokwi[span_1](start_span)[span_1](end_span). It classifies movements (e.g., walking vs. standing) locally on the device using a quantized neural network model.

## Setup and Running in Wokwi
1. Open [Wokwi](https://wokwi.com/) and create a new **ESP32** project[span_2](start_span)[span_2](end_span).
2. Replace `sketch.ino` with the code provided in this repository.
3. Replace `diagram.json` with the MPU6050 wiring configuration.
4. Install the required Arduino libraries in the Library Manager:
   - `Adafruit MPU6050`
   - `TensorFlowLite_ESP32`
5. Run the simulation and open the Serial Monitor to observe real-time inference results.

## Model Training & Quantization
The model was trained in Python using TensorFlow/Keras and converted to a compact `int8` FlatBuffer using Post-Training Quantization (PTQ) to fit within the ESP32's memory constraints[span_3](start_span)[span_3](end_span)[span_4](start_span)[span_4](end_span).
