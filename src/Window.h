#pragma once

#include "Common.h"
#include "Scene.h"

class Window {
public:
	static std::unique_ptr<Window> createWindow();
	~Window() { }
	void cleanup();

	void bindScene(Scene* scene);
	GLFWwindow* getWindow() { return window; }

private:
	Window() {}

	GLFWwindow* window;

	void initWindow();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void OnCursorPos(GLFWwindow* window, double x, double y);
	static void OnMouseButton(GLFWwindow* window, int button, int action, int modifier);
};