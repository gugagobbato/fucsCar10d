#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "opengl_utils.cpp"
#include "stb_image.h"

#define JPG 0
#define PNG 1

GLint LARGURA = 1080;
GLint ALTURA = 720;

bool lbutton_down = false;

std::string TITULO = "Projeto 2 - Carro em Movimento";

std::string SHADER = "../shaders/SynthwaveSunset.glsl";
//std::string SHADER = "../shaders/tutorial.glsl";

GLint u_time;
GLint u_resolution;
GLint u_mouse;

static void loadTexture(const std::string& path, unsigned int slot, int tipo)
{
    std::string m_FilePath;
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_BPP;

    //stbi_set_flip_vertically_on_load(1);
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    // falta carga

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Corrige o alinhamento da imagem em imagens cujas dimensões não são potências de dois
    // NPOT (Not Power-of-Two)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(tipo == PNG)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_LocalBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);

    glCall(glActiveTexture(GL_TEXTURE0 + slot));

    if (m_LocalBuffer)
    {
        stbi_image_free(m_LocalBuffer);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        std::cout << "Pressionado ESC : Saindo da aplicação!" << std::endl;
    }
}

void showMousePosition(GLFWwindow * window, double x, double y)
{
    std::stringstream ss;
    ss << TITULO << " X: " << x/LARGURA << " Y: " << 1.0 - y/ALTURA;
    glfwSetWindowTitle(window, ss.str().c_str());
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    showMousePosition(window,xpos,ypos);
    auto MOUSE_X = (float) xpos;
    auto MOUSE_Y = (float) ALTURA - (float) ypos;
    if(lbutton_down) {
        glUniform4f(u_mouse, MOUSE_X, MOUSE_Y, MOUSE_X, MOUSE_Y);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            lbutton_down = true;
        } else if (action == GLFW_RELEASE) {
            lbutton_down = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { // reseta última posição registrada
        glUniform4f(u_mouse, 0.0, 0.0, 0.0, 0.0);
    }

}

void window_size_callback(GLFWwindow* window, GLint width, GLint height)
{
    LARGURA = width;
    ALTURA = height;
    glUniform3f(u_resolution, (GLfloat) LARGURA, (GLfloat) ALTURA, 0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int main() {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(LARGURA, ALTURA, TITULO.c_str(), nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Set keyboard events function */
    glfwSetKeyCallback(window, key_callback);

    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    //glfwSetWindowSizeCallback(window, window_size_callback);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);


    if(glewInit()!=GLEW_OK) {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
    } else {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }
    /*
    float positions[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,// 0
         1.0f, -1.0f, 1.0f, 0.0f,// 1
         1.0f,  1.0f, 1.0f, 1.0f,// 2
        -1.0f,  1.0f, 0.0f, 1.0f// 3
    };
    */

    float positions[] = {
        -1.0f, -1.0f, // 0
         1.0f, -1.0f, // 1
         1.0f,  1.0f, // 2
        -1.0f,  1.0f // 3
    };


    unsigned int indices[] = {
      0, 1, 2,
      2, 3, 0
    };

    // Have to set VAO before binding attrbutes
	unsigned int vao;
	glCall( glGenVertexArrays(1, &vao) );
	glCall( glBindVertexArray(vao) );

    // Create buffer and copy data
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), positions, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    // define vertex layout
    // This links the attrib pointer wih the buffer at index 0 in the vertex array object
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Create index buffer
    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


    std::string vertexShader = parseShader("../shaders/vertex.glsl");
    //std::cout << "Vertex Shader : \n\n" << vertexShader << std::endl;
    std::string fragmentShader = parseShader(SHADER);
    //std::cout << "Fragment Shader : \n\n" << fragmentShader << std::endl;

    unsigned int shader = createShaders(vertexShader, fragmentShader);

    // Limpa a memória VRAM
    glCall( glUseProgram(0) );
    glCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    glCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
    glCall( glBindVertexArray(0) );

    // Carrega a textura
//    loadTexture("../res/images/gremio.png", 1, PNG); // SLOT 0
    loadTexture("../res/images/ucs.png", 1 ,PNG); //SLOT 1.

    // Invoca o shader
    glCall(glUseProgram(shader));

    // localiza os uniforms
    u_time = glGetUniformLocation(shader, "iTime");
    u_resolution = glGetUniformLocation(shader, "iResolution");
    u_mouse = glGetUniformLocation(shader, "iMouse");

    // Instead of binding vertex buffer, attrib pointer, just bind Vertex Array Object
    glCall( glBindVertexArray(vao) );
    // Bind index buffer
    glCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo) );

//    glCall( glEnable(GL_BLEND) );
//    glCall( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //glCall(glDrawArrays(GL_TRIANGLES,0,3));
        //glDrawArrays(GL_TRIANGLES,0,3);
         glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        glUniform3f(u_resolution, (GLfloat) LARGURA, (GLfloat) ALTURA, 0);
        glUniform1f(u_time, (float) glfwGetTime());


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Cleanup VBO
	glCall( glDeleteBuffers(1, &buffer) );
	glCall( glDeleteVertexArrays(1, &vao) );
	glCall( glDeleteProgram(shader) );
    glfwTerminate();
    return 0;
}
