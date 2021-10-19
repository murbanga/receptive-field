#include <assert.h>

#include "graph.h"
#include "graph_view.h"

using namespace std;

void update_va(GLuint arr, GLuint buf, const Point *points, size_t npoints)
{
	glBindVertexArray(arr);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, npoints*2*sizeof(float), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GraphView::GraphView(Graph *g) :g(g)
{
	glGenVertexArrays(1, &array_projection);
	glGenBuffers(1, &buf_projection);

	float max_width = 0;
	float max_level = 0;

	std::vector<std::vector<Row>> projection;

	g->walk_forward("data_0", [&](const Graph &g, const set<string> &names, int level) 
	{
		float width = ((float)names.size() - 1) * node_width_margin;
		
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

			if (projection.size() <= level)
				projection.resize(level + 1);

			projection[level].push_back({ n, name });
			width += n * cell_width;
		}

		max_width = max(max_width, width);
		max_level = max(max_level, (float)level * (cell_width + node_height_margin));

		return 0;
	});

	graph_width = max_width;
	graph_height = max_level;

	printf("graph size %f x %f\n", max_width, max_level);

	auto projection_array = projection_view(projection);
	update_va(array_projection, buf_projection, projection_array.data(), projection_array.size());
	array_size = (GLsizei)projection_array.size();
}

GraphView::~GraphView()
{
	glDeleteBuffers(1, &buf_projection);
	glDeleteVertexArrays(1, &array_projection);
}

vector<Point> GraphView::projection_view(const vector<vector<Row>> &proj) const
{
	vector<Point> points;

	float x = -(proj.size() * node_height_margin) / 2;

	for (auto &level : proj)
	{
		float width = (level.size() - 1) * node_width_margin;
		
		for (auto &row : level)
		{
			width += (float)row.n * cell_width;
		}

		float y = -width / 2;

		for (auto &row : level)
		{
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

	return points;
}

void GraphView::draw()
{
	glColor3f(1, 1, 1);
	glDrawArrays(GL_LINES, 0, array_size);
}