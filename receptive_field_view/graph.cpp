#include <fstream>

#pragma warning (push)
#pragma warning (disable: 4244 4267)
#define ONNX_NAMESPACE onnx
#include "../onnx/onnx/onnx_pb.h"
#include "../onnx/onnx/version_converter/convert.h"
#include "../onnx/onnx/shape_inference/implementation.h"
#pragma warning (pop)

#include "graph.h"

using namespace std;

Range find_input(const std::vector<FromTo> &field, int beg, int end)
{
	Range range = { -1, -1 };

	for (size_t i = 0; i < field.size(); ++i)
	{
		if (field[i].from_output <= beg && beg < field[i].to_output)
			range.beg = (range.beg > -1 ? min(range.beg, field[i].from_input) : field[i].from_input);

		if (field[i].from_output < end && end <= field[i].to_output)
			range.end = (range.end > -1 ? max(range.end, field[i].to_input) : field[i].to_input);
	}
	
	assert(range.beg <= range.end);

	return range;
}

Range find_output(const std::vector<FromTo> &field, int beg, int end)
{
	Range range = { -1, -1 };

	for (size_t i = 0; i < field.size(); ++i)
	{
		if (field[i].from_input <= beg && beg < field[i].to_input)
			range.beg = (range.beg > -1 ? min(range.beg, field[i].from_output) : field[i].from_output);

		if (field[i].from_input < end && end <= field[i].to_input)
			range.end = (range.end > -1 ? max(range.end, field[i].to_output) : field[i].to_output);
	}

	assert(range.beg <= range.end);

	return range;
}

bool operator==(const Tensor &a, const Tensor &b)
{
	return a.n == b.n && a.channel == b.channel && a.width == b.width && a.height == b.height;
}

OpType str_to_op_type(const string &name)
{
#define STR_TO_ENUM(x)	if(name == #x)return OpType::x;

	OP_TYPE_ENUM(STR_TO_ENUM);

#undef STR_TO_ENUM

	assert(false);
	return OpType::Undefined;
}

const char *str_from_op_type(OpType op_type)
{
#define ENUM_TO_STR(x)	case OpType::x: return #x;

	switch (op_type)
	{
		OP_TYPE_ENUM(ENUM_TO_STR);
	}

#undef ENUM_TO_STR

	assert(false);
	return "Undefined";
}

Tensor value_info(const onnx::ValueInfoProto proto)
{
	auto shape = proto.type().tensor_type().shape();
	Tensor tensor = { 1,1,1,1 };

	if (!shape.dim_size())
	{
		printf("warning: strange weird value '%s' of size 0\n", proto.name().c_str());
		return {0,0,0,0};
	}

	tensor.n = (int)shape.dim(0).dim_value();
	if (shape.dim_size() > 1)
		tensor.channel = (int)shape.dim(1).dim_value();
	if (shape.dim_size() > 2)
		tensor.width = (int)shape.dim(2).dim_value();
	if (shape.dim_size() > 3)
		tensor.height = (int)shape.dim(3).dim_value();

	assert(shape.dim_size() <= 4);
	return tensor;
}

template <typename Iterable>
vector<string> filter_inputs(OpType op_type, const Iterable inputs)
{
	switch (op_type)
	{
	case OpType::Add:
	case OpType::Relu:
	case OpType::Transpose:
	case OpType::Concat:
		return { inputs.begin(), inputs.end() };

	case OpType::Resize:
	case OpType::Conv:
	case OpType::MaxPool:
	case OpType::Dropout:
	case OpType::GlobalAveragePool:
		return { inputs[0] };

	default:
		printf("unsupported op %s\n", str_from_op_type(op_type));
	}

	return {};
}

