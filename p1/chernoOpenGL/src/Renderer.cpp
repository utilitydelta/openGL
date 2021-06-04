#include "Renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
	while (auto err = glGetError()) {
		std::cout << "OpenGL Error: " << err << " function: " << function << " file: " << file << " line: " << line << std::endl;
		return false;
	}
	return true;
}