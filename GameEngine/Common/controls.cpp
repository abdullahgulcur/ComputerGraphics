// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

float sceneStartPosX;
float sceneStartPosY;
float sceneEndPosX;
float sceneEndPosY;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

bool leftClicked = false;
bool middleClicked = false;
bool rightClicked = false;

bool G_Pressed = false;
bool R_Pressed = false;
bool S_Pressed = false;

bool objCanMove = false;
bool objCanRotate = false;
bool objCanScale = false;

bool transformAt_X = false;
bool transformAt_Y = false;
bool transformAt_Z = false;

float deltaX;
float deltaY;

float getMouseDeltaX() {
	return deltaX;
}

float getMouseDeltaY() {
	return deltaY;
}

bool getObjCanMove() {
	return objCanMove;
}

bool getObjCanRotate() {
	return objCanRotate;
}

bool getObjCanScale() {
	return objCanScale;
}

bool getCanTransformAt_X() {
	return transformAt_X;
}

bool getCanTransformAt_Y() {
	return transformAt_Y;
}

bool getCanTransformAt_Z() {
	return transformAt_Z;
}

void setSceneViewPortStartEndValues(float startX, float startY, float endX, float endY) {
	
	sceneStartPosX = startX;
	sceneStartPosY = startY;
	sceneEndPosX = endX;
	sceneEndPosY = endY;
}

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 0);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseHorizontalSpeed = 0.001f;
float mouseVerticalSpeed = 0.0005f;

float horizontalMoveSpeed = 0.6f / 3;
float verticalMoveSpeed = 0.6f / 3;

float scrollYSpeed = 0.5;

int count = 0;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		leftClicked = true;

		objCanMove = false;
		objCanRotate = false;
		objCanScale = false;

		transformAt_X = false;
		transformAt_Y = false;
		transformAt_Z = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		rightClicked = true;
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		middleClicked = true;
	}


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		count = 0;
		leftClicked = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		count = 0;
		rightClicked = false;
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
		count = 0;
		middleClicked = false;
	}

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_G && action == GLFW_PRESS){

		if (transformAt_X)
			transformAt_X = false;

		if (transformAt_Y)
			transformAt_Y = false;

		if (transformAt_Z)
			transformAt_Z = false;

		G_Pressed = true;
		objCanMove = true;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		R_Pressed = true;
		objCanRotate = true;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		S_Pressed = true;
		objCanScale = true;
	}

	if (key == GLFW_KEY_G && action == GLFW_RELEASE)
		G_Pressed = false;

	if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		R_Pressed = false;

	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		S_Pressed = false;

	if (key == GLFW_KEY_X && action == GLFW_PRESS) {

		if(!(transformAt_Y || transformAt_Z))
			transformAt_X = true;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		
		if (!(transformAt_X || transformAt_Z))
			transformAt_Y = true;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		
		if (!(transformAt_X || transformAt_Y))
			transformAt_Z = true;
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	position += direction * (float)yoffset * scrollYSpeed;
}

void computeMatricesFromInputs() {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	static float lastX;
	static float lastY;

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;

	glfwGetCursorPos(window, &xpos, &ypos);

	deltaX = xpos - lastX;
	deltaY = ypos - lastY;

	static bool allowed = true;

	if (rightClicked) {

		if (count == 0) {

			if (xpos < sceneEndPosX + 1 && xpos > sceneStartPosX - 1 && ypos < sceneEndPosY + 1 && ypos > sceneStartPosY - 1)
				allowed = true;
			else
				allowed = false;
		}

		if (allowed) {
			if (xpos >= sceneEndPosX - 15) {
				glfwSetCursorPos(window, sceneStartPosX, ypos);
			}
			else if (xpos < sceneStartPosX) {
				glfwSetCursorPos(window, sceneEndPosX - 16, ypos);
			}

			if (ypos >= sceneEndPosY - 35) {
				glfwSetCursorPos(window, xpos, sceneStartPosY);
			}
			else if (ypos < sceneStartPosY) {
				glfwSetCursorPos(window, xpos, sceneEndPosY - 36);
			}

			horizontalAngle -= mouseHorizontalSpeed * deltaX;
			verticalAngle -= mouseVerticalSpeed * deltaY;
		}
		count++;
	}

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	if (middleClicked) {

		if (count == 0) {

			if (xpos < sceneEndPosX + 1 && xpos > sceneStartPosX - 1 && ypos < sceneEndPosY + 1 && ypos > sceneStartPosY - 1)
				allowed = true;
			else
				allowed = false;
		}

		if (allowed) {
			if (xpos >= sceneEndPosX - 15) {
				glfwSetCursorPos(window, sceneStartPosX, ypos);
			}
			else if (xpos < sceneStartPosX) {
				glfwSetCursorPos(window, sceneEndPosX - 16, ypos);
			}

			if (ypos >= sceneEndPosY - 35) {
				glfwSetCursorPos(window, xpos, sceneStartPosY);
			}
			else if (ypos < sceneStartPosY) {
				glfwSetCursorPos(window, xpos, sceneEndPosY - 36);
			}

			// Horizontal movement
			position -= right * deltaX * deltaTime * horizontalMoveSpeed;

			// Vertical movement
			position += up * deltaY * deltaTime * verticalMoveSpeed;
		}

		count++;
	}

	float FoV = initialFoV; // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), (float)1920 / 1080, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;

	bool flag = true;
	
	if (rightClicked && xpos >= sceneEndPosX - 15) {
		lastX = sceneStartPosX;
		flag = false;
	}
	if (rightClicked && xpos < sceneStartPosX) {
		lastX = sceneEndPosX - 16;
		flag = false;
	}
	else if(flag)
		lastX = xpos;

	if (rightClicked && ypos >= sceneEndPosY - 35) {
		lastY = sceneStartPosY;
		flag = false;
	}
	else if (rightClicked && ypos < sceneStartPosY) {
		lastY = sceneEndPosY - 36;
		flag = false;
	}
	else if (flag)
		lastY = ypos;

	if (middleClicked && xpos >= sceneEndPosX - 15) {
		lastX = sceneStartPosX;
		flag = false;
	}
	if (middleClicked && xpos < sceneStartPosX) {
		lastX = sceneEndPosX - 16;
		flag = false;
	}
	else if (flag)
		lastX = xpos;

	if (middleClicked && ypos >= sceneEndPosY - 35) {
		lastY = sceneStartPosY;
		flag = false;
	}
	else if (middleClicked && ypos < sceneStartPosY) {
		lastY = sceneEndPosY - 36;
		flag = false;
	}
	else if (flag)
		lastY = ypos; 
}