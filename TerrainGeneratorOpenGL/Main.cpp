// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


// Include for Perlin Noise
# include <cassert>
# include <fstream>
# include <sstream>
# include "common/PerlinNoise.hpp"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

glm::vec3 cameraPosition(0.f, 0.f, 0.f);
glm::vec3 cameraDirection(0.f, 0.f, 0.f);

ImVec4 lightColor = ImVec4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
ImVec4 albedoColor = ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);

glm::vec3 lightPosition(0.f, 0.f, 0.f);
glm::vec3 lightRotation(0.f, 0.f, 0.f);
float lightPower = 1.f;

int albedo_current_0 = 1;
int normal_current_0 = 1;
int metallic_current_0 = 0;
int roughness_current_0 = 1;
int ao_current_0 = 1;

int albedo_current_1 = 2;
int normal_current_1 = 2;
int metallic_current_1 = 0;
int roughness_current_1 = 2;
int ao_current_1 = 2;

int albedo_current_2 = 3;
int normal_current_2 = 3;
int metallic_current_2 = 0;
int roughness_current_2 = 3;
int ao_current_2 = 3;

float tilingColor_0 = 1;
float tilingColor_1 = 1;
float tilingColor_2 = 1;

float normal_0 = 0.0f;
float metallic_0 = 0.0f;
float roughness_0 = 0.0f;
float ao_0 = 0.0f;

float normal_1 = 0.0f;
float metallic_1 = 0.0f;
float roughness_1 = 0.0f;
float ao_1 = 0.0f;

float normal_2 = 0.0f;
float metallic_2 = 0.0f;
float roughness_2 = 0.0f;
float ao_2 = 0.0f;

float terrainHeight = 0.f;
int octaves = 0.f;
float frequency = 0.f;
float terrainSizeX = 25.f;
float terrainSizeZ = 25.f;

std::vector<GLuint> albedoTextures;
std::vector<GLuint> normalTextures;
std::vector<GLuint> metallicTextures;
std::vector<GLuint> roughnessTextures;
std::vector<GLuint> aoTextures;

void createCameraPanel();
void createLightPanel();
void createTexturesPanel();
void createAppPropertiesPanel();
void createTerrain(std::vector<unsigned short>& indices, std::vector<glm::vec3>& in_vertices, int xSize, int zSize);
void realculateNormals(std::vector<unsigned short>& indices, std::vector<glm::vec3>& in_vertices, std::vector<glm::vec3>& in_normals);
void createColors(std::vector<glm::vec3>& indexed_colors, std::vector<glm::vec3>& indexed_normals, int terrainVertsX, int terrainVertsZ);
void createUVs(std::vector<glm::vec2>& indexed_uvs, int terrainSizeX, int terrainSizeZ);
float perlinNoise(double frequency, int octaves, int seed, int xSize, int zSize, int x, int y);
void drawAllAxis();
bool preffileExistsBefore();
void fileSave();
void createTerrainPanel();
float angleBetweenTwoVectors(glm::vec3 v0, glm::vec3 v1);
glm::vec3 evaluateColor(float slope);
void drawAllAxis(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix, glm::mat4 ModelMatrix);
void loadAllData();
void initTextures();
void updateTerrain(std::vector<unsigned short>& indices, std::vector<glm::vec3>& in_vertices, int xSize, int zSize);

