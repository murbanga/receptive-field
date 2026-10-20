#include "gguf_reader.h"

int main(int argc, char **argv)
{
	/*if (argc < 2) {
		printf("no enough arguments\n");
		return -1;
	}*/

	const char *filename = "D:\\hugging-face-models\\dots.ocr-GGUF\\dots.ocr.f16.gguf";

	auto graph = read(filename);
	return 0;
}
