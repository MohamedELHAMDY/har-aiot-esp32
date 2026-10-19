import tensorflow as tf

# Load your trained Keras model
model = tf.keras.models.load_model('har_model.h5')

# Convert the model to TensorFlow Lite with Post-Training Quantization (PTQ) (int8)
def representative_dataset_gen():
  for _ in range(100):
    # Provide representative input samples for quantization calibration
    import numpy as np
    data = np.random.uniform(-8.0, 8.0, (1, 50, 3)).astype(np.float32)
    yield [data]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset_gen
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# Save the model flatbuffer
with open('model.tflite', 'wb') as f:
  f.write(tflite_model)

print("Model successfully quantized and converted to model.tflite")
