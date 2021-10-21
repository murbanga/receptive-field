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

struct BasePoint
{
	Point base;
	int n;
};

/*struct Row
{
	int n;
	std::string name;
	std::vector<Field> fields;
};*/

struct VertexArray
{
	GLuint arr = 0;
	GLuint buf = 0;
	GLsizei size = 0;
};

struct FieldView
{
	GLint offset;
	Field ray_field;
	std::vector<GLsizei> ray_indexes;
};

class GraphView
{
	Graph *g;
	std::string start_node;
	float cell_width = 0.3f;
	float cell_height = 0.1f;
	float node_width_margin = 1.0f;
	float node_height_margin = 1.0f;
	int bezier_interp_steps = 20;
	bool is_draw_field_per_pixel = false;

	Direction direction = Direction::ByRows;

	float graph_width;
	float graph_height;
	int graph_depth;

	VertexArray tensors;
	VertexArray fields;
	VertexArray fields_borders;
	std::vector<FieldView> field_views;
	std::map<std::string, BasePoint> base_points;

	std::string selected_name;
	int selected_beg_pixel = -1;
	int selected_end_pixel = -1;

	std::string hovered_name;
	int hovered_pixel = -1;

	std::pair<std::vector<Point3f>, std::vector<GLsizei>> render_field(const Field &field, const Point &from, const Point &to, float dz, float *z) const;
	std::vector<Point3f> render_ray(const Point &from, const Point &to, const FromTo &ray, float z) const;
	void update_layout();
public:
	GraphView(Graph *g, const std::string &start_node);
	~GraphView();

	float width() const { return graph_width; }
	float height() const { return graph_height; }
	const Graph *graph() const { return g; }
	std::string selected()const { return selected_name; }

	void draw();

	float get_cell_width() const { return cell_width; }
	float get_node_width_margin() const { return node_width_margin; }
	float get_node_height_margin() const { return node_height_margin; }

	void set_layout(float cell_width, float node_width_margin, float node_height_margin);
	void set_draw_field_per_pixel(bool val) { is_draw_field_per_pixel = val; }

	std::pair<std::string, int> hit_test(double x, double y) const;

	void set_selected(const std::string &name, int beg = -1, int end = -1) { selected_name = name; selected_beg_pixel = beg; selected_end_pixel = end; }
	void set_hovered(const std::string &name, int index) { hovered_name = name; hovered_pixel = index; }

};