#include "glad.h"
#include "glfw3.h"
#include <cassert>
#include <cstdio>
#include <string>
#include "stb_image.h"

static std::string getShaderTypeText( unsigned int type )
{
    return ( type == GL_VERTEX_SHADER )
        ? "vertex"
        : "fragment";
};

static unsigned int compileShader( unsigned int type, const std::string& source )
{
    unsigned int id = glCreateShader( type );
    const char* src = source.c_str();
    glShaderSource( id, 1, &src, nullptr );
    glCompileShader( id );

    int result;
    glGetShaderiv( id, GL_COMPILE_STATUS, &result );
    if ( result == GL_FALSE )
    {
        int message_length;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &message_length );
        char* message = ( char* )( alloca( message_length * sizeof( char ) ) );
        glGetShaderInfoLog( id, message_length, &message_length, message );
        printf( "Failed to compile %s shader: %s\n", getShaderTypeText( type ).c_str(), message );
        glDeleteShader( id );
        return 0;
    }

    return id;
};

static unsigned int createShader( const std::string& vertex_shader_code, const std::string& fragment_shader_code )
{
    unsigned int program = glCreateProgram();
    unsigned int vertex_shader = compileShader( GL_VERTEX_SHADER, vertex_shader_code );
    unsigned int fragment_shader = compileShader( GL_FRAGMENT_SHADER, fragment_shader_code );
    glAttachShader( program, vertex_shader );
    glAttachShader( program, fragment_shader );
    glLinkProgram( program );
    glValidateProgram( program );
    glDeleteShader( vertex_shader );
    glDeleteShader( fragment_shader );
    return program;
};

int main( int argc, char** argv )
{
    GLFWwindow* window;

    /* Initialize the library */
    if ( !glfwInit() )
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow( 640, 640, "Hello World", NULL, NULL );
    if ( !window )
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent( window );

    // Load all OpenGL functions using the glfw loader function
    if ( !gladLoadGLLoader( ( GLADloadproc )( glfwGetProcAddress ) ) )
    {
        printf( "Failed to initialize OpenGL context\n" );
        return -1;
    }

    printf( "%s\n", glGetString( GL_VERSION ) );
    printf( "%s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

    float vertex_positions[ 16 ] = {
        -0.5f, -0.75f, 0.0f, 0.0f, // Left Bottom
        0.5f, -0.75f, 1.0f, 0.0f, // Right Bottom
        0.5f, 0.75f, 1.0f, 1.0f, // Right Top
        -0.5f, 0.75f, 0.0f, 1.0f // Left Top
    };

    unsigned int vertex_indices[ 6 ] =
    {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int vertex_array_id;
    glGenVertexArrays( 1, &vertex_array_id );
    glBindVertexArray( vertex_array_id );

    unsigned int vertex_buffer;
    glGenBuffers( 1, &vertex_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
    glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW );\

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, 0 );

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, ( const void* )( sizeof( float ) * 2 ) );

    unsigned int ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( unsigned int ), vertex_indices, GL_STATIC_DRAW );

    std::string vertex_shader_code =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "\n"
        "out vec2 v_TexCoord;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = position;\n"
        "   v_TexCoord = texCoord;\n"
        "}";
    std::string fragment_shader_code =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) out vec4 color;\n"
        "\n"
        "in vec2 v_TexCoord;\n"
        "\n"
        "uniform sampler2D u_Texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   vec4 texColor = texture(u_Texture, v_TexCoord);\n"
        "   color = texColor;\n"
        "}";
    unsigned int shader = createShader( vertex_shader_code, fragment_shader_code );
    glUseProgram( shader );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    stbi_set_flip_vertically_on_load( 1 );
    int texture_width = 0;
    int texture_height = 0;
    int texture_bpp = 0;
    const char* texture_filename = "bin/autumn.png";
    unsigned char* texture_buffer = stbi_load( texture_filename, &texture_width, &texture_height, &texture_bpp, 4 );
    if ( texture_buffer == nullptr )
    {
        printf( "Error loading texture: %s\n", texture_filename );
    }
    unsigned int texture_id;
    glGenTextures( 1, &texture_id );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer );
    glBindTexture( GL_TEXTURE_2D, 0 );

    if ( texture_buffer )
    {
        stbi_image_free( texture_buffer );
    }

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    int texture_uniform_location = glGetUniformLocation( shader, "u_Texture" );
    assert( texture_uniform_location != -1 );
    glUniform1i( texture_uniform_location, 0 );

    /* Loop until the user closes the window */
    while ( !glfwWindowShouldClose( window ) )
    {
        /* Render here */
        glClear( GL_COLOR_BUFFER_BIT );

        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr );

        /* Swap front and back buffers */
        glfwSwapBuffers( window );

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteTextures( 1, &texture_id );
    glfwTerminate();
    return 0;
}