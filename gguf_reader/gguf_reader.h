#pragma once
#include <map>
#include <string>
#include <vector>
#include <variant>

enum ggml_type : uint32_t {
        GGML_TYPE_F32 = 0,
        GGML_TYPE_F16 = 1,
        GGML_TYPE_Q4_0 = 2,
        GGML_TYPE_Q4_1 = 3,
        // GGML_TYPE_Q4_2 = 4, support has been removed
        // GGML_TYPE_Q4_3 = 5, support has been removed
        GGML_TYPE_Q5_0 = 6,
        GGML_TYPE_Q5_1 = 7,
        GGML_TYPE_Q8_0 = 8,
        GGML_TYPE_Q8_1 = 9,
        GGML_TYPE_Q2_K = 10,
        GGML_TYPE_Q3_K = 11,
        GGML_TYPE_Q4_K = 12,
        GGML_TYPE_Q5_K = 13,
        GGML_TYPE_Q6_K = 14,
        GGML_TYPE_Q8_K = 15,
        GGML_TYPE_IQ2_XXS = 16,
        GGML_TYPE_IQ2_XS = 17,
        GGML_TYPE_IQ3_XXS = 18,
        GGML_TYPE_IQ1_S = 19,
        GGML_TYPE_IQ4_NL = 20,
        GGML_TYPE_IQ3_S = 21,
        GGML_TYPE_IQ2_S = 22,
        GGML_TYPE_IQ4_XS = 23,
        GGML_TYPE_I8 = 24,
        GGML_TYPE_I16 = 25,
        GGML_TYPE_I32 = 26,
        GGML_TYPE_I64 = 27,
        GGML_TYPE_F64 = 28,
        GGML_TYPE_IQ1_M = 29,
        GGML_TYPE_BF16 = 30,
        // GGML_TYPE_Q4_0_4_4 = 31, support has been removed from gguf files
        // GGML_TYPE_Q4_0_4_8 = 32,
        // GGML_TYPE_Q4_0_8_8 = 33,
        GGML_TYPE_TQ1_0 = 34,
        GGML_TYPE_TQ2_0 = 35,
        // GGML_TYPE_IQ4_NL_4_4 = 36,
        // GGML_TYPE_IQ4_NL_4_8 = 37,
        // GGML_TYPE_IQ4_NL_8_8 = 38,
        GGML_TYPE_MXFP4 = 39, // MXFP4 (1 block)
        GGML_TYPE_COUNT = 40,
};

const char *ggml_type_to_str[];


enum gguf_metadata_value_type : uint32_t
{
	// The value is a 8-bit unsigned integer.
	GGUF_METADATA_VALUE_TYPE_UINT8 = 0,
	// The value is a 8-bit signed integer.
	GGUF_METADATA_VALUE_TYPE_INT8 = 1,
	// The value is a 16-bit unsigned little-endian integer.
	GGUF_METADATA_VALUE_TYPE_UINT16 = 2,
	// The value is a 16-bit signed little-endian integer.
	GGUF_METADATA_VALUE_TYPE_INT16 = 3,
	// The value is a 32-bit unsigned little-endian integer.
	GGUF_METADATA_VALUE_TYPE_UINT32 = 4,
	// The value is a 32-bit signed little-endian integer.
	GGUF_METADATA_VALUE_TYPE_INT32 = 5,
	// The value is a 32-bit IEEE754 floating point number.
	GGUF_METADATA_VALUE_TYPE_FLOAT32 = 6,
	// The value is a boolean.
	// 1-byte value where 0 is false and 1 is true.
	// Anything else is invalid, and should be treated as either the model being invalid or the reader being buggy.
	GGUF_METADATA_VALUE_TYPE_BOOL = 7,
	// The value is a UTF-8 non-null-terminated string, with length prepended.
	GGUF_METADATA_VALUE_TYPE_STRING = 8,
	// The value is an array of other values, with the length and type prepended.
	///
	// Arrays can be nested, and the length of the array is the number of elements in the array, not the number of
	// bytes.
	GGUF_METADATA_VALUE_TYPE_ARRAY = 9,
	// The value is a 64-bit unsigned little-endian integer.
	GGUF_METADATA_VALUE_TYPE_UINT64 = 10,
	// The value is a 64-bit signed little-endian integer.
	GGUF_METADATA_VALUE_TYPE_INT64 = 11,
	// The value is a 64-bit IEEE754 floating point number.
	GGUF_METADATA_VALUE_TYPE_FLOAT64 = 12,
};

const char *gguf_metadata_value_type_to_str[];

using gguf_value_type = std::variant<std::string, uint64_t, int64_t, double, bool>;

struct metadata_entry
{
	std::vector<gguf_value_type> value;
	gguf_metadata_value_type type;
};

struct gguf
{
	std::map<std::string, metadata_entry> meta;
};

gguf read(const char *filename);

void release(gguf *graph);
