#include <assert.h>

#include "graph.h"
#include "graph_view.h"

#include "../bazier/include/bezier.h"
#include "colors.h"
#include "utils.h"

using namespace std;

template<>
void VertexArray::update<Point>(const Point *points, size_t npoints)
{
	glBindVertexArray(arr);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, npoints*2*sizeof(float), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	size = (GLsizei)npoints;
}

template<>
void VertexArray::update<Point3f>(const Point3f *points, size_t npoints)
{
	glBindVertexArray(arr);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, npoints * 3 * sizeof(float), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	size = (GLsizei)npoints;
}

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &arr);
	glGenBuffers(1, &buf);
}

VertexArray::~VertexArray()
{
	glDeleteBuffers(1, &buf);
	glDeleteVertexArrays(1, &arr);
}

GraphView::GraphView(Graph *g, const std::string &start_node) :g(g), start_node(start_node), font("Arial", 0.1f)
{
	graph_depth = 0;
	g->walk_forward(start_node, [&](const Graph &g, const set<string> &names, int level) {graph_depth = level + 1; return 0; });

	update_layout();
}

GraphView::~GraphView()
{
}

vector<vector<int>> GraphView::get_rows_heights() const
{
	map<string, int> levels;
	vector<vector<int>> rows_heights(graph_depth);

	g->walk_forward(start_node, [&](const Graph &g, const set<string> &names, int level)
	{
		for (auto &name : names)
		{
			rows_heights[level].push_back(g.length(name, direction));
		}

		for (auto &name : names)
		{
			levels.emplace(name, level);

			auto it = g.back.find(name);
			if (it == g.back.end())continue;

			int height = g.length(name, direction);

			for (auto &input : it->second)
			{
				int input_level = levels.at(input);

				// add extra row for skipped connections
				for (int i = input_level; i < level - 1; ++i)
				{
					rows_heights[i].push_back(height);
				}
			}
		}

		return 0;
	});

	return rows_heights;
}

template <typename T>
T sum(const vector<T> &v)
{
	T s = 0;
	for (auto &i : v)s += i;
	return s;
}

void GraphView::update_layout()
{
	float max_width = 0;
	float max_level = 0;

	vector<Point> tensor_points;
	vector<Point3f> fields_points;
	vector<FieldView> f_views;

	float x = -graph_depth * node_width_margin / 2;
	float z = 0;

	vector<vector<int>> rows_heights = get_rows_heights();

	g->walk_forward(start_node, [&](const Graph &g, const set<string> &names, int level) 
	{
		/*float row_height = (names.size() - 1) * node_height_margin;

		for (auto &name : names)
		{
			int n = g.length(name, direction);
			row_height += (float)n * cell_height;
		}*/

		float y = -(sum(rows_heights[level]) * cell_height + (rows_heights.size() - 1) * node_height_margin) / 2;

		for (auto &name : names)
		{
			int n = g.length(name, direction);

			base_points[name] = { {x,y}, n, level };
			tensor_points.reserve(tensor_points.size() + n * 2 + 2);

			tensor_points.push_back({ x, y });
			tensor_points.push_back({ x + cell_width, y });

			for (int i = 0; i < n; ++i)
			{
				tensor_points.push_back({ x, y });
				tensor_points.push_back({ x, y + cell_height });

				tensor_points.push_back({ x, y + cell_height });
				tensor_points.push_back({ x + cell_width,y + cell_height });

				tensor_points.push_back({ x + cell_width, y + cell_height });
				tensor_points.push_back({ x + cell_width, y });

				y += cell_height;
			}
			y += node_height_margin;

			auto fields = g.receptive_field(name, direction);

			for (auto &field : fields)
			{
				auto &from = base_points.at(field.input);
				auto &to = base_points.at(name);

				assert(from.level < to.level);

				auto [points, indexes] = render_field(field, from.base, to.base, 0.01f, &z);

				FieldView view;
				view.offset = (GLint)fields_points.size();
				view.ray_field = field;
				view.ray_indexes = indexes;
				f_views.push_back(view);
				field_views_of_output[name].push_back(f_views.size() - 1);

				fields_points.insert(fields_points.end(), points.begin(), points.end());
			}
		}

		max_width = max(max_width, y);
		max_level = max(max_level, x);

		x += node_width_margin;
		
		return 0;
	});

	graph_width = max_width;
	graph_height = max_level;
	field_views = f_views;
	topmost_point_z = z;

	printf("graph size %f x %f\n", max_width, max_level);

	tensors.update(tensor_points.data(), tensor_points.size());
	fields.update(fields_points.data(), fields_points.size());
}

