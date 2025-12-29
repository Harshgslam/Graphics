#include <iostream>
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <vector>
using namespace std;

int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuit = false;

//VAO
GLuint gVertexArrayObject = 0;
//VBO
GLuint gVertexBufferObject = 0;

// Program Objecct (for our shaders)
GLuint gGraphicsPipelineShaderProgram = 0;


// We are going to get vertex and fragment shaders from some txt files. but for simplicity, currently we are setting global variables for them 
const std::string gVertexShaderSource =
"#version 410 core\n"
"in vec4 position;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position.x,position.y,position.z,position.w);\n"
"}\n";

const std::string gFragmentShaderSource =
"#version 410 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = vec4(1.0f,0.5f,0.0f,1.0f);\n"
"}\n";

void GOpenGLVersionInfo() {
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void InitializeProgram() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
		std::cout << "SDL3 could not initialize video subsystem" << std::endl;

		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gWindow = SDL_CreateWindow("Graphic Window", gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

	if (gWindow == nullptr) {
		SDL_Log("Could not create window: %s", SDL_GetError());
		std::cout << "SDL3 could not create window" << std::endl;
		//SDL_Quit();
		exit(1);
	}

	gOpenGLContext = SDL_GL_CreateContext(gWindow);
	if (gOpenGLContext == nullptr) {
		SDL_Log("Could not create OpenGL context: %s", SDL_GetError());
		std::cout << "SDL3 could not create OpenGL context" << std::endl;
		//SDL_DestroyWindow(gWindow);
		//SDL_Quit();
		exit(1);
	}
	SDL_GL_MakeCurrent(gWindow,gOpenGLContext);
	// Initialize the glad library, just hoops in all the OpenGL functions
	if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		//SDL_GL_DeleteContext(gOpenGLContext);
		//SDL_DestroyWindow(gWindow);
		//SDL_Quit();
		exit(1);
	}
	GOpenGLVersionInfo();
}


void Input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			gQuit = true;
		}
	}
}
void PreDraw() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(gGraphicsPipelineShaderProgram);
}
void Draw() {
	glBindVertexArray(gVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	// cleanup program 
	glUseProgram(0);
}
void MainLoop() {
	while(!gQuit) {
		Input();
		
		PreDraw();

		Draw();

		//Update the screen of our specified window
		SDL_GL_SwapWindow(gWindow);
	}

}
void CleanUp() {

	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}

void VertexSpecifiation() {
	// Lives on the CPU
	
	const std::vector<GLfloat> vertexPositions = {
		// x     y	    z
		-0.5f, -0.5f, 0.0f, // vertex 1
		 0.5f, -0.5f, 0.0f, // vertex 2
		 0.0f,  0.5f, 0.0f  // vertex 3
	};
	
	// We Start setting things up on the GPU
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// Start generating our VBO
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexArrayObject);
	glBufferData(GL_ARRAY_BUFFER,
		vertexPositions.size() * sizeof(GLfloat),
		vertexPositions.data(),
		GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);  // Enable the Vertex Attribute Array at index 0 of the buffer array
	glVertexAttribPointer(0, // Attribute 0 corresponds to the enabled glEnableVertexAttribArray
		3,                   // size (number of components per vertex attribute)
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		0,                   // stride
		(void*)0             // array buffer offset
	);
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
}

GLuint CompileShader(GLuint type, const std::string& source) {
	GLuint shaderObject;

	if (type == GL_VERTEX_SHADER) {
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER) {
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();

	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	// Retrieve the compile status
	int result;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE) {
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> errorLog(length);
		glGetShaderInfoLog(shaderObject, length, &length, &errorLog[0]);
		std::cerr << "Shader compilation failed: " << &errorLog[0] << std::endl;

		if(type == GL_VERTEX_SHADER) {
			std::cerr << "Vertex Shader compilation error." << std::endl;
		}
		else if(type == GL_FRAGMENT_SHADER) {
			std::cerr << "Fragment Shader compilation error." << std::endl;
		}

		//errorLog.clear();
		// delete the shader object
		glDeleteShader(shaderObject);
		return 0;
	}

	return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexshadersource, const std::string& fragmentshadersource) {
	GLuint programObject = glCreateProgram();

	GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexshadersource);
	GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentshadersource);

	glAttachShader(programObject, myVertexShader);
	glAttachShader(programObject, myFragmentShader);
	glLinkProgram(programObject);

	//validate our program
	glValidateProgram(programObject);

	//glDetachShader, glDeleteShader

	return programObject;
}

void CreateGraphicsPipeline() {
	gGraphicsPipelineShaderProgram = CreateShaderProgram(gVertexShaderSource, gFragmentShaderSource);
}

int main(int argc, char* argv[]) {


	// 1. Setup the graphics program
	InitializeProgram();

	// 2. Setup our geometry
	VertexSpecifiation();

	// 3. Create our graphics pipeline(shaders)
	//  - At a minimum, this means the vertex and fragment shader
	CreateGraphicsPipeline();

	// 4. Call the main application loop
	MainLoop();

	// 5. Cleanup and close the application
	CleanUp();

	return 0;
}
