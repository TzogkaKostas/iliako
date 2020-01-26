#include "graphics\Include\glad\glad.h"
#include "graphics\Include\GLFW\glfw3.h"

#include "graphics\Include\glm\glm.hpp"
#include "graphics\Include\glm\gtc\matrix_transform.hpp"
#include "graphics\Include\glm\gtc\type_ptr.hpp"

#include "graphics\Include\learnopengl\shader_m.h"
#include "graphics\Include\learnopengl\camera.h"
#include "graphics\Include\learnopengl\model.h"

#define WINDOWS
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <iostream>
#include <math.h>
#include <algorithm> 


const float PI = 3.1415926535897932384626433832795;
const float PI_2 = 1.57079632679489661923;
const float PI_4 = 0.785398163397448309616;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RotationStop();
string GetCurrentWorkingDir(void);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool paused = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(0.0f, 16.0f, -50.0f);
glm::vec3 spacePos(0.0f, 10.0f, -50.0f);

glm::mat4 earth_matrix = glm::mat4(1.0f);

float a = 0.0, b = PI_2;
float old_camX = 0.0f, old_camZ = 0.0f, old_camY = 0.0f;
float camX = 0.0f, camZ = 0.0f, camY = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
    Shader lightingShader("2.2.basic_lighting.vs", "2.2.basic_lighting.fs");
	Shader lampShader("2.2.lamp.vs", "2.2.lamp.fs");
    // load models
    // -----------
	string current_path = GetCurrentWorkingDir();
    Model sun(current_path + "/resources/sun/planet.obj");
    Model earth(current_path + "/resources/earth/Model/Globe.obj");
    Model moon(current_path + "/resources/rock/rock/rock.obj");

	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	float rotatePos = 0.00000;
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        old_camX = camX, old_camZ = camZ, old_camY = camY;
        glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        const float radius = 70.0f;
        camX = sin(1.0 * a) * sin(1.0 * b) * radius;
        camZ = -50.0 + cos(1.0 * a) * sin(1.0 * b) * radius;
        camY = cos(1.0 * b) * radius;

        glm::vec3 cameraTarget, cameraDirection;
        cameraDirection = glm::normalize(glm::vec3(camX, camY, camZ) - cameraTarget);
        cameraTarget = glm::vec3(0.0f, 0.0f, -50.0f);

        view = glm::lookAt(glm::vec3(camX, camY, camZ), cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("light.diffuse", 100.0f, 100.0f, 100.0f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("light.constant", 1.0f);
		lightingShader.setFloat("light.linear", 0.09f);
		lightingShader.setFloat("light.quadratic", 0.032f);
		lightingShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

		//SUN
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);

        glm::mat4 sun_matrix = glm::mat4(1.0f);
        sun_matrix = glm::translate(sun_matrix, glm::vec3(0.0f, 0.0f, -50.0f));
        sun_matrix = glm::scale(sun_matrix, glm::vec3(0.7f, 0.7f, 0.7f));	// it's a bit too big for our scene, so scale it down
		lampShader.setMat4("model", sun_matrix);
        
        sun.Draw(lampShader);

		lightingShader.use();

		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		lightingShader.setVec3("light.ambient", 10.0f, 10.0f, 10.0f);
		lightingShader.setVec3("light.diffuse", 10.0f, 10.0f, 10.0f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		lightingShader.setFloat("light.constant", 1.0f);
		lightingShader.setFloat("light.linear", 0.09f);
		lightingShader.setFloat("light.quadratic", 0.032f);

		lightingShader.setFloat("material.shininess", 32.0f);

		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		sun_matrix = glm::scale(sun_matrix, glm::vec3(1.50f, 1.50f, 1.50f));
		lightingShader.setMat4("model", sun_matrix);

		sun.Draw(lightingShader);

        //EARTH

		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		lightingShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("light.diffuse", 100.0f, 100.0f, 100.0f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		lightingShader.setFloat("light.constant", 1.0f);
		lightingShader.setFloat("light.linear", 0.09f);
		lightingShader.setFloat("light.quadratic", 0.032f);

		lightingShader.setFloat("material.shininess", 32.0f);

        earth_matrix = glm::translate(sun_matrix, glm::vec3(30.0f, 0.0f, 0.0f));

		if (!paused) {
			rotatePos += 0.01;
		}

		earth_matrix = glm::translate(earth_matrix, glm::vec3(-30.0f, 0.0f, 0.0f));
		earth_matrix = glm::rotate(earth_matrix, (float)rotatePos, glm::vec3(0.0f, 1.0f, 0.0f));
		earth_matrix = glm::translate(earth_matrix, glm::vec3(30.0f, 0.0f, 0.0f));
		earth_matrix = glm::rotate(earth_matrix, (float)rotatePos*3, glm::vec3(0.0f, 1.0f, 0.0f));
		earth_matrix = glm::scale(earth_matrix, glm::vec3(0.2f, 0.2f, 0.2f));

		lightingShader.setMat4("model", earth_matrix);
        earth.Draw(lightingShader);

        //MOON
        glm::mat4 moon_matrix = glm::translate(earth_matrix, glm::vec3(20.0f , 0.0f, 0.0f));

		moon_matrix = glm::translate(moon_matrix, glm::vec3(-20.0f, 0.0f, 0.0f));
		moon_matrix = glm::rotate(moon_matrix, (float)(rotatePos * 4), glm::vec3(0.0f, 1.0f, 0.0f));
		moon_matrix = glm::translate(moon_matrix, glm::vec3(20.0f, 0.0f, 0.0f));


		lightingShader.setMat4("model", moon_matrix);
        moon.Draw(lightingShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
   
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        b -= 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        b += 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        a -= 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        a += 0.01;
    }

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		paused = true;
	}

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		paused = false;
	}
}

void RotationStop() {
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

std::string GetCurrentWorkingDir(void) {
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}