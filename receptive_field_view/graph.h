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
	std::string name;
	OpType op_type;
	std::map < std::string, std::variant<int64_t, std::vector<int64_t>, std::string>> attrs;
	std::vector<std::string> inputs;
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
enum class Direction
{
	ByRows = 0,
	ByColumns = 1,
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
	std::vector<Field> receptive_field(const std::string &node, Direction dir) const;

private:
	std::vector<Field> identity_field(const Node &node, Direction dir) const;
	std::vector<Field> conv_field(const Node &node, Direction dir) const;
	std::vector<Field> concat_field(const Node &node, Direction dir) const;
};

