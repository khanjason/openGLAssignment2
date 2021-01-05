/**
 * Object Loader: Example class to demonstratre the use of the tiny_object_loader
 * to import an .obj file and create and display the object in OpenGL
 * This example uses the following:
 *	object_loader.cpp (this file)
 *	tiny_loader.cpp (uses tiny_obj_loader.cc/h to load and parse the .obj file)
 *				    (then creates the OpenGL VBOs from the tin_obj_loader attributes)
 *	sphere_tex.cpp (to display a seperate object in this example)
 */

//Jason Khan		
/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include our sphere and object loader classes

#include "tiny_loader_texture.h"
#include "cube_tex.h"
#include "skybox.h"
#include "terrain_object.h"
#include "particle_object.h"
#include "stb_image.h"
#include <particle_object.h>

//GLuint jellyprogram;		/* Identifier for the shader prgoram */
GLuint skyprogram;
GLuint particle_program;
GLuint programlist[11];

GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons

/* Uniforms*/


GLuint modelIDs[11];
GLuint viewIDs[11];
GLuint projectionIDs[11];
GLuint colourmodeIDs[11];
//GLuint texID;// , tex_matrixID;

GLuint skymodelID;
GLuint skyviewID;
GLuint skyprojectionID;
GLuint skyboxTexID;
GLuint 	skycolourmodeID;
GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLfloat bridgeset;
GLuint particlemodelID, particleviewID, particleprojectionID;
GLuint particlecolourmodeID;
//camera variables
GLfloat camerax = 0;
GLfloat cameray = 1;
GLfloat cameraup = 1;
GLfloat cameraright;
GLfloat camerax_inc;
GLfloat cameray_inc;
GLfloat cameraup_inc;
GLfloat cameraright_inc;
GLfloat angle_inc;
GLfloat bridge_angle;
//camera vectors
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
glm::vec3 cameraSide;


TinyObjLoader jelly;			// This is an instance of our basic object loaded
Skybox skycube;		// Create our sphere with no texture coordinates because they aren't handled in the shaders for this example
Cube candycane(true);
TinyObjLoader branch;
TinyObjLoader bridge;
TinyObjLoader tower[4];
GLuint texSky[6];
GLuint tentaTex;
TinyObjLoader walls[4];
particle_object particleObject;

//terrain vars
terrain_object* terrain;
GLfloat sealevel;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;
bool recreate_terrain;

//textures
GLuint textureIDs[7];
int loc[5];
int skyloc;
/* I don't like using namespaces in header files but have less issues with them in
   seperate cpp files */
using namespace std;
using namespace glm;
float cameraSpeed = 0.2f;

std::string vertexShader[] = { "../../Shaders/jelly.vert" ,"../../Shaders/terrain.vert" , "../../Shaders/branch.vert" ,"../../Shaders/tower.vert" ,"../../Shaders/object_loader.vert" ,"../../Shaders/object_loader.vert" ,"../../Shaders/object_loader.vert","../../Shaders/walls.vert","../../Shaders/skybox.vert" ,"../../Shaders/candy.vert","../../Shaders/wood.vert" };
std::string fragShader[] = {"../../Shaders/jelly.frag" ,"../../Shaders/terrain.frag" ,"../../Shaders/branch.frag","../../Shaders/tower.frag" ,"../../Shaders/object_loader.frag" ,"../../Shaders/object_loader.frag" ,"../../Shaders/object_loader.frag","../../Shaders/walls.frag" ,"../../Shaders/skybox.frag" ,"../../Shaders/candy.frag","../../Shaders/wood.frag" };
float pitchh;
bool bridgeDown;

std::string textureFiles[] = { "../../Images/spots.png","../../Images/spots.png" ,"../../Images/pinkjelly.png","../../Images/candycane.png"  , "../../Images/castle_wall.png","../../Images/tower_wall.png" ,"../../Images/wood_texture.png" };// "../../Images/back.png", "../../Images/front.png" };
int width, height, nrChannels;
/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/

