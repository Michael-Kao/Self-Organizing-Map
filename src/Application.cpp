#include "Application.h"

Application::Application(Window *wnd_ptr)
    :   m_window(wnd_ptr) {
    Init();
}

Application::Application() {
    m_window = new Window();
    Init();
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void Application::Init() {
    m_cube = new Object("assets/objfiles/cube.obj");
    m_shader = new Shader("assets/shaders/basic.vs", "assets/shaders/basic.fs");
    m_som_shader = new Shader("assets/shaders/som.vs", "assets/shaders/som.fs");
    m_camera = new Camera();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window->getWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void Application::drawTest() {
    m_shader->bind();
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    // projection = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.0f);
    // view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
    projection = glm::perspective(glm::radians(m_camera->fov), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.0f);
    view       = m_camera->getLookAt();
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1, 0, 0));
    m_shader->setMat4("model", model);
    m_cube->draw();
    m_shader->unbind();
}

void Application::processCamera(float delta_time) {
    if(m_window->isPressed(GLFW_KEY_W)){
        m_camera->updateInput(FORWARD, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_S)){
        m_camera->updateInput(BACKWARD, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_A)){
        m_camera->updateInput(LEFT, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_D)){
        m_camera->updateInput(RIGHT, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_K)){
        m_camera->updateInput(RPITCH, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_J)){
        m_camera->updateInput(PITCH, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_L)){
        m_camera->updateInput(YAW, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_H)){
        m_camera->updateInput(RYAW, delta_time);
    }
}

void Application::processInput() {
    if(m_window->isPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(m_window->getWindow(), true);
    }
}

void Application::run() {
    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;
    
    float startLearningRate = 0.05;
    int totalIteration = 100000;
    int time2update = 100;
    int latticeSize = 32;
    Som som("vaseSurface.txt", 100000, 0.005);
    m_camera->Position = glm::vec3(som.midPos.x, som.midPos.y, som.midPos.z + 500);
    
    while(!m_window->shouldClose()) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        m_window->setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        m_window->clear();

        processCamera(deltaTime);
        processInput();

        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

        som.train();
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            m_som_shader->bind();
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 projection = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(m_camera->fov), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 1000.0f);
            view       = m_camera->getLookAt();
            m_som_shader->setMat4("view", view);
            m_som_shader->setMat4("projection", projection);
            glm::mat4 model = glm::mat4(1.0f);
            m_som_shader->setMat4("model", model);
            som.draw_lattice();
            m_som_shader->unbind();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Control Panel");
        ImGui::SliderFloat("Start Learning Rate", &startLearningRate, 0.0f, 1.0f);
        int iter_step = (totalIteration - 100000) / 10000;
        if(ImGui::SliderInt("Iteration Step", &iter_step, -10, 10)) {
            totalIteration = 100000 + iter_step * 10000;
        }
        ImGui::Text("Total Iteration: %d", totalIteration);
        int lat_step = (latticeSize - 16) / 2;
        if(ImGui::SliderInt("Lattice Size Step", &lat_step, -7, 8)) {
            latticeSize = 16 + lat_step * 2;
        }
        ImGui::Text("Lattice Size: %d", latticeSize);
        ImGui::Text("Every %d iteration update lattice", time2update);
        ImGui::Text("%d / %d (%.1lf %%)", som.current_iteration, totalIteration, som.current_iteration / (double)totalIteration * 100.0);
        if(ImGui::Button("Apply")) {
            som.update(totalIteration, startLearningRate, latticeSize, time2update);
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset")) {
            startLearningRate = 0.05;
            totalIteration = 100000;
            latticeSize = 32;
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->swapBuffers();
    }
}