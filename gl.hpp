#include "camera.hpp"

#ifdef defined(__MACOSX__) || defined(APPLE)
	#include <OpenCL/cl.hpp>
	#include <OpenGL/glew.h>
	#include <GLUT/glut.h>
#else
	#include <CL/cl.hpp>
	#include <GL/glew.h>
	#include <GL/glut.h>
#endif

#ifndef GL_HPP
#define GL_HPP

extern const int width;
extern const int height;
extern uint framenumber;

extern cl::Platform platform;
extern cl::Device device;
extern cl::Context context;
extern cl::CommandQueue queue;
extern cl::Program program;
extern cl::Kernel kernel;
extern cl::Buffer sceneBuffer;
extern cl::Buffer cameraBuffer;
extern cl::Buffer accumulatedBuffer;
extern cl::BufferGL vboBuffer;
extern std::vector<cl::Memory> vbosBuffer;
extern GLuint vbo;
extern Camera *hostRendercam;
extern sphere_t scene[];

extern uint hashFrame(uint);

int lastX = 0, lastY = 0;
int theButtonState = 0;
int theModifierState = 0;

int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float translate_z = -30.0;

void render();
void initCamera();
void runKernel();
void keyboard(unsigned char key, int, int);
void specialkeys(int key, int, int);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);

void initGL(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
	glutCreateWindow("rayTracer");

	glutDisplayFunc(render);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialkeys);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glewInit();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, width, 0.0, height);
}

void createVBO(GLuint* vbo) {
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	unsigned int size = width * height * sizeof(cl_float3);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexPointer(2, GL_FLOAT, 16, 0);
	glColorPointer(4, GL_UNSIGNED_BYTE, 16, (GLvoid*)8);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_POINTS, 0, width * height);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glutSwapBuffers();
}

void Timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(15, Timer, 0); // (1000 ms / 60 fps = 15 ms)
}

bool reset;
InteractiveCamera* interactiveCamera;

void keyboard(unsigned char key, int, int) {
	if(key == 27)
        exit(0);
	
    if(key == ' ') {
        initCamera();
        reset = true;
    }

	if( key == 'a') {
        interactiveCamera->strafe(-0.025f);
        reset = true;
    }

	if(key == 'd') {
        interactiveCamera->strafe(0.025f);
        reset = true;
    }

	if(key == 'w') {
        interactiveCamera->mov(0.045f);
        reset = true;
    }

	if(key == 's') {
        interactiveCamera->mov(-0.045f);
        reset = true;
    }
}

void specialkeys(int key, int, int) {
	if(key == GLUT_KEY_LEFT) {
        interactiveCamera->changedivi(-0.017f);
        reset = true;
    }

	if(key == GLUT_KEY_RIGHT) {
        interactiveCamera->changedivi(0.017f);
        reset = true;
    }

	if(key == GLUT_KEY_UP) {
        interactiveCamera->changePitch(0.017f);
        reset = true;
    }

	if(key == GLUT_KEY_DOWN) {
        interactiveCamera->changePitch(-0.017f);
        reset = true;
	}
}

void motion(int x, int y)
{
	int deltaX = lastX - x;
	int deltaY = lastY - y;

	if (deltaX != 0 || deltaY != 0) {

		if (theButtonState == GLUT_LEFT_BUTTON)
		{
			interactiveCamera->changedivi(deltaX * 0.01);
			interactiveCamera->changePitch(-deltaY * 0.01);
		}
		else if (theButtonState == GLUT_MIDDLE_BUTTON)
		{
			interactiveCamera->changeAltitude(-deltaY * 0.01);
		}

		if (theButtonState == GLUT_RIGHT_BUTTON)
		{
			interactiveCamera->changeRadius(-deltaY * 0.01);
		}

		lastX = x;
		lastY = y;
		reset = true;
		glutPostRedisplay();
	}
}

void mouse(int button, int state, int x, int y)
{
	theButtonState = button;
	theModifierState = glutGetModifiers();
	lastX = x;
	lastY = y;
	motion(x, y);
}

void render(){
	queue.enqueueWriteBuffer(sceneBuffer, CL_TRUE, 0, sceneSize * sizeof(sphere_t), scene);

	if (reset){
		queue.enqueueFillBuffer(accumulatedBuffer, 0, 0, width * height * sizeof(cl_float3));
		framenumber = 0;
	}
	reset = false;
	framenumber++;

	interactiveCamera->buildRenderCamera(hostRendercam);
	
	queue.enqueueWriteBuffer(cameraBuffer, CL_TRUE, 0, sizeof(Camera), hostRendercam);
	queue.finish();

	kernel.setArg(5, framenumber);
	kernel.setArg(6, cameraBuffer);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(10, hashFrame(framenumber));

	runKernel();
	draw();

    std::cout << framenumber << std::endl;
}

void runKernel(){
	std::size_t globalGroup = width * height;
	std::size_t localGroup = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);;

	if (globalGroup % localGroup != 0)
		globalGroup = (globalGroup / localGroup + 1) * localGroup;

	glFinish();

	queue.enqueueAcquireGLObjects(&vbosBuffer);
	queue.finish();
	queue.enqueueNDRangeKernel(kernel, NULL, globalGroup, localGroup);
	queue.finish();
	queue.enqueueReleaseGLObjects(&vbosBuffer);
	queue.finish();
}

#endif // GH_HPP