//this was moved to skybox class
GLuint loadCubeMap(vector<std::string> faces, GLuint program) {
	unsigned int texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	int width, height, nrChannels;
	for (int i = 0; i < 6; i++) {
		//pos x , neg x, pos y , neg y, pos z, neg z
		//right,left,top,bottom,back,front
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			int pixel_format = 0;
			if (nrChannels == 3)
				pixel_format = GL_RGB;
			else
				pixel_format = GL_RGBA;
		
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 0, pixel_format, width, height, 0, pixel_format,
				GL_UNSIGNED_BYTE, data);
		
			stbi_image_free(data);
		}
		else
		{
			printf("stb_image loading error:");
			exit(0);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}


void loadTex(std::string fname, GLuint id, GLuint program) {

	int width, height, nrChannels;
	unsigned char* data = stbi_load(fname.c_str(), &width, &height, &nrChannels, 0);
	// check for an error during the load process
	if (data)
	{
		cout << "image found" << endl;
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;
		// Create the texture, passing in the pointer to the loaded image pixel data
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		printf("stb_image loading error:");
		exit(0);
	}
	int loc = glGetUniformLocation(program, "tex1");
	if (loc >= 0) glUniform1i(loc, 0);
}

void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	//camera vars
	camerax = 0;
	cameray = 1;
	cameraright = -90.0f;
	cameraup = 1;
	camerax_inc = cameray_inc = cameraright_inc = cameraup_inc = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	angle_inc = 1.0f;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;
	cameraPos = glm::vec3(2.0f, 1.0f, 0.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraSide = glm::vec3(cameraright, 0.0, 0.0);
	recreate_terrain = false;
	sealevel = 1.0f;
	pitchh = 0.0f;
	//drawbridge controls
	bridge_angle = 0.0f;
	bridgeDown = false;
	bridgeset = -1.3f;
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);
	//load in blender objs
	
	jelly.load_obj("..\\..\\objects\\jelly.obj");



	bridge.load_obj("..\\..\\objects\\drawbridge.obj");

	branch.load_obj("..\\..\\objects\\branch.obj");

	for (int x = 0; x < 4; x++) {
		tower[x].load_obj("..\\..\\objects\\tower.obj");

	}

	for (int x = 0; x < 4; x++) {
		walls[x].load_obj("..\\..\\objects\\castle_wall.obj");


	}

	/* override color if needed
	float skyColor[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,




	};*/
	//skycube.make();
	candycane.makeCube(NULL);
	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 50.f;
	land_resolution = 100;
	terrain = new terrain_object(octaves, perlin_frequency, perlin_scale);
	terrain->createTerrain(land_resolution, land_resolution, land_size, land_size,sealevel);
	terrain->setColour(vec3(0.0, 1.0, 0.0));
	terrain->createObject();
	/* Load and build the vertex and fragment shaders */
	try
	{
		for (int i = 0; i < 11; i++) {

			programlist[i]= glw->LoadShader(vertexShader[i].c_str(), fragShader[i].c_str());
		}
		
		skyprogram = glw->LoadShader("../../Shaders/skybox.vert", "../../Shaders/skybox.frag");
		//programlist[1] = glw->LoadShader("../../Shaders/terrain.vert", "../../Shaders/terrain.frag");
		particle_program = glw->LoadShader("../../shaders/particle_object.vert", "../../shaders/particle_object.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	for (int n = 0; n < 11; n++) {
		modelIDs[n]= glGetUniformLocation(programlist[n], "model");
		colourmodeIDs[n] = glGetUniformLocation(programlist[n], "colourmode");
		viewIDs[n] = glGetUniformLocation(programlist[n], "view");
		projectionIDs[n] = glGetUniformLocation(programlist[n], "projection");
	}

	particlemodelID = glGetUniformLocation(particle_program, "model");
	particleviewID = glGetUniformLocation(particle_program, "view");
	particlecolourmodeID = glGetUniformLocation(particle_program, "colourmode");
	particleprojectionID = glGetUniformLocation(particle_program, "projection");
	glGenTextures(7, textureIDs);


	//tex_matrixID = glGetUniformLocation(programlist[1], "tex_matrix");
	//loadTex("../../images/grass.png", tentaTex,programlist[2]);
	for (int i = 0; i < 7; i++) {
		std::string fname = textureFiles[i];
		unsigned char* data = stbi_load(fname.c_str(), &width, &height, &nrChannels, 0);
		// check for an error during the load process+		modelIDs	0x0035fc40 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	unsigned int[11]



		if (data)
		{
			cout << "image found" << endl;
			// Note: this is not a full check of all pixel format types, just the most common two!
			int pixel_format = 0;
			if (nrChannels == 3)
				pixel_format = GL_RGB;
			else
				pixel_format = GL_RGBA;
			// Create the texture, passing in the pointer to the loaded image pixel data
			glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format,
				GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else
		{
			printf("stb_image loading error:");
			exit(0);
		}
	}

	
	skycube.make();
	//prepare skybox
	GLuint programArray[] = {  programlist[2],programlist[0],programlist[9],programlist[7],programlist[3],programlist[10]};
	
	
	skymodelID = glGetUniformLocation(skyprogram, "model");
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		// Process/log the error.
		cout << err << endl;
	}
	//get samplers
	for (int t = 0; t < 5; t++) {
		
		loc[t] = glGetUniformLocation(programArray[t], "tex1");
		if (loc[t] >= 0) glUniform1i(loc[t], 0);
	}
	particleObject.create(particle_program);
	

	cout << "W - move forward" << endl;
	cout << "A - move left" << endl;
	cout << "S - move backward" << endl;
	cout << "D - move right" << endl;

	cout << "X - move upwards" << endl;
	cout << "Z - move downwards" << endl;

	cout << "I - camera down" << endl;
	cout << "K - camera up" << endl;
	cout << "L - camera right" << endl;
	cout << "J - camera left" << endl;

	cout << "P - move drawbridge" << endl;

}




