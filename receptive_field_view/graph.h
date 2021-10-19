#pragma once
#include <variant>
#include <map>
#include <vector>
#include <functional>
#include <set>

enum class OpType
{
	Undefined,
	Conv,
	Relu,
	MaxPool,
	Concat,
	Dropout,
	Constant,
	GlobalAveragePool,
	Flatten,
	Softmax,
	Reshape,
};

struct Node
{
	OpType op_type;
	std::map < std::string, std::variant<int64_t, std::vector<int64_t>, std::string>> attrs;
};

struct Tensor
{
	int n;
	int channel;
	int width;
	int height;
};

bool operator == (const Tensor &a, const Tensor &b);

struct Graph
{
	std::map<std::string, Node> nodes;
	std::map<std::string, Tensor> tensors;
	std::map<std::string, std::vector<std::string>> forw;
	std::map<std::string, std::vector<std::string>> back;

	int walk_forward(const std::string &beg, std::function<int(const Graph &g, const std::set<std::string> &names, int level)> f);
};

Graph load(const char *filename);
