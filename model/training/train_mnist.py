#!/usr/bin/env python3
"""
MNIST Model Training Script
Trains a CNN for digit recognition on MNIST dataset
"""

import os
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import matplotlib.pyplot as plt

# Configuration
INPUT_SHAPE = (28, 28, 1)
NUM_CLASSES = 10
BATCH_SIZE = 128
EPOCHS = 10
VALIDATION_SPLIT = 0.1
MODEL_PATH = './mnist_model.keras'
TFLITE_PATH = './mnist_model.tflite'
TFLITE_QUANT_PATH = './mnist_model_quantized.tflite'

def load_and_prepare_data():
    """
    Load MNIST dataset and prepare for training
    """
    print("[*] Loading MNIST dataset...")
    (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
    
    # Normalize to [0, 1]
    x_train = x_train.astype('float32') / 255.0
    x_test = x_test.astype('float32') / 255.0
    
    # Add channel dimension
    x_train = np.expand_dims(x_train, -1)
    x_test = np.expand_dims(x_test, -1)
    
    # Convert labels to one-hot
    y_train = keras.utils.to_categorical(y_train, NUM_CLASSES)
    y_test = keras.utils.to_categorical(y_test, NUM_CLASSES)
    
    print(f"[*] Training set shape: {x_train.shape}")
    print(f"[*] Test set shape: {x_test.shape}")
    
    return (x_train, y_train), (x_test, y_test)

def build_model():
    """
    Build CNN model for MNIST
    Optimized for embedded deployment (small & fast)
    """
    print("[*] Building CNN model...")
    
    model = keras.Sequential([
        # Block 1
        layers.Conv2D(32, (3, 3), activation='relu', padding='same', input_shape=INPUT_SHAPE),
        layers.MaxPooling2D((2, 2)),
        layers.Dropout(0.25),
        
        # Block 2
        layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
        layers.MaxPooling2D((2, 2)),
        layers.Dropout(0.25),
        
        # Flatten and Dense
        layers.Flatten(),
        layers.Dense(128, activation='relu'),
        layers.Dropout(0.5),
        
        # Output
        layers.Dense(NUM_CLASSES, activation='softmax')
    ])
    
    model.summary()
    
    model.compile(
        optimizer='adam',
        loss='categorical_crossentropy',
        metrics=['accuracy']
    )
    
    return model

def train_model(model, train_data, test_data):
    """
    Train the model
    """
    x_train, y_train = train_data
    x_test, y_test = test_data
    
    print("\n[*] Training model...")
    
    history = model.fit(
        x_train, y_train,
        batch_size=BATCH_SIZE,
        epochs=EPOCHS,
        verbose=1,
        validation_data=(x_test, y_test)
    )
    
    # Evaluate
    print("\n[*] Evaluating model...")
    test_loss, test_acc = model.evaluate(x_test, y_test, verbose=0)
    print(f"Test accuracy: {test_acc * 100:.2f}%")
    print(f"Test loss: {test_loss:.4f}")
    
    return history

def save_model(model):
    """
    Save trained model
    """
    print(f"\n[*] Saving model to {MODEL_PATH}")
    model.save(MODEL_PATH)
    print(f"[+] Model saved: {os.path.getsize(MODEL_PATH) / 1024:.1f} KB")

def convert_to_tflite(model):
    """
    Convert Keras model to TensorFlow Lite
    """
    print(f"\n[*] Converting to TensorFlow Lite...")
    
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS,
        tf.lite.OpsSet.SELECT_TF_OPS
    ]
    tflite_model = converter.convert()
    
    with open(TFLITE_PATH, 'wb') as f:
        f.write(tflite_model)
    
    print(f"[+] TFLite model saved: {os.path.getsize(TFLITE_PATH) / 1024:.1f} KB")
    
    return tflite_model

def convert_to_tflite_quantized(x_test):
    """
    Convert and quantize model for embedded deployment
    """
    print(f"\n[*] Converting to quantized TensorFlow Lite...")
    
    # Build converter with quantization
    converter = tf.lite.TFLiteConverter.from_keras_model(keras.models.load_model(MODEL_PATH))
    
    # Enable quantization
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    
    # Representative dataset for post-training quantization
    def representative_dataset():
        for i in range(100):
            yield [x_test[i:i+1].astype(np.float32)]
    
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS_INT8
    ]
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8
    
    tflite_quant_model = converter.convert()
    
    with open(TFLITE_QUANT_PATH, 'wb') as f:
        f.write(tflite_quant_model)
    
    print(f"[+] Quantized model saved: {os.path.getsize(TFLITE_QUANT_PATH) / 1024:.1f} KB")
    
    return tflite_quant_model

def plot_training_history(history):
    """
    Plot training and validation metrics
    """
    fig, axes = plt.subplots(1, 2, figsize=(14, 4))
    
    # Accuracy
    axes[0].plot(history.history['accuracy'], label='Train')
    axes[0].plot(history.history['val_accuracy'], label='Val')
    axes[0].set_title('Model Accuracy')
    axes[0].set_ylabel('Accuracy')
    axes[0].set_xlabel('Epoch')
    axes[0].legend()
    axes[0].grid(True)
    
    # Loss
    axes[1].plot(history.history['loss'], label='Train')
    axes[1].plot(history.history['val_loss'], label='Val')
    axes[1].set_title('Model Loss')
    axes[1].set_ylabel('Loss')
    axes[1].set_xlabel('Epoch')
    axes[1].legend()
    axes[1].grid(True)
    
    plt.tight_layout()
    plt.savefig('training_history.png')
    print("[+] Training history saved to training_history.png")
    plt.show()

def main():
    print("\n" + "="*60)
    print("MNIST Model Training for STM32 Edge AI")
    print("="*60)
    
    # Load data
    train_data, test_data = load_and_prepare_data()
    
    # Build and train model
    model = build_model()
    history = train_model(model, train_data, test_data)
    
    # Save models
    save_model(model)
    convert_to_tflite(model)
    convert_to_tflite_quantized(test_data[0])
    
    # Plot results
    plot_training_history(history)
    
    print("\n" + "="*60)
    print("Training complete! Ready for deployment")
    print("="*60)
    print(f"\nGenerated files:")
    print(f"  - {MODEL_PATH} (Keras model)")
    print(f"  - {TFLITE_PATH} (TFLite model)")
    print(f"  - {TFLITE_QUANT_PATH} (Quantized, for STM32)")
    print(f"\nNext: Copy {TFLITE_QUANT_PATH} to firmware/stm32f7_mnist/models/")

if __name__ == '__main__':
    main()
