#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
void setSceneViewPortStartEndValues(float startX, float startY, float endX, float endY);
float getMouseDeltaX();
float getMouseDeltaY();
bool getObjCanMove();
bool getObjCanRotate();
bool getObjCanScale();
bool getCanTransformAt_X();
bool getCanTransformAt_Y();
bool getCanTransformAt_Z();

#endif