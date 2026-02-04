#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include <iostream>
#include <algorithm>

// ================= SETTINGS =================
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// ================= TRIANGLE STRUCT =================
//struct TrianglePosition
//{
//    glm::vec3 startWorldPos;
//    glm::vec3 currentWorldPos;
//    glm::vec3 targetWorldPos;
//    float t = 0.0f;
//    bool animating = false;
//};
//
//TrianglePosition tris[100];


// ================= MAIN =================
int main()
{
    // ---------- GLFW INIT ----------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "100 Triangles Dance", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ---------- GLAD ----------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // ---------- SHADER PROGRAM ----------
    Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

    // ---------- TRIANGLE DATA ----------
    float triangleVertices[] = {
         0.0f,  0.1f, 0.0f,  0.5f, 1.0f,// top
         0.1f, -0.1f, 0.0f,  1.0f, 0.0f,   // bottom right
        -0.1f, -0.1f, 0.0f,  0.0f, 0.0f // bottom left
    };


    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	// ---------- TEXTURE SETUP ----------
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(FileSystem::getPath("resources/textures/football2.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ourShader.setInt("texture1", 0);

    // ---------- ROTATION STATE ----------
    float angle = 0.0f;
    float speed = 10.0f; // degrees per second
    int phase = 0;
    int type = 0;

    float lastTime = glfwGetTime();

    // ---------- RENDER LOOP ----------
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window);

        // ---------- RENDER ----------
        glClearColor(0.7f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();
        glBindVertexArray(VAO);

        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");

        // ---------- ANGLE ----------
        if (phase == 0) {
            angle -= speed * deltaTime;
            if (angle <= -28.0f) phase = 1;
        }
        else if (phase == 1) {
            angle += speed * deltaTime;
            if (angle >= 0.0f) phase = 2;
        }
        else if (phase == 2) {
            angle += speed * deltaTime;
            if (angle >= 28.0f) phase = 3;
        }
        else if (phase == 3) {
            angle -= speed * deltaTime;
            if (angle <= 0.0f) phase = 0;
        }

        // ---------- 100 TRIANGLES ----------
        for (int i = 0; i < 100; i++)
        {
            int row = i / 10;
            int col = i % 10;

            float x = -0.9f + col * 0.2f;
            float y = 0.9f - row * 0.2f;

            glm::mat4 transform = glm::mat4(1.0f);
            glm::mat4 tf_l = glm::mat4(1.0f);
            glm::mat4 tf_lro = glm::mat4(1.0f);
            glm::mat4 tf_lto0 = glm::mat4(1.0f);

            glm::mat4 tf_r = glm::mat4(1.0f);
            glm::mat4 tf_rro = glm::mat4(1.0f);
            glm::mat4 tf_rto0 = glm::mat4(1.0f);

            // move to grid position
            transform = glm::translate(transform, glm::vec3(x, y, 0.0f));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);

            /*tris[i].currentWorldPos = glm::vec3(x, y, 0.0f);*/

            /*if (type == 0) {*/
                if (angle < 0.0f)
                {
                    // even pivot: bottom-left (-0.1, -0.1)
                    if (i % 2 == 0) {
                        //glm::vec3 v0 = glm::vec3(-0.1f, -0.1f, 0.0f);
                        //glm::vec3 v1 = glm::vec3(0.1f, -0.1f, 0.0f);
                        glm::vec3 v2 = glm::vec3(0.0f, 0.1f, 0.0f);
                        glm::vec3 pivotLocal = glm::vec3(0.0f, 0.1f, 0.0f);;


                        glm::mat4 pivotRotation =
                            glm::translate(glm::mat4(1.0f), -pivotLocal) *
                            glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 0, -1)) *
                            glm::translate(glm::mat4(1.0f), pivotLocal);

                        transform = transform * pivotRotation;
                    }
                    // odd pivot: bottom-right (0.1, -0.1)
                    else {
                        //glm::vec3 v0 = glm::vec3(-0.1f, -0.1f, 0.0f);
                        //glm::vec3 v1 = glm::vec3(0.1f, -0.1f, 0.0f);
                        glm::vec3 v2 = glm::vec3(0.0f, 0.1f, 0.0f);
                        glm::vec3 pivotLocal = glm::vec3(0.0f, 0.1f, 0.0f);;


                        glm::mat4 pivotRotation =
                            glm::translate(glm::mat4(1.0f), -pivotLocal) *
                            glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 0, 1)) *
                            glm::translate(glm::mat4(1.0f), pivotLocal);

                        transform = transform * pivotRotation;                    }

                }
                else if (angle > 0.0f)
                {
                    // even pivot: bottom-right (0.1, -0.1)
                    if (i % 2 == 0) {
                        glm::vec3 v2 = glm::vec3(0.0f, 0.1f, 0.0f);
                        glm::vec3 pivotLocal = glm::vec3(0.0f, 0.1f, 0.0f);;


                        glm::mat4 pivotRotation =
                            glm::translate(glm::mat4(1.0f), -pivotLocal) *
                            glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, -1.0f)) *
                            glm::translate(glm::mat4(1.0f), pivotLocal);

                        transform = transform * pivotRotation;
                        //transform = glm::translate(transform, glm::vec3(0.1f, -0.1f, 0.0f));
                        //transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, -1.0f));
                        //transform = glm::translate(transform, glm::vec3(-0.1f, 0.1f, 0.0f));
                    }
                    // odd pivot: bottom-right (-0.1, -0.1)
                    else {
                        glm::vec3 v2 = glm::vec3(0.0f, 0.1f, 0.0f);
                        glm::vec3 pivotLocal = glm::vec3(0.0f, 0.1f, 0.0f);;


                        glm::mat4 pivotRotation =
                            glm::translate(glm::mat4(1.0f), -pivotLocal) *
                            glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
                            glm::translate(glm::mat4(1.0f), pivotLocal);

                        transform = transform * pivotRotation;
                        //transform = glm::translate(transform, glm::vec3(-0.1f, -0.1f, 0.0f));
                        //transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
                        //transform = glm::translate(transform, glm::vec3(0.1f, 0.1f, 0.0f));
                    }
                }

                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
                glDrawArrays(GL_TRIANGLES, 0, 3);

                if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
                    type = 1;
			/*}*/
			/*else if (type == 1 && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                glm::mat4 transform =
                    glm::translate(glm::mat4(1.0f), tris[i].currentWorldPos);

                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
                glDrawArrays(GL_TRIANGLES, 0, 3);
				
			}
            else
            {
				type = 0;
            }*/
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            type = 1;
		
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---------- CLEANUP ----------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// ================= CALLBACKS =================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}