int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);

	setVideoScreenSize(mode);

	if (preffileExistsBefore())
		loadAllData();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // So that glBegin/glVertex/glEnd work

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1600, 900, "Terrain Editor", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwMaximizeWindow(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, mode->width / 2, mode->height / 2);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders("shaders/NormalMapping.vertexshader", "shaders/NormalMapping.fragmentshader");

	GLuint ViewMatrixID = glGetUniformLocation(programID, "view");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "model");
	GLuint ProjectionMatrixID = glGetUniformLocation(programID, "projection");

	GLuint albedoMap_0_IndexID = glGetUniformLocation(programID, "albedoMap_0_Index");
	GLuint normalMap_0_IndexID = glGetUniformLocation(programID, "normalMap_0_Index");
	GLuint metallicMap_0_IndexID = glGetUniformLocation(programID, "metallicMap_0_Index");
	GLuint roughnessMap_0_IndexID = glGetUniformLocation(programID, "roughnessMap_0_Index");
	GLuint aoMap_0_IndexID = glGetUniformLocation(programID, "aoMap_0_Index");

	GLuint albedoMap_1_IndexID = glGetUniformLocation(programID, "albedoMap_1_Index");
	GLuint normalMap_1_IndexID = glGetUniformLocation(programID, "normalMap_1_Index");
	GLuint metallicMap_1_IndexID = glGetUniformLocation(programID, "metallicMap_1_Index");
	GLuint roughnessMap_1_IndexID = glGetUniformLocation(programID, "roughnessMap_1_Index");
	GLuint aoMap_1_IndexID = glGetUniformLocation(programID, "aoMap_1_Index");

	GLuint albedoMap_2_IndexID = glGetUniformLocation(programID, "albedoMap_2_Index");
	GLuint normalMap_2_IndexID = glGetUniformLocation(programID, "normalMap_2_Index");
	GLuint metallicMap_2_IndexID = glGetUniformLocation(programID, "metallicMap_2_Index");
	GLuint roughnessMap_2_IndexID = glGetUniformLocation(programID, "roughnessMap_2_Index");
	GLuint aoMap_2_IndexID = glGetUniformLocation(programID, "aoMap_2_Index");

	GLuint AlbedoID = glGetUniformLocation(programID, "albedoFromColorPicker");

	GLuint NormalID_0 = glGetUniformLocation(programID, "normalAmount_0");
	GLuint MetallicID_0 = glGetUniformLocation(programID, "metallicAmount_0");
	GLuint RoughnessID_0 = glGetUniformLocation(programID, "roughnessAmount_0");
	GLuint AOID_0 = glGetUniformLocation(programID, "aoAmount_0");

	GLuint NormalID_1 = glGetUniformLocation(programID, "normalAmount_1");
	GLuint MetallicID_1 = glGetUniformLocation(programID, "metallicAmount_1");
	GLuint RoughnessID_1 = glGetUniformLocation(programID, "roughnessAmount_1");
	GLuint AOID_1 = glGetUniformLocation(programID, "aoAmount_1");

	GLuint NormalID_2 = glGetUniformLocation(programID, "normalAmount_2");
	GLuint MetallicID_2 = glGetUniformLocation(programID, "metallicAmount_2");
	GLuint RoughnessID_2 = glGetUniformLocation(programID, "roughnessAmount_2");
	GLuint AOID_2 = glGetUniformLocation(programID, "aoAmount_2");

	GLuint LightPositionID = glGetUniformLocation(programID, "lightPosition");
	GLuint LightDirectionID = glGetUniformLocation(programID, "lightDirection");
	GLuint LightColorID = glGetUniformLocation(programID, "lightColor");
	GLuint LightPowerID = glGetUniformLocation(programID, "lightPower");
	GLuint CameraPositionID = glGetUniformLocation(programID, "camPos");

	GLuint AlbedoTextureID_0 = glGetUniformLocation(programID, "albedoMap_0");
	GLuint NormalTextureID_0 = glGetUniformLocation(programID, "normalMap_0");
	GLuint MetallicTextureID_0 = glGetUniformLocation(programID, "metallicMap_0");
	GLuint RoughnessTextureID_0 = glGetUniformLocation(programID, "roughnessMap_0");
	GLuint AOTextureID_0 = glGetUniformLocation(programID, "aoMap_0");

	GLuint AlbedoTextureID_1 = glGetUniformLocation(programID, "albedoMap_1");
	GLuint NormalTextureID_1 = glGetUniformLocation(programID, "normalMap_1");
	GLuint MetallicTextureID_1 = glGetUniformLocation(programID, "metallicMap_1");
	GLuint RoughnessTextureID_1 = glGetUniformLocation(programID, "roughnessMap_1");
	GLuint AOTextureID_1 = glGetUniformLocation(programID, "aoMap_1");

	GLuint AlbedoTextureID_2 = glGetUniformLocation(programID, "albedoMap_2");
	GLuint NormalTextureID_2 = glGetUniformLocation(programID, "normalMap_2");
	GLuint MetallicTextureID_2 = glGetUniformLocation(programID, "metallicMap_2");
	GLuint RoughnessTextureID_2 = glGetUniformLocation(programID, "roughnessMap_2");
	GLuint AOTextureID_2 = glGetUniformLocation(programID, "aoMap_2");

	GLuint TilingColorID_0 = glGetUniformLocation(programID, "tilingColor_0");
	GLuint TilingColorID_1 = glGetUniformLocation(programID, "tilingColor_1");
	GLuint TilingColorID_2 = glGetUniformLocation(programID, "tilingColor_2");

	initTextures();

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_colors;

	int terrainVertsX = 128;
	int terrainVertsZ = 128;
	createTerrain(indices, indexed_vertices, terrainVertsX, terrainVertsZ);
	realculateNormals(indices, indexed_vertices, indexed_normals);
	createUVs(indexed_uvs, terrainVertsX, terrainVertsZ);
	createColors(indexed_colors, indexed_normals, terrainVertsX, terrainVertsZ);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_colors.size() * sizeof(glm::vec3), &indexed_colors[0], GL_STATIC_DRAW);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Create ImGui and initialize
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	int temp = 0;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ImGui New Frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (getCanMove()) {
			std::vector<float>camPosAndRot = computeMatricesFromInputs();
			cameraPosition.x = camPosAndRot[0];
			cameraPosition.y = camPosAndRot[1];
			cameraPosition.z = camPosAndRot[2];

			cameraDirection.x = camPosAndRot[3];
			cameraDirection.y = camPosAndRot[4];
			cameraDirection.z = camPosAndRot[5];

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			computeMatricesFromPanelInputs(cameraPosition, cameraDirection);
		}

		// Use our shader
		glUseProgram(programID);

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);

		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(ProjectionMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

		glUniform1i(albedoMap_0_IndexID, albedo_current_0);
		glUniform1i(normalMap_0_IndexID, normal_current_0);
		glUniform1i(metallicMap_0_IndexID, metallic_current_0);
		glUniform1i(roughnessMap_0_IndexID, roughness_current_0);
		glUniform1i(aoMap_0_IndexID, ao_current_0);

		glUniform1i(albedoMap_1_IndexID, albedo_current_1);
		glUniform1i(normalMap_1_IndexID, normal_current_1);
		glUniform1i(metallicMap_1_IndexID, metallic_current_1);
		glUniform1i(roughnessMap_1_IndexID, roughness_current_1);
		glUniform1i(aoMap_1_IndexID, ao_current_1);

		glUniform1i(albedoMap_2_IndexID, albedo_current_2);
		glUniform1i(normalMap_2_IndexID, normal_current_2);
		glUniform1i(metallicMap_2_IndexID, metallic_current_2);
		glUniform1i(roughnessMap_2_IndexID, roughness_current_2);
		glUniform1i(aoMap_2_IndexID, ao_current_2);

		// Material Parameter Amounts
		glUniform3f(AlbedoID, albedoColor.x, albedoColor.y, albedoColor.z);

		glUniform1f(NormalID_0, normal_0);
		glUniform1f(MetallicID_0, metallic_0);
		glUniform1f(RoughnessID_0, roughness_0);
		glUniform1f(AOID_0, ao_0);

		glUniform1f(NormalID_1, normal_1);
		glUniform1f(MetallicID_1, metallic_1);
		glUniform1f(RoughnessID_1, roughness_1);
		glUniform1f(AOID_1, ao_0);

		glUniform1f(NormalID_2, normal_2);
		glUniform1f(MetallicID_2, metallic_2);
		glUniform1f(RoughnessID_2, roughness_2);
		glUniform1f(AOID_2, ao_2);

		glUniform3f(LightPositionID, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(LightDirectionID, -computeLightAndCameraAxis(lightRotation).x,
			-computeLightAndCameraAxis(lightRotation).y, -computeLightAndCameraAxis(lightRotation).z);

		glm::vec3 camPos = getGetCameraPosition();
		glUniform3f(CameraPositionID, camPos.x, camPos.y, camPos.z);

		glUniform3f(LightColorID, lightColor.x, lightColor.y, lightColor.z);
		glUniform1f(LightPowerID, lightPower);

		//-- Tilings
		glUniform1f(TilingColorID_0, tilingColor_0);
		glUniform1f(TilingColorID_1, tilingColor_1);
		glUniform1f(TilingColorID_2, tilingColor_2);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedoTextures[albedo_current_0 - 1]);
		glUniform1i(AlbedoTextureID_0, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTextures[normal_current_0 - 1]);
		glUniform1i(NormalTextureID_0, 1);

		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, getMetallicMap(metallic_current_0));
		//glUniform1i(MetallicTextureID_0, 2);
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughnessTextures[roughness_current_0 - 1]);
		glUniform1i(RoughnessTextureID_0, 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, aoTextures[ao_current_0 - 1]);
		glUniform1i(AOTextureID_0, 4);


		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, albedoTextures[albedo_current_1 - 1]);
		glUniform1i(AlbedoTextureID_1, 5);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, normalTextures[normal_current_1 - 1]);
		glUniform1i(NormalTextureID_1, 6);

		//glActiveTexture(GL_TEXTURE7);
		//glBindTexture(GL_TEXTURE_2D, getMetallicMap(metallic_current_1));
		//glUniform1i(MetallicTextureID_1, 7);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, roughnessTextures[roughness_current_1 - 1]);
		glUniform1i(RoughnessTextureID_1, 8);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, aoTextures[ao_current_1 - 1]);
		glUniform1i(AOTextureID_1, 9);


		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, albedoTextures[albedo_current_2 - 1]);
		glUniform1i(AlbedoTextureID_2, 10);

		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, normalTextures[normal_current_2 - 1]);
		glUniform1i(NormalTextureID_2, 11);

		//glActiveTexture(GL_TEXTURE12);
		//glBindTexture(GL_TEXTURE_2D, getMetallicMap(metallic_current_2));
		//glUniform1i(MetallicTextureID_2, 12);

		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, roughnessTextures[roughness_current_2 - 1]);
		glUniform1i(RoughnessTextureID_2, 13);

		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, aoTextures[ao_current_2 - 1]);
		glUniform1i(AOTextureID_2, 14);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		drawAllAxis(ProjectionMatrix, ViewMatrix, ModelMatrix);


		createCameraPanel();
		createLightPanel();
		createTexturesPanel();
		createTerrainPanel();
		createAppPropertiesPanel();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		/*if (temp % 20 == 0) {

			updateTerrain(indices, indexed_vertices, terrainVertsX, terrainVertsZ);
			realculateNormals(indices, indexed_vertices, indexed_normals);
		}*/

		temp++;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &uvbuffer);
	/*
	
	glDeleteBuffers(1, &tangentbuffer);
	glDeleteBuffers(1, &bitangentbuffer);*/
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);

	for (int i = 0; i < albedoTextures.size(); i++)
		glDeleteTextures(1, &albedoTextures[i]);

	for (int i = 0; i < albedoTextures.size(); i++)
		glDeleteTextures(1, &normalTextures[i]);

	for (int i = 0; i < albedoTextures.size(); i++)
		glDeleteTextures(1, &roughnessTextures[i]);

	for (int i = 0; i < albedoTextures.size(); i++)
		glDeleteTextures(1, &aoTextures[i]);

	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	fileSave();

	return 0;
}

