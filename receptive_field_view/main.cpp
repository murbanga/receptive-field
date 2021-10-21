#ifdef WIN32
#include <Windows.h>
#endif

#include <glad/gl.h>
#include <GL/glu.h>
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include "graph.h"
#include "graph_view.h"
#include "utils.h"

enum class OperatorMode
{
	Constructing,
	Running,
};

enum class DragMode
{
	Disabled = 0,
	Panning = 1,
	SelectingPixels = 2,
};

static const int point_selection_max_distance_px = 4;

static OperatorMode operator_mode;
static double zoom = 1;
static double modelx = 0;
static double modely = 0;
static double last_pixel_size = -1;

static DragMode drag_mode = DragMode::Disabled;
static double drag_startx = -1;
static double drag_starty = -1;
static double drag_start_modelx;
static double drag_start_modely;
static int moving_model_index = -1;

//static GLuint current_array;
//static GLuint current_buf;

void unproj(double x, double y, double &objx, double &objy)
{
	double model[16];
	double proj[16];
	int viewport[4];
	double objz;

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluUnProject(x, viewport[3] - y, 0, model, proj, viewport, &objx, &objy, &objz);
}

void key(GLFWwindow *window, int key, int scancode, int action, int flags)
{}

void mouse(GLFWwindow *window, int button, int state, int flags)
{
	ImGuiIO &io = ImGui::GetIO();
	if (io.WantCaptureMouse)return;

	GraphView *view = reinterpret_cast<GraphView *>(glfwGetWindowUserPointer(window));

	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		if (state == GLFW_PRESS)
		{
			drag_mode = DragMode::Panning;
			double x, y;
			glfwGetCursorPos(window, &x, &y);

			double ptx, pty;
			unproj(x, y, ptx, pty);

			drag_startx = x;
			drag_starty = y;
			drag_start_modelx = modelx;
			drag_start_modely = modely;
		}
		else if (state == GLFW_RELEASE)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			if (x == drag_startx && y == drag_starty)
			{
				double ptx, pty;
				unproj(x, y, ptx, pty);

				auto [tensor_name, end] = view->hit_test(ptx, pty);
				view->set_selected(tensor_name);
			}

			drag_mode = DragMode::Disabled;
		}
		break;

	case GLFW_MOUSE_BUTTON_RIGHT:
		if (state == GLFW_PRESS)
		{
			/*double x, y;
			glfwGetCursorPos(window, &x, &y);
			double ptx, pty;
			unproj(x, y, ptx, pty);
			Point mouse{ static_cast<float>(ptx),static_cast<float>(pty) };
			auto [nearest, i] = fractal.nearest(mouse);
			if (i >= 0 && distance(nearest, mouse) < point_selection_max_distance_px * last_pixel_size &&
				operator_mode == OperatorMode::Constructing)
			{
				fractal.model.erase(fractal.model.begin() + i);
			}*/
		}
		break;
	}
	glfwPostEmptyEvent();
}

void scroll(GLFWwindow *window, double xscroll, double yscroll)
{
	ImGuiIO &io = ImGui::GetIO();
	if (io.WantCaptureMouse)return;

	if (yscroll > 0)
	{
		zoom /= 1.05;
	}
	else
	{
		zoom *= 1.05;
	}
	glfwPostEmptyEvent();
}

void motion(GLFWwindow *window, double x, double y)
{
	GraphView *view = reinterpret_cast<GraphView *>(glfwGetWindowUserPointer(window));

	if (drag_mode == DragMode::Disabled)
	{
		double mx, my;
		unproj(x, y, mx, my);
		auto [name, idx] = view->hit_test(mx, my);
		view->set_hovered(name, idx);
	}
	else
	{
		double startx, starty, endx, endy;
		unproj(drag_startx, drag_starty, startx, starty);
		unproj(x, y, endx, endy);

		switch (drag_mode) {
		case DragMode::Panning:
		{
			modelx = drag_start_modelx + startx - endx;
			modely = drag_start_modely + starty - endy;
			//drag_iterations++;
			break;
		}
		case DragMode::SelectingPixels:
		{

		}
		break;
		default:
			break;
		}
	}
}

