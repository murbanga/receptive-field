#include "gguf_reader.h"
// see for reference #include "gguf.h"
#include <functional>
#include <vector>
#include <assert.h>

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

std::vector<char> read_string(FILE *f)
{
	uint64_t size;
	fread(&size, sizeof(size), 1, f);
	std::vector<char> v(size);
	fread(v.data(), size, 1, f);
	return v;
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

		switch (type) {
		case GGUF_METADATA_VALUE_TYPE_UINT8:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_INT8:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_UINT16:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_INT16:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_UINT32:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_INT32:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_FLOAT32:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_BOOL:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_STRING:
		{
			auto val = read_string(f);
			ggpuf.add(key, val);
			break;
		}
		case GGUF_METADATA_VALUE_TYPE_ARRAY:
		{
			gguf_metadata_value_type element_type;
			fread(&element_type, sizeof(element_type), 1, f);
			uint64_t len;
			fread(&len, sizeof(len), 1, f);
			for (uint64_t i = 0; i < len; ++i) {
				read_element(f, type);
			}

		}
		case GGUF_METADATA_VALUE_TYPE_UINT64:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_INT64:
		{
			assert(false);
		}
		case GGUF_METADATA_VALUE_TYPE_FLOAT64:
		{
			assert(false);
		}
		default:
			printf("error: unsupported metadata type %d\n", type);
		}
	}

	return ggpuf;
}

void release(gguf *graph) { (void)graph; }
