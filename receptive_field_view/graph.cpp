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

bool operator==(const Tensor &a, const Tensor &b)
{
	return a.n == b.n && a.channel == b.channel && a.width == b.width && a.height == b.height;
}

OpType str_to_op_type(const string &name)
{
	if (name == "Conv")
		return OpType::Conv;
	if (name == "Relu")
		return OpType::Relu;
	if (name == "MaxPool")
		return OpType::MaxPool;
	if (name == "Concat")
		return OpType::Concat;
	if (name == "Dropout")
		return OpType::Dropout;
	if (name == "Constant")
		return OpType::Constant;
	if (name == "GlobalAveragePool")
		return OpType::GlobalAveragePool;
	if (name == "Flatten")
		return OpType::Flatten;
	if (name == "Softmax")
		return OpType::Softmax;
	if (name == "Reshape")
		return OpType::Reshape;

	assert(false);
	return OpType::Undefined;
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

Graph Graph::load(const char *filename)
{
	ifstream file{ filename, ios::in | ios::binary };
	onnx::ModelProto model;

	if (!model.ParseFromIstream(&file))
	{
		printf("fail to load graph from '%s'\n", filename);
		return {};
	}

	constexpr int64_t original_version = 15;
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

		for (auto &input : n.input())
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

int Graph::walk_forward(const string &beg, std::function<int(const Graph &g, const set<string> &names, int level)> f) const
{
	assert(forw.find(beg) != forw.end());

	int level = 0;
	set<string> children = { beg };

	while (!children.empty())
	{
		int ret = f(*this, children, level);
		if (ret)return ret;

		set<string> next;
		for(auto &child:children)
		{
			if (forw.find(child) != forw.end())
			{
				auto next_child = forw.at(child);
				for (auto &n : next_child)
				{
					next.insert(n);
				}
			}
		}
		children = next;
		++level;
	}

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
		assert(node.inputs.size() == 1);
		return identity_field(node, dir);

	case OpType::Conv:
		assert(node.inputs.size() == 3);
		return conv_field(node, dir);

	case OpType::MaxPool:
		assert(node.inputs.size() == 1);
		return conv_field(node, dir);

	case OpType::Concat:
		return concat_field(node, dir);

	default:
		//assert(false);
		printf("receptive field for op '%d' not implemented\n", (int)node.op_type);
		return {};
	}
}

std::vector<Field> Graph::identity_field(const Node &node, Direction dir) const
{
	return std::vector<Field>();
}

std::vector<Field> Graph::conv_field(const Node &node, Direction direction) const
{
	int dir = (int)direction;
	auto &out_tensor = tensors.at(node.name);
	auto &in_tensor = tensors.at(node.inputs[0]);
	
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

	for (int i = -pad_beg, j = 0; i < in_length + pad_end; i += stride, ++j)
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
	return std::vector<Field>();
}