void drawAllAxis(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix, glm::mat4 ModelMatrix) {


	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((const GLfloat*)&ProjectionMatrix[0]);
	glMatrixMode(GL_MODELVIEW);
	glm::mat4 MV = ViewMatrix * ModelMatrix;
	glLoadMatrixf((const GLfloat*)&MV[0]);

	glUseProgram(0);

	// normals
	/*glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < indices.size(); i++) {
		glm::vec3 p = indexed_vertices[indices[i]];
		glVertex3fv(&p.x);
		glm::vec3 o = glm::normalize(indexed_normals[indices[i]]);
		p += o * 0.1f;
		glVertex3fv(&p.x);
	}
	glEnd();*/
	// tangents
	//glColor3f(1, 0, 0);
	//glBegin(GL_LINES);
	//for (unsigned int i = 0; i < indices.size(); i++) {
	//	glm::vec3 p = indexed_vertices[indices[i]];
	//	glVertex3fv(&p.x);
	//	glm::vec3 o = glm::normalize(indexed_tangents[indices[i]]);
	//	p += o * 0.1f;
	//	glVertex3fv(&p.x);
	//}
	//glEnd();
	//// bitangents
	//glColor3f(0, 1, 0);
	//glBegin(GL_LINES);
	//for (unsigned int i = 0; i < indices.size(); i++) {
	//	glm::vec3 p = indexed_vertices[indices[i]];
	//	glVertex3fv(&p.x);
	//	glm::vec3 o = glm::normalize(indexed_bitangents[indices[i]]);
	//	p += o * 0.1f;
	//	glVertex3fv(&p.x);
	//}
	//glEnd();
	// 

	// Coordinate Axis
	glBegin(GL_LINES);
	glm::vec3 origin(0, 0, 0);
	glColor3f(1, 0, 0);
	glVertex3fv(&origin.x);
	origin += glm::vec3(1, 0, 0) * 100.0f;
	glVertex3fv(&origin.x);
	origin -= glm::vec3(1, 0, 0) * 100.0f;
	glColor3f(0, 1, 0);
	glVertex3fv(&origin.x);
	origin += glm::vec3(0, 1, 0) * 100.0f;
	glVertex3fv(&origin.x);
	origin -= glm::vec3(0, 1, 0) * 100.0f;
	glColor3f(0, 0, 1);
	glVertex3fv(&origin.x);
	origin += glm::vec3(0, 0, 1) * 100.0f;
	glVertex3fv(&origin.x);
	glEnd();

	// Light Axis (Z axis is direction vector)
	glBegin(GL_LINES);
	glm::vec3 lightDir = computeLightAndCameraAxis(lightRotation);
	glm::vec3 p = lightPosition;
	glColor3f(1, 1, 1);
	glVertex3fv(&p.x);
	p += lightDir;
	glVertex3fv(&p.x);
	p -= lightDir;
	/*glColor3f(0, 1, 0);
	glVertex3fv(&p.x);
	p += glm::vec3(m * glm::vec4(0, 1, 0, 0));
	glVertex3fv(&p.x);
	p -= glm::vec3(m * glm::vec4(0, 1, 0, 0));
	glColor3f(0, 0, 1);
	glVertex3fv(&p.x);
	p += glm::vec3(m * glm::vec4(0, 0, 1, 0));
	glVertex3fv(&p.x);*/
	glEnd();
}

