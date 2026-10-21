#include "gguf_reader.h"
// see for reference #include "gguf.h"
#include <functional>
#include <vector>
#include <assert.h>

using namespace std;

const char *ggml_type_to_str[] = {
    "F32",        "F16",        "Q4_0",       "Q4_1",       "Q4_2",       "Q4_3",     "Q5_0",     "Q5_1",    "Q8_0",
    "Q8_1",       "Q2_K",       "Q3_K",       "Q4_K",       "Q5_K",       "Q6_K",     "Q8_K",     "IQ2_XXS", "IQ2_XS",
    "IQ3_XXS",    "IQ1_S",      "IQ4_NL",     "IQ3_S",      "IQ2_S",      "IQ4_XS",   "I8",       "I16",     "I32",
    "I64",        "F64",        "IQ1_M",      "BF16",       "Q4_0_4_4",   "Q4_0_4_8", "Q4_0_8_8", "TQ1_0",   "TQ2_0",
    "IQ4_NL_4_4", "IQ4_NL_4_8", "IQ4_NL_8_8", "MXFP4 = 39", "COUNT = 40",
};

const char *gguf_metadata_value_type_to_str[] = {
    "UINT8", "INT8",   "UINT16", "INT16",  "UINT32", "INT32",   "FLOAT32",
    "BOOL",  "STRING", "ARRAY",  "UINT64", "INT64",  "FLOAT64",
};

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

metadata_entry read_element(FILE *f, gguf_metadata_value_type type)
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
		return {{(uint64_t)val}, type};
	}
	case GGUF_METADATA_VALUE_TYPE_INT32: {
		int32_t val;
		fread(&val, sizeof(val), 1, f);
		return {{(int64_t)val}, type};
	}
	case GGUF_METADATA_VALUE_TYPE_FLOAT32: {
		float val;
		fread(&val, sizeof(val), 1, f);
		return {{(double)val}, type};
	}
	case GGUF_METADATA_VALUE_TYPE_BOOL: {
		uint8_t val;
		fread(&val, sizeof(val), 1, f);
		return {{(bool)val}, type};
	}
	case GGUF_METADATA_VALUE_TYPE_STRING: {
		return {{read_string(f)}, type};
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
		v.reserve(len);
		for (uint64_t i = 0; i < len; ++i) {
			v.push_back(read_element(f, element_type).value[0]);
		}
		return {v, element_type};
	}
	default:
		printf("error: unsupported metadata type %d\n", type);
		assert(false);
	}
}

void print(const gguf_value_type &val, gguf_metadata_value_type type)
{
	printf("%s ", gguf_metadata_value_type_to_str[type]);

	switch (val.index()) {
	case 0:
		printf("%s", get<0>(val).c_str());
		break;
	case 1:
		printf("%llu", get<1>(val));
		break;
	case 2:
		printf("%lld", get<2>(val));
		break;
	case 3:
		printf("%f", get<3>(val));
		break;
	case 4:
		printf("%s", get<4>(val) ? "true" : "false");
		break;
	default:
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
	printf("tensors %llu\n", ntensors);

	uint64_t nmetadata;
	size = fread(&nmetadata, sizeof(nmetadata), 1, f);

	gguf ggpuf;

	printf("metadata:\n");
	for (uint64_t i = 0; i < nmetadata; ++i) {
		auto key = read_string(f);

		gguf_metadata_value_type type;
		fread(&type, sizeof(type), 1, f);

		auto val = read_element(f, type);
		ggpuf.meta.insert({key, val});

		printf("\t%s:", key.c_str());
		if (val.value.size() > 1)
			printf("%s[%zu]\n", gguf_metadata_value_type_to_str[val.type], val.value.size());
		else {
			print(val.value[0], val.type);
			printf("\n");
		}
	}

	printf("tensors:\n");
	for (uint64_t i = 0; i < ntensors; ++i) {
		auto name = read_string(f);

		uint32_t ndims;
		fread(&ndims, sizeof(ndims), 1, f);

		std::vector<uint64_t> dims;

		for (uint32_t d = 0; d < ndims; ++d) {
			uint64_t dim;
			fread(&dim, sizeof(dim), 1, f);
			dims.push_back(dim);
		}

		ggml_type type;
		fread(&type, sizeof(type), 1, f);

		uint64_t offset;
		fread(&offset, sizeof(offset), 1, f);

		printf("\"%s\" %dD %s %016x\n", name.c_str(), ndims, ggml_type_to_str[type],offset);
	}

	return ggpuf;
}

void release(gguf *graph) { (void)graph; }
