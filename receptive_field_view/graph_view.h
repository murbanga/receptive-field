#pragma once
#include <glad/gl.h>

class Graph;

struct Point
{
	float x;
	float y;
};

struct Point3f
{
	float x;
	float y;
	float z;
};

struct Row
{
	int n;
	std::string name;
	std::vector<Field> fields;
};

class GraphView
{
	Graph *g;
	std::string start_node;
	float cell_width = 0.1f;
	float node_width_margin = 1.0f;
	float node_height_margin = 3.0f;

	Direction direction = Direction::ByRows;

	float graph_width;
	float graph_height;
	int graph_depth;
	
	GLuint array_tensors;
	GLuint buf_tensors;
	GLsizei size_tensors;
	std::map<std::string, Point> base_points;

	std::vector<GLuint> array_fields;
	std::vector<GLuint> buf_fields;
	GLsizei size_fields;

	std::tuple<std::vector<Point>,std::map<std::string, Point>> render_projection(const std::vector<std::vector<Row>> &proj) const;
	float width_offset(const std::vector<Row> &level) const;
	std::vector<Point3f> GraphView::render_field(const Field &field, const Point &from, const Point &to, int interp_steps, float dz, float startz) const;

	void update_layout();
public:
	GraphView(Graph *g, const std::string &start_node);
	~GraphView();

	float width() const { return graph_width; }
	float height() const { return graph_height; }
	void draw();

	float get_cell_width() const { return cell_width; }
	float get_node_width_margin() const { return node_width_margin; }
	float get_node_height_margin() const { return node_height_margin; }

	void set_layout(float cell_width, float node_width_margin, float node_height_margin);
};