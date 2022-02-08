
#include "openCL.hpp"


#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifdef _OPENCL_HPP

OpenCLHandler::~OpenCLHandler()
{
	// release the opencl objects
    // release the unreleased mem object
    for(int i = 0; i < bufferManager.size();i++)
    {
        clReleaseMemObject(*bufferManager[i]);    
    }
    bufferManager.clear();

    // Release the kernels
    std::map<std::string,cl_kernel*>::iterator it;
    for(it = kernelManager.begin(); it != kernelManager.end(); ++it)
    {
        clReleaseKernel( *(it->second) );
    }
    kernelManager.clear();

    // release the computation program
    if (program)
    {
        clReleaseProgram(*program);
        delete program;
    }

    // Release the command queues
    for(int i = 0; i < commandQueueList.size();i++)
    {
        clReleaseCommandQueue(*commandQueueList[i]);
    }
    commandQueueList.clear();

    // Release the context
    if (context)
    {
        clReleaseContext(*context);
        delete context;
    }



}

OpenCLHandler::OpenCLHandler(
	cl_device_info type,
	char* programfileName
	)
{
	cl_int error = 0;
	platformSelected = 0;

	error = clGetPlatformIDs(0, 0, &platformList_size);
    if(error != CL_SUCCESS) throw CLException(error,"OpenCLHandler::Platform count");

	platformList = (cl_platform_id*)malloc(platformList_size * sizeof(cl_platform_id));
	error = clGetPlatformIDs(platformList_size, platformList, NULL);
    if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Platform list creation");
	context = (cl_context*) malloc(sizeof(cl_context));


	devicesListSizePerPlatform = (cl_uint*) malloc(sizeof(cl_uint) * platformList_size);
	devicesListPerPlatform = (cl_device_id**) malloc ( sizeof(cl_device_id) * platformList_size);

    for (unsigned int i = 0; i < platformList_size; i++)
	{
		
		error = clGetDeviceIDs(platformList[i], type, 0, NULL, &(devicesListSizePerPlatform[i]));
        if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Device Count");
	
		devicesListPerPlatform[i] = (cl_device_id*)malloc(sizeof(cl_device_id) * devicesListSizePerPlatform[i]);
		error = clGetDeviceIDs(platformList[i], type, devicesListSizePerPlatform[i], devicesListPerPlatform[i], NULL);
        if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Devices Lists creation");
	
	}

	// Display for checking :
    for (unsigned int i = 0; i < platformList_size; i++)
	{
		char info[256];
		error = clGetPlatformInfo(platformList[i], CL_PLATFORM_NAME, 256, info, NULL);
        if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Display Platform information");

        std::cout << "Platform " << i << " : " << info << std::endl;
        for (unsigned int j = 0; j < devicesListSizePerPlatform[i]; j++)
		{
			error = clGetDeviceInfo(devicesListPerPlatform[i][j], CL_DEVICE_NAME, 256, info, NULL);
            if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Display Device information");
            std::cout << "- Device " << j << " : " << info << std::endl;
		}

	}

	// TODO : si nb platform > 1 : active the platforme selector

	// Context d'execution
	(*context) = clCreateContext(0, devicesListSizePerPlatform[platformSelected], devicesListPerPlatform[platformSelected], NULL, NULL, &error);
    if(error != CL_SUCCESS) throw CLException(error, "OpenCLHandler::Context creation");

	// List of command queues (1 for each device)
    for (unsigned int i = 0; i < devicesListSizePerPlatform[platformSelected]; i++)
	{
        commandQueueList.push_back(new cl_command_queue);
        *(commandQueueList.back()) = clCreateCommandQueue((*context), devicesListPerPlatform[platformSelected][i], 0, &error);
        char num[33];
        itoa(i,num,10);
        std::string logMsg = (std::string("OpenCLHandler::commandQueueList[") + num + std::string("]"));
        if(error != CL_SUCCESS) throw(CLException(error,logMsg));

	}

	// If a program
	program = NULL;
	if (programfileName != NULL)
	{
		loadProgramFromFile(programfileName);
	}

}

