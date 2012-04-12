/*
 *Skeleton lighting program
 *SEIS750
 *Spring 2012
 **/

#include <GL/Angel.h>
#include <math.h>
#include <IL/il.h>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib,"ILUT.lib")
#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ILU.lib")

//store window width and height
int ww=700, wh=700;

#define M_PI 3.14159265358979323846

//If you want more than one type of shader, store references to your shader program objects
GLuint program1, program2, program3;

GLuint vao[1];
GLuint vbo[3];
GLuint texName[3];
int spherevertcount;

//Let's have some mouse dragging rotation
int right_button_down = FALSE;
int left_button_down = FALSE;

int prevMouseX;
int prevMouseY;

double view_rotx = 0.0;
double view_roty = 0.0;
double view_rotz = 0.0;
double z_distance;

//our modelview and perspective matrices
mat4 mv, p;

//and we'll need pointers to our shader variables
GLuint model_view;
GLuint projection;

//material properties
GLuint vPosition; //
GLuint vAmbientDiffuseColor; //Ambient and Diffuse can be the same for the material
GLuint vSpecularColor;
GLuint vSpecularExponent;
GLuint vNormal; //this is the standard name
GLuint texCoord;
GLuint texMap;


//Some light properties
GLuint light_position;
GLuint light_color;
GLuint ambient_light;


vec4* sphere_verts;
vec3* sphere_normals;



#define RECTANGLE


//Modified slightly from the OpenIL tutorials
ILuint loadTexFile(const char* filename)
{	
	/* ILboolean is type similar to GLboolean and can equal GL_FALSE (0) or GL_TRUE (1)
    it can have different value (because it's just typedef of unsigned char), but this sould be
    avoided. Variable success will be used to determine if some function returned success or failure. */
	ILboolean success; 

	/* Before calling ilInit() version should be checked. */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		/* wrong DevIL version */
		printf("Wrong IL version");
		exit(1);
	}
 
	success = ilLoadImage(filename); /* Loading of image from file */
	if (success)
	{ /* If no error occured: */

		//We need to figure out whether we have an alpha channel or not
		if(ilGetInteger(IL_IMAGE_BPP) == 3)
		{
			/* Convert every color component into unsigned byte. If your image contains alpha channel you can replace IL_RGB with IL_RGBA */
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE); 
		}
		else if(ilGetInteger(IL_IMAGE_BPP) == 4)
		{
			success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		}
		else
		{
			success = false;
		}
		if (!success)
		{
			/* Error occured */
			printf("failed conversion to unsigned byte");
			exit(1);
		}
	}
	else
	{
		/* Error occured */
		printf("Failed to load image ");
		printf(filename);
		exit(1);
	}
}

void reshape(int width, int height)
{
	ww= width;
	wh = height;
	//field of view angle, aspect ratio, closest distance from camera to object, largest distanec from camera to object
	p = Perspective(45.0, (float)width/(float)height, 1.0, 100.0);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);
	glViewport( 0, 0, width, height );
}


//In this particular case, our normal vectors and vertex vectors are identical since the sphere is centered at the origin
//For most objects this won't be the case, so I'm treating them as separate values for that reason
//This could also be done as separate triangle strips, but I've chosen to make them just triangles so I don't have to execute multiple glDrawArrays() commands
int generateSphere(float radius, int subdiv)
{
	float step = (360.0/subdiv)*(M_PI/180.0);

	int totalverts = ceil(subdiv/2.0)*subdiv * 6;

	if(sphere_normals){
		delete[] sphere_normals;
	}
	sphere_normals = new vec3[totalverts];
	if(sphere_verts){
		delete[] sphere_verts;
	}
	sphere_verts = new vec4[totalverts];

	int k = 0;
	for(float i = -M_PI/2; i<=M_PI/2; i+=step){
		for(float j = -M_PI; j<=M_PI; j+=step){
			//triangle 1
			sphere_normals[k]= vec3(radius*sin(j)*cos(i), radius*cos(j)*cos(i), radius*sin(i));
			sphere_verts[k]=   vec4(radius*sin(j)*cos(i), radius*cos(j)*cos(i), radius*sin(i), 1.0);
			k++;
	
			sphere_normals[k]= vec3(radius*sin(j)*cos(i+step), radius*cos(j)*cos(i+step), radius*sin(i+step));
			sphere_verts[k]=   vec4(radius*sin(j)*cos(i+step), radius*cos(j)*cos(i+step), radius*sin(i+step), 1.0);
			k++;
			
			sphere_normals[k]= vec3(radius*sin((j+step))*cos((i+step)), radius*cos(j+step)*cos(i+step), radius*sin(i+step));
			sphere_verts[k]=   vec4(radius*sin((j+step))*cos((i+step)), radius*cos(j+step)*cos(i+step), radius*sin(i+step), 1.0);
			k++;

			//triangle 2
			sphere_normals[k]= vec3(radius*sin((j+step))*cos((i+step)), radius*cos(j+step)*cos(i+step), radius*sin(i+step));
			sphere_verts[k]=   vec4(radius*sin((j+step))*cos((i+step)), radius*cos(j+step)*cos(i+step), radius*sin(i+step), 1.0);
			k++;

			sphere_normals[k]= vec3(radius*sin(j+step)*cos(i), radius*cos(j+step)*cos(i), radius*sin(i));
			sphere_verts[k]=   vec4(radius*sin(j+step)*cos(i), radius*cos(j+step)*cos(i), radius*sin(i), 1.0);
			k++;

			sphere_normals[k]= vec3(radius*sin(j)*cos(i), radius*cos(j)*cos(i), radius*sin(i));
			sphere_verts[k]=   vec4(radius*sin(j)*cos(i), radius*cos(j)*cos(i), radius*sin(i), 1.0);
			k++;
		}
	}
	return totalverts;
}

