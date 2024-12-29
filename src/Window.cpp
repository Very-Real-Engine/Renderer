#include "Window.h"

std::unique_ptr<Window> Window::createWindow() {
    std::unique_ptr<Window> window = std::unique_ptr<Window>(new Window());
    window->initWindow();
    return window;
}


void Window::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}


void Window::initWindow() {
    glfwInit();

    // glfw로 창관리만 하고 렌더링 API는 안 쓴다는 설정
    // 렌더링 및 그래픽 처리는 외부 API인 Vulkan으로 함
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    // // window에 현재 App 객체를 바인딩
    // glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, OnKeyEvent);
    glfwSetCursorPosCallback(window, OnCursorPos);
    glfwSetMouseButtonCallback(window, OnMouseButton);
}


void Window::bindScene(Scene* scene) {
    glfwSetWindowUserPointer(window, scene);
}


void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {

}

void Window::OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void Window::OnCursorPos(GLFWwindow* window, double x, double y) {
    auto scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(window));
    scene->mouseMove(x, y);
}

void Window::OnMouseButton(GLFWwindow* window, int button, int action, int modifier) {
    auto scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(window));
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    scene->mouseButton(button, action, x, y);
}
