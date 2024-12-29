#include "Scene.h"

std::unique_ptr<Scene> Scene::createScene() {
    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());
    scene->initScene();
    return scene;
}


// object는 cleanup 해줄 필요 없지만 model과 texture는 해줘야함
void Scene::cleanup() { 
    m_boxModel->cleanup();
    m_sphereModel->cleanup();
    m_planeModel->cleanup();

    m_vikingModel->cleanup();
    m_catModel->cleanup();

    m_vikingTexture->cleanup();
    m_sampleTexture->cleanup();
    m_catTexture->cleanup();
    m_karinaTexture->cleanup();
}


glm::mat4 Scene::getViewMatrix() {
    m_cameraFront =
        glm::rotate(glm::mat4(1.0f),
        glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f),
        glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}


glm::mat4 Scene::getProjMatrix(VkExtent2D swapChainExtent) {
    return glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 100.0f);
}


void Scene::updateLightPos(glm::vec3 lightPos) {
    m_lightPos = lightPos;
    m_lightObject->setPosition(lightPos);
}


void Scene::initScene() {
    m_boxModel = Model::createBoxModel();
    m_sphereModel = Model::createSphereModel();
    m_planeModel = Model::createPlaneModel();

    m_vikingModel = Model::createModel("models/viking_room.obj");
    m_catModel = Model::createModel("models/cat.obj");

    //texture 해야함
    m_vikingTexture = Texture::createTexture("textures/viking_room.png");
    m_sampleTexture = Texture::createTexture("textures/texture.png");
    m_catTexture = Texture::createTexture("textures/cat.bmp");
    m_karinaTexture = Texture::createTexture("textures/karina.jpg");

    m_objects.push_back(Object::createObject(m_planeModel, m_karinaTexture, 
    glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(5.0f * 0.74f, 5.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_boxModel, m_sampleTexture, 
    glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_vikingModel, m_vikingTexture, 
    glm::vec3(0.0f, -0.3f, 0.0f), glm::vec3(-90.0f, 0.0f, -90.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_sphereModel, m_vikingTexture, 
    glm::vec3(0.3f, 1.3f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    
    m_objects.push_back(Object::createObject(m_catModel, m_catTexture, 
    glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 45.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

    m_objects.push_back(Object::createObject(m_boxModel, m_sampleTexture,
    glm::vec3(2.0f, -0.5f, 0.0f), glm::vec3(30.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_lightObject = Object::createObject(m_sphereModel, m_sampleTexture, m_lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    m_objects.push_back(m_lightObject);
    m_objectCount = m_objects.size();
}


void Scene::processInput(GLFWwindow* window) {
    if (!m_cameraControl)
        return;
    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}


void Scene::mouseMove(double x, double y) {
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;
}

void Scene::mouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}
