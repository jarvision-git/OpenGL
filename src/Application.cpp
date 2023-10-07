#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x,__FILE__,__LINE__))


static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << endl;
        return false;
    }
    return true;
}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {


    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    ifstream stream(filepath);
    string line;
    stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                //setting mode to vertex shader
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                //setting mode to fragment 
                type = ShaderType::FRAGMENT;
            }
        }
        //entering specific shader type into the required shader
        else {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(),ss[1].str() };

}

using namespace std;

static unsigned int CompileShader(unsigned int type, const string& source) {

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();  //&source[0]
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // TODO: error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader" << endl;
        cout << message << endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        cout << "Error";
    }

    float positions[]{
        -0.5f,-0.5f,
        0.5f,-0.5f,
        0.5f, 0.5f,
        -0.5f,0.5f,
    };

    unsigned int indices[] = {
        0,1,2,
        2,3,0
    };

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    //specify data format and position of an array of generic vertex attrib
//specifies layout of buffer
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));


    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));



    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    //we are moving color vector out of shader into cpp to provide more flexiblity to user
    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);





    //cout << "Vertex" << endl;
    //cout << source.VertexSource << endl;
    //cout << "Fragment" << endl;
    //cout << source.FragmentSource << endl;

    //    unsigned int shader = CreateShader(vertexShader,fragmentShader);
    //    glUseProgram(shader);









    float r = 0.0f;
    float increment = 0.05f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        //        glBegin(GL_TRIANGLES);
        //        glVertex2f(-0.5f,-0.5f);
        //        glVertex2f(0.0f, 0.5f);
         //      glVertex2f(0.5f, -0.5f);


        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));//they are per draw
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f) {
            increment = -0.05f;
        }
        else if (r < 0.0f) {
            increment = +0.05f;
        }
        r += increment;



        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }

    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}