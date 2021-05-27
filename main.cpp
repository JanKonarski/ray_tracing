#include <iostream>
#include <fstream>
#include <vector>

#include "camera.hpp"
#include "gl.hpp"
#include "cl.hpp"
#include "math.hpp"

#ifdef defined(__MACOSX__) || defined(APPLE)
	#include <OpenCL/cl.hpp>
	#include <OpenGL/glx.hpp>
#else
	#include <CL/cl.hpp>
	#include <GL/glx.h>
#endif

const int width = 1280;
const int height = 720;

cl::Platform platform;
cl::Device device;
cl::Context context;
cl::CommandQueue queue;
cl::Program program;
cl::Kernel kernel;
cl::Buffer sceneBuffer;
cl::Buffer cameraBuffer;
cl::Buffer accumulatedBuffer;
cl::BufferGL vboBuffer;
std::vector<cl::Memory> vbosBuffer;
GLuint vbo;

uint framenumber = 0;

Camera *hostRendercam = NULL;
sphere_t scene[sceneSize];

uint hashFrame(uint a) {
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}

void initCamera()
{
	delete interactiveCamera;
	interactiveCamera = new InteractiveCamera();

	interactiveCamera->setResolution(width, height);
	interactiveCamera->setFov(45);
}

int main(int argc, char** argv){
	initGL(argc, argv);
	initOpenCL();

	createVBO(&vbo);

	Timer(0);
	glFinish();
	initScene(scene);

	sceneBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sceneSize * sizeof(sphere_t));
	queue.enqueueWriteBuffer(sceneBuffer, CL_TRUE, 0, sceneSize * sizeof(sphere_t), scene);

	initCamera();

	hostRendercam = new Camera();
	interactiveCamera->buildRenderCamera(hostRendercam);

	cameraBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(Camera));
	queue.enqueueWriteBuffer(cameraBuffer, CL_TRUE, 0, sizeof(Camera), hostRendercam);

	vboBuffer = cl::BufferGL(context, CL_MEM_WRITE_ONLY, vbo);
	vbosBuffer.push_back(vboBuffer);

	accumulatedBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_float3));

	initCLKernel();

	glutMainLoop();

	return EXIT_SUCCESS;
}