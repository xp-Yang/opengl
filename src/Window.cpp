#include "Window.hpp"
#include <assert.h>
#include <utility>

float WINDOW_WIDTH(1920.0f);
float WINDOW_HEIGHT(1080.0f);

Window::Window(int width, int height)
{
	glfwInit();//初始化GLFW; glfwWindowHint()配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//告诉GLFW我们要使用的OpenGL版本是3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//告诉GLFW我们要使用的OpenGL版本是3.3,这样GLFW会在创建OpenGL上下文时做出适当的调整
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//明确告诉GLFW我们使用的是核心模式(Core-profile)
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);//不允许用户调整窗口的大小
	//glEnable(GL_MULTISAMPLE);
	//glfwWindowHint(GLFW_SAMPLES, 16);

	m_window = glfwCreateWindow(width, height, "RenderEngine", nullptr, nullptr);
	if (m_window == nullptr)
	{
		assert(false);
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_window);

	m_viewports["MainView"] = Viewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Viewport::Coordinates::GLCoordinates);
	glfwSetWindowUserPointer(m_window, &m_viewports["MainView"]);

	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		if (width <= 0 || height <= 0)
			return;

		float scale_factor = std::min((float)width / WINDOW_WIDTH, (float)height / WINDOW_HEIGHT);
		WINDOW_WIDTH = width;
		WINDOW_HEIGHT = height;

		//Viewport& main_viewport = *(Viewport*)glfwGetWindowUserPointer(window);
		//main_viewport.transToScreenCoordinates();
		//main_viewport.width *= scale_factor;
		//main_viewport.height *= scale_factor;
		//main_viewport.transToGLCoordinates();
		//TODO 封装进rhi
		//glViewport(main_viewport.x, main_viewport.y, main_viewport.width, main_viewport.height);
		});
}

void Window::shutdown()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(m_window);
}

void Window::update() const
{
	glfwPollEvents();//检查触发事件（键盘输入、鼠标移动等）
	glfwSwapBuffers(m_window);//交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色的大缓冲），输出在屏幕上。
}

void Window::setMainViewport(const Viewport& viewport)
{
	m_viewports["MainView"] = viewport;
	// TODO 封装进rhi
	glViewport(m_viewports["MainView"].x, m_viewports["MainView"].y, m_viewports["MainView"].width, m_viewports["MainView"].height);
}

void Window::setViewport(std::string id, const Viewport& viewport)
{
	m_viewports[id] = viewport;
	// TODO 封装进rhi
	glViewport(m_viewports[id].x, m_viewports[id].y, m_viewports[id].width, m_viewports[id].height);
}

std::optional<Viewport> Window::getViewport(std::string id) const
{
	if (m_viewports.find(id) != m_viewports.end())
		return m_viewports.at(id);
	else
		return {};
}

GLFWwindow* Window::getNativeWindowHandle() const
{
	return m_window;
}

int Window::getWidth() const
{
	return m_width;
}

int Window::getHeight() const
{
	return m_height;
}

std::optional<Viewport> Window::getMainViewport() const
{
	if (m_viewports.find("MainView") != m_viewports.end())
		return m_viewports.at("MainView");
	else
		return {};
}