void createTerrain(std::vector<unsigned short>& indices, std::vector<glm::vec3>& in_vertices, int xSize, int zSize) {
	
	int size = xSize;
	float distanceBetweenVertsX = terrainSizeX / size;
	float distanceBetweenVertsZ = terrainSizeZ / size;

	double frequency = 1;
	int octaves = 6;
	int seed = 4683;
	float heightScale = 13.f;

	for (int i = 0; i <= size; i++) {

		for (int j = 0; j <= size; j++) {

			glm::vec3 vertex;
			vertex.x = i * distanceBetweenVertsX;
			vertex.y = perlinNoise(frequency, octaves, seed, size, size, i, j) * heightScale;
			vertex.z = j * distanceBetweenVertsZ;

			in_vertices.push_back(vertex);
		}
	}

	int vert = 0;

	for (int i = 0; i < size; i++) {

		for (int j = 0; j < size; j++) {

			indices.push_back(vert + 1);
			indices.push_back(vert + size + 1);
			indices.push_back(vert + 0);
			indices.push_back(vert + size + 2);
			indices.push_back(vert + size + 1);
			indices.push_back(vert + 1);

			vert++;
		}
		vert++;
	}
}

void updateTerrain(std::vector<unsigned short>& indices, std::vector<glm::vec3>& in_vertices, int xSize, int zSize) {

	int size = xSize;
	float distanceBetweenVertsX = terrainSizeX / size;
	float distanceBetweenVertsZ = terrainSizeZ / size;
	
	int count = 0;

	for (int i = 0; i <= size; i++) {

		for (int j = 0; j <= size; j++) {

			in_vertices[count].x = i * distanceBetweenVertsX;
			in_vertices[count].y = perlinNoise(frequency, octaves, 10, size, size, i, j) * terrainHeight;
			in_vertices[count].z = j * distanceBetweenVertsZ;

			count++;
		}
	}
}


// For smooth shading
// ref: https://mrl.cs.nyu.edu/~perlin/courses/fall2002ugrad/meshnormals.html
void realculateNormals(std::vector<unsigned short>& indices,
	std::vector<glm::vec3>& in_vertices, std::vector<glm::vec3>& in_normals) {

	// init all normals
	for (int i = 0; i < in_vertices.size(); i++) {

		glm::vec3 normal;
		normal.x = 0;
		normal.y = 0;
		normal.z = 0;

		in_normals.push_back(normal);
	}

	// loop through all the faces and take cross product
	for (int i = 0; i < indices.size(); i += 3) {

		glm::vec3 v0;
		glm::vec3 v1;
		glm::vec3 v2;

		v0 = in_vertices[indices[i]];
		v1 = in_vertices[indices[i + 1]];
		v2 = in_vertices[indices[i + 2]];

		in_normals[indices[i]] += glm::cross((v1 - v0), (v2 - v0));
		in_normals[indices[i + 1]] += glm::cross((v2 - v1), (v0 - v1));
		in_normals[indices[i + 2]] += glm::cross((v0 - v2), (v1 - v2));
	}

	// normalize all the vectors
	for (int i = 0; i < in_normals.size(); i++) {

		glm::vec3 temp;
		temp = in_normals[i];
		in_normals[i] = glm::normalize(temp);
	}

}

