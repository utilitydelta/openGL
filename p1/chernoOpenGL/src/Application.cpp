#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
	while (auto err = glGetError()) {
		std::cout << "OpenGL Error: " << err << " function: " << function << " file: " << file << " line: " << line << std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filePath) {
	std::ifstream stream(filePath);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			ss[(int)type] << line << '\n';
		}
	}

	return ShaderProgramSource{
		ss[0].str(), ss[1].str()
	};
}

/// <summary>
/// Create a shader, set its source code and compile it.
/// </summary>
/// <param name="shader"></param>
/// <param name="type">eg GL_VERTEX_SHADER or GL_FRAGMENT_SHADER</param>
/// <returns></returns>
static unsigned int CompileShader(const std::string& shader, unsigned int type) {
	auto id = glCreateShader(type);

	auto rawStr = shader.c_str();
	glShaderSource(id, 1, &rawStr, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

/// <summary>
/// Create a program with both a vertex and fragment shader, compiled and ready to execute.
/// </summary>
/// <param name="vertexShader"></param>
/// <param name="fragmentShader"></param>
/// <returns></returns>
static unsigned int CreateShaderProgram(const std::string& vertexShader, const std::string& fragmentShader)
{
	//program contains set of shaders
	auto program = glCreateProgram();

	auto vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
	auto fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	//link the compiled shaders into the program
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	//creates executables that will run on GPU processors (eg. vertex / geometry / fragment shader)
	glLinkProgram(program);

	//checks to see whether the executables contained in program can execute given the current OpenGL state
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

static void PrepareVertices() {
	//triangle verticies
	float positions[] = {
		-0.5f, -0.5f,	//0
		 0.5f,  -0.5f,	//1
		 0.5f, 0.5f,	//2
		 -0.5f, 0.5f,	//3
	};

	//allocate a buffer to use
	unsigned int buffer;
	glGenBuffers(1, &buffer);

	//bind the buffer to the GL_ARRAY_BUFFER target
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//populate the buffer with the actual data - optimised as static & for drawing
	auto positionsLength = sizeof(positions) / sizeof(*positions);
	glBufferData(GL_ARRAY_BUFFER, positionsLength * sizeof(float), positions, GL_STATIC_DRAW);

	//describes the layout of an individual vertex (contains two floats)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	//specify that we are using this vertex array
	glEnableVertexAttribArray(0);
}

static void PrepareIndexBuffer() {
	//Index Buffers in OpenGL
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0,
	};
	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	auto indicesLength = sizeof(indices) / sizeof(*indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesLength * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

static void UpdateIncrementExample(float& r, float& increment) {
	if (r > 1.0f) {
		increment = -0.05;
	}
	else if (r < 0.05f) {
		increment = 0.05f;
	}

	r += increment;
}

int main(void)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		throw;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	PrepareVertices();
	PrepareIndexBuffer();

	auto shaderSource = ParseShader("res/shaders/basic.shader");
	auto programId = CreateShaderProgram(shaderSource.VertexSource, shaderSource.FragmentSource);
	glUseProgram(programId);

	//get reference to the 'uniform' in the shader called u_Color
	int location_u_Color = glGetUniformLocation(programId, "u_Color");
	ASSERT(location_u_Color != -1);

	float r = 0.0f;
	float increment = 0.05f;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		//set the variable in the shader code
		glUniform4f(location_u_Color, r, 1.0, 0.5, 0.2);

		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		UpdateIncrementExample(r, increment);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(programId);

	glfwTerminate();
	return 0;
}