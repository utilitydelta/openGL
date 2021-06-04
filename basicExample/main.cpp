#define GLEW_STATIC

#include <GL/glew.h> //must be included before glfw
#include <GLFW/glfw3.h>

#include <iostream>

static GLuint SetupTriangle() {
	//Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Create a vertex buffer object to store the data
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//Add data (x,y,color)
	GLfloat verticies[] = {
		-1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, 5  * 3 * sizeof(GLfloat), &verticies, GL_STATIC_DRAW);

	//setup vertex attribute data which is the input to the vertex shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//The fifth parameter is set to 5*sizeof(float) now, because each vertex consists of 5 floating point 
	//attribute values. The offset of 2*sizeof(float) for the color attribute is there because each vertex 
	//starts with 2 floating point values for the position that it has to skip over.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2 * sizeof(float)));

	//finished setup, can unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return vao;
}

static GLuint CompileShader(const char* source, int type) {
	auto shaderId = glCreateShader(type);
	glShaderSource(shaderId, 1, &source, nullptr);
	glCompileShader(shaderId);

	int result;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length = 1024;
		char log[1024] = { 0 };
		glGetShaderInfoLog(shaderId, length, &length, log);
		std::cout << log << std::endl;
		throw;
	}
	return shaderId;
}

static GLuint CreateVertexShader() {
	const char* source =
		"\n #version 330 core"
		"\n layout(location=0) in vec2 position; " //https://stackoverflow.com/questions/4635913/explicit-vs-automatic-attribute-location-binding-for-opengl-shaders/4638906
		"\n layout(location=1) in vec3 color; "
		"\n out vec3 Color; "
		"\n uniform float scaleFactor; "
		"\n void main () "
		"\n { "
		"\n  Color = color; "
		"\n  gl_Position = vec4(scaleFactor*position.x, scaleFactor*position.y, 0.0, 1.0); " //gl_Position is built into open gl vertex shader (an out var)
		"\n } ";
	return CompileShader(source, GL_VERTEX_SHADER);
}

static GLuint CreateFragmentShader() {
	const char* source =
		"\n #version 330 core"
		"\n in vec3 Color; "
		"\n out vec4 outColor; " //only 1 output variable (the pixel colour), can be called whatever
		"\n uniform vec4 triangleColor; " //allow input variable to change the colour dynamically
		"\n void main () "
		"\n { "
		"\n  outColor = vec4(Color, 1.0); "
		"\n } ";
	return CompileShader(source, GL_FRAGMENT_SHADER);
}

static GLuint CreateProgram() {
	//Setup program with shaders
	GLuint programId = glCreateProgram();

	auto vertexShaderId = CreateVertexShader();
	auto fragmentShaderId = CreateFragmentShader();

	//attach shaders
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	// Link our program
	glLinkProgram(programId);
	int result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		char log[1024] = { 0 };
		glGetProgramInfoLog(programId, 1024, NULL, log);
		std::cout << log << std::endl;
		throw;
	}

	//checks to see whether the executables contained in program can execute given the current OpenGL state
	glValidateProgram(programId);

	//detach or shaders now its linked
	glDetachShader(programId, vertexShaderId);
	glDetachShader(programId, fragmentShaderId);

	return programId;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	//Set the opengl version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//Use core profile (strict)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//Ensure we sync with the monitor refresh rate or we will kill the GPU/CPU
	glfwSwapInterval(1);

	//initialise glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		return -1;
	}
	std::cout << "Status: Using GLEW: " << glewGetString(GLEW_VERSION) << std::endl;

	auto vao = SetupTriangle();
	auto programId = CreateProgram();

	int red = 0;
	int green = 50;
	int scaleFactor = 0;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		//bind - what shader / data do we want to draw
		glUseProgram(programId);
		glBindVertexArray(vao);

		GLint uniColor = glGetUniformLocation(programId, "triangleColor");
		glUniform4f(uniColor, red / 100.0f, green / 100.0f, 0.0, 1.0);

		GLint uniScale = glGetUniformLocation(programId, "scaleFactor");
		glUniform1f(uniScale, scaleFactor / 100.0f);

		//issue the draw call
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//unbind
		glBindVertexArray(0);
		glUseProgram(0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		red++;
		if (red > 100) red = 0;
		scaleFactor++;
		if (scaleFactor > 100) scaleFactor = 0;
		green--;
		if (green < 0) green = 50;
	}

	glfwTerminate();
	return 0;
}