void GraphView::draw_pixel_range(const Point &base, int beg, int end) const
{
	float x0 = base.x;
	float x1 = x0 + cell_width;
	float y0 = base.y + beg * cell_height;
	float y1 = base.y + end * cell_height;

	glBegin(GL_QUADS);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glVertex2f(x1, y0);
	glEnd();
}

void GraphView::draw()
{
#if 0 // FIXME: brokes drawing of fields
	glColor3f(1, 1, 1);
	for (auto &base_point : base_points)
	{
		glPushMatrix();
		glTranslatef(base_point.second.base.x, base_point.second.base.y, 0);

		auto it_node = g->nodes.find(base_point.first);
		if (it_node != g->nodes.end())
		{
			font.draw(ssprintf("%s %d", it_node->second.name.c_str(), base_point.second.n));
		}
		else
		{
			auto &tensor = g->tensors.find(base_point.first);
			font.draw(ssprintf("%s", base_point.first.c_str()));
		}

		glPopMatrix();
	}
#endif

	glBindVertexArray(tensors.arr);
	glBindBuffer(GL_ARRAY_BUFFER, tensors.buf);
	glColor3f(1, 1, 1);
	glDrawArrays(GL_LINES, 0, tensors.size);

	glColor4fv(Colors::inactive_field);
	glBindVertexArray(fields.arr);
	glBindBuffer(GL_ARRAY_BUFFER, fields.buf);

	if (is_draw_field_per_pixel)
	{
		for (auto &field_view : field_views)
		{
			auto &indexes = field_view.ray_indexes;
			size_t n = indexes.size();
			GLint offset = field_view.offset;

			GLsizei size = indexes[n - 1] - indexes[0];
			glDrawArrays(GL_TRIANGLE_STRIP, offset + indexes[0], size);
		}
	}
	else
	{
		for (auto &field : field_views)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, field.offset, field.ray_indexes[0]);
		}
	}

	auto it_selected = base_points.find(selected_name);
	if (it_selected != base_points.end())
	{
		set<string> receptive_field_visited_cache, affected_output_visited_cache;

		glPushMatrix();
		glTranslatef(0, 0, topmost_point_z + 0.5f);

		glColor4fv(Colors::selected_receptive_field);
		draw_receptive_field(selected_name, receptive_field_visited_cache, selected_beg_pixel, selected_end_pixel);

		glColor4fv(Colors::selected_affected_output);
		draw_affected_output(selected_name, affected_output_visited_cache, selected_beg_pixel, selected_end_pixel);

		glPopMatrix();

		float margin = cell_width * 0.3f;
		auto b = it_selected->second;
		glColor4fv(Colors::selected_tensor);

		glBegin(GL_LINE_LOOP);
		glVertex2f(b.base.x - margin, b.base.y - margin);
		glVertex2f(b.base.x - margin, b.base.y + b.n * cell_height + margin);
		glVertex2f(b.base.x + cell_width + margin, b.base.y + b.n * cell_height + margin);
		glVertex2f(b.base.x + cell_width + margin, b.base.y - margin);
		glEnd();

		if (selected_beg_pixel >= 0 && selected_end_pixel >= 0)
		{
			glColor4fv(Colors::selected_pixel);
			draw_pixel_range(b.base, selected_beg_pixel, selected_end_pixel);
		}
	}

	auto it_hovered = base_points.find(hovered_name);
	if (it_hovered != base_points.end())
	{
		set<string> receptive_field_visited_cache, affected_output_visited_cache;

		glPushMatrix();
		glTranslatef(0, 0, topmost_point_z + 0.1f);

		glColor4fv(Colors::hovered_receptive_field);
		draw_receptive_field(hovered_name, receptive_field_visited_cache, hovered_idx, hovered_idx + 1);

		glColor4fv(Colors::hovered_affected_output);
		draw_affected_output(hovered_name, affected_output_visited_cache, hovered_idx, hovered_idx + 1);

		auto &hov = it_hovered->second;
		float x0 = hov.base.x;
		float x1 = x0 + cell_width;
		float y0 = hov.base.y + hovered_idx * cell_height;
		float y1 = y0 + cell_height;

		glColor4fv(Colors::hovered_pixel);
		glBegin(GL_QUADS);
		glVertex2f(x0, y0);
		glVertex2f(x0, y1);
		glVertex2f(x1, y1);
		glVertex2f(x1, y0);
		glEnd();

		glPopMatrix();
	}

#if 0
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
#endif
}

