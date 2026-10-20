#pragma once
#include <map>
#include <string>
#include <vector>
#include <variant>

using gguf_value_type = std::variant<std::string, uint64_t, int64_t, double, bool>;

struct gguf
{
	std::map<std::string, std::vector<gguf_value_type>> meta;
};

gguf read(const char *filename);

void release(gguf *graph);
