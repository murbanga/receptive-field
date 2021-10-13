#include <fstream>

#pragma warning (push)
#pragma warning (disable: 4244 4267)
#define ONNX_NAMESPACE onnx
#include "../onnx/onnx/onnx_pb.h"
#include "../onnx/onnx/version_converter/convert.h"
#include "../onnx/onnx/shape_inference/implementation.h"
#pragma warning (pop)

#include "graph.h"


Graph load(const char *filename)
{
	std::ifstream file{ filename, std::ios::in | std::ios::binary };
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
	return g;
}