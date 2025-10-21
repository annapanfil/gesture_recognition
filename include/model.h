/**
 * @file model.h
 * @brief Declares the TensorFlow Lite model data and its length.
 *
 * This file provides external declarations for the TensorFlow Lite model
 * binary data and its size, which are typically generated from a .tflite file
 * and embedded into the firmware.
 */

#ifndef MODEL_H
#define MODEL_H

/**
 * @brief The TensorFlow Lite model data.
 *
 * This array contains the binary representation of the TensorFlow Lite model.
 * It is typically generated from a .tflite file using a conversion tool (e.g. xxd).
 */
extern const unsigned char model_tflite[];

/**
 * @brief The length of the TensorFlow Lite model data.
 *
 * This variable stores the size of the `model_tflite` array in bytes.
 */
extern const unsigned int model_tflite_len;

#endif // MODEL_H