Graph Graph::load(const char *filename)
{
	ifstream file{ filename, ios::in | ios::binary };
	onnx::ModelProto model;

	if (!model.ParseFromIstream(&file))
	{
		printf("fail to load graph from '%s'\n", filename);
		return {};
	}

	constexpr int64_t original_version = 13;
	int64_t version = 15;

	for (auto &i : model.opset_import())
		if (i.domain() == "" || i.domain() == "ai.onnx")
		{
			version = i.version();
			break;
		}
	
	if (version < original_version)
		model = onnx::version_conversion::ConvertVersion(model, original_version);

	onnx::shape_inference::InferShapes(model);

	Graph g;
	
	for (auto &i : model.graph().initializer())
	{
		Tensor tensor = { 1,1,1,1 };
		tensor.n = (int)i.dims(0);
		if(i.dims_size() > 1)
			tensor.channel = (int)i.dims(1);
		if(i.dims_size() > 2)
			tensor.width = (int)i.dims(2);
		if(i.dims_size() > 3)
			tensor.height = (int)i.dims(3);

		assert(i.dims_size() <= 4);
		assert(g.tensors.find(i.name()) == g.tensors.end());

		g.tensors.emplace(i.name(), tensor);

		auto data_type = i.data_type();
		switch (data_type)
		{
		case onnx::TensorProto::INT64:
		{
			auto data = onnx::ParseData<int64_t>(&i);
			g.values.emplace(i.name(), data);
		}
		break;
		case onnx::TensorProto::FLOAT:
		{
			auto data = onnx::ParseData<float>(&i);
			g.values.emplace(i.name(), data);
		}
		break;
		default:
			printf("unsupported data type %d for initializer '%s'\n",
				data_type, i.name().c_str());
			break;
		}
	}

	for (auto &i : model.graph().input())
	{
		Tensor tensor = value_info(i);

		assert(g.tensors.find(i.name()) == g.tensors.end() || g.tensors.at(i.name()) == tensor);

		g.tensors.emplace(i.name(), tensor);
	}

	for (auto &o : model.graph().output())
	{
		Tensor tensor = value_info(o);
		assert(g.tensors.find(o.name()) == g.tensors.end() || g.tensors.at(o.name()) == tensor);
		g.tensors.emplace(o.name(), tensor);
	}

	for (auto &v : model.graph().value_info())
	{
		Tensor tensor = value_info(v);
		assert(g.tensors.find(v.name()) == g.tensors.end() || g.tensors.at(v.name()) == tensor);
		g.tensors.emplace(v.name(), tensor);
	}

	for (auto &n : model.graph().node())
	{
		std::string name = n.output(0);
		Node node;

		node.name = name;
		//node.op_name = n.op_type();
		node.op_type = str_to_op_type(n.op_type());

		for (auto &a : n.attribute())
		{
			switch (a.type())
			{
			case onnx::AttributeProto::INT:
				node.attrs.emplace(a.name(), a.i());
				break;
			case onnx::AttributeProto::INTS:
				node.attrs.emplace(a.name(), std::vector<int64_t>{a.ints().begin(), a.ints().end()});
				break;
			case onnx::AttributeProto::STRING:
				node.attrs.emplace(a.name(), a.s());
				break;
			case onnx::AttributeProto::TENSOR:
				break;
			default:
				printf("unsupported attribute '%s' of type %d for node '%s'\n", a.name().c_str(), a.type(), name.c_str());
				return {};
			}
		}

		auto filtered_inputs = filter_inputs(node.op_type, n.input());
		for (auto &input : filtered_inputs)
		{
			node.inputs.push_back(input);

			if (g.forw.find(input) != g.forw.end())
				g.forw.at(input).push_back(name);
			else
				g.forw.emplace(input, vector<string>{ name });
		}

		g.nodes.emplace(name, node);
	}

	for (auto &begin : g.forw)
	{
		for (auto &end : begin.second)
		{
			g.back[end].push_back(begin.first);
		}
	}

	return g;
}

bool is_all_present(const set<string> &visited, const vector<string> &inputs)
{
	for (auto &i : inputs)
	{
		if (visited.find(i) == visited.end())
			return false;
	}
	return true;
}

int Graph::walk_forward(const string &beg, Callback f) const
{
	assert(forw.find(beg) != forw.end());

	int level = 0;
	set<string> children = { beg };
	set<string> visited;

	while (!children.empty())
	{
		int ret = f(*this, children, level);
		if (ret)return ret;

		visited.insert(children.begin(), children.end());

		set<string> next;
		for(auto &child:children)
		{
			auto it_child = forw.find(child);
			if (it_child != forw.end())
			{
				for (auto &n : it_child->second)
				{
					if(is_all_present(visited, nodes.at(n).inputs))
						next.insert(n);
				}
			}
		}
		children = next;
		++level;
	}

	return 0;
}

int Graph::walk_backward(const std::string &beg, Callback f) const
{
	assert(back.find(beg) != back.end());
	return 0;
}

vector<Field> Graph::receptive_field(const string &name, Direction dir) const
{
	if (nodes.find(name) == nodes.end())
	{
		assert(tensors.find(name) != tensors.end());
		return {};
	}

	const auto &node = nodes.at(name);
	
	switch (node.op_type)
	{
	case OpType::Relu:
	case OpType::Dropout:
	case OpType::LRN:
		return identity_field(node, dir);

	case OpType::Conv:
		return conv_field(node, dir);

	case OpType::MaxPool:
		assert(node.inputs.size() == 1);
		return conv_field(node, dir);

	case OpType::Concat:
		return concat_field(node, dir);

	case OpType::GlobalAveragePool:
		return gapool_field(node, dir);

	case OpType::Resize:
		return resize_field(node, dir);

	case OpType::Add:
		return add_field(node, dir);

	default:
		printf("receptive field for op '%s' not implemented\n", str_from_op_type(node.op_type));
		return {};
	}
}

