#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in float aIntensity;
    out float intensity;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        intensity = aIntensity;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in float intensity;
    out vec4 FragColor;

    void main() {
        // Cap intensity at 1.0 for maximum brightness
        float cappedIntensity = clamp(intensity, 0.0, 1.0);

        // Define color as a gradient from blue to red to yellow to white
        vec3 color;
        if (cappedIntensity < 0.25) {
            color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), cappedIntensity * 4.0);  // Blue to cyan
        } else if (cappedIntensity < 0.5) {
            color = mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), (cappedIntensity - 0.25) * 4.0);  // Cyan to green
        } else if (cappedIntensity < 0.75) {
            color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), (cappedIntensity - 0.5) * 4.0);  // Green to yellow
        } else {
            color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (cappedIntensity - 0.75) * 4.0);  // Yellow to red
        }

        FragColor = vec4(color, 1.0);  // Final color with full opacity
    }
)";


class OpenGLRenderer {
public:
    OpenGLRenderer(int width, int height) : width(width), height(height) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "Heat Simulation", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, width, height);

        shaderProgram = compileShaders();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    ~OpenGLRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void pollEvents() const {
        glfwPollEvents();
    }

    void renderGrid(const std::vector<std::vector<float>>& grid) {
        glClear(GL_COLOR_BUFFER_BIT);

        std::vector<float> vertices;
        for (std::size_t i = 0; i < grid.size(); i++) {
            for (std::size_t j = 0; j < grid[i].size(); j++) {
                float intensity = grid[i][j];
                float x = (2.0f * i / grid.size()) - 1.0f;
                float y = (2.0f * j / grid[i].size()) - 1.0f;
                vertices.push_back(x); 
                vertices.push_back(y); 
                vertices.push_back(intensity);  
            }
        }

        // Bind VAO and VBO, and upload vertex data
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        // Configure vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float))); // Intensity attribute
        glEnableVertexAttribArray(1);

        // Use the shader program and draw the points
        glUseProgram(shaderProgram);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

        // Clean up
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

private:
    GLFWwindow* window;
    int width, height;
    GLuint VAO, VBO, shaderProgram;

    GLuint compileShaders() {
        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        // Link shaders into a program
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        checkCompileErrors(program, "PROGRAM");

        // Clean up shaders (they're no longer needed after linking)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    void checkCompileErrors(GLuint shader, const std::string& type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
            }
        }
    }
};