void GraphView::draw_receptive_field(const string &name, set<string> &visited, int beg, int end, int level) const
{
	auto it = field_views_of_output.find(name);
	if (it == field_views_of_output.end())return;

	for (auto idx : it->second)
	{
		auto &indexes = field_views[idx].ray_indexes;
		GLint offset = field_views[idx].offset;

		int clamped_beg = max(0, beg);
		int clamped_end = min((int)indexes.size() - 1, end);

		glDrawArrays(GL_TRIANGLE_STRIP, offset + indexes[clamped_beg], indexes[clamped_end] - indexes[clamped_beg]);

		if (!level)
		{
			draw_pixel_range(base_points.at(name).base, clamped_beg, clamped_end);
		}

		auto range = find_input(field_views[idx].ray_field.field, clamped_beg, clamped_end);

		// TODO: avoid drawing joined receptive fields twice
		draw_receptive_field(field_views[idx].ray_field.input, visited, range.beg, range.end, level + 1);
	}
}

void GraphView::draw_affected_output(const std::string &name, set<string> &visited, int beg, int end) const
{
	if (end == beg)return;

	auto users = g->forw.find(name);

	if (users == g->forw.end())return;

	for (auto &user : users->second)
	{
		auto it = field_views_of_output.find(user);
		if (it == field_views_of_output.end())
			continue;

		for(auto idx : it->second)
		{
			if (field_views[idx].ray_field.input != name)
				continue;

			auto &indexes = field_views[idx].ray_indexes;
			GLint offset = field_views[idx].offset;

			auto range = find_output(field_views[idx].ray_field.field, beg, end);

			glDrawArrays(GL_TRIANGLE_STRIP, offset + indexes[range.beg], indexes[range.end] - indexes[range.beg]);

			draw_pixel_range(base_points.at(user).base, range.beg, range.end);

			draw_affected_output(user, visited, range.beg, range.end);
		}
	}
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
	Bezier::Bezier<3> curve({ {from.x, from.y},{(from.x + to.x) / 2,from.y}, {(from.x + to.x) / 2, to.y}, {to.x,to.y} });

	std::vector<Point> points(n + 1);
	points[0] = { from.x, from.y };
	for (int i = 0; i < n; ++i)
	{
		auto pt = curve.valueAt(float(i + 1) / n);
		points[i + 1] = { pt.x,pt.y };
	}
	return points;
}

vector<Point3f> zip(const vector<Point> &left, const vector<Point> &right, float z)
{
	assert(left.size() == right.size());
	vector<Point3f> points{ left.size() + right.size() };
	for (int i = 0; i < left.size(); ++i)
	{
		points[2 * i + 0] = { left[i].x, left[i].y, z };
		points[2 * i + 1] = { right[i].x, right[i].y, z };
	}
	return points;
}

vector<Point3f> GraphView::render_ray(const Point &from, const Point &to, const FromTo &ray, float z) const
{
	Point left_beg = { from.x + cell_width, from.y + ray.from_input * cell_height };
	Point left_end = { to.x,to.y + ray.from_output * cell_height };

	auto left = bezier(left_beg, left_end, bezier_interp_steps);

	Point right_beg = { from.x + cell_width,from.y + ray.to_input * cell_height };
	Point right_end = { to.x,to.y + ray.to_output * cell_height };

	auto right = bezier(right_beg, right_end, bezier_interp_steps);

	return zip(left, right, z);
}

pair<vector<Point3f>,vector<GLsizei>> GraphView::render_field(const Field &field, const Point &from, const Point &to, float dz, float *pz) const
{
	FromTo one;
	one.from_input = field.field.front().from_input;
	one.from_output = field.field.front().from_output;
	one.to_input = field.field.back().to_input;
	one.to_output = field.field.back().to_output;

	std::vector<GLsizei> indexes;
	auto points = render_ray(from, to, one, *pz);
	*pz += dz;
	indexes.push_back((GLsizei)points.size());

	for (auto &ray : field.field)
	{
		auto r = render_ray(from, to, ray, *pz);
		*pz += dz;
		points.insert(points.end(), r.begin(), r.end());
		indexes.push_back((GLsizei)points.size());
	}

	return { points, indexes };
}

pair<string, int> GraphView::hit_test(double x, double y) const
{
	for (auto &base_point : base_points)
	{
		BasePoint b = base_point.second;
		if (b.base.x <= x && x < b.base.x + cell_width &&
			b.base.y <= y && y < b.base.y + b.n * cell_height)
		{
			int idx = int((y - b.base.y) / cell_height);
			return { base_point.first, idx };
		}
	}
	return { "", -1 };
}
