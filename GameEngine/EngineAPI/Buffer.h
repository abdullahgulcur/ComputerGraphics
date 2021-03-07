#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL\glew.h>

#ifndef BUFFER_H
#define BUFFER_H


class Buffer {

public:
	std::string meshName;

	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint tangentbuffer;
	GLuint bitangentbuffer;
	GLuint elementbuffer;
	
	std::vector<unsigned short> indices; // temp

	Buffer(std::string meshName);


private:

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;

};

#endif