#include <iostream>
#include <fstream>

#ifdef defined(__MACOSX__) || defined(APPLE)
	#include <OpenCL/cl.hpp>
	#include <OpenGL/glx.hpp>
#else
	#include <CL/cl.hpp>
	#include <GL/glx.h>
#endif

#ifndef CL_HPP
#define CL_HPP

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

extern uint framenumber;

extern uint hashFrame(uint);

void selectPlatform(const std::vector<cl::Platform> &platforms, cl::Platform &platform) {
	if(platforms.size() == 1)
		platform = platforms[0];
		
	else {
		std::size_t number = 1;
		std::cout << "Available platforms:" << std::endl;
		for(auto platform : platforms)
			std::cout << "\t" << number++ << ". " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

		std::size_t idx = SIZE_MAX;
		while(idx > platforms.size()) {
			std::cout << "Select platform: ";
			std::cin >> idx;
		}
		std::cout << std::endl << std::endl;

		platform = platforms[idx - 1];
	}
}

void selectDevice(const std::vector<cl::Device> &devices, cl::Device &device) {
	if(devices.size() == 1)
		device = devices[0];
		
	else {
		std::size_t number = 1;
		std::cout << "Available devices:" << std::endl;
		for(auto device : devices)
			std::cout << "\t" << number++ << ". " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
			
		std::size_t idx = SIZE_MAX;
		while(idx > devices.size()) {
			std::cout << "Select device: ";
			std::cin >> idx;
		}
		std::cout << std::endl << std::endl;

		device = devices[idx - 1];
	}
}

void initOpenCL()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	selectPlatform(platforms, platform);

	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

	selectDevice(devices, device);

#ifdef _WIN32
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_DISPLAY_KHR, (cl_context_properties)wglGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};
#else
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};
#endif

	context = cl::Context(device, properties);
	queue = cl::CommandQueue(context, device);


	std::string source;
	std::ifstream file("kernel.cl");
	if (!file){
		std::cerr << "No OpenCL kernel file" << std::endl;
		exit(1);
	}
	while (!file.eof()){
		char line[1024];
		file.getline(line, 1023);
		source += line;
	}

	program = cl::Program(context, source);

	if (program.build({ device }) == CL_BUILD_PROGRAM_FAILURE) {
		std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
		exit(1);
	}

	kernel = cl::Kernel(program, "render");
}

void initCLKernel(){
	kernel.setArg(0, sceneBuffer);
	kernel.setArg(1, width);
	kernel.setArg(2, height);
	kernel.setArg(3, sceneSize);
	kernel.setArg(4, vboBuffer);
	kernel.setArg(5, framenumber);
	kernel.setArg(6, cameraBuffer);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(9, accumulatedBuffer);
	kernel.setArg(10, hashFrame(framenumber));
}

#endif // CL_HPP