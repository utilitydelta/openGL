#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

/// <summary>
/// Create a shader, set its source code and compile it.
/// </summary>
/// <param name="shader"></param>
/// <param name="type">eg GL_VERTEX_SHADER or GL_FRAGMENT_SHADER</param>
/// <returns></returns>
static unsigned int CompileShader(const std::string& shader, unsigned int type) {
	auto id = glCreateShader(type);

	auto rawStr = shader.c_str();
	glShaderSource(id, 1, &rawStr,nullptr);
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

	if (glewInit() != GLEW_OK) {
		throw;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	//triangle verticies
	float positions[6] = {
		-0.5f, -0.5f,
		 0.0f,  0.5f,
		 0.5f, -0.5f
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

	//create some shaders
	std::string vertexShader = 
		"\n #version 330 core "
		"\n layout(location = 0) in vec4 position; "
		"\n void main() "
		"\n {"
		"\n   gl_Position = position; "
		"\n }";
	std::string fragmentShader =
		"\n #version 330 core "
		"\n layout(location = 0) out vec4 color; "
		"\n void main() "
		"\n {"
		"\n   color = vec4(1.0, 0.0, 0.0, 1.0); "
		"\n }";
	auto programId = CreateShaderProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		//for (auto i = 0; i < 6; i++) {
		//	positions[i] = positions[i] * -1;
		//}

		//old school openGL
		//glBegin(GL_TRIANGLES);
		//glVertex2f(-0.5f, -0.5f);
		//glVertex2f(0.0f, 0.5f);
		//glVertex2f(0.5f, -0.5f);
		//glEnd();

		glDrawArrays(GL_TRIANGLES, 0, 3);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteShader(programId);

	glfwTerminate();
	return 0;
}