void createColors(std::vector<glm::vec3>& indexed_colors, std::vector<glm::vec3>& indexed_normals, int terrainVertsX, int terrainVertsZ) {

	for (int i = 0; i < terrainVertsZ + 1; i++)
	{
		for (int j = 0; j < terrainVertsX + 1; j++)
		{
			float slopeAngle = angleBetweenTwoVectors(glm::vec3(0,1,0), indexed_normals[terrainVertsX * i + j]);
			indexed_colors.push_back(evaluateColor(slopeAngle));
		}
	}
}

glm::vec3 evaluateColor(float slope) {

	float angle0 = glm::radians(22.f);
	float angle1 = glm::radians(30.f);
	//float angle2 = glm::radians(35);

	if (slope <= angle0)
		return glm::vec3(1, 0, 0);
	if (slope > angle0 && slope <= angle1)
		return glm::vec3(0, 0, 1);
	else 
		return glm::vec3(0, 1, 0);

	//if (slope <= angle0)
	//	return glm::vec3(0, 0, 0);
	//if (slope > angle0 && slope <= angle1)
	//	return glm::vec3(0, 0, 0);
	//else if (slope > angle1 && slope <= angle2)
	//	return glm::vec3(0, 0, 0);
	//else
	//	return new Color(1f, 0f, 0f, 0f); // red channel

;}

float angleBetweenTwoVectors(glm::vec3 v0, glm::vec3 v1) {

	return glm::acos(glm::dot(v0, v1) / (glm::length(v0) * glm::length(v1)));
}

void createUVs(std::vector<glm::vec2>& indexed_uvs, int terrainSizeX, int terrainSizeZ) {

	for (int i = 0; i < terrainSizeZ + 1; i++)
	{
		for (int j = 0; j < terrainSizeX + 1; j++)
		{
			glm::vec3 uv;
			uv.x = (float)j / terrainSizeX;
			uv.y = (float)i / terrainSizeZ;

			indexed_uvs.push_back(uv);
		}
	}
}

//void perlinNoiseHeightList(double frequency, int octaves, int seed, int xSize, int zSize, std::vector<float>& heights) {
//
//	frequency = std::clamp(frequency, 0.1, 64.0);
//	octaves = std::clamp(octaves, 1, 16);
//
//	const siv::PerlinNoise perlin(seed);
//	const double fx = xSize / frequency;
//	const double fy = zSize / frequency;
//
//	for (std::int32_t y = 0; y < zSize; ++y)
//		for (std::int32_t x = 0; x < xSize; ++x)
//			heights.push_back(perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves));
//
//}

float perlinNoise(double frequency, int octaves, int seed, int xSize, int zSize, int x, int y) {

	frequency = std::clamp(frequency, 0.1, 64.0);
	octaves = std::clamp(octaves, 1, 16);

	const siv::PerlinNoise perlin(100);
	const double fx = xSize / frequency;
	const double fy = zSize / frequency;

	return perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
}

