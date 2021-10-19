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

struct FromTo
{
	int from_input;
	int to_input;
	int from_output;
	int to_output;
};

struct Field
{
	std::string input;
	std::string output;
	std::vector<FromTo> field;
};

bool operator == (const Tensor &a, const Tensor &b);

class Graph
{
public:
	std::map<std::string, Node> nodes;
	std::map<std::string, Tensor> tensors;
	std::map<std::string, std::vector<std::string>> forw;
	std::map<std::string, std::vector<std::string>> back;

	static Graph load(const char *filename);
	int walk_forward(const std::string &beg, std::function<int(const Graph &g, const std::set<std::string> &names, int level)> f) const;
	std::vector<Field> receptive_field(const std::string &node) const;

private:
	std::vector<Field> identity_field(const std::string &input, const std::string &output) const;
	std::vector<Field> conv_field(const std::string &input, const std::string &output) const;
	std::vector<Field> concat_field(const std::vector<std::string> &inputs, const std::string &output) const;
};

