#include "config.hpp"
#include <cstdio>
#include "glad.h"
#include "glfw3.h"
#include "glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "ogl_error.hpp"
#include "rect.hpp"
#include "render.hpp"
#include <cstring>

#include <unordered_map>


#define PALETTE_COLORS 256
#define CHANNELS_PER_COLOR 4
#define MAX_TEXTURES 50
#define MAX_FILENAME 255


//
//  PRIVATE FUNCTION DECLARATIONS
//
///////////////////////////////////////////////////////////

static unsigned int createShader( const char* vertex_shader_code, const char* fragment_shader_code );
static unsigned int compileShader( unsigned int type, const char* source );
static const char* getShaderTypeText( unsigned int type );
static void render_init_texture_buffer();
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
    "uniform float u_Alpha;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   vec4 texColor = texture2D(u_Texture, v_TexCoord);\n"
    "   vec2 index = vec2( texColor.r + u_PaletteIndex, 0 );\n"
    "   vec4 indexedColor = texture2D( u_Palette, index );\n"
    "   indexedColor.a *= u_Alpha;\n"
    "   color = indexedColor;\n"
    "}";

static float palette_colors[ PALETTE_COLORS ][ CHANNELS_PER_COLOR ] = {};
static float background_color[ CHANNELS_PER_COLOR ] = { 0.0f, 0.5f, 1.0f, 1.0f };

static float vertex_positions[ 16 ] = {
    0.0f, 0.0f, 0.0f, 1.0f,// Left Bottom
    1.0f, 0.0f, 1.0f, 1.0f, // Right Bottom
    1.0f, 1.0f, 1.0f, 0.0f, // Right Top
    0.0f, 1.0f, 0.0f, 0.0f // Left Top
};

static unsigned int vertex_indices[ 6 ] =
{
    0, 1, 2,
    2, 3, 0
};

static GLFWwindow* window;
static unsigned int rect_shader;
static unsigned int sprite_shader;
static unsigned int palette_id;
static glm::mat4 projection_matrix;
static Rect canvas = { 0.0f, 0.0f, CONFIG_WINDOW_WIDTH_PIXELS, CONFIG_WINDOW_HEIGHT_PIXELS };

static unsigned int texture_vao;
static unsigned int rect_vao;
static int sprite_mvp_uniform_location;
static int palette_index_uniform_location;
static int sprite_alpha_uniform_location;
static int rect_color_uniform_location;
static int rect_mvp_uniform_location;

struct TextureData
{
    unsigned char* buffer;
    int width;
    int height;
};

static std::unordered_map<std::string, int> texture_map;
static TextureData textures[ MAX_TEXTURES ];
static Texture number_of_textures = 0;

//
//  PUBLIC FUNCTIONS
//
///////////////////////////////////////////////////////////

void render_texture( Texture texture, const Rect& src, const Rect& dest, int palette, bool flip_x, bool flip_y, float alpha )
{
    glUseProgram( sprite_shader );
    glm::mat4 view_matrix = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ) );
    glm::mat4 model_matrix = glm::scale( glm::translate( glm::mat4( 1.0f ), glm::vec3( dest.x, dest.y, 0.0f ) ), glm::vec3( dest.w * ( ( flip_x ) ? -1.0f : 1.0f ), dest.h * ( ( flip_y ) ? -1.0f : 1.0f ), 0.0f ) );
    glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;
    glUniformMatrix4fv( sprite_mvp_uniform_location, 1, GL_FALSE, &mvp[ 0 ][ 0 ] );
    glUniform1f( palette_index_uniform_location, ( 1.0f / 255.0f ) * 8.0f * ( float )( palette ) );
    glUniform1f( sprite_alpha_uniform_location, alpha );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, textures[ texture ].width, textures[ texture ].height, 0, GL_RED, GL_UNSIGNED_BYTE, textures[ texture ].buffer );
    glBindTexture( GL_TEXTURE_2D, 1 );
    ogl_call( glBindVertexArray( texture_vao ) );
    ogl_call( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr ) );
}

void render_rect( const Rect& rect, int color )
{
    glUseProgram( rect_shader );
    glm::mat4 view_matrix = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ) );
    glm::mat4 model_matrix = glm::scale( glm::translate( glm::mat4( 1.0f ), glm::vec3( rect.x, rect.y, 0.0f ) ), glm::vec3( rect.w, rect.h, 0.0f ) );
    glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;
    glUniformMatrix4fv( rect_mvp_uniform_location, 1, GL_FALSE, &mvp[ 0 ][ 0 ] );
    if ( color == 0 ) // If 0, color in background ’stead.
    {
        ogl_call( glUniform4f( rect_color_uniform_location, background_color[ 0 ], background_color[ 1 ], background_color[ 2 ], background_color[ 3 ] ) );
    }
    else
    {
        ogl_call( glUniform4f( rect_color_uniform_location, palette_colors[ color ][ 0 ], palette_colors[ color ][ 1 ], palette_colors[ color ][ 2 ], palette_colors[ color ][ 3 ] ) );
    }
    ogl_call( glBindVertexArray( rect_vao ) );
    ogl_call( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr ) );
}

