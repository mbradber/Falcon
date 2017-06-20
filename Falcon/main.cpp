#define GLFW_INCLUDE_GLCOREARB

#include "gl_utils.h"		// utility functions discussed in earlier tutorials
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#define MESH_FILE "mesh/cube.dae"
#define MESH_FILE "mesh/monkey2.obj"

#define GL_LOG_FILE "gl.log"

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow *g_window = NULL;

/* load a mesh using the assimp library */
bool load_mesh( const char *file_name, GLuint *vao, int *point_count ) {
    const aiScene *scene = aiImportFile( file_name, aiProcess_Triangulate );
    if ( !scene ) {
        fprintf( stderr, "ERROR: reading mesh %s\n", file_name );
        return false;
    }
    printf( "  %i animations\n", scene->mNumAnimations );
    printf( "  %i cameras\n", scene->mNumCameras );
    printf( "  %i lights\n", scene->mNumLights );
    printf( "  %i materials\n", scene->mNumMaterials );
    printf( "  %i meshes\n", scene->mNumMeshes );
    printf( "  %i textures\n", scene->mNumTextures );
    
    /* get first mesh in file only */
    const aiMesh *mesh = scene->mMeshes[0];
    printf( "    %i vertices in mesh[0]\n", mesh->mNumVertices );
    
    /* pass back number of vertex points in mesh */
    *point_count = mesh->mNumVertices;
    
    /* generate a VAO, using the pass-by-reference parameter that we give to the
     function */
    glGenVertexArrays( 1, vao );
    glBindVertexArray( *vao );
    
    /* we really need to copy out all the data from AssImp's funny little data
     structures into pure contiguous arrays before we copy it into data buffers
     because assimp's texture coordinates are not really contiguous in memory.
     i allocate some dynamic memory to do this. */
    GLfloat *points = NULL;		 // array of vertex points
    GLfloat *normals = NULL;	 // array of vertex normals
    GLfloat *texcoords = NULL; // array of texture coordinates
    if ( mesh->HasPositions() ) {
        points = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
        for ( int i = 0; i < *point_count; i++ ) {
            const aiVector3D *vp = &( mesh->mVertices[i] );
            points[i * 3] = (GLfloat)vp->x;
            points[i * 3 + 1] = (GLfloat)vp->y;
            points[i * 3 + 2] = (GLfloat)vp->z;
        }
    }
    if ( mesh->HasNormals() ) {
        normals = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
        for ( int i = 0; i < *point_count; i++ ) {
            const aiVector3D *vn = &( mesh->mNormals[i] );
            normals[i * 3] = (GLfloat)vn->x;
            normals[i * 3 + 1] = (GLfloat)vn->y;
            normals[i * 3 + 2] = (GLfloat)vn->z;
        }
    }
    if ( mesh->HasTextureCoords( 0 ) ) {
        texcoords = (GLfloat *)malloc( *point_count * 2 * sizeof( GLfloat ) );
        for ( int i = 0; i < *point_count; i++ ) {
            const aiVector3D *vt = &( mesh->mTextureCoords[0][i] );
            texcoords[i * 2] = (GLfloat)vt->x;
            texcoords[i * 2 + 1] = (GLfloat)vt->y;
        }
    }
    
    /* copy mesh data into VBOs */
    if ( mesh->HasPositions() ) {
        GLuint vbo;
        glGenBuffers( 1, &vbo );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), points,
                     GL_STATIC_DRAW );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
        glEnableVertexAttribArray( 0 );
        free( points );
    }
    if ( mesh->HasNormals() ) {
        GLuint vbo;
        glGenBuffers( 1, &vbo );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), normals,
                     GL_STATIC_DRAW );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
        glEnableVertexAttribArray( 1 );
        free( normals );
    }
    if ( mesh->HasTextureCoords( 0 ) ) {
        GLuint vbo;
        glGenBuffers( 1, &vbo );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, 2 * *point_count * sizeof( GLfloat ), texcoords,
                     GL_STATIC_DRAW );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, NULL );
        glEnableVertexAttribArray( 2 );
        free( texcoords );
    }
    if ( mesh->HasTangentsAndBitangents() ) {
        // NB: could store/print tangents here
    }
    
    aiReleaseImport( scene );
    printf( "mesh loaded\n" );
    
    return true;
}

