#include <assert.h>

#include "graph.h"
#include "graph_view.h"

#include "../bazier/include/bezier.h"

using namespace std;

void update_va(GLuint arr, GLuint buf, const Point *points, size_t npoints)
{
	glBindVertexArray(arr);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, npoints*2*sizeof(float), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GraphView::GraphView(Graph *g, const std::string &start_node) :g(g), start_node(start_node)
{
	glGenVertexArrays(1, &array_tensors);
	glGenBuffers(1, &buf_tensors);

	graph_depth = 0;
	g->walk_forward(start_node, [&](const Graph &g, const set<string> &names, int level) {graph_depth = level+1; return 0; });

	update_layout();
}

void GraphView::update_layout()
{
	float max_width = 0;
	float max_level = 0;

	//projection.clear();
	//std::vector<std::vector<Row>> projection;
	std::vector<Point> tensor_points;

	float x = -graph_depth * node_height_margin / 2;

	g->walk_forward(start_node, [&](const Graph &g, const set<string> &names, int level) 
	{
		//float width = ((float)names.size() - 1) * node_width_margin;
		//float x = (float)level * (cell_width + node_height_margin);

		float row_width = (names.size() - 1) * node_width_margin;
		for (auto &name : names)
		{
			Tensor t = g.tensors.at(name);
			int n;

			switch (direction)
			{
			case Direction::ByRows:
				n = t.height;
				break;
			case Direction::ByColumns:
				n = t.width;
				break;
			default:
				assert(false);
				break;
			}
			row_width += (float)n * cell_width;
		}
		float y = -row_width / 2;

		for (auto &name : names)
		{
			Tensor t = g.tensors.at(name);
			int n;

			switch (direction)
			{
			case Direction::ByRows:
				n = t.height;
				break;
			case Direction::ByColumns:
				n = t.width;
				break;
			default:
				assert(false);
				break;
			}

#if 0
			if (projection.size() <= level)
				projection.resize(level + 1);

			projection[level].push_back({ n, name, g.receptive_field(name, direction) });
#else

			base_points[name] = { x,y };
			tensor_points.reserve(tensor_points.size() + n * 2 + 2);

			tensor_points.push_back({ x, y });
			tensor_points.push_back({ x + cell_width, y });

			for (int i = 0; i < n; ++i)
			{
				tensor_points.push_back({ x, y });
				tensor_points.push_back({ x, y + cell_width });

				tensor_points.push_back({ x, y + cell_width });
				tensor_points.push_back({ x + cell_width,y + cell_width });

				tensor_points.push_back({ x + cell_width, y + cell_width });
				tensor_points.push_back({ x + cell_width, y });

				y += cell_width;
			}
			y += node_width_margin;
#endif
		}

		max_width = max(max_width, y);
		max_level = max(max_level, x);

		x += node_height_margin;
		
		return 0;
	});

	graph_width = max_width;
	graph_height = max_level;

	printf("graph size %f x %f\n", max_width, max_level);

	//auto [projection_array, points] = render_projection(projection);

//	base_points = points;
	update_va(array_tensors, buf_tensors, tensor_points.data(), tensor_points.size());
	size_tensors = (GLsizei)tensor_points.size();

	//for (auto &base_point : base_points)
	//{
	//	render_field()
	//}
}

GraphView::~GraphView()
{
	glDeleteBuffers(1, &buf_tensors);
	glDeleteVertexArrays(1, &array_tensors);
}

/*float GraphView::width_offset(const std::vector<Row> &level) const
{
	float width = (level.size() - 1) * node_width_margin;

	for (auto &row : level)
	{
		width += (float)row.n * cell_width;
	}

	return -width / 2;
}

std::tuple<std::vector<Point>, std::map<std::string, Point>> GraphView::render_projection(const vector<vector<Row>> &proj) const
{
	vector<Point> points;
	map<string, Point> base_points;

	float x = -(proj.size() * node_height_margin) / 2;

	for (auto &level : proj)
	{
		float y = width_offset(level);

		for (auto &row : level)
		{
			base_points[row.name] = { x,y };
			points.reserve(points.size() + row.n * 2 + 2);

			points.push_back({ x, y });
			points.push_back({ x + cell_width, y });

			for (int i = 0; i < row.n; ++i)
			{
				points.push_back({ x, y});
				points.push_back({ x, y + cell_width });

				points.push_back({ x, y + cell_width });
				points.push_back({ x + cell_width,y + cell_width });

				points.push_back({ x + cell_width, y + cell_width });
				points.push_back({ x + cell_width, y });

				y += cell_width;
			}
			y += node_width_margin;
		}
		x += node_height_margin;
	}

	return { points, base_points };
}*/

void GraphView::draw()
{
	glColor3f(1, 1, 1);
	glDrawArrays(GL_LINES, 0, size_tensors);

	//glBegin(GL_LINES);
	g->walk_forward("data_0", [&](const Graph &g, const std::set<std::string> &names, int level)
	{
		for (auto &name : names)
		{
			auto ot = g.back.find(name);
			if (ot == g.back.end())
				continue;

			for (auto &input : ot->second)
			{
				auto it = base_points.find(input);
				if (it == base_points.end())
					continue;
				auto from = it->second;
				auto to = base_points.at(name);

				from.x += cell_width;

				Bezier::Bezier<3> curve({ {from.x, from.y},{to.x,from.y}, {from.x, to.y}, {to.x,to.y} });

				int n = 10;
				glBegin(GL_LINE_STRIP);
				glVertex2f(from.x, from.y);
				for (int i = 0; i < n; ++i)
				{
					auto pt = curve.valueAt(float(i + 1) / n);
					glVertex2f(pt.x, pt.y);
				}
				glEnd();
			}
		}
		return 0;
	});
	//glEnd();
}

void GraphView::set_layout(float cell_width, float node_width_margin, float node_height_margin)
{
	this->cell_width = cell_width;
	this->node_width_margin = node_width_margin;
	this->node_height_margin = node_height_margin;

	update_layout();
}

vector<Point> bezier(const Point &from, const Point &to, int n)
{
	Bezier::Bezier<3> curve({ {from.x, from.y},{to.x,from.y}, {from.x, to.y}, {to.x,to.y} });

	std::vector<Point> points(n + 1);
	points[0] = { from.x, from.y };
	for (int i = 0; i < n; ++i)
	{
		auto pt = curve.valueAt(float(i + 1) / n);
		points[i + 1] = { pt.x,pt.y };
	}
	return points;
}

vector<Point> zip(const vector<Point> &left, const vector<Point> &right)
{
	assert(left.size() == right.size());
	vector<Point> points{ left.size() + right.size() };
	for (int i = 0; i < left.size(); ++i)
	{
		points[2 * i + 0] = left[i];
		points[2 * i + 1] = right[i];
	}
	return points;
}

vector<Point3f> GraphView::render_field(const Field &field, const Point &from, const Point &to, int interp_steps, float dz, float startz) const
{
	vector<FromTo> one{ 1 };
	one[0].from_input = field.field.front().from_input;
	one[0].from_output = field.field.front().from_output;
	one[0].to_input = field.field.back().to_input;
	one[0].to_output = field.field.back().to_output;

	float z = startz;
	vector<Point3f> result;

	for (auto &ray : one)
	{
		Point left_beg = { from.x + cell_width, from.y + ray.from_input * cell_width };
		Point left_end = { to.x,to.y + ray.from_output * cell_width };

		auto left = bezier(left_beg, left_end, interp_steps);

		Point right_beg = { from.x + cell_width,from.y + ray.to_input * cell_width };
		Point right_end = { to.x,to.y + ray.to_output * cell_width };

		auto right = bezier(right_beg, right_end, interp_steps);

		auto r = zip(left, right);
		result.reserve(result.size() + r.size());
		for (auto &pt : r)
		{
			result.push_back({ pt.x, pt.y, z });
		}
		z += dz;
	}

	return result;
}
