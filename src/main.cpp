#include "glad.h"
#include "glfw3.h"
#include <cassert>
#include "config.h"
#include <cstdio>
#include <cstring>
#include <string>
#include "stb_image.h"

#include "glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"

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
    window = glfwCreateWindow( CONFIG_WINDOW_WIDTH_PIXELS, CONFIG_WINDOW_HEIGHT_PIXELS, "Hello World", NULL, NULL );
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
    
    glClearColor( 0.0f, 0.85f, 1.0f, 1.0f );

    float vertex_positions[ 16 ] = {
        0.0f, 0.0f, 0.0f, 1.0f, // Left Bottom
        16.0f, 0.0f, 1.0f, 1.0f, // Right Bottom
        16.0f, 25.0f, 1.0f, 0.0f, // Right Top
        0.0f, 25.0f, 0.0f, 0.0f // Left Top
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
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, nullptr );

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
        "uniform mat4 u_MVP;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = u_MVP * position;\n"
        "   v_TexCoord = texCoord;\n"
        "}";
    std::string fragment_shader_code =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) out vec4 color;\n"
        "\n"
        "in vec2 v_TexCoord;\n"
        "\n"
        "uniform sampler2D u_Palette;\n"
        "uniform sampler2D u_Texture;\n"
        "uniform float u_PaletteIndex;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   vec4 texColor = texture2D(u_Texture, v_TexCoord);\n"
        "   vec2 index = vec2( texColor.r + u_PaletteIndex, 0 );\n"
        "   vec4 indexedColor = texture2D( u_Palette, index );\n"
        "   color = indexedColor;\n"
        "}";
    unsigned int shader = createShader( vertex_shader_code, fragment_shader_code );
    glUseProgram( shader );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    stbi_set_flip_vertically_on_load( 1 );
    int palette_width = 256;
    int palette_height = 1;
    unsigned char palette_buffer[ 256 * 4 ] =
    {
        0, 0, 0, 0,
        0, 0, 0, 255,
        248, 56, 8, 255,
        248, 152, 80, 255,
        112, 64, 24, 255,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,

        0, 0, 0, 0,
        0, 0, 0, 255,
        120, 80, 24, 255,
        248, 152, 80, 255,
        255, 255, 255, 255,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,

        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };
    unsigned int palette_id;
    glGenTextures( 0, &palette_id );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, palette_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, palette_width, palette_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, palette_buffer );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glGenTextures( 0, &palette_id );
    glBindTexture( GL_TEXTURE_2D, palette_id );
    int palette_uniform_location = glGetUniformLocation( shader, "u_Palette" );
    assert( palette_uniform_location != -1 );
    glUniform1i( palette_uniform_location, 0 );
    int palette_index_uniform_location = glGetUniformLocation( shader, "u_PaletteIndex" );
    assert( palette_index_uniform_location != -1 );
    unsigned int palette = 0;



    unsigned int texture_id;
    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    FILE* gfx_file;
    long file_size;
    unsigned char* file_buffer;
    size_t fread_flag;
    gfx_file = fopen( "bin/autumn.jwi", "rb" );
    if ( !gfx_file )
    {
        printf( "File didn’t load: %s\n", "bin/autumn.jwi" );
    }
    else
    {
        fseek( gfx_file, 0, SEEK_END );
        file_size = ftell( gfx_file );
        rewind( gfx_file );
        file_buffer = ( unsigned char* )( malloc( sizeof( unsigned char ) * file_size ) );
        if ( !file_buffer )
        {
            printf( "Somehow run out o’ memory for loading file %s\n", "bin/autumn.jwi" );
        }
        fread_flag = fread( file_buffer, 1, file_size, gfx_file );
        if ( ( long )( fread_flag ) != file_size )
        {
            fputs( "Reading error", stderr );
        }

        int texture_width = ( ( unsigned int )( file_buffer[ 0 ] ) << 8 ) | file_buffer[ 1 ];
        int texture_height = ( ( unsigned int )( file_buffer[ 2 ] ) << 8 ) | file_buffer[ 3 ];
        unsigned char* texture_buffer = nullptr;

        const size_t image_data_size = file_size - 4;

        if ( image_data_size != ( size_t )( texture_width * texture_height ) )
        {
            printf( "GFX Load Error: File data doesn’t match width & height given!\n" );
        }
        else
        {
            texture_buffer = ( unsigned char* )( calloc( image_data_size, sizeof( unsigned char ) ) );
            memcpy( ( void* )( texture_buffer ), ( const void* )( &file_buffer[ 4 ] ), image_data_size );
        }

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer );
        glBindTexture( GL_TEXTURE_2D, 1 );
        int texture_uniform_location = glGetUniformLocation( shader, "u_Texture" );
        assert( texture_uniform_location != -1 );
        glUniform1i( texture_uniform_location, 1 );

        fclose( gfx_file );
        free( file_buffer );
    }

    glm::mat4 position_matrix = glm::ortho( 0.0f, 1.0f * CONFIG_WINDOW_WIDTH_PIXELS, 1.0f * CONFIG_WINDOW_HEIGHT_PIXELS, 0.0f, -1.0f, 1.0f );
    int position_matrix_uniform_location = glGetUniformLocation( shader, "u_MVP" );
    assert( position_matrix_uniform_location != -1 );
    glUniformMatrix4fv( position_matrix_uniform_location, 1, GL_FALSE, &position_matrix[ 0 ][ 0 ] );

    /* Loop until the user closes the window */
    while ( !glfwWindowShouldClose( window ) )
    {
        glUniform1f( palette_index_uniform_location, ( 1.0f / 255.0f ) * 8.0f * ( float )( palette ) );
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