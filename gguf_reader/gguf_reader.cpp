#include "gguf_reader.h"
// see for reference #include "gguf.h"
#include <functional>
#include <vector>
#include <assert.h>

using namespace std;

template <typename T> class destroyer
{
	public:
	destroyer(std::function<int(T *)> collapser, T *thing_to_destroy)
	    : collapser(collapser), thing_to_destroy(thing_to_destroy)
	{
	}

	~destroyer() { collapser(thing_to_destroy); }

	private:
	std::function<int(T *)> collapser;
	T *thing_to_destroy;
};

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

std::string read_string(FILE *f)
{
	uint64_t size;
	fread(&size, sizeof(size), 1, f);
	char buf[1024];
	assert(sizeof(buf) > size);
	uint64_t size_to_read = min(size, sizeof(buf) - 1);
	fread(buf, size_to_read, 1, f);
	buf[size_to_read] = 0;
	return buf;
}

std::vector<gguf_value_type> read_element(FILE *f, gguf_metadata_value_type type)
{
	switch (type) {
	case GGUF_METADATA_VALUE_TYPE_UINT8: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_INT8: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_UINT16: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_INT16: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_UINT32: {
		uint32_t val;
		fread(&val, sizeof(val), 1, f);
		return {(uint64_t)val};
	}
	case GGUF_METADATA_VALUE_TYPE_INT32: {
		int32_t val;
		fread(&val, sizeof(val), 1, f);
		return { (int64_t)val };
	}
	case GGUF_METADATA_VALUE_TYPE_FLOAT32: {
		float val;
		fread(&val, sizeof(val), 1, f);
		return { (double)val };
	}
	case GGUF_METADATA_VALUE_TYPE_BOOL: {
		uint8_t val;
		fread(&val, sizeof(val), 1, f);
		return { (bool)val };
	}
	case GGUF_METADATA_VALUE_TYPE_STRING: {
		return {read_string(f)};
	}

	case GGUF_METADATA_VALUE_TYPE_UINT64: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_INT64: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_FLOAT64: {
		assert(false);
	}
	case GGUF_METADATA_VALUE_TYPE_ARRAY: {
		gguf_metadata_value_type element_type;
		fread(&element_type, sizeof(element_type), 1, f);
		assert(element_type != GGUF_METADATA_VALUE_TYPE_ARRAY);

		uint64_t len;
		fread(&len, sizeof(len), 1, f);

		vector<gguf_value_type> v;
		for (uint64_t i = 0; i < len; ++i) {
			v.push_back(read_element(f, element_type)[0]);
		}
		return v;
	}
	default:
		printf("error: unsupported metadata type %d\n", type);
		assert(false);
	}
}

gguf read(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	destroyer<FILE> _(fclose, f);

	if (!f) {
		printf("error: fail to open file \"%s\"\n", filename);
		return {};
	}

	uint32_t magic = 0;
	size_t size = fread(&magic, sizeof(magic), 1, f);
	if (size != 1 || magic != 0x46554747) {
		printf("error: wrong magic number %08x for file \"%s\"\n", magic, filename);
		return {};
	}

	uint32_t version;
	size = fread(&version, sizeof(version), 1, f);
	printf("GGUF version %d\n", version);

	uint64_t ntensors;
	size = fread(&ntensors, sizeof(ntensors), 1, f);

	uint64_t nmetadata;
	size = fread(&nmetadata, sizeof(nmetadata), 1, f);

	gguf ggpuf;

	for (uint64_t i = 0; i < nmetadata; ++i) {
		auto key = read_string(f);

		gguf_metadata_value_type type;
		fread(&type, sizeof(type), 1, f);

		auto val = read_element(f, type);
		ggpuf.meta.insert({key, val});
	}

	return ggpuf;
}

void release(gguf *graph) { (void)graph; }