void display(void)
{
	/*clear all pixels*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Take care of any mouse rotations or panning
    mv = LookAt(vec4(0, 0, 10+z_distance, 1.0), vec4(0, 0, 0, 1.0), vec4(0, 1, 0, 0.0));

	mv = mv * RotateX(view_rotx) * RotateY(view_roty) * RotateZ(view_rotz);

	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	
	
	//You need to send some vertex attributes and uniform variables here
	glUniform4fv(ambient_light, 1, vec4(0.2, 0.2, 0.2, 1));
	glUniform4fv(light_color, 1, vec4(1, 1, 1, 1));
	glUniform4fv(light_position, 1, mv*vec4(50, 50, 50, 1));		// Light will follow the object (Because of mv)
	
	glVertexAttrib4fv(vAmbientDiffuseColor, vec4(0.5, 0.5, 0.5, 1));
    glVertexAttrib4fv(vSpecularColor, vec4(1, 1, 1, 1));
	glVertexAttrib1f(vSpecularExponent, 10);

#ifndef RECTANGLE
	glBindVertexArray( vao[0] );
	glDrawArrays( GL_TRIANGLES, 0, spherevertcount );    // draw the sphere 
#else
	glBindVertexArray( vao[0] );
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texName[0]); //which texture do we want?
	glDrawArrays( GL_TRIANGLES, 0, 6 );
#endif
    
    glFlush();
	
	/*start processing buffered OpenGL routines*/
	glutSwapBuffers();
}

//Connect all of the shader variables to local pointers
void setupShader(GLuint prog){
	glUseProgram( prog );
	model_view = glGetUniformLocation(prog, "model_view");
	projection = glGetUniformLocation(prog, "projection");
	
	vAmbientDiffuseColor = glGetAttribLocation(prog, "vAmbientDiffuseColor");
	vSpecularColor = glGetAttribLocation(prog, "vSpecularColor");
	vSpecularExponent = glGetAttribLocation(prog, "vSpecularExponent");
	light_position = glGetUniformLocation(prog, "light_position");
	light_color = glGetUniformLocation(prog, "light_color");
	ambient_light = glGetUniformLocation(prog, "ambient_light");

	//Our vertex array has to deal with a different shader now
	glBindVertexArray( vao[0] );

	glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
	vPosition = glGetAttribLocation(prog, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
#ifndef RECTANGLE
	glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
	vNormal = glGetAttribLocation(prog, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
	texMap = glGetUniformLocation(prog, "texture");
	glUniform1i(texMap, 0);//assign this one to texture unit 0

	glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );
	texCoord = glGetAttribLocation(prog, "texCoord");
	glEnableVertexAttribArray(texCoord);
	glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

#endif

	//Don't forget to send the projection matrix to our new shader!
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);
}


void Keyboard(unsigned char key, int x, int y) {
	/*exit when the escape key is pressed*/
	if (key == 27)
		exit(0);

	if (key == 'g'){
		setupShader(program1);
	}
	//eventually we'll have a second shader program
	if (key == 'p'){
		setupShader(program2);
	}  
	if (key == 'c')
	{
		setupShader(program3);
	}

	glutPostRedisplay();

}

