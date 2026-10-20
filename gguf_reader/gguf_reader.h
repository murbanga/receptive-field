#pragma once
#include <map>
#include <string>

struct gguf
{
	std::map<std::string, std::string> meta;
};

gguf *read(const char *filename);

void release(gguf *graph);