int main() {
    restart_gl_log();
    start_gl();
    glEnable( GL_DEPTH_TEST ); // enable depth-testing
    glDepthFunc( GL_LESS );		 // depth-testing interprets a smaller value as "closer"
    glEnable( GL_CULL_FACE );	// cull face
    glCullFace( GL_BACK );		 // cull back face
    glFrontFace( GL_CCW ); // set counter-clock-wise vertex order to mean the front
    glClearColor( 67 / 255.f, 180 / 255.f, 211 / 255.f, 1.f);
    glViewport( 0, 0, g_gl_width, g_gl_height );
    
   	/* load the mesh using assimp */
    GLuint monkey_vao;
    int monkey_point_count = 0;
    ( load_mesh( MESH_FILE, &monkey_vao, &monkey_point_count ) );
    
   	/*-------------------------------CREATE
     * SHADERS-------------------------------*/
    GLuint shader_programme = create_programme_from_files( "shaders/test_vs.glsl", "shaders/test_fs.glsl" );
    
    // setup matrices / uniforms
    float cam_speed = 1.0f;
    float cam_yaw_speed = 100.0f;
    float cam_yaw = 0.0f;
    glm::vec3 cam_pos(0.0f, 0.0f, 2.0f);
    glm::mat4 mat_model = glm::rotate(glm::mat4(1.f), glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
    mat_model = glm::mat4(1.f);
    glm::mat4 mat_view = glm::translate(glm::mat4(1.0f), glm::vec3(-cam_pos.x, -cam_pos.y, -cam_pos.z));
    glm::mat4 mat_projection = glm::perspective(glm::radians(67.0f), float(g_gl_width / g_gl_height), 0.1f, 100.f);
    
    int mat_loc_model = glGetUniformLocation( shader_programme, "mat_model" );
    int mat_loc_view = glGetUniformLocation( shader_programme, "mat_view" );
    int mat_loc_projection = glGetUniformLocation( shader_programme, "mat_projection" );
    
    if(mat_loc_model < 0 || mat_loc_view < 0 || mat_loc_projection < 0) {
        fprintf(stderr, "Error: could not find locations for all uniforms!\n");
        return 0;
    }
    
    glUseProgram( shader_programme );
    glUniformMatrix4fv( mat_loc_model, 1, GL_FALSE, (const float*)glm::value_ptr(mat_model) );
    glUniformMatrix4fv( mat_loc_view, 1, GL_FALSE, (const float*)glm::value_ptr(mat_view) );
    glUniformMatrix4fv( mat_loc_projection, 1, GL_FALSE, (const float*)glm::value_ptr(mat_projection) );
    
    // render loop
    while ( !glfwWindowShouldClose( g_window ) ) {
        // add a timer for doing animation
        static double previous_seconds = glfwGetTime();
        double current_seconds = glfwGetTime();
        double elapsed_seconds = current_seconds - previous_seconds;
        previous_seconds = current_seconds;
        
        _update_fps_counter( g_window );
        // wipe the drawing surface clear
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glViewport( 0, 0, g_gl_width, g_gl_height );
        
        // Note: this call is not necessary, but I like to do it anyway before any
        // time that I call glDrawArrays() so I never use the wrong shader programme
        glUseProgram( shader_programme );
        
        // Note: this call is not necessary, but I like to do it anyway before any
        // time that I call glDrawArrays() so I never use the wrong vertex data
        glBindVertexArray( monkey_vao );
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays( GL_TRIANGLES, 0, monkey_point_count );
        // update other events like input handling
        glfwPollEvents();
        
        /*-----------------------------move camera
         * here-------------------------------*/
        // control keys
        bool cam_moved = false;
        
        if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
            cam_pos[2] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
            cam_pos[0] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
            cam_pos[2] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
            cam_pos[0] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
            cam_pos[1] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
            cam_pos[1] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
            cam_yaw += cam_yaw_speed * elapsed_seconds;
            cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
            cam_yaw -= cam_yaw_speed * elapsed_seconds;
            cam_moved = true;
        }
        /* update view matrix */
        if ( cam_moved ) {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-cam_pos.x, -cam_pos.y, -cam_pos.z));
            mat_view = glm::rotate(T, -glm::radians(cam_yaw), glm::vec3(0.0f, 1.0f, 0.0f));

            glUniformMatrix4fv( mat_loc_view, 1, GL_FALSE, (const float*)glm::value_ptr(mat_view) );
        }
        
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
            glfwSetWindowShouldClose( g_window, 1 );
        }
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers( g_window );
    }
    
    // close GL context and any other GLFW resources
    glfwTerminate();
    return 0;
}
