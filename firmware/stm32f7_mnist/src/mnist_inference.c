#include "mnist_inference.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <string.h>

/* MNIST model data (quantized, 45KB) - included as C array */
extern const unsigned char mnist_model_data[];
extern const unsigned int mnist_model_data_len;

/* Static interpreter and resolver instances */
static tflite::MicroInterpreter *interpreter = nullptr;
static tflite::MicroMutableOpResolver<10> *resolver = nullptr;
static tflite::ErrorReporter *error_reporter = nullptr;

/* Model and tensor arena pointers */
static const tflite::Model *model = nullptr;
static uint8_t *tensor_arena = nullptr;
static size_t tensor_arena_size = 0;

/* Input/Output tensor pointers (cached for performance) */
static TfLiteTensor *input_tensor = nullptr;
static TfLiteTensor *output_tensor = nullptr;

/**
 * @brief Initialize MNIST inference engine
 */
MNIST_Status MNIST_Init(uint8_t *arena, size_t arena_size) {
    /* Validate inputs */
    if (arena == nullptr || arena_size < 65536) {
        return MNIST_ERROR_INVALID_ARENA;
    }
    
    tensor_arena = arena;
    tensor_arena_size = arena_size;
    
    /* Load model from flash */
    model = tflite::GetModel(mnist_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        return MNIST_ERROR_INVALID_MODEL;
    }
    
    /* Setup operations resolver */
    resolver = new tflite::MicroMutableOpResolver<10>();
    if (resolver == nullptr) {
        return MNIST_ERROR_MEMORY;
    }
    
    /* Add operations used by MNIST model */
    resolver->AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                        tflite::ops::micro::Register_CONV_2D());
    resolver->AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                        tflite::ops::micro::Register_MAX_POOL_2D());
    resolver->AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                        tflite::ops::micro::Register_FULLY_CONNECTED());
    resolver->AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                        tflite::ops::micro::Register_RESHAPE());
    resolver->AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                        tflite::ops::micro::Register_SOFTMAX());
    resolver->AddBuiltin(tflite::BuiltinOperator_QUANTIZE,
                        tflite::ops::micro::Register_QUANTIZE());
    resolver->AddBuiltin(tflite::BuiltinOperator_DEQUANTIZE,
                        tflite::ops::micro::Register_DEQUANTIZE());
    
    /* Create interpreter */
    static tflite::MicroInterpreter 
        static_interpreter(model, *resolver, tensor_arena, tensor_arena_size);
    interpreter = &static_interpreter;
    
    /* Allocate tensors */
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        return MNIST_ERROR_ALLOCATE_TENSORS;
    }
    
    /* Get input and output tensors */
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);
    
    if (input_tensor == nullptr || output_tensor == nullptr) {
        return MNIST_ERROR_TENSOR_NOT_FOUND;
    }
    
    /* Validate tensor dimensions */
    if (input_tensor->dims->data[0] != 1 ||
        input_tensor->dims->data[1] != 28 ||
        input_tensor->dims->data[2] != 28 ||
        input_tensor->dims->data[3] != 1) {
        return MNIST_ERROR_INVALID_INPUT_SHAPE;
    }
    
    if (output_tensor->dims->data[1] != 10) {
        return MNIST_ERROR_INVALID_OUTPUT_SHAPE;
    }
    
    return MNIST_OK;
}

/**
 * @brief Run inference on input image
 */
MNIST_Status MNIST_Infer(const uint8_t *input_data, uint8_t *output_probs) {
    if (interpreter == nullptr || input_data == nullptr || output_probs == nullptr) {
        return MNIST_ERROR_INVALID_PARAMS;
    }
    
    /* Copy input data to input tensor */
    if (input_tensor->type == kTfLiteUInt8) {
        uint8_t *input_ptr = interpreter->typed_input_tensor<uint8_t>(0);
        if (input_ptr == nullptr) {
            return MNIST_ERROR_TENSOR_NOT_FOUND;
        }
        memcpy(input_ptr, input_data, 28 * 28 * sizeof(uint8_t));
    } else {
        return MNIST_ERROR_UNSUPPORTED_TYPE;
    }
    
    /* Run inference */
    if (interpreter->Invoke() != kTfLiteOk) {
        return MNIST_ERROR_INVOKE_FAILED;
    }
    
    /* Get output predictions */
    if (output_tensor->type == kTfLiteUInt8) {
        uint8_t *output_ptr = interpreter->typed_output_tensor<uint8_t>(0);
        if (output_ptr == nullptr) {
            return MNIST_ERROR_TENSOR_NOT_FOUND;
        }
        memcpy(output_probs, output_ptr, 10 * sizeof(uint8_t));
    } else {
        return MNIST_ERROR_UNSUPPORTED_TYPE;
    }
    
    return MNIST_OK;
}

/**
 * @brief Get top-1 prediction from output probabilities
 */
uint8_t MNIST_GetTopPrediction(const uint8_t *predictions, uint8_t *confidence) {
    uint8_t top_digit = 0;
    uint8_t top_prob = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        if (predictions[i] > top_prob) {
            top_prob = predictions[i];
            top_digit = i;
        }
    }
    
    if (confidence != nullptr) {
        *confidence = top_prob;
    }
    
    return top_digit;
}

/**
 * @brief Get model size in bytes
 */
uint32_t MNIST_GetModelSize(void) {
    return mnist_model_data_len;
}

/**
 * @brief Get tensor arena usage
 */
uint32_t MNIST_GetArenaUsage(void) {
    if (interpreter == nullptr) {
        return 0;
    }
    /* Note: TensorFlow Lite doesn't expose this directly,
       returning arena size as upper bound */
    return tensor_arena_size;
}
