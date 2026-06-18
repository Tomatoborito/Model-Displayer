#include <filesystem>
namespace fs = std::filesystem;

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"

std::vector<Vertex> planeVertices = {
    Vertex{glm::vec3(-25.0f, 0.0f, -25.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f, 0.0f)},
    Vertex{glm::vec3(-25.0f, 0.0f, 25.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f, 1.0f)},
    Vertex{glm::vec3(25.0f, 0.0f, 25.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(1.0f, 1.0f)},
    Vertex{glm::vec3(25.0f, 0.0f, -25.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f)}
};

std::vector<GLuint> planeIndices = { 0, 1, 2, 0, 2, 3 };

const unsigned int width = 800;
const unsigned int height = 800;

void changeTransform(Model& model, const std::vector<glm::mat4>& originalMatrices, float angle,
    const glm::vec3& translation, const glm::vec3& scale, float x, float y, float z)
{
    glm::vec3 axis(x, y, z);
    glm::mat4 rot(1.0f);
    if (glm::length(axis) > 0.1f) {
        rot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::normalize(axis));
    }
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);
    std::vector<glm::mat4> modified;
    for (size_t i = 0; i < originalMatrices.size(); i++) {
        modified.push_back(trans * rot * sca * originalMatrices[i]);
    }
    model.setMatricesMeshes(modified);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "YoutubeOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, width, height);

    Shader shaderProgram("default.vert", "default.frag");
    glm::vec4 lightColor(1.0f);
    glm::vec3 lightPos(0.5f, 50.0f, 0.5f);
    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    Shader shaderProgram1("default.vert", "default.frag");
    shaderProgram1.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram1.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram1.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    float rotation = 0.0f;
    double prevTime = glfwGetTime();
    glEnable(GL_DEPTH_TEST);

    std::string parentDir = fs::current_path().string();
    std::string modelPath = "/grilled_turkey_leg/wd5lcfcaa_tier_1.gltf";
    std::string texturePath = parentDir + "/unnamed.png";

    std::vector<Texture> textures = { Texture(texturePath.c_str(), "texture", 0) };

    Camera camera(window, width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    Mesh plane(planeVertices, planeIndices, textures);
    Model model1((parentDir + modelPath).c_str());
    std::vector<glm::mat4> originalMatrices = model1.getMatricesMeshes();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float scale = 100.0f;
    float speed = 0.5f;
    bool rotx = false;
    bool roty = false;
    bool rotz = false;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.578f, 0.747f, 0.802f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!io.WantCaptureMouse) {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            camera.Inputs(window);
        }
        else {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        }
        camera.updateMatrix(45.0f, 0.1f, 1000000.0f);

        ImGui::Begin("chicken params");
        ImGui::SliderFloat("Size", &scale, 10.0f, 2000.0f);
        ImGui::SliderFloat("Speed", &speed, -50.0f, 50.0f);
        ImGui::Checkbox("x rotation", &rotx);
        ImGui::Checkbox("y rotation", &roty);
        ImGui::Checkbox("z rotation", &rotz);
        ImGui::End();

        double crntTime = glfwGetTime();
        if (crntTime - prevTime >= 1.0 / 60.0) {
            rotation += speed;
            prevTime = crntTime;
        }

        glm::vec3 translation(0.0f, 0.0f, 0.0f);
        glm::vec3 scaling(scale, scale, scale);
        changeTransform(model1, originalMatrices, rotation, translation, scaling, rotx, roty, rotz);

        plane.Draw(shaderProgram, camera, glm::mat4(1.0f), glm::vec3(0.0f, 100.0f, 0.0f), glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 10.0f, 10.0f));
        model1.Draw(shaderProgram1, camera);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    shaderProgram.Delete();
    shaderProgram1.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}