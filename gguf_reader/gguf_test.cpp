#include "gguf_reader.h"

int main(int argc, char **argv)
{
	auto graph = read(argv[1]);
	return 0;
}