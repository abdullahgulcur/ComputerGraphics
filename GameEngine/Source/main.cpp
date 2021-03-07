// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdio>
#include <sstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <ImGuiDocking/imgui.h>
#include <ImGuiDocking/imgui_impl_glfw.h>
#include <ImGuiDocking/imgui_impl_opengl3.h>

const char* glsl_version = "#version 330";

#include <EngineAPI\Scene.h>
#include <EngineAPI\Buffer.h>


bool firstTime = true;

static void setUpImGuiLayout(Scene scene, GLuint tex);
void ShowExampleAppDockSpace();
static ImVec2 getLargestSizeForViewport();
static ImVec2 getCenteredPositionForViewport(ImVec2 aspectSize);

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

std::string message;
int lastClickedObjectID = -1;

float mouseScreenX_Percentage = 0;
float mouseScreenY_Percentage = 0;

int SceneObject::objectCount = 0;



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(mode->width, mode->height, "Engine", NULL, NULL);
	glfwMaximizeWindow(window);

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	int windowWidth = mode->width;
	int windowHeight = mode->height;
	// But on MacOS X with a retina screen it'll be 1024*2 and 768*2, so we get the actual framebuffer size:
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	// Handle mouse inputs
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	// Handle mouse scroll
	glfwSetScrollCallback(window, scrollCallback);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark gray background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("Shaders/SimpleVertexShader.vertexshader", "Shaders/SimpleFragmentShader.fragmentshader"); 
	GLuint pickingProgramID = LoadShaders("Shaders/Picking.vertexshader", "Shaders/Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");
	GLuint PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");


	// Load the texture
	//GLuint Texture = loadDDS("UV_Box.DDS");

	// Get a handle for our "myTextureSampler" uniform
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint DiffuseTextureID = glGetUniformLocation(programID, "DiffuseTextureSampler");
	GLuint NormalTextureID = glGetUniformLocation(programID, "NormalTextureSampler");
	GLuint SpecularTextureID = glGetUniformLocation(programID, "SpecularTextureSampler");



	///////---- Scene Initilatization ------////////

	SceneObject sceneObj2("object_2");
	SceneObject sceneObj1("object_1");
	SceneObject sceneObj3("object_3");
	SceneObject sceneObj4("object_4");/*
	SceneObject sceneObj5("object_5");
	SceneObject sceneObj6("object_6");
	SceneObject sceneObj7("object_7");
	SceneObject sceneObj8("object_8");*/

	sceneObj1.transform.position.x = 0;
	sceneObj2.transform.position.x = 3;
	sceneObj3.transform.position.x = 6;
	sceneObj4.transform.position.x = 9;

	//sceneObj5.transform.position.x = 0;
	//sceneObj6.transform.position.x = 3;
	//sceneObj7.transform.position.x = 6;
	//sceneObj8.transform.position.x = 9;

	//sceneObj5.transform.position.y = 6;
	//sceneObj6.transform.position.y = 2;
	//sceneObj7.transform.position.y = 2;
	//sceneObj8.transform.position.y = 2;

	sceneObj1.transform.position.z = -3;
	sceneObj2.transform.position.z = -3;
	sceneObj3.transform.position.z = -3;
	sceneObj4.transform.position.z = -3;/*
	sceneObj5.transform.position.z = -3;
	sceneObj6.transform.position.z = -3;
	sceneObj7.transform.position.z = -3;
	sceneObj8.transform.position.z = -3;*/


	sceneObj1.setMesh("HU.obj", "UV_Box.DDS", "brick_normal.bmp", "spec.DDS");
	//sceneObj2.setMesh("suzanne.obj", "soil.DDS", "brick_normal.bmp", "spec.DDS");
	//sceneObj3.setMesh("torus.obj", "uvmap.DDS", "brick_normal.bmp", "spec.DDS");
	/*sceneObj1.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS");
	sceneObj2.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS");
	sceneObj3.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS");*/
	//sceneObj4.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS");
	
	/*
	sceneObj5.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS", glm::vec3(0.0f, 0.4f, 0.5f));
	sceneObj6.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS", glm::vec3(0.7f, 0.0f, 0.3f));
	sceneObj7.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS", glm::vec3(0.2f, 0.0f, 0.6f));
	sceneObj8.setMesh("cylinder.obj", "brickwall.DDS", "brick_normal.bmp", "spec.DDS", glm::vec3(0.8f, 0.3f, 0.2f));*/


	Scene scene("SampleScene");
	scene.AddSceneObject(sceneObj1);
	/*scene.AddSceneObject(sceneObj1);
	scene.AddSceneObject(sceneObj4);
	scene.AddSceneObject(sceneObj3);*/
	
	/*
	scene.AddSceneObject(sceneObj5);
	scene.AddSceneObject(sceneObj6);
	scene.AddSceneObject(sceneObj7);
	scene.AddSceneObject(sceneObj8);*/

	///////--------------------------------////////


	std::vector<Buffer> bufferList;
	std::vector<std::string> objNameList;

	// Extract different .obj file names
	for (int i = 0; i < scene.sceneObjectList.size(); i++) {

		if (!std::count(objNameList.begin(), objNameList.end(), scene.sceneObjectList[i].mesh.meshName)) {
			objNameList.push_back(scene.sceneObjectList[i].mesh.meshName);
		}
	}

	// create buffers with .obj file name
	for (int i = 0; i < objNameList.size(); i++) {

		bufferList.push_back(Buffer(objNameList[i]));
	}

	// Get a handle for our "pickingColorID" uniform
	GLuint pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	GLuint offsetID = glGetUniformLocation(programID, "offset");
	glUniform1f(offsetID, 1);



	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// ---------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	//// Alternative : Depth texture. Slower, but you can sample it later in your shader
	//GLuint depthTexture;
	//glGenTextures(1, &depthTexture);
	//glBindTexture(GL_TEXTURE_2D, depthTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 768, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	//// Depth texture alternative : 
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);


	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;


	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	GLuint quad_programID = LoadShaders("Shaders/Passthrough.vertexshader", "Shaders/WobblyTexture.fragmentshader");
	GLuint texID = glGetUniformLocation(quad_programID, "renderedTexture");
	GLuint timeID = glGetUniformLocation(quad_programID, "time");








	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// For ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	do {
		
		//setInputEvents();

		// Maintain designated frequency of 5 Hz (200 ms per frame)
		a = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> work_time = a - b;

		if (work_time.count() < 16.67)
		{
			std::chrono::duration<double, std::milli> delta_ms(16.67 - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}

		b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - a;



		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0, 0, windowWidth, windowHeight); // Render on the whole framebuffer, complete from the lower left corner to the upper right

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// For ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();


		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {

			// Clear the screen in white
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(pickingProgramID);

			for (SceneObject obj : scene.sceneObjectList) {
			
				glm::mat4 ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(obj.transform.position.x, obj.transform.position.y, obj.transform.position.z));
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

				/*glm::quat orientation = glm::quat(glm::vec3(0, 0, 0));
				glm::vec3 position = glm::vec3(obj.transform.position.x, obj.transform.position.y, obj.transform.position.z);
				glm::mat4 RotationMatrix = glm::toMat4(orientation);
				glm::mat4 TranslationMatrix = translate(mat4(), position);
				glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;*/

				glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

				// Convert "i", the integer mesh ID, into an RGB color
				int r = (obj.ID & 0x000000FF) >> 0;
				int g = (obj.ID & 0x0000FF00) >> 8;
				int b = (obj.ID & 0x00FF0000) >> 16;

				// OpenGL expects colors to be in [0,1], so divide by 255.
				glUniform4f(pickingColorID, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);

				GLuint vertexbuffer;
				GLuint elementbuffer;
				std::vector<unsigned short> indices;

				// tamamen yeni bufferlar, butun vertex icin butun degerler(color vs) ayri texture ler. All hardcoded trasnportation

				for (int i = 0; i < bufferList.size(); i++) {

					if (obj.mesh.meshName.compare(bufferList[i].meshName) == 0) {

						vertexbuffer = bufferList[i].vertexbuffer;
						elementbuffer = bufferList[i].elementbuffer;
						indices = bufferList[i].indices;
					}
				}

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glVertexAttribPointer(
					0,                  // attribute
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// Index buffer
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

				// Draw the triangles !
				glDrawElements(
					GL_TRIANGLES,      // mode
					indices.size(),    // count
					GL_UNSIGNED_SHORT,   // type
					(void*)0           // element array buffer offset
				);
			}
			glDisableVertexAttribArray(0);

			// Wait until all the pending drawing commands are really done.
			// Ultra-mega-over slow ! 
			// There are usually a long time between glDrawElements() and
			// all the fragments completely rasterized.
			glFlush();
			glFinish();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// Read the pixel at the center of the screen.
			// You can also use glfwGetMousePos().
			// Ultra-mega-over slow too, even for 1 pixel, 
			// because the framebuffer is on the GPU.
			unsigned char data[4];
			glReadPixels(mouseScreenX_Percentage * 1920, mouseScreenY_Percentage * 1080, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

			// Convert the color back to an integer ID
			int pickedID =
				data[0] +
				data[1] * 256 +
				data[2] * 256 * 256;

			if (pickedID == 0x00ffffff) { // Full white, must be the background !
				message = "background";
				lastClickedObjectID = -1;
			}
			else {

				for (SceneObject obj : scene.sceneObjectList) {
					
					if (obj.ID == pickedID) {
						message = obj.name;
						lastClickedObjectID = obj.ID;
					}
				}
			}

			// Uncomment these lines to see the picking shader in effect
			//glfwSwapBuffers(window);
			//continue; // skips the normal rendering
		}

		// Dark blue background
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		// Re-clear the screen for real rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(0, 2, 7);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


		// get all objects from scene
		for (SceneObject obj : scene.sceneObjectList) {

			glm::mat4 ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(obj.transform.position.x, obj.transform.position.y, obj.transform.position.z));
			glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
			glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
					// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);

			GLuint vertexbuffer;
			GLuint uvbuffer;
			GLuint normalbuffer;
			GLuint tangentbuffer;
			GLuint bitangentbuffer;
			GLuint elementbuffer;
			std::vector<unsigned short> indices;

			// tamamen yeni bufferlar, butun vertex icin butun degerler(color vs) ayri texture ler. All hardcoded trasnportation

			for (int i = 0; i < bufferList.size(); i++) {

				if (obj.mesh.meshName.compare(bufferList[i].meshName) == 0) {

					vertexbuffer = bufferList[i].vertexbuffer;
					uvbuffer = bufferList[i].uvbuffer;
					normalbuffer = bufferList[i].normalbuffer;
					tangentbuffer = bufferList[i].tangentbuffer;
					bitangentbuffer = bufferList[i].bitangentbuffer;
					elementbuffer = bufferList[i].elementbuffer;
					indices = bufferList[i].indices;
				}
			}

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, obj.mesh.texture);
			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(DiffuseTextureID, 0);

			// Bind our normal texture in Texture Unit 1
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, obj.mesh.NormalTexture);
			// Set our "NormalTextureSampler" sampler to use Texture Unit 1
			glUniform1i(NormalTextureID, 1);

			// Bind our specular texture in Texture Unit 2
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, obj.mesh.SpecularTexture);
			// Set our "SpecularTextureSampler" sampler to use Texture Unit 2
			glUniform1i(SpecularTextureID, 2);


			// The rest is exactly the same as the first object

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 4th attribute buffer : tangents
			glEnableVertexAttribArray(3);
			glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
			glVertexAttribPointer(
				3,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 5th attribute buffer : bitangents
			glEnableVertexAttribArray(4);
			glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
			glVertexAttribPointer(
				4,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);


		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Render on the whole framebuffer, complete from the lower left corner to the upper right
		glViewport(0, 0, windowWidth, windowHeight);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(quad_programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		// Set our "renderedTexture" sampler to use Texture Unit 0
		glUniform1i(texID, 0);

		//glUniform1f(timeID, (float)(glfwGetTime() * 10.0f));

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangles !
		//glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);


		setUpImGuiLayout(scene, renderedTexture);
		//// For ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		SceneObject* object = NULL;

		if (getObjCanMove()) {

			if (lastClickedObjectID != -1) {

				
				int index = - 1;
				for (int i = 0; i < scene.sceneObjectList.size(); i++) { // poor finding

					if (scene.sceneObjectList[i].ID == lastClickedObjectID) {
						index = i;
						break;
					}
				}

				if (index != -1) {

					if (getCanTransformAt_X()) {
						scene.sceneObjectList[index].transform.position.x += getMouseDeltaX() * 0.003;
					}

					if (getCanTransformAt_Y()) {
						scene.sceneObjectList[index].transform.position.y -= getMouseDeltaY() * 0.003;
					}

					if (getCanTransformAt_Z()) {
						scene.sceneObjectList[index].transform.position.z += getMouseDeltaX() * 0.003;
					}
				}
			}
		}



		firstTime = false;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader

	for (int i = 0; i < bufferList.size(); i++) {

		glDeleteBuffers(1, &bufferList[i].vertexbuffer);
		glDeleteBuffers(1, &bufferList[i].uvbuffer);
		glDeleteBuffers(1, &bufferList[i].normalbuffer);
		glDeleteBuffers(1, &bufferList[i].tangentbuffer);
		glDeleteBuffers(1, &bufferList[i].bitangentbuffer);
		glDeleteBuffers(1, &bufferList[i].elementbuffer);
	}

	for (int i = 0; i < scene.sceneObjectList.size(); i++) {

		glDeleteTextures(1, &scene.sceneObjectList[i].mesh.texture);
	}
	
	glDeleteProgram(programID);

	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &depthrenderbuffer);
	glDeleteBuffers(1, &quad_vertexbuffer);

	glDeleteVertexArrays(1, &VertexArrayID);

	// End ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


void ShowExampleAppDockSpace()
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	bool p_open = true;
	ImGui::Begin("MyDockSpace", &p_open, window_flags);
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

}



