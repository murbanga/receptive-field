#pragma once
#include <map>
#include <string>
#include <vector>

struct gguf
{
	std::map<std::string, std::string> meta;

	void add(const std::vector<char> &key, const std::vector<char> &value)
	{
		meta.emplace(std::string(key.data(), key.size()), std::string(value.data(), value.size()));
	}
};

gguf read(const char *filename);

void release(gguf *graph);
