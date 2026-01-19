#!/usr/bin/env python3
"""
Convert Keras/TensorFlow model to TensorFlow Lite format
"""

import argparse
import sys
import tensorflow as tf
import os

def convert_to_tflite(keras_model_path, output_path, quantize=False):
    """
    Convert Keras model to TFLite
    """
    print(f"[*] Loading model from {keras_model_path}")
    
    try:
        model = tf.keras.models.load_model(keras_model_path)
    except Exception as e:
        print(f"[ERROR] Failed to load model: {e}")
        return False
    
    print("[*] Converting to TensorFlow Lite...")
    
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS,
        tf.lite.OpsSet.SELECT_TF_OPS
    ]
    
    if quantize:
        print("[*] Enabling quantization...")
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
    
    try:
        tflite_model = converter.convert()
    except Exception as e:
        print(f"[ERROR] Conversion failed: {e}")
        return False
    
    # Save
    print(f"[*] Saving to {output_path}")
    with open(output_path, 'wb') as f:
        f.write(tflite_model)
    
    file_size = os.path.getsize(output_path)
    print(f"[+] Success! Model size: {file_size / 1024:.1f} KB")
    
    return True

def main():
    parser = argparse.ArgumentParser(
        description='Convert Keras model to TensorFlow Lite'
    )
    parser.add_argument('model', help='Path to Keras model (.keras or .h5)')
    parser.add_argument('-o', '--output', default='model.tflite',
                       help='Output path for TFLite model')
    parser.add_argument('-q', '--quantize', action='store_true',
                       help='Enable quantization')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.model):
        print(f"[ERROR] Model file not found: {args.model}")
        sys.exit(1)
    
    success = convert_to_tflite(args.model, args.output, args.quantize)
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
