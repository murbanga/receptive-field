#pragma once
#include <variant>
#include <map>
#include <vector>
#include <functional>
#include <set>

// clang-format off
#define OP_TYPE_ENUM(f) \
	f(Undefined)	\
	f(Conv)		\
	f(Relu)		\
	f(MaxPool)	\
	f(Concat)	\
	f(Dropout)	\
	f(Constant)	\
	f(GlobalAveragePool)\
	f(Flatten)	\
	f(Softmax)	\
	f(Reshape)	\
	f(LRN)		\
	f(Transpose)	\
	f(Resize)	\
	f(Add)		\
	f(Sub)		\
	f(Mul)		\
	f(Identity)	\
	f(Unsqueeze)	\
	f(Cast)		\
	f(Gather)	\
	f(ReduceMean)	\
	f(Pow)		\
	f(Sqrt)		\
	f(Div)		\
	f(MatMul)	\
	f(Neg)		\
	f(Slice)	\
	f(Erf)		\
	f(Where)

// clang-format on

#define DECL(x) x,

enum class OpType
{
	OP_TYPE_ENUM(DECL)
};

#undef DECL

const char *str_from_op_type(OpType op_type);
OpType str_to_op_type(const std::string &name, bool is_assert = true);

typedef std::variant<int64_t, std::vector<int64_t>, std::string> Attribute;
typedef std::variant<std::vector<int64_t>, std::vector<int>, std::vector<float>> Value;

struct Node
{
	std::string name;
	// std::string op_name;
	OpType op_type;
	std::map<std::string, Attribute> attrs;
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

bool operator==(const Tensor &a, const Tensor &b);

class Graph
{
	public:
	typedef std::function<int(const Graph &g, const std::set<std::string> &names, int level)> Callback;

	std::map<std::string, Node> nodes;
	std::map<std::string, Tensor> tensors;
	std::set<std::string> inputs;
	std::map<std::string, Value> values;
	std::map<std::string, std::vector<std::string>> forw;
	std::map<std::string, std::vector<std::string>> back;

	static Graph load(const char *filename);
	int walk_forward(const std::string &beg, Callback f) const;
	// UNUSER int walk_backward(const std::string &beg, Callback f) const;

	std::vector<Field> receptive_field(const std::string &node, Direction dir) const;
	int length(const std::string &name, Direction dir) const;
	bool empty() const { return nodes.empty(); }

	private:
	std::vector<Field> identity_field(const Node &node, Direction dir, int input_idx = 0) const;
	std::vector<Field> conv_field(const Node &node, Direction dir) const;
	std::vector<Field> concat_field(const Node &node, Direction dir) const;
	std::vector<Field> gapool_field(const Node &node, Direction dir) const;
	std::vector<Field> add_field(const Node &node, Direction dir) const;
	std::vector<Field> resize_field(const Node &node, Direction dir) const;
};