void 
OpenCLHandler::loadProgramFromFile(	
	std::string fileName
)
{
	cl_int error;
    std::ifstream sourceFile(fileName.c_str(), std::ifstream::in);
    error = sourceFile.is_open() ? CL_SUCCESS : CL_INVALID_KERNEL_FILE;
    if(error != CL_SUCCESS) throw CLException(error, fileName.c_str());

	std::string srcString(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

    const char* srcArray[1] = { srcString.c_str() };
	size_t srcLengthArray[1] = { srcString.length() };
	
	program = new cl_program;
	(*program) = clCreateProgramWithSource(*context, 1, srcArray, srcLengthArray, &error);

    std::string logMessage("Program creation from source in ");
    logMessage.append(fileName);
    if(error != CL_SUCCESS) throw CLException(error,logMessage);

	error = clBuildProgram(*program, devicesListSizePerPlatform[platformSelected], devicesListPerPlatform[platformSelected], "", NULL, NULL);
    logMessage = std::string("Program building process of ");
    logMessage.append(fileName);
    if (error != CL_SUCCESS)
    {
        logMessage.append("\nError log for devices : \n");
        for(int i = 0; i < devicesListSizePerPlatform[platformSelected];i++)
        {
            size_t len;
            clGetProgramBuildInfo(*program, devicesListPerPlatform[platformSelected][i],
                                  CL_PROGRAM_BUILD_LOG,
                                  0, NULL, &len);
            char* buffer = new char[len + 1];
            clGetProgramBuildInfo(*program, devicesListPerPlatform[platformSelected][i],
                                  CL_PROGRAM_BUILD_LOG,
                                  sizeof(char) * len + 1, buffer, &len);

            logMessage.append(buffer);
            logMessage.append("\n");
        }
        throw CLException(error,logMessage);

    }

}

cl_kernel*
OpenCLHandler::getKernel(
	std::string name
)
{
    // Associative version of kernel manager : <name,kernelObject>
    bool isAlreadyIn = false;
    std::map<std::string,cl_kernel*>::iterator it;
    for(it = kernelManager.begin(); it != kernelManager.end(); ++it)
    {
        if(it->first.compare(name) == 0)
        {
            return it->second;
            isAlreadyIn = true;
        }
    }

    if(!isAlreadyIn)
    {
        cl_int error;
        cl_kernel* kernelObject = new cl_kernel;
        *kernelObject = clCreateKernel(*program, name.c_str(), &error);
        if(error == CL_SUCCESS)
        {
            kernelManager[name] = kernelObject;
            return kernelObject;
        }
        else
        {
            throw CLException(error,std::string("Retrieving kernel ") + name);
            return NULL;
        }
    }


}

cl_command_queue* 
OpenCLHandler::getQueue(
	int i
)
{
    if (i >= 0 && (unsigned int)i < commandQueueList.size()) return commandQueueList[i];
	else return NULL;
}

cl_command_queue*
OpenCLHandler::resetQueue(
    int i
    )
{
    cl_int error = CL_SUCCESS;
    if (i >= 0 && (unsigned int)i < commandQueueList.size())
    {
        clReleaseCommandQueue(*commandQueueList[i]);
        *commandQueueList[i] = clCreateCommandQueue((*context), devicesListPerPlatform[platformSelected][i], 0, &error);
        if(error != CL_SUCCESS)
        {
            throw CLException(error,"Reset Command Queue");
            return NULL;
        }
        else return commandQueueList[i];
    }
    else return NULL;
}

cl_device_id
OpenCLHandler::getDeviceOfQueue(
    int i
    )
{
    return devicesListPerPlatform[platformSelected][i];
}



cl_mem*
OpenCLHandler::createBuffer(
	cl_map_flags memoryAttributes,
	size_t dataSize,
	void* dataHostPtr
	)
{
	cl_int error;

    bufferManager.push_back(new cl_mem);
    *(bufferManager.back()) = clCreateBuffer(*context,memoryAttributes, dataSize, dataHostPtr, &error);
    if(error == CL_SUCCESS)
    {
        return bufferManager.back();
    }
    else
    {
        bufferManager.pop_back();
        throw  CLException(error,std::string("OpenCLHandler::createBuffer"));
        return NULL;
    }
}

void
OpenCLHandler::releaseNBuffer(
    int nLastBuffers
    )
{
    for(int i = 0; i < nLastBuffers; i++)
    {
        clReleaseMemObject(*(bufferManager.back()));
        bufferManager.pop_back();
    }
}


// C version
void
OpenCLHandler::checkError(
    cl_int errorCode,	// OpenCL error code
    const char* name   // Error custom message
)
{
    if (errorCode != CL_SUCCESS) {

        char* errorType = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        switch (errorCode)
        {
        case CL_DEVICE_NOT_FOUND: errorType = "CL_DEVICE_NOT_FOUND"; break;
        case CL_DEVICE_NOT_AVAILABLE: errorType = "CL_DEVICE_NOT_AVAILABLE"; break;
        case CL_COMPILER_NOT_AVAILABLE: errorType = "CL_COMPILER_NOT_AVAILABLE"; break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: errorType = "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
        case CL_OUT_OF_RESOURCES: errorType = "CL_OUT_OF_RESOURCES"; break;
        case CL_OUT_OF_HOST_MEMORY: errorType = "CL_OUT_OF_HOST_MEMORY"; break;
        case CL_PROFILING_INFO_NOT_AVAILABLE: errorType = "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
        case CL_MEM_COPY_OVERLAP: errorType = "CL_MEM_COPY_OVERLAP"; break;
        case CL_IMAGE_FORMAT_MISMATCH: errorType = "CL_IMAGE_FORMAT_MISMATCH"; break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: errorType = "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
        case CL_BUILD_PROGRAM_FAILURE: errorType = "CL_BUILD_PROGRAM_FAILURE"; break;
        case CL_MAP_FAILURE: errorType = "CL_MAP_FAILURE"; break;
        case CL_INVALID_VALUE: errorType = "CL_INVALID_VALUE"; break;
        case CL_INVALID_DEVICE_TYPE: errorType = "CL_INVALID_DEVICE_TYPE"; break;
        case CL_INVALID_PLATFORM: errorType = "CL_INVALID_PLATFORM"; break;
        case CL_INVALID_DEVICE: errorType = "CL_INVALID_DEVICE"; break;
        case CL_INVALID_CONTEXT: errorType = "CL_INVALID_CONTEXT"; break;
        case CL_INVALID_QUEUE_PROPERTIES: errorType = "CL_INVALID_QUEUE_PROPERTIES"; break;
        case CL_INVALID_COMMAND_QUEUE: errorType = "CL_INVALID_COMMAND_QUEUE"; break;
        case CL_INVALID_HOST_PTR: errorType = "CL_INVALID_HOST_PTR"; break;
        case CL_INVALID_MEM_OBJECT: errorType = "CL_INVALID_MEM_OBJECT"; break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: errorType = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
        case CL_INVALID_IMAGE_SIZE: errorType = "CL_INVALID_IMAGE_SIZE"; break;
        case CL_INVALID_SAMPLER: errorType = "CL_INVALID_SAMPLER"; break;
        case CL_INVALID_BINARY: errorType = "CL_INVALID_BINARY"; break;
        case CL_INVALID_BUILD_OPTIONS: errorType = "CL_INVALID_BUILD_OPTIONS"; break;
        case CL_INVALID_PROGRAM: errorType = "CL_INVALID_PROGRAM"; break;
        case CL_INVALID_PROGRAM_EXECUTABLE: errorType = "CL_INVALID_PROGRAM_EXECUTABLE"; break;
        case CL_INVALID_KERNEL_NAME: errorType = "CL_INVALID_KERNEL_NAME"; break;
        case CL_INVALID_KERNEL_DEFINITION: errorType = "CL_INVALID_KERNEL_DEFINITION"; break;
        case CL_INVALID_KERNEL: errorType = "CL_INVALID_KERNEL"; break;
        case CL_INVALID_ARG_INDEX: errorType = "CL_INVALID_ARG_INDEX"; break;
        case CL_INVALID_ARG_VALUE: errorType = "CL_INVALID_ARG_VALUE"; break;
        case CL_INVALID_ARG_SIZE: errorType = "CL_INVALID_ARG_SIZE"; break;
        case CL_INVALID_KERNEL_ARGS: errorType = "CL_INVALID_KERNEL_ARGS"; break;
        case CL_INVALID_WORK_DIMENSION: errorType = "CL_INVALID_WORK_DIMENSION"; break;
        case CL_INVALID_WORK_GROUP_SIZE: errorType = "CL_INVALID_WORK_GROUP_SIZE"; break;
        case CL_INVALID_WORK_ITEM_SIZE: errorType = "CL_INVALID_WORK_ITEM_SIZE"; break;
        case CL_INVALID_GLOBAL_OFFSET: errorType = "CL_INVALID_GLOBAL_OFFSET"; break;
        case CL_INVALID_EVENT_WAIT_LIST: errorType = "CL_INVALID_EVENT_WAIT_LIST"; break;
        case CL_INVALID_EVENT: errorType = "CL_INVALID_EVENT"; break;
        case CL_INVALID_OPERATION: errorType = "CL_INVALID_OPERATION"; break;
        case CL_INVALID_GL_OBJECT: errorType = "CL_INVALID_GL_OBJECT"; break;
        case CL_INVALID_BUFFER_SIZE: errorType = "CL_INVALID_BUFFER_SIZE"; break;
        case CL_INVALID_MIP_LEVEL: errorType = "CL_INVALID_MIP_LEVEL"; break;
        case CL_INVALID_GLOBAL_WORK_SIZE: errorType = "CL_INVALID_GLOBAL_WORK_SIZE"; break;
        case CL_INVALID_KERNEL_FILE: errorType = "CL_INVALID_KERNEL_FILE"; break;

        default: errorType = "UNDEFINED ERROR"; break;
        }

        std::string errorMessage("CL ERROR : ");
        errorMessage.append(name);
        errorMessage.append(" threw ");
        errorMessage.append(errorType);
        errorMessage.append(" \n");

        fprintf(stderr, "\n%s",errorMessage.c_str());

        //exit(EXIT_FAILURE);
    }
}

// C++ exceptions

CLException::CLException(cl_int error, std::string helper)
    : std::exception()
{
    errorCode = error;
    helpMessage = helper;
}

CLException::CLException(const CLException& e)
{
    errorCode = e.errorCode;
    helpMessage = e.helpMessage;
}

const char* CLException::what()
{
    std::string errorMessage("CL EXCEPTION : ");
    if (errorCode != CL_SUCCESS) {

        char* errorType = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        switch (errorCode)
        {
        case CL_DEVICE_NOT_FOUND: errorType = "CL_DEVICE_NOT_FOUND"; break;
        case CL_DEVICE_NOT_AVAILABLE: errorType = "CL_DEVICE_NOT_AVAILABLE"; break;
        case CL_COMPILER_NOT_AVAILABLE: errorType = "CL_COMPILER_NOT_AVAILABLE"; break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: errorType = "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
        case CL_OUT_OF_RESOURCES: errorType = "CL_OUT_OF_RESOURCES"; break;
        case CL_OUT_OF_HOST_MEMORY: errorType = "CL_OUT_OF_HOST_MEMORY"; break;
        case CL_PROFILING_INFO_NOT_AVAILABLE: errorType = "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
        case CL_MEM_COPY_OVERLAP: errorType = "CL_MEM_COPY_OVERLAP"; break;
        case CL_IMAGE_FORMAT_MISMATCH: errorType = "CL_IMAGE_FORMAT_MISMATCH"; break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: errorType = "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
        case CL_BUILD_PROGRAM_FAILURE: errorType = "CL_BUILD_PROGRAM_FAILURE"; break;
        case CL_MAP_FAILURE: errorType = "CL_MAP_FAILURE"; break;
        case CL_INVALID_VALUE: errorType = "CL_INVALID_VALUE"; break;
        case CL_INVALID_DEVICE_TYPE: errorType = "CL_INVALID_DEVICE_TYPE"; break;
        case CL_INVALID_PLATFORM: errorType = "CL_INVALID_PLATFORM"; break;
        case CL_INVALID_DEVICE: errorType = "CL_INVALID_DEVICE"; break;
        case CL_INVALID_CONTEXT: errorType = "CL_INVALID_CONTEXT"; break;
        case CL_INVALID_QUEUE_PROPERTIES: errorType = "CL_INVALID_QUEUE_PROPERTIES"; break;
        case CL_INVALID_COMMAND_QUEUE: errorType = "CL_INVALID_COMMAND_QUEUE"; break;
        case CL_INVALID_HOST_PTR: errorType = "CL_INVALID_HOST_PTR"; break;
        case CL_INVALID_MEM_OBJECT: errorType = "CL_INVALID_MEM_OBJECT"; break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: errorType = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
        case CL_INVALID_IMAGE_SIZE: errorType = "CL_INVALID_IMAGE_SIZE"; break;
        case CL_INVALID_SAMPLER: errorType = "CL_INVALID_SAMPLER"; break;
        case CL_INVALID_BINARY: errorType = "CL_INVALID_BINARY"; break;
        case CL_INVALID_BUILD_OPTIONS: errorType = "CL_INVALID_BUILD_OPTIONS"; break;
        case CL_INVALID_PROGRAM: errorType = "CL_INVALID_PROGRAM"; break;
        case CL_INVALID_PROGRAM_EXECUTABLE: errorType = "CL_INVALID_PROGRAM_EXECUTABLE"; break;
        case CL_INVALID_KERNEL_NAME: errorType = "CL_INVALID_KERNEL_NAME"; break;
        case CL_INVALID_KERNEL_DEFINITION: errorType = "CL_INVALID_KERNEL_DEFINITION"; break;
        case CL_INVALID_KERNEL: errorType = "CL_INVALID_KERNEL"; break;
        case CL_INVALID_ARG_INDEX: errorType = "CL_INVALID_ARG_INDEX"; break;
        case CL_INVALID_ARG_VALUE: errorType = "CL_INVALID_ARG_VALUE"; break;
        case CL_INVALID_ARG_SIZE: errorType = "CL_INVALID_ARG_SIZE"; break;
        case CL_INVALID_KERNEL_ARGS: errorType = "CL_INVALID_KERNEL_ARGS"; break;
        case CL_INVALID_WORK_DIMENSION: errorType = "CL_INVALID_WORK_DIMENSION"; break;
        case CL_INVALID_WORK_GROUP_SIZE: errorType = "CL_INVALID_WORK_GROUP_SIZE"; break;
        case CL_INVALID_WORK_ITEM_SIZE: errorType = "CL_INVALID_WORK_ITEM_SIZE"; break;
        case CL_INVALID_GLOBAL_OFFSET: errorType = "CL_INVALID_GLOBAL_OFFSET"; break;
        case CL_INVALID_EVENT_WAIT_LIST: errorType = "CL_INVALID_EVENT_WAIT_LIST"; break;
        case CL_INVALID_EVENT: errorType = "CL_INVALID_EVENT"; break;
        case CL_INVALID_OPERATION: errorType = "CL_INVALID_OPERATION"; break;
        case CL_INVALID_GL_OBJECT: errorType = "CL_INVALID_GL_OBJECT"; break;
        case CL_INVALID_BUFFER_SIZE: errorType = "CL_INVALID_BUFFER_SIZE"; break;
        case CL_INVALID_MIP_LEVEL: errorType = "CL_INVALID_MIP_LEVEL"; break;
        case CL_INVALID_GLOBAL_WORK_SIZE: errorType = "CL_INVALID_GLOBAL_WORK_SIZE"; break;
        case CL_INVALID_KERNEL_FILE: errorType = "CL_INVALID_KERNEL_FILE"; break;

        default: errorType = "UNDEFINED ERROR"; break;
        }

        errorMessage.append(errorType);
        errorMessage.append(" was thrown \n");
    }
    else
    {
        errorMessage.append("CL_Success was thrown ?? ");
    }
    errorMessage.append("(");
    errorMessage.append(helpMessage);
    errorMessage.append(") \n");
    return errorMessage.c_str();
}

#else
#endif /* _OPENCL_HPP */