Texture render_get_texture( const char* name )
{
    char full_filename[ MAX_FILENAME + 9 ] = "bin/";
    strcat( full_filename, name );
    strcat( full_filename, ".jwi" );

    if ( number_of_textures == MAX_TEXTURES )
    {
        printf( "Not ’nough room for any mo’ textures." );
        return -1;
    }

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
    gfx_file = fopen( full_filename, "rb" );
    if ( !gfx_file )
    {
        printf( "File didn’t load: %s\n", full_filename );
        return -1;
    }
    else
    {
        fseek( gfx_file, 0, SEEK_END );
        file_size = ftell( gfx_file );
        rewind( gfx_file );
        file_buffer = ( unsigned char* )( malloc( sizeof( unsigned char ) * file_size ) );
        if ( !file_buffer )
        {
            printf( "Somehow run out o’ memory for loading file %s\n", full_filename );
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

        textures[ number_of_textures ] =
        {
            texture_buffer,
            texture_width,
            texture_height
        };
        ++number_of_textures;

        fclose( gfx_file );
        free( file_buffer );
        return number_of_textures - 1;
    }
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
    projection_matrix = glm::ortho( 0.0f, 1.0f * CONFIG_WINDOW_WIDTH_PIXELS, 1.0f * CONFIG_WINDOW_HEIGHT_PIXELS, 0.0f, -1.0f, 1.0f );

    rect_shader = createShader( rect_vertex_shader_code, rect_fragment_shader_code );
    ogl_call( glUseProgram( rect_shader ) );
    rect_color_uniform_location = glGetUniformLocation( rect_shader, "u_Color" );
    dassert( rect_color_uniform_location != -1 );
    rect_mvp_uniform_location = glGetUniformLocation( rect_shader, "u_MVP" );
    assert( rect_mvp_uniform_location != -1 );

    ogl_call( glGenVertexArrays( 1, &rect_vao ) );
    ogl_call( glBindVertexArray( rect_vao ) );

    unsigned int buffer;
    ogl_call( glGenBuffers( 1, &buffer ) );
    ogl_call( glBindBuffer( GL_ARRAY_BUFFER, buffer ) );
    ogl_call( glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW ) );

    ogl_call( glEnableVertexAttribArray( 0 ) );
    ogl_call( glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, nullptr ) );

    unsigned int ibo;
    ogl_call( glGenBuffers( 1, &ibo ) );
    ogl_call( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ) );
    ogl_call( glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( unsigned int ), vertex_indices, GL_STATIC_DRAW ) );

    sprite_shader = createShader( sprite_vertex_shader_code, sprite_fragment_shader_code );
    ogl_call( glUseProgram( sprite_shader ) );
    int texture_uniform_location = glGetUniformLocation( sprite_shader, "u_Texture" );
    assert( texture_uniform_location != -1 );
    glUniform1i( texture_uniform_location, 1 );
    sprite_alpha_uniform_location = glGetUniformLocation( sprite_shader, "u_Alpha" );
    assert( sprite_alpha_uniform_location != -1 );
    int palette_uniform_location = glGetUniformLocation( sprite_shader, "u_Palette" );
    assert( palette_uniform_location != -1 );
    glUniform1i( palette_uniform_location, 0 );
    int sprite_mvp_uniform_location = glGetUniformLocation( sprite_shader, "u_MVP" );
    assert( sprite_mvp_uniform_location != -1 );
    palette_index_uniform_location = glGetUniformLocation( sprite_shader, "u_PaletteIndex" );
    assert( palette_index_uniform_location != -1 );

    render_init_texture_buffer();
    render_init_palette();
};

void render_present()
{
    ogl_call( glfwSwapBuffers( window ) );
}

void render_start()
{
    ogl_call( glClear( GL_COLOR_BUFFER_BIT ) );
    render_rect( canvas, 0 );
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

static void render_init_texture_buffer()
{
    glGenVertexArrays( 1, &texture_vao );
    glBindVertexArray( texture_vao );

    unsigned int buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW );\

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, nullptr );

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, ( const void* )( sizeof( float ) * 2 ) );

    unsigned int ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( unsigned int ), vertex_indices, GL_STATIC_DRAW );
}

static void render_init_palette()
{
    unsigned char palette_buffer[ PALETTE_COLORS * CHANNELS_PER_COLOR ] =
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
        255,255,255,255,
        226, 184, 255, 255,
        154,  96, 246, 255,
         81,  34, 177, 255,
          9,   0,  38, 255,
        0, 0, 0, 255,
        0, 0, 0, 0,

        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
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
    ogl_call( glUseProgram( sprite_shader ) );
    int palette_width = 256;
    int palette_height = 1;
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

    // Set palette colors based on palette: unsigned byte -> float
    for ( int color = 0; color < PALETTE_COLORS; ++color )
    {
        for ( int channel = 0; channel < CHANNELS_PER_COLOR; ++channel )
        {
            int buffer_index = color * CHANNELS_PER_COLOR + channel;
            palette_colors[ color ][ channel ] = ( float )( palette_buffer[ buffer_index ] ) / 255.0f;
        }
    }
}