//Mouse drags = rotation
void mouse_dragged(int x, int y) {
	double thetaY, thetaX;
	if (left_button_down) {
		thetaY = 360.0 *(x-prevMouseX)/ww;    
		thetaX = 360.0 *(prevMouseY - y)/wh;
		prevMouseX = x;
		prevMouseY = y;
		view_rotx += thetaX;
		view_roty += thetaY;
	}
	else if (right_button_down) {
		z_distance = 5.0*(prevMouseY-y)/wh;
	}
  glutPostRedisplay();
}


void mouse(int button, int state, int x, int y) {
  //establish point of reference for dragging mouse in window
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      left_button_down = TRUE;
	  prevMouseX= x;
      prevMouseY = y;
    }

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
      right_button_down = TRUE;
      prevMouseX = x;
      prevMouseY = y;
    }
    else if (state == GLUT_UP) {
      left_button_down = FALSE;
	  right_button_down = FALSE;
	}
}

void init() {

	/*select clearing (background) color*/
	glClearColor(1.0, 1.0, 1.0, 1.0);

	//populate our arrays
	spherevertcount = generateSphere(2, 10);

	// Load shaders and use the resulting shader program
    program1 = InitShader( "vshader-transform.glsl", "fshader-transform.glsl" );
	program2 = InitShader( "vshader-phongshading.glsl", "fshader-phongshading.glsl" );
	program3 = InitShader( "vshader-celshading.glsl", "fshader-celshading.glsl" );

	// Create a vertex array object
    glGenVertexArrays( 1, &vao[0] );

#ifndef RECTANGLE
	/* Do a sphere */

    // Create and initialize any buffer objects
	glBindVertexArray( vao[0] );
	glGenBuffers( 2, &vbo[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData( GL_ARRAY_BUFFER, spherevertcount*sizeof(vec4), sphere_verts, GL_STATIC_DRAW);	

	//and now our colors for each vertex
	glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, spherevertcount*sizeof(vec3), sphere_normals, GL_STATIC_DRAW );

#else
	/* Do a rectangle for test */
	vec4 squareverts[6];
	vec2 texcoords[6];
	squareverts[0] = vec4(-1, -1, 0, 1);
	texcoords[0] = vec2(0, 0);
	squareverts[1] = vec4(1, -1, 0, 1);
	texcoords[1] = vec2(1, 0);
	squareverts[2] = vec4(1, 1, 0, 1);
	texcoords[2] = vec2(1, 1);
	squareverts[3] = vec4(1, 1, 0, 1);
	texcoords[3] = vec2(1, 1);
	squareverts[4] = vec4(-1, 1, 0, 1);
	texcoords[4] = vec2(0, 1);
	squareverts[5] = vec4(-1, -1, 0, 1);
	texcoords[5] = vec2(0, 0);

    // Create and initialize any buffer objects
	glBindVertexArray( vao[0] );
	glGenBuffers( 3, &vbo[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(squareverts), squareverts, GL_STATIC_DRAW);

	glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	ILuint ilTexID[3]; /* ILuint is a 32bit unsigned integer.
    //Variable texid will be used to store image name. */

	ilInit(); /* Initialization of OpenIL */
	ilGenImages(3, ilTexID); /* Generation of three image names for OpenIL image loading */
	glGenTextures(3, texName); //and we eventually want the data in an OpenGL texture
 


	ilBindImage(ilTexID[0]); /* Binding of IL image name */
	loadTexFile("images/domokun.png");
	glBindTexture(GL_TEXTURE_2D, texName[0]); //bind OpenGL texture name

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   //Note how we depend on OpenIL to supply information about the file we just loaded in
   glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
	   ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());

    ilDeleteImages(3, ilTexID); //we're done with OpenIL, so free up the memory

#endif



	setupShader(program1);

	//Only draw the things in the front layer
	glEnable(GL_DEPTH_TEST);
}

int main(int argc, char **argv)
{
	/*set up window for display*/
	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0); 
	glutInitWindowSize(ww, wh);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Assignment 5 - Ross Anderson");  

	glewExperimental = GL_TRUE;

	glewInit();
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(Keyboard);
	glutReshapeFunc(reshape);

	glutMouseFunc(mouse);
	glutMotionFunc(mouse_dragged);

	glutMainLoop();
	return 0;
}