static void setUpImGuiLayout(Scene scene, GLuint tex) {
	
	static int selectedObjIndex = -1;

	ShowExampleAppDockSpace();

	ImGuiWindowFlags windowFlagsForSceneViewport = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	bool p_open = true;

	ImGui::Begin("Scene", &p_open, windowFlagsForSceneViewport);

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);


	ImVec2 startPos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 mousePos = ImGui::GetMousePos();

	float deltaX = mousePos.x - startPos.x;
	float deltaY = mousePos.y - startPos.y;
	mouseScreenX_Percentage = deltaX / size.x;
	mouseScreenY_Percentage = deltaY / (size.y - 30);
	
	//ImGui::Text("xpos: %.1f", deltaX);
	//ImGui::Text("ypos: %.1f", deltaY);

	setSceneViewPortStartEndValues(startPos.x, startPos.y, startPos.x + size.x, startPos.y + size.y);

	ImVec2 windowSize = getLargestSizeForViewport();
	ImVec2 windowPos = getCenteredPositionForViewport(windowSize);

	ImGui::SetCursorPos(windowPos);
	ImTextureID textureId = (ImTextureID)tex;
	ImGui::Image(textureId, windowSize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();

	ImGui::Begin("Project");
	ImGui::End();

	ImGui::Begin("Console");
	ImGui::End();

	ImGui::Begin("Hieararchy");

	char name[1024];
	strcpy(name, scene.sceneName.c_str());

	if (firstTime)
		ImGui::SetNextTreeNodeOpen(true);

	if (ImGui::CollapsingHeader(name))
	{
		for (int n = 0; n < scene.sceneObjectList.size(); n++)
		{
			char objName[1024];
			strcpy(objName, scene.sceneObjectList.at(n).name.c_str());
			char buf[1024];
			sprintf(buf, objName, n);
			if (ImGui::Selectable(buf, selectedObjIndex == n))
				selectedObjIndex = n;
		}
	}
	ImGui::End();

	ImGui::Begin("Inspector");

	if (firstTime)
		ImGui::SetNextTreeNodeOpen(true);

	if (selectedObjIndex != -1) {
		if (ImGui::CollapsingHeader("Transform"))
		{
			static float position[4] = { 0, 0, 0, 0 };
			static float rotation[4] = { 0, 0, 0, 0 };
			static float scale[4] = { 0, 0, 0, 0 };

			position[0] = scene.sceneObjectList.at(selectedObjIndex).transform.position.x;
			position[1] = scene.sceneObjectList.at(selectedObjIndex).transform.position.y;
			position[2] = scene.sceneObjectList.at(selectedObjIndex).transform.position.z;

			rotation[0] = scene.sceneObjectList.at(selectedObjIndex).transform.rotation.x;
			rotation[1] = scene.sceneObjectList.at(selectedObjIndex).transform.rotation.y;
			rotation[2] = scene.sceneObjectList.at(selectedObjIndex).transform.rotation.z;

			scale[0] = scene.sceneObjectList.at(selectedObjIndex).transform.scale.x;
			scale[1] = scene.sceneObjectList.at(selectedObjIndex).transform.scale.y;
			scale[2] = scene.sceneObjectList.at(selectedObjIndex).transform.scale.z;

			ImGui::InputFloat3("Position(xyz)", position);
			ImGui::InputFloat3("Rotation(xyz)", rotation);
			ImGui::InputFloat3("   Scale(xyz)", scale);
		}
	}
	
	ImGui::Text("Last picked object: %s", message.c_str());//deltaXPercentage
	ImGui::Text("Percentages X: %.2f Y: %.2f", mouseScreenX_Percentage, mouseScreenY_Percentage);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	// this end is for viewport
	ImGui::End();
}

static ImVec2 getLargestSizeForViewport() {

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	windowSize.x -= ImGui::GetScrollX();
	windowSize.y -= ImGui::GetScrollY();

	float aspectWidth = windowSize.x;
	float aspectHeight = aspectWidth / ((float)1920 / 1080);
	if (aspectHeight > windowSize.y) {
		// We must switch to pillarbox mode
		aspectHeight = windowSize.y;
		aspectWidth = aspectHeight * ((float)1920 / 1080);
	}

	return ImVec2(aspectWidth, aspectHeight);
}

static ImVec2 getCenteredPositionForViewport(ImVec2 aspectSize) {

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	windowSize.x -= ImGui::GetScrollX();
	windowSize.y -= ImGui::GetScrollY();

	float viewportX = (windowSize.x / 2.0f) - (aspectSize.x / 2.0f);
	float viewportY = (windowSize.y / 2.0f) - (aspectSize.y / 2.0f);

	return ImVec2(viewportX + ImGui::GetCursorPosX(), viewportY + ImGui::GetCursorPosY());
}