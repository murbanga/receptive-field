#include "gguf_reader.h"
#include <functional>

template <typename T> class destroyer
{
public:
	destroyer(std::function<int(T *)> collapser, T *thing_to_destroy)
		: collapser(collapser), thing_to_destroy(thing_to_destroy)
	{}

	~destroyer() { collapser(thing_to_destroy); }

private:
	std::function<int(T *)> collapser;
	T *thing_to_destroy;
};

gguf *read(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	destroyer<FILE> _(fclose, f);

	if (!f) {
		printf("error: fail to open file \"%s\"\n", filename);
		return nullptr;
	}

	uint32_t magic;
	fread(&magic, sizeof(magic), 1, f);
	if (magic != 0x46554747) {
		printf("error: wrong magic number %08x for file \"%s\"\n", magic, filename);
		return nullptr;
	}

	return nullptr;
}

void release(gguf *graph) { (void)graph; }