void display(GLFWwindow *window, GraphView *view)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	double mousex, mousey;
	unproj(x, y, mousex, mousey);

	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);

	double model_width = view->width();
	double model_height = view->height();
	double model_ar = model_width / model_height;
	double ar = double(width) / double(height);

	if (ar < model_ar) {
		model_height = model_width / ar;
	}
	else {
		model_width = ar * model_height;
	}

	double model_left = modelx - model_width / 2 * zoom;
	double model_right = modelx + model_width / 2 * zoom;
	double model_top = modely - model_height / 2 * zoom;
	double model_bottom = modely + model_height / 2 * zoom;
	double pixel_size = (model_right - model_left) / width;

	last_pixel_size = pixel_size;

	glViewport(0, 0, width, height);

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(model_left, model_right, model_top, model_bottom, -1e3, 1e3);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.1f, 0.1f, 0.1f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	const float gridsize = 20;
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_LINES);
	for (float i = -gridsize; i <= gridsize; i += 1) {
		glVertex3f(i, -gridsize, 0);
		glVertex3f(i, gridsize, 0);
	}
	for (float i = -gridsize; i <= gridsize; i += 1) {
		glVertex3f(-gridsize, i, 0);
		glVertex3f(gridsize, i, 0);
	}
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-gridsize, 0, 0);
	glVertex3f(gridsize, 0, 0);
	glVertex3f(0, -gridsize, 0);
	glVertex3f(0, gridsize, 0);
	glEnd();
#endif

	glPushMatrix();
	glTranslatef(0.f, 0.f, 1.f);

	view->draw();

	glPopMatrix();
}

void glfw_error_callback(int error, const char *description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

std::string attribute_to_string(const Attribute &a)
{
	switch(a.index())
	{
	case 0: // int64_t
	{
		auto v = std::get<int64_t>(a);
		return std::to_string(v);
	}
	case 1:
	{
		auto vec = std::get<std::vector<int64_t>>(a);
		std::string s;
		for (auto &v : vec)
		{
			s += std::to_string(v) + ", ";
		}
		return s;
	}
	case 2:
		return std::get<std::string>(a);
	}
	assert(false);
	return "";
}

static std::map<std::string, std::string> attribute_edit_cache;
static bool per_pixel_field = false;
static double current_fps = 0;

void draw_ui(GraphView *view)
{
	using namespace ImGui;
	if (Begin("settings"))
	{
		Text("%.2f fps", current_fps);
		//Button("reset zoom and pos");
		if (Checkbox("Field each pixel", &per_pixel_field))
		{
			view->set_draw_field_per_pixel(per_pixel_field);
		}
		
		Separator();
		Text("Layout");

		float cw = view->get_cell_width();
		float nwm = view->get_node_width_margin();
		float nhm = view->get_node_height_margin();
		bool is_update_layout = false;

		is_update_layout |= SliderFloat("Cell width", &cw, 0.001f, 10.0f);
		is_update_layout |= SliderFloat("Node width margin", &nwm, 0.001f, 10.0f);
		is_update_layout |= SliderFloat("Node height margin", &nhm, 0.001f, 10.0f);

		if (is_update_layout)
		{
			view->set_layout(cw, nwm, nhm);
		}
	}
	End();

	if (Begin("selected"))
	{
		auto *graph = view->graph();
		auto selected = view->selected();
		if(!selected.empty())
		{
			const Tensor &tensor = graph->tensors.at(selected);
			Text("%s", selected.c_str());
			Text("%dx%dx%dx%d", tensor.n, tensor.channel, tensor.height, tensor.width);
			auto it_node = graph->nodes.find(selected);
			if (it_node != graph->nodes.end())
			{
				Separator();
				Text("%s", it_node->second.op_name.c_str());
				Separator();
				Text("attributes");

				for (auto &a : it_node->second.attrs)
				{
					std::string key = selected + "_" + a.first;
					if (attribute_edit_cache.find(key) == attribute_edit_cache.end())
					{
						attribute_edit_cache[key] = attribute_to_string(a.second);
					}
					InputText(a.first.c_str(), &attribute_edit_cache.at(key));
				}
			}
		}
		else
		{
			Text("Nothing selected");
		}
	}
	End();
}

int main(int argc, char **argv)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	GLFWwindow *window = glfwCreateWindow(1280, 720, "receptive field viewer", nullptr, nullptr);
	if (!window)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetMouseButtonCallback(window, mouse);
	glfwSetScrollCallback(window, scroll);
	glfwSetCursorPosCallback(window, motion);
	glfwSetKeyCallback(window, key);

	gladLoadGL(glfwGetProcAddress);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	io.FontDefault = io.Fonts->AddFontDefault();

	// D:/models/vision/body_analysis/age_gender/models/age_googlenet.onnx
	auto graph = Graph::load("C:/temp/squeezenet1.0-3.onnx");
	GraphView graph_view(&graph, "data_0");

	glfwSetWindowUserPointer(window, &graph_view);

	FpsCounter counter(10);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		draw_ui(&graph_view);

		ImGui::Render();

		display(window, &graph_view);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);

		current_fps = counter.tick();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}