void createCameraPanel() {

	ImGui::Begin("Camera Transform");

	ImGui::Text("Pos");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("X##0", &cameraPosition.x, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Y##0", &cameraPosition.y, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Z##0", &cameraPosition.z, 0.1f);

	ImGui::Text("Rot");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("X##1", &cameraDirection.x, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Y##1", &cameraDirection.y, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Z##1", &cameraDirection.z, 0.1f);

	ImGui::End();
}

void createLightPanel() {

	ImGui::Begin("Light");

	ImGui::Text("Pos");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("X##2", &lightPosition.x, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Y##2", &lightPosition.y, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Z##2", &lightPosition.z, 0.1f);


	ImGui::Text("Rot");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("X##3", &lightRotation.x, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Y##3", &lightRotation.y, 0.1f);
	ImGui::SameLine();

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Z##3", &lightRotation.z, 0.1f);

	ImGui::Text("Power");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("##0", &lightPower, 0.01f);
	ImGui::SameLine();
	ImGui::Text("Color");
	ImGui::SameLine();
	ImGui::ColorEdit4("Light Color##0", (float*)&lightColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | 0);
	

	/*ImGui::Text("Power");
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::DragFloat("", &lightPower, 0.005f);*/

	ImGui::End();
}

void createTerrainPanel() {

	ImGui::Begin("Terrain");

	ImGui::PushItemWidth(90);
	ImGui::DragFloat("Height", &terrainHeight, 0.01f);

	ImGui::PushItemWidth(90);
	ImGui::DragFloat("Frequency", &frequency, 0.01f);

	ImGui::PushItemWidth(90);
	ImGui::SliderInt("Octaves", &octaves, 1, 10);

	ImGui::PushItemWidth(90);
	ImGui::DragFloat("Size X", &terrainSizeX, 0.01f);

	ImGui::PushItemWidth(90);
	ImGui::DragFloat("Size Y", &terrainSizeZ, 0.01f);

	ImGui::End();
}


void createTexturesPanel() {

	const char* albedoMapItems[] = { "empty", "albedo_0", "albedo_1", "albedo_2", "albedo_3" , "albedo_4", "albedo_5" };
	const char* normalMapItems[] = { "empty", "normal_0", "normal_1", "normal_2", "normal_3" , "normal_4", "normal_5" };
	const char* metallicMapItems[] = { "empty",  "metallic_0", "metallic_1", "metallic_2", "metallic_3" , "metallic_4", "metallic_5" };
	const char* roughnessMapItems[] = { "empty", "roughness_0", "roughness_1", "roughness_2", "roughness_3" , "roughness_4", "roughness_5" };
	const char* ambientOcclusionMapItems[] = { "empty", "occlusion_0", "occlusion_1", "occlusion_2", "occlusion_3" , "occlusion_4", "occlusion_5" };

	struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((const char**)data)[n]; return true; } };

	ImGui::Begin("Textures");

	// COLOR_0

	ImGui::Text("Color_0");

	ImGui::PushItemWidth(120);
	ImGui::Combo("Albedo##0", &albedo_current_0, &Funcs::ItemGetter, albedoMapItems, IM_ARRAYSIZE(albedoMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("Normal##0", &normal_current_0, &Funcs::ItemGetter, normalMapItems, IM_ARRAYSIZE(normalMapItems));

	/*ImGui::PushItemWidth(120);
	ImGui::Combo("Metallic##0", &metallic_current_0, &Funcs::ItemGetter, metallicMapItems, IM_ARRAYSIZE(metallicMapItems));*/

	ImGui::PushItemWidth(120);
	ImGui::Combo("Roughness##0", &roughness_current_0, &Funcs::ItemGetter, roughnessMapItems, IM_ARRAYSIZE(roughnessMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("AO##0", &ao_current_0, &Funcs::ItemGetter, ambientOcclusionMapItems, IM_ARRAYSIZE(ambientOcclusionMapItems));

	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Normal##10", &normal_0, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Metallic##10", &metallic_0, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Roughness##10", &roughness_0, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("AO##10", &ao_0, 0.0f, 3.0f, "%.2f");

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Tiling##0", &tilingColor_0, 0.1f);

	// COLOR_1

	ImGui::Text("Color_1");

	ImGui::PushItemWidth(120);
	ImGui::Combo("Albedo##1", &albedo_current_1, &Funcs::ItemGetter, albedoMapItems, IM_ARRAYSIZE(albedoMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("Normal##1", &normal_current_1, &Funcs::ItemGetter, normalMapItems, IM_ARRAYSIZE(normalMapItems));

	/*ImGui::PushItemWidth(120);
	ImGui::Combo("Metallic##1", &metallic_current_1, &Funcs::ItemGetter, metallicMapItems, IM_ARRAYSIZE(metallicMapItems));*/

	ImGui::PushItemWidth(120);
	ImGui::Combo("Roughness##1", &roughness_current_1, &Funcs::ItemGetter, roughnessMapItems, IM_ARRAYSIZE(roughnessMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("AO##1", &ao_current_1, &Funcs::ItemGetter, ambientOcclusionMapItems, IM_ARRAYSIZE(ambientOcclusionMapItems));

	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Normal##11", &normal_1, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Metallic##11", &metallic_1, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Roughness##11", &roughness_1, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("AO##11", &ao_1, 0.0f, 3.0f, "%.2f");

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Tiling##1", &tilingColor_1, 0.1f);
	// COLOR_2

	ImGui::Text("Color_2");

	ImGui::PushItemWidth(120);
	ImGui::Combo("Albedo##2", &albedo_current_2, &Funcs::ItemGetter, albedoMapItems, IM_ARRAYSIZE(albedoMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("Normal##2", &normal_current_2, &Funcs::ItemGetter, normalMapItems, IM_ARRAYSIZE(normalMapItems));

	/*ImGui::PushItemWidth(120);
	ImGui::Combo("Metallic##2", &metallic_current_2, &Funcs::ItemGetter, metallicMapItems, IM_ARRAYSIZE(metallicMapItems));*/

	ImGui::PushItemWidth(120);
	ImGui::Combo("Roughness##2", &roughness_current_2, &Funcs::ItemGetter, roughnessMapItems, IM_ARRAYSIZE(roughnessMapItems));

	ImGui::PushItemWidth(120);
	ImGui::Combo("AO##2", &ao_current_2, &Funcs::ItemGetter, ambientOcclusionMapItems, IM_ARRAYSIZE(ambientOcclusionMapItems));

	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Normal##12", &normal_2, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Metallic##12", &metallic_2, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("Roughness##12", &roughness_2, 0.0f, 3.0f, "%.2f");
	ImGui::PushItemWidth(90);
	ImGui::SliderFloat("AO##12", &ao_2, 0.0f, 3.0f, "%.2f");

	ImGui::PushItemWidth(50);
	ImGui::DragFloat("Tiling##2", &tilingColor_2, 0.1f);

	//-------------------

	ImGui::ColorEdit4("Albedo Color##0", (float*)&albedoColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | 0);
	ImGui::SameLine();
	ImGui::Text("Albedo Color");


	

	ImGui::End();
}

void createAppPropertiesPanel() {

	ImGui::Begin("Properties");

	ImGui::Text("Application average %.1f FPS", ImGui::GetIO().Framerate);

	ImGui::End();
}


void loadAllData() {

	std::string myText;
	std::ifstream prefsFile("prefs.txt");

	getline(prefsFile, myText);
	cameraPosition.x = std::stof(myText);
	getline(prefsFile, myText);
	cameraPosition.y = std::stof(myText);
	getline(prefsFile, myText);
	cameraPosition.z = std::stof(myText);

	getline(prefsFile, myText);
	cameraDirection.x = std::stof(myText);
	getline(prefsFile, myText);
	cameraDirection.y = std::stof(myText);
	getline(prefsFile, myText);
	cameraDirection.z = std::stof(myText);

	getline(prefsFile, myText);
	lightPosition.x = std::stof(myText);
	getline(prefsFile, myText);
	lightPosition.y = std::stof(myText);
	getline(prefsFile, myText);
	lightPosition.z = std::stof(myText);

	getline(prefsFile, myText);
	lightRotation.x = std::stof(myText);
	getline(prefsFile, myText);
	lightRotation.y = std::stof(myText);
	getline(prefsFile, myText);
	lightRotation.z = std::stof(myText);

	getline(prefsFile, myText);
	lightPower = std::stof(myText);

	getline(prefsFile, myText);
	lightColor.x = std::stof(myText);
	getline(prefsFile, myText);
	lightColor.y = std::stof(myText);
	getline(prefsFile, myText);
	lightColor.z = std::stof(myText);

	getline(prefsFile, myText);
	albedo_current_0 = std::stof(myText);
	getline(prefsFile, myText);
	normal_current_0 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_current_0 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_current_0 = std::stof(myText);
	getline(prefsFile, myText);
	ao_current_0 = std::stof(myText);

	getline(prefsFile, myText);
	normal_0 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_0 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_0 = std::stof(myText);
	getline(prefsFile, myText);
	ao_0 = std::stof(myText);

	getline(prefsFile, myText);
	tilingColor_0 = std::stof(myText);

	getline(prefsFile, myText);
	albedo_current_1 = std::stof(myText);
	getline(prefsFile, myText);
	normal_current_1 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_current_1 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_current_1 = std::stof(myText);
	getline(prefsFile, myText);
	ao_current_1 = std::stof(myText);

	getline(prefsFile, myText);
	normal_1 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_1 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_1 = std::stof(myText);
	getline(prefsFile, myText);
	ao_1 = std::stof(myText);

	getline(prefsFile, myText);
	tilingColor_1 = std::stof(myText);

	getline(prefsFile, myText);
	albedo_current_2 = std::stof(myText);
	getline(prefsFile, myText);
	normal_current_2 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_current_2 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_current_2 = std::stof(myText);
	getline(prefsFile, myText);
	ao_current_2 = std::stof(myText);

	getline(prefsFile, myText);
	normal_2 = std::stof(myText);
	getline(prefsFile, myText);
	metallic_2 = std::stof(myText);
	getline(prefsFile, myText);
	roughness_2 = std::stof(myText);
	getline(prefsFile, myText);
	ao_2 = std::stof(myText);

	getline(prefsFile, myText);
	tilingColor_2 = std::stof(myText);

	getline(prefsFile, myText);
	albedoColor.x = std::stof(myText);
	getline(prefsFile, myText);
	albedoColor.y = std::stof(myText);
	getline(prefsFile, myText);
	albedoColor.z = std::stof(myText);

	getline(prefsFile, myText);
	terrainHeight = std::stof(myText);
	getline(prefsFile, myText);
	frequency = std::stof(myText);
	getline(prefsFile, myText);
	octaves = std::stof(myText);
	getline(prefsFile, myText);
	terrainSizeX = std::stof(myText);
	getline(prefsFile, myText);
	terrainSizeZ = std::stof(myText);




	prefsFile.close();
}

bool preffileExistsBefore() {

	if (!std::filesystem::exists("prefs.txt")) {

		std::ofstream prefsFile("prefs.txt");
		prefsFile.close();
		return false;
	}
	return true;
}

void fileSave() {

	std::ofstream prefsFile("prefs.txt");

	prefsFile << std::to_string(cameraPosition.x) << '\n';
	prefsFile << std::to_string(cameraPosition.y) << '\n';
	prefsFile << std::to_string(cameraPosition.z) << '\n';

	prefsFile << std::to_string(cameraDirection.x) << '\n';
	prefsFile << std::to_string(cameraDirection.y) << '\n';
	prefsFile << std::to_string(cameraDirection.z) << '\n';

	prefsFile << std::to_string(lightPosition.x) << '\n';
	prefsFile << std::to_string(lightPosition.y) << '\n';
	prefsFile << std::to_string(lightPosition.z) << '\n';

	prefsFile << std::to_string(lightRotation.x) << '\n';
	prefsFile << std::to_string(lightRotation.y) << '\n';
	prefsFile << std::to_string(lightRotation.z) << '\n';

	prefsFile << std::to_string(lightPower) << '\n';

	prefsFile << std::to_string(lightColor.x) << '\n';
	prefsFile << std::to_string(lightColor.y) << '\n';
	prefsFile << std::to_string(lightColor.z) << '\n';

	prefsFile << std::to_string(albedo_current_0) << '\n';
	prefsFile << std::to_string(normal_current_0) << '\n';
	prefsFile << std::to_string(metallic_current_0) << '\n';
	prefsFile << std::to_string(roughness_current_0) << '\n';
	prefsFile << std::to_string(ao_current_0) << '\n';

	prefsFile << std::to_string(normal_0) << '\n';
	prefsFile << std::to_string(metallic_0) << '\n';
	prefsFile << std::to_string(roughness_0) << '\n';
	prefsFile << std::to_string(ao_0) << '\n';

	prefsFile << std::to_string(tilingColor_0) << '\n';

	prefsFile << std::to_string(albedo_current_1) << '\n';
	prefsFile << std::to_string(normal_current_1) << '\n';
	prefsFile << std::to_string(metallic_current_1) << '\n';
	prefsFile << std::to_string(roughness_current_1) << '\n';
	prefsFile << std::to_string(ao_current_1) << '\n';

	prefsFile << std::to_string(normal_1) << '\n';
	prefsFile << std::to_string(metallic_1) << '\n';
	prefsFile << std::to_string(roughness_1) << '\n';
	prefsFile << std::to_string(ao_1) << '\n';

	prefsFile << std::to_string(tilingColor_1) << '\n';

	prefsFile << std::to_string(albedo_current_2) << '\n';
	prefsFile << std::to_string(normal_current_2) << '\n';
	prefsFile << std::to_string(metallic_current_2) << '\n';
	prefsFile << std::to_string(roughness_current_2) << '\n';
	prefsFile << std::to_string(ao_current_2) << '\n';

	prefsFile << std::to_string(normal_2) << '\n';
	prefsFile << std::to_string(metallic_2) << '\n';
	prefsFile << std::to_string(roughness_2) << '\n';
	prefsFile << std::to_string(ao_2) << '\n';

	prefsFile << std::to_string(tilingColor_2) << '\n';

	prefsFile << std::to_string(albedoColor.x) << '\n';
	prefsFile << std::to_string(albedoColor.y) << '\n';
	prefsFile << std::to_string(albedoColor.z) << '\n';

	prefsFile << std::to_string(terrainHeight) << '\n';
	prefsFile << std::to_string(frequency) << '\n';
	prefsFile << std::to_string(octaves) << '\n';
	prefsFile << std::to_string(terrainSizeX) << '\n';
	prefsFile << std::to_string(terrainSizeZ) << '\n';



	prefsFile.close();
}

void initTextures() {

	GLuint AlbedoTexture_0 = loadBMP_custom("assets/textures/Nature_Moss_1K_albedo.bmp");
	GLuint AlbedoTexture_1 = loadBMP_custom("assets/textures/Rock_CliffGranite_1K_albedo.bmp");
	GLuint AlbedoTexture_2 = loadBMP_custom("assets/textures/Rock_Soil_1K_albedo.bmp");
	GLuint AlbedoTexture_3 = loadBMP_custom("assets/textures/Brick_Rustic2_1K_albedo.bmp");
	GLuint AlbedoTexture_4 = loadBMP_custom("assets/textures/Marble_Checkerboard2_1K_albedo.bmp");
	GLuint AlbedoTexture_5 = loadBMP_custom("assets/textures/scmklhp_4K_Albedo.bmp");

	albedoTextures.push_back(AlbedoTexture_0);
	albedoTextures.push_back(AlbedoTexture_1);
	albedoTextures.push_back(AlbedoTexture_2);
	albedoTextures.push_back(AlbedoTexture_3);
	albedoTextures.push_back(AlbedoTexture_4);
	albedoTextures.push_back(AlbedoTexture_5);

	GLuint NormalTexture_0 = loadBMP_custom("assets/textures/Nature_Moss_1K_normal.bmp");
	GLuint NormalTexture_1 = loadBMP_custom("assets/textures/Rock_CliffGranite_1K_normal.bmp");
	GLuint NormalTexture_2 = loadBMP_custom("assets/textures/Rock_Soil_1K_normal.bmp");
	GLuint NormalTexture_3 = loadBMP_custom("assets/textures/Brick_Rustic2_1K_normal.bmp");
	GLuint NormalTexture_4 = loadBMP_custom("assets/textures/Marble_Checkerboard2_1K_normal.bmp");
	GLuint NormalTexture_5 = loadBMP_custom("assets/textures/scmklhp_4K_Normal.bmp");

	normalTextures.push_back(NormalTexture_0);
	normalTextures.push_back(NormalTexture_1);
	normalTextures.push_back(NormalTexture_2);
	normalTextures.push_back(NormalTexture_3);
	normalTextures.push_back(NormalTexture_4);
	normalTextures.push_back(NormalTexture_5);

	//MetallicTexture_0 = loadBMP_custom("assets/textures/Nature_Moss_1K_metallic.bmp");
	//MetallicTexture_1 = loadBMP_custom("assets/textures/Nature_Moss_1K_metallic.bmp");
	//MetallicTexture_2 = loadBMP_custom("assets/textures/Nature_Moss_1K_metallic.bmp");

	GLuint RoughnessTexture_0 = loadBMP_custom("assets/textures/Nature_Moss_1K_roughness.bmp");
	GLuint RoughnessTexture_1 = loadBMP_custom("assets/textures/Rock_CliffGranite_1K_roughness.bmp");
	GLuint RoughnessTexture_2 = loadBMP_custom("assets/textures/Rock_Soil_1K_roughness.bmp");
	GLuint RoughnessTexture_3 = loadBMP_custom("assets/textures/Brick_Rustic2_1K_roughness.bmp");
	GLuint RoughnessTexture_4 = loadBMP_custom("assets/textures/Marble_Checkerboard2_1K_roughness.bmp");
	GLuint RoughnessTexture_5 = loadBMP_custom("assets/textures/scmklhp_4K_Roughness.bmp");

	roughnessTextures.push_back(RoughnessTexture_0);
	roughnessTextures.push_back(RoughnessTexture_1);
	roughnessTextures.push_back(RoughnessTexture_2);
	roughnessTextures.push_back(RoughnessTexture_3);
	roughnessTextures.push_back(RoughnessTexture_4);
	roughnessTextures.push_back(RoughnessTexture_5);

	GLuint AOTexture_0 = loadBMP_custom("assets/textures/Nature_Moss_1K_ao.bmp");
	GLuint AOTexture_1 = loadBMP_custom("assets/textures/Rock_CliffGranite_1K_ao.bmp");
	GLuint AOTexture_2 = loadBMP_custom("assets/textures/Rock_Soil_1K_ao.bmp");
	GLuint AOTexture_3 = loadBMP_custom("assets/textures/Brick_Rustic2_1K_ao.bmp");
	GLuint AOTexture_4 = loadBMP_custom("assets/textures/Marble_Checkerboard2_1K_ao.bmp");
	GLuint AOTexture_5 = loadBMP_custom("assets/textures/scmklhp_4K_AO.bmp");

	aoTextures.push_back(AOTexture_0);
	aoTextures.push_back(AOTexture_1);
	aoTextures.push_back(AOTexture_2);
	aoTextures.push_back(AOTexture_3);
	aoTextures.push_back(AOTexture_4);
	aoTextures.push_back(AOTexture_5);
}