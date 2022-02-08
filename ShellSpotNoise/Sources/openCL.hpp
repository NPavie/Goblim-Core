/*
 Opencl handler
*/
#pragma once

#ifdef _OPENCL_INCLUDES

#include <CL\cl.h>
#include <vector>
#include <string>
#include <exception>
#include <map>

#define CL_INVALID_KERNEL_FILE -64



/*
	OpenCLHandler : depending on the device type wanted, handle an application context with its command queues and a single opencl program containing kernels
	HOWTO :
	first declare an OpenCLHandler objetc whith the corresponding device type
	If not done in the constructor, load an opencl file with the function loadProgramFromFile
	Create the buffer you need for the context with the function
*/
class OpenCLHandler
{
public:
	/*
		Initialize a context and command queues from a chose device type (see CL_DEVICE_TYPE) 
	*/
	OpenCLHandler(
		cl_device_info type = CL_DEVICE_TYPE_ALL, 
		char* programfileName = NULL
		);

	/*
		Destructor : release resources
	*/
	~OpenCLHandler();

	/*
		Load an openCL program file in the handler
	*/
	void loadProgramFromFile(
		std::string fileName
		);

	/*
		Get a kernel from the loaded opencl program
	*/
    cl_kernel* getKernel(
		std::string name
		);

    cl_mem* createBuffer(
		cl_map_flags memoryAttributes,
		size_t dataSize,
		void* dataHostPtr = NULL
		);

    void releaseNBuffer(int nLastBuffers);


	cl_command_queue* getQueue(
		int i
		);

    cl_command_queue* resetQueue(
        int i
        );

	cl_device_id getDeviceOfQueue(
		int i
		);

	cl_context* getContext();

	/*
	Error checking function
	*/
	static inline void checkError(
		cl_int errorCode, // OpenCL error code
		const char* name  // Error custom message 
		);

private:
	cl_context* context;

	int platformSelected;

	cl_platform_id* platformList;
	cl_uint	platformList_size;
	
	cl_device_id** devicesListPerPlatform;
	cl_uint* devicesListSizePerPlatform;


    // Manager of openCL resources (a améliorer)
    std::vector<cl_command_queue*> commandQueueList;
    std::vector<cl_mem*>     bufferManager;
    std::map<std::string,cl_kernel*>  kernelManager;

	std::vector<cl_uint> computeUnits;
	std::vector<size_t> workGroupSize;
	std::vector<size_t> workItemSize;

	cl_program* program;
	
	

};

/*
 * Exception classe for error handling
 */

class CLException : public std::exception
{
public:
    CLException(cl_int error, std::string helper = "");
    CLException(const CLException& e);
    ~CLException() throw() {}

    const char* what();
private:
    cl_int errorCode;
    std::string helpMessage;
};

#define _OPENCL_HPP 1
#endif