/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);


	/* Make the compiled shader program current */
	glUseProgram(programlist[0]);
	//camera controls adapted from https://learnopengl.com/Getting-started/Camera
	
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	cameraSide.x = cos(glm::radians(cameraright));
	cameraSide.y = sin(glm::radians(pitchh));
	cameraSide.z = sin(glm::radians(cameraright)) * cos(glm::radians(pitchh));
	cameraFront = normalize(cameraSide);
	mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	// Camera matrix
	

	glUniform1ui(colourmodeIDs[0], colourmode);
	glUniformMatrix4fv(viewIDs[0], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[0], 1, GL_FALSE, &projection[0][0]);

	// Define the model transformations for the object
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x, y + sealevel, z));
	model = scale(model, vec3(scaler / 3.f, scaler / 3.f, scaler / 3.f));//scale equally in all axis

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelIDs[0], 1, GL_FALSE, &model[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[2]);
	/* Draw our Blender jelly object */
	jelly.drawObject(drawmode);

	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
	
	

	//tentacle branch

	glUseProgram(programlist[2]);

	glUniform1ui(colourmodeIDs[2], colourmode);
	glUniformMatrix4fv(viewIDs[2], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[2], 1, GL_FALSE, &projection[0][0]);
	mat4 model3 = mat4(1.0f);
	model3 = translate(model3, vec3(x+6.5, sealevel - 2.0f, z+5.0));
	glUniformMatrix4fv(modelIDs[2], 1, GL_FALSE, &model3[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	
	branch.drawObject(drawmode);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(programlist[9]);

	mat4 candymodel = mat4(1.0f);
	candymodel = translate(candymodel, vec3(2.0, 1.5, 4.0));
	candymodel = scale(candymodel, vec3(2.0, 2.0, 2.0));//scale equally in all axis
	candymodel = rotate(candymodel, radians(180.0f), vec3(0, 0, 1)); //rotating in clockwise direction around x-axis
	
	glUniformMatrix4fv(modelIDs[9], 1, GL_FALSE, &candymodel[0][0]);
	glUniformMatrix4fv(viewIDs[9], 1, GL_FALSE, &view[0][0]);

	glUniform1ui(colourmodeIDs[9], colourmode);
	glUniformMatrix4fv(projectionIDs[9], 1, GL_FALSE, &projection[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[3]);

	//Draw our candy
	candycane.drawCube(drawmode);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(programlist[3]);
	//draw the towers

	glUniform1ui(colourmodeIDs[3], colourmode);
	glUniformMatrix4fv(viewIDs[3], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[3], 1, GL_FALSE, &projection[0][0]);
	mat4 model4 = mat4(1.0f);
	model4 = translate(model4, vec3(x + 15.5, sealevel - 1.0f, z));
	glUniformMatrix4fv(modelIDs[3], 1, GL_FALSE, &model4[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[5]);
	tower[0].drawObject(drawmode);

	glUniform1ui(colourmodeIDs[4], colourmode);
	glUniformMatrix4fv(viewIDs[4], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[4], 1, GL_FALSE, &projection[0][0]);
	mat4 model5 = mat4(1.0f);
	model5 = translate(model5, vec3(x + 6.5, sealevel - 1.0f, z));
	glUniformMatrix4fv(modelIDs[4], 1, GL_FALSE, &model5[0][0]);
	tower[1].drawObject(drawmode);

	glUniform1ui(colourmodeIDs[5], colourmode);
	glUniformMatrix4fv(viewIDs[5], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[5], 1, GL_FALSE, &projection[0][0]);
	mat4 model6 = mat4(1.0f);
	model6 = translate(model6, vec3(x + 6.5, sealevel - 1.0f, z-5.5f));
	glUniformMatrix4fv(modelIDs[5], 1, GL_FALSE, &model6[0][0]);
	tower[2].drawObject(drawmode);


	glUniform1ui(colourmodeIDs[6], colourmode);
	glUniformMatrix4fv(viewIDs[6], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[6], 1, GL_FALSE, &projection[0][0]);
	mat4 model7 = mat4(1.0f);
	model7 = translate(model7, vec3(x + 15.5, sealevel - 1.0f, z - 5.5f));
	glUniformMatrix4fv(modelIDs[6], 1, GL_FALSE, &model7[0][0]);
	tower[3].drawObject(drawmode);
	glBindTexture(GL_TEXTURE_2D, 0);
	//draw the walls
	glUseProgram(programlist[7]);
	glUniform1ui(colourmodeIDs[7], colourmode);
	glUniformMatrix4fv(viewIDs[7], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[7], 1, GL_FALSE, &projection[0][0]);
	mat4 model8 = mat4(1.0f);
	model8 = translate(model8, vec3(x + 9.5, sealevel - 1.0f, z - 5.5f));
	model8 = scale(model8, vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(modelIDs[7], 1, GL_FALSE, &model8[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[4]);
	walls[0].drawObject(drawmode);
	
	mat4 model9 = mat4(1.0f);
	model9 = translate(model9, vec3(x + 9.5, sealevel - 1.0f, z));
	model9 = scale(model9, vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(modelIDs[7], 1, GL_FALSE, &model9[0][0]);
	
	walls[1].drawObject(drawmode);

	
	mat4 model10 = mat4(1.0f);
	model10 = translate(model10, vec3(x + 15.5, sealevel - 1.0f, z - 1.5f));
	model10 = scale(model10, vec3(0.2f, 0.5f, 0.5f));
	model10 = rotate(model10, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelIDs[7], 1, GL_FALSE, &model10[0][0]);
	
	walls[2].drawObject(drawmode);
	
	mat4 model11 = mat4(1.0f);
	model11 = translate(model11, vec3(x + 6.5, sealevel - 1.0f, z - 1.5f));
	model11 = scale(model11, vec3(0.2f, 0.5f, 0.5f));
	model11 = rotate(model11, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelIDs[7], 1, GL_FALSE, &model11[0][0]);
	
	walls[3].drawObject(drawmode);
	glBindTexture(GL_TEXTURE_2D, 0);




	/* Draw our heightfield */
	glUseProgram(programlist[1]);

	glUniform1ui(colourmodeIDs[1], colourmode);
	glUniformMatrix4fv(viewIDs[1], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[1], 1, GL_FALSE, &projection[0][0]);
	mat4 model2 = mat4(1.0f);
	model2 = translate(model2, vec3(x, sealevel-3.0f, z));
	glUniformMatrix4fv(modelIDs[1], 1, GL_FALSE, &model2[0][0]);
	terrain->drawObject(drawmode);

	//drawbridge
	glUseProgram(programlist[10]);

	glUniform1ui(colourmodeIDs[10], colourmode);
	glUniformMatrix4fv(viewIDs[10], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionIDs[10], 1, GL_FALSE, &projection[0][0]);
	mat4 model13 = mat4(1.0f);
	
	model13 = translate(model3, vec3(0.0f, 0.0f ,0.0f));

	
	//pivot solution adapted from https://stackoverflow.com/questions/58533065/modern-opengl-rotate-a-cuboid-with-pivot-at-one-end-of-the-cuboid
	
	vec3 pivot = vec3(x + 3.8, sealevel + 0.4f, z - 3.4);
	
	model13 = translate(model3, vec3(x + 3.8, sealevel + 0.4f+bridgeset, z - 3.4));
	model13 = glm::rotate(model13, glm::radians(bridge_angle), glm::vec3(1, 0, 0));
	
	glUniformMatrix4fv(modelIDs[10], 1, GL_FALSE, &model13[0][0]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[6]);

	bridge.drawObject(drawmode);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	
	
	glUseProgram(0);
	
	

	//Draw our sky
	skycube.Draw(view,projection,skyprogram);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	

	/* Modify our camera variables */

	camerax += camerax_inc;
	cameray += cameray_inc;

	cameraup += cameraup_inc;
	/* Recreate the terrain if we have changed the terrain parameters */
	if (recreate_terrain)
	{
		delete terrain;
		terrain = new terrain_object(octaves, perlin_frequency, perlin_scale);
		terrain->createTerrain(200, 200, land_size, land_size,sealevel);
		terrain->createObject();
		terrain->setColour(vec3(0.0, 1.0, 0.0));
		recreate_terrain = false;
	}
	mat4 ParticleView = lookAt(
		vec3(0, 0, 4.f),
		vec3(0, -6.2, 0),
		vec3(0, 1.f, 0)
	);
	/* Update and draw our particle animation */
	particleObject.drawParticles(projection, ParticleView);
	

}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//movement controls and camera
	if (key == 'W') cameraPos += cameraSpeed * cameraFront;
	if (key == 'S')  cameraPos -= cameraSpeed * cameraFront;
	if (key == 'A') cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (key == 'D') cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if(key=='X') cameraPos+= cameraUp *cameraSpeed;
	if (key == 'Z') cameraPos -= cameraUp * cameraSpeed;
	if (key == 'J' && action != GLFW_PRESS) {

		cameraright -= angle_inc;



	}
	if (key == 'L' && action != GLFW_PRESS) {

		cameraright += angle_inc;
	}

	if (key == 'I' && action != GLFW_PRESS) {

		pitchh -= angle_inc;



	}
	if (key == 'K' && action != GLFW_PRESS) {

		pitchh += angle_inc;
	}

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}
	//move drawbridge
	if (key == 'P' && action != GLFW_PRESS)
	{
		
		if (bridgeDown == false) { 
			
			bridge_angle += angle_inc;// 90.0f;
			if (bridge_angle >= 90.0f) {
				bridgeDown = true;
			}
			bridgeset = -1.3f;
		
		}
		else {

			bridge_angle = 0.0f;
			bridgeDown = false;
			bridgeset = -1.3f;
		}
	}

}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "My Trip - A Fantasy Scene");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);
	
	glw->eventLoop();
	
	delete(glw);
	return 0;
}