int Graph::length(const std::string &name, Direction dir) const
{
	auto &tensor = tensors.at(name);

	switch (dir)
	{
	case Direction::ByRows:
		return tensor.height;
	case Direction::ByColumns:
		return tensor.width;
	default:
		assert(false);
		return -1;
	}
}

std::vector<Field> Graph::identity_field(const Node &node, Direction dir, int input_idx) const
{
	int in_length = length(node.inputs[input_idx], dir);
	int out_length = length(node.name, dir);

	assert(in_length == out_length);

	Field field;
	field.input = node.inputs[input_idx];
	field.output = node.name;
	field.field.reserve(in_length);

	for (int i = 0; i < in_length; ++i)
	{
		field.field.push_back({ i,i + 1, i, i + 1 });
	}

	return {field};
}

std::vector<Field> Graph::conv_field(const Node &node, Direction direction) const
{
	int dir = (int)direction;
	auto &in_tensor = tensors.at(node.inputs[0]);
	auto &out_tensor = tensors.at(node.name);

	int in_length;
	int out_length;
	switch (direction)
	{
	case Direction::ByRows:
		in_length = in_tensor.height;
		out_length = out_tensor.height;
		break;
	case Direction::ByColumns:
		in_length = in_tensor.width;
		out_length = out_tensor.width;
		break;
	default:
		assert(false);
		break;
	}

	int stride = 1;
	int dilation = 1;
	int kernel = 0;
	int pad_beg = 0;
	int pad_end = 0;

	auto end = node.attrs.end();

	auto it_strides = node.attrs.find("strides");
	if (it_strides != end)
	{
		auto &attr_strides = std::get<std::vector<int64_t>>(it_strides->second);
		stride = (int)attr_strides[dir];
	}

	auto it_dilations = node.attrs.find("dilations");
	if (it_dilations != end)
	{
		auto &attr_dilations = std::get<std::vector<int64_t>>(it_dilations->second);
		dilation = (int)attr_dilations[dir];
	}

	auto it_kernel = node.attrs.find("kernel_shape");
	if (it_kernel != end)
	{
		auto &attr_kernel = std::get<std::vector<int64_t>>(it_kernel->second);
		kernel = (int)attr_kernel[dir];
	}
	else
	{
		assert(false);
	}

	auto it_pads = node.attrs.find("pads");
	if (it_pads != end)
	{
		auto &attr_pads = std::get<std::vector<int64_t>>(it_pads->second);
		pad_beg = (int)attr_pads[2 * dir + 0];
		pad_end = (int)attr_pads[2 * dir + 1];
	}

	Field field;
	field.input = node.inputs[0];
	field.output = node.name;
	field.field.reserve(out_length);

	for (int i = -pad_beg, j = 0; i < in_length + pad_end && j < out_length; i += stride, ++j)
	{
		FromTo ray;
		ray.from_input = i;
		ray.to_input = min(i + kernel * dilation, in_length + pad_end);
		ray.from_output = j;
		ray.to_output = j + 1;
		field.field.push_back(ray);
	}

	return { field };
}

std::vector<Field> Graph::concat_field(const Node &node, Direction dir) const
{
	int axis = 1;

	auto it_axis = node.attrs.find("axis");
	if (it_axis != node.attrs.end())
	{
		axis = (int)std::get<int64_t>(it_axis->second);
	}

	if (axis != 2 && axis != 3)
	{
		std::vector<Field> ff;
		for (int i = 0; i < node.inputs.size(); ++i)
		{
			auto f = identity_field(node, dir, i);
			ff.push_back(f[0]);
		}

		return ff;
	}
	assert(false);
	return {};
}

std::vector<Field> Graph::add_field(const Node &node, Direction dir) const
{
	std::vector<Field> fields;
	for (int i = 0; i < node.inputs.size(); ++i)
	{
		auto f = identity_field(node, dir, i);
		fields.push_back(f[0]);
	}
	return fields;
}

std::vector<Field> Graph::gapool_field(const Node &node, Direction dir) const
{
	int in_length = length(node.inputs[0], dir);
	Field f;
	f.input = node.inputs[0];
	f.output = node.name;
	f.field.push_back({ 0,in_length,0,1 });

	return { f };
}

std::vector<Field> Graph::resize_field(const Node &node, Direction dir) const
{
	return {};
}
