#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include <iostream>
#include<vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float currentTime = 0.0f, PrevTime = 0.0f, deltaTime;

bool firstClick;


int shaderProgram;

class Particle
{
	private:
		unsigned int VAO, VBO;
			
		glm::vec3 vertices[12];

		glm::vec2 vel;

		void SetBufferData()
		{
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
	
			//std::cout << "BufferdataSet";
		}
	public:
		float radius;
		glm::vec2 position;
		
		Particle()
		{
			Particle(0.0,0.0,0.1);
		}
		Particle(float x, float y)
		{
			Particle(x, y, 0.1);
		}
		Particle(float x, float y,float rad)
		{
			//std::cout << "In main constructor";
			radius = rad;

			position.x = x;
			position.y = y;

			do
			{
				vel.x = (rand() % 10 - 5) * 0.01;
				vel.y = (rand() % 10 - 5) * 0.01;
			} while (vel.x == 0 && vel.y == 0);

			for (int i = 0; i < 12; i++)
			{
				vertices[i] = glm::vec3(cos(glm::radians(i*30.0)), sin(glm::radians(i*30.0)) * (float)SCR_WIDTH/SCR_HEIGHT, 0) * rad;
				//std::cout << vertices[i].x << " "<<vertices[i].y<<"\n";
			}

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			SetBufferData();

		}
		~Particle()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);

			//std::cout << "Particle Destroyed";
		}

		void draw()
		{
			//std::cout << "InParticleDraw";
			glBindVertexArray(VAO);

			glUseProgram(shaderProgram);

			glUniform1f(glGetUniformLocation(shaderProgram, "xOffset"), position.x);
			glUniform1f(glGetUniformLocation(shaderProgram, "yOffset"), position.y);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 12);

			
			
			position.x += (vel.x * deltaTime);
			position.y += (vel.y * deltaTime);
		}
};


class ParticleEmitter
{
	private:
		int size;
		std::vector<Particle*> particles;
		
	public:
		int lifetime;

		ParticleEmitter()
		{
			int randSize = rand() % 10 + 15;
			ParticleEmitter(glm::vec2(0, 0), 1, 500);
		}
		ParticleEmitter(glm::vec2 position,int size,int lifetime)
		{
			this->size = size;
			this->lifetime = lifetime;
			for (int i = 0; i < size; i++)
			{
				particles.push_back(new Particle(position.x, position.y,0.02f));
			}
		}
		~ParticleEmitter()
		{
			for (int i = 0; i < particles.size(); i++)
				delete particles[i];
			std::cout << "Particle EMitter destroyed\n";
		}
		void draw()
		{
			for (int i = 0; i < size; i++)
			{
				particles[i]->draw();

				
			}
			lifetime--;
		}
};


glm::vec2 ScreenToWorldPoint(double x, double y);
ParticleEmitter* GetPointer(glm::vec2 position, int size, int lifetime);

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform float xOffset,yOffset;"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x + xOffset, aPos.y + yOffset, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";


//Vector to store pointers of ParticleEmitters
std::vector<ParticleEmitter*> ParticleSystem;
int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwInit();

														 // glfw window creation
														 // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	glewExperimental = true;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		// Problem: glewInit failed, something is seriously wrong.
		char ch;
		std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
		std::cin >> ch;
		exit(1);

	}


	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// ------------------------------------------------------------------

	
	Particle p(0,0,0.1);
	PrevTime = glfwGetTime();
	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		currentTime = glfwGetTime();
		deltaTime = currentTime - PrevTime;
		PrevTime = currentTime;

		//std::cout << deltaTime<<"\n";
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		
		for (int i = 0; i < ParticleSystem.size() && ParticleSystem[i]!=nullptr; i++)
		{
			ParticleSystem[i]->draw();
			if (ParticleSystem[i]->lifetime < 0)
			{
				ParticleEmitter *temp = ParticleSystem[i];
				ParticleSystem[i] = nullptr;
				ParticleSystem.erase(ParticleSystem.begin() + i);
				delete temp;
			}
		} 
		//p.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		if (firstClick)
		{
			firstClick = false;

			double x, y;

			glfwGetCursorPos(window, &x, &y);

			ParticleSystem.push_back(GetPointer(ScreenToWorldPoint(x, y),25,10000));

			std::cout <<"ParticleSystem Size : " <<ParticleSystem.size()<<"\n";
		}
	}
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		firstClick = true;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


glm::vec2 ScreenToWorldPoint(double x, double y)
{
	
	float _x = (x - (float)SCR_WIDTH / 2) / (SCR_WIDTH/2);
	float _y = (-y + (float)SCR_HEIGHT / 2) / (SCR_HEIGHT / 2);

	//std::cout << _x << _y;
	return glm::vec2(_x, _y);
}


ParticleEmitter* GetPointer(glm::vec2 position, int size, int lifetime)
{
	return new ParticleEmitter(position, size, lifetime);
}