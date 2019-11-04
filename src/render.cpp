#include "config.hpp"
#include <cstdio>
#include "glad.h"
#include "glfw3.h"
#include "glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "ogl_error.hpp"
#include "rect.hpp"
#include "rect_gfx.hpp"
#include "render.hpp"
#include "texture.hpp"



//
//  PRIVATE FUNCTION DECLARATIONS
//
///////////////////////////////////////////////////////////

static unsigned int createShader( const char* vertex_shader_code, const char* fragment_shader_code );
static unsigned int compileShader( unsigned int type, const char* source );
static const char* getShaderTypeText( unsigned int type );
static void render_init_palette();



//
//  PRIVATE VARIABLES
//
///////////////////////////////////////////////////////////

const char* rect_vertex_shader_code =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) in vec4 position;\n"
    "\n"
    "uniform mat4 u_MVP;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = u_MVP * position;\n"
    "}";

const char* rect_fragment_shader_code =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "uniform vec4 u_Color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   color = u_Color;\n"
    "}";

const char* sprite_vertex_shader_code =
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

const char* sprite_fragment_shader_code =
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
static GLFWwindow* window;
static unsigned int rect_shader;
static unsigned int sprite_shader;
static unsigned int palette_id;
static glm::mat4 position_matrix;
static Color background_color = { 0.0f, 0.85f, 1.0f, 1.0f };
static Rect canvas = { 0.0f, 0.0f, CONFIG_WINDOW_WIDTH_PIXELS, CONFIG_WINDOW_HEIGHT_PIXELS };
static RectGFX background;


//
//  PUBLIC FUNCTIONS
//
///////////////////////////////////////////////////////////

void render_texture( const Texture& texture )
{
    glUseProgram( sprite_shader );
    
    //glBindTexture( GL_TEXTURE_2D, 1 );

    int texture_uniform_location = glGetUniformLocation( sprite_shader, "u_Texture" );
    assert( texture_uniform_location != -1 );
    glUniform1i( texture_uniform_location, 1 );

    //glBindTexture( GL_TEXTURE_2D, palette_id );
    int palette_uniform_location = glGetUniformLocation( sprite_shader, "u_Palette" );
    assert( palette_uniform_location != -1 );
    glUniform1i( palette_uniform_location, 0 );

    int palette_index_uniform_location = glGetUniformLocation( sprite_shader, "u_PaletteIndex" );
    assert( palette_index_uniform_location != -1 );
    glUniform1f( palette_index_uniform_location, ( 1.0f / 255.0f ) * 8.0f * ( float )( 0 ) );

    //glBindTexture( GL_TEXTURE_2D, texture.id );

    ogl_call( glBindVertexArray( texture.vao ) );
    ogl_call( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr ) );
}

void render_rect( const RectGFX& rect_gfx )
{
    glUseProgram( rect_shader );
    int uniform_location = glGetUniformLocation( rect_shader, "u_Color" );
    dassert( uniform_location != -1 );
    ogl_call( glUniform4f( uniform_location, rect_gfx.color.r, rect_gfx.color.g, rect_gfx.color.b, rect_gfx.color.a ) );

    ogl_call( glBindVertexArray( rect_gfx.vao ) );
    ogl_call( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr ) );
}

bool render_init_window()
{
    window = glfwCreateWindow( CONFIG_WINDOW_WIDTH_PIXELS, CONFIG_WINDOW_HEIGHT_PIXELS, "Hello World", NULL, NULL );
    if ( !window )
    {
        printf( "Failed to initialize GLFW window.\n" );
        return false;
    }
    glfwMakeContextCurrent( window );
    return true;
}

int render_window_closed()
{
    return glfwWindowShouldClose( window );
}

void render_init_gfx()
{
    ogl_call( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
    ogl_call( glEnable( GL_BLEND ) );
    ogl_call( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
    rect_shader = createShader( rect_vertex_shader_code, rect_fragment_shader_code );
    sprite_shader = createShader( sprite_vertex_shader_code, sprite_fragment_shader_code );
    ogl_call( glUseProgram( rect_shader ) );

    background = rect_gfx_create( canvas, background_color );

    position_matrix = glm::ortho( 0.0f, 1.0f * CONFIG_WINDOW_WIDTH_PIXELS, 1.0f * CONFIG_WINDOW_HEIGHT_PIXELS, 0.0f, -1.0f, 1.0f );

    ogl_call( glUseProgram( rect_shader ) );
    int position_matrix_uniform_location = glGetUniformLocation( rect_shader, "u_MVP" );
    assert( position_matrix_uniform_location != -1 );
    glUniformMatrix4fv( position_matrix_uniform_location, 1, GL_FALSE, &position_matrix[ 0 ][ 0 ] );

    ogl_call( glUseProgram( sprite_shader ) );
    position_matrix_uniform_location = glGetUniformLocation( sprite_shader, "u_MVP" );
    assert( position_matrix_uniform_location != -1 );
    glUniformMatrix4fv( position_matrix_uniform_location, 1, GL_FALSE, &position_matrix[ 0 ][ 0 ] );

    render_init_palette();
};

void render_present()
{
    ogl_call( glfwSwapBuffers( window ) );
}

void render_start()
{
    ogl_call( glClear( GL_COLOR_BUFFER_BIT ) );
    render_rect( background );
}



//
//  PRIVATE FUNCTIONS
//
///////////////////////////////////////////////////////////

static unsigned int createShader( const char* vertex_shader_code, const char* fragment_shader_code )
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

static unsigned int compileShader( unsigned int type, const char* source )
{
    unsigned int id = glCreateShader( type );
    glShaderSource( id, 1, &source, nullptr );
    glCompileShader( id );

    int result;
    glGetShaderiv( id, GL_COMPILE_STATUS, &result );
    if ( result == GL_FALSE )
    {
        int message_length;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &message_length );
        char* message = ( char* )( alloca( message_length * sizeof( char ) ) );
        glGetShaderInfoLog( id, message_length, &message_length, message );
        printf( "Failed to compile %s shader: %s\n", getShaderTypeText( type ), message );
        glDeleteShader( id );
        return 0;
    }

    return id;
};

static const char* getShaderTypeText( unsigned int type )
{
    return ( type == GL_VERTEX_SHADER )
        ? "vertex"
        : "fragment";
};

static void render_init_palette()
{
    ogl_call( glUseProgram( sprite_shader ) );
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
}