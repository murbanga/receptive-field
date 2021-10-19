#pragma once
#include <glad/gl.h>

class Graph;

struct Row
{
	int n;
	std::string name;
};

struct Point
{
	float x;
	float y;
};

class GraphView
{
	Graph *g;

	float cell_width = 0.1f;
	float node_width_margin = 1.0f;
	float node_height_margin = 3.0f;

	enum class Direction
	{
		ByRows = 0,
		ByColumns = 1,
	} direction = Direction::ByRows;

	float graph_width;
	float graph_height;
	
	GLuint array_projection;
	GLuint buf_projection;
	GLsizei array_size;

	std::vector<Point> projection_view(const std::vector<std::vector<Row>> &proj) const;
	void update_layout();
public:
	GraphView(Graph *g);
	~GraphView();

	float width() const { return graph_width; }
	float height() const { return graph_height; }
	void draw();

	float get_cell_width() const { return cell_width; }
	float get_node_width_margin() const { return node_width_margin; }
	float get_node_height_margin() const { return node_height_margin; }

	void set_layout(float cell_width, float node_width_margin, float node_height_margin);
};