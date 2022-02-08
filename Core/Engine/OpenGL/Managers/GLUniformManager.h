#ifndef _GLUNIFORM_BUFFER_
#define _GLUNIFORM_BUFFER_

#include <GL/glew.h>
#include <string>
#include <map>
#include <vector>
#include "GPUVariable.hpp"
#include "Engine/OpenGL/Managers/GLUniformBlock.h"

class GLProgram;


class GLUniformManager
{
	public:
		GLUniformManager(GLProgram *program);
		~GLUniformManager();

		void parseUniforms();

		void bindUniformBuffers();

		GPUfloat* getGPUfloat(string name);
		GPUvec2* getGPUvec2(string name);
		GPUvec3* getGPUvec3(string name);
		GPUvec4* getGPUvec4(string name);

		GPUdouble* getGPUdouble(string name);
		GPUdvec2* getGPUdvec2(string name);
		GPUdvec3* getGPUdvec3(string name);
		GPUdvec4* getGPUdvec4(string name);

		GPUint* getGPUint(string name);
		GPUivec2* getGPUivec2(string name);
		GPUivec3* getGPUivec3(string name);
		GPUivec4* getGPUivec4(string name);

		GPUmat2* getGPUmat2(string name);
		GPUmat3* getGPUmat3(string name);
		GPUmat4* getGPUmat4(string name);

		GPUdmat2* getGPUdmat2(string name);
		GPUdmat3* getGPUdmat3(string name);
		GPUdmat4* getGPUdmat4(string name);

		GPUbool* getGPUbool(string name);

		GPUsampler* getGPUsampler(string name);
		
		GPUimage* getGPUimage(string name);


		
		map< string, GPUsampler*> listSamplers;		
		map< string, GPUimage*> listImages;

		void mapBufferToBlock(GPUBuffer* ubo,std::string blockName);

	private:
		GLProgram* m_Program;
		GLuint m_UBuffer;
		std::vector< GLUniformBlock* > listBlocks;
		


		void parseShader(GLint shaderName);
		GPUVariableBase* createGPUVariable( GLenum type,string name,GLint program,GLuint uBuffer,GLint offset,GLint loc,GLint size);

		GPUVariableBase* getUniformBase(string name);
		map< string  , GPUVariableBase* > m_Uniforms;

		// Dummy GPUVariables for inactive uniforms
		static GPUfloat m_Dfloat;
		static GPUvec2 m_Dvec2;
		static GPUvec3 m_Dvec3;
		static GPUvec4 m_Dvec4;

		static GPUdouble m_Ddouble;
		static GPUdvec2 m_Ddvec2;
		static GPUdvec3 m_Ddvec3;
		static GPUdvec4 m_Ddvec4;

		static GPUint m_Dint;
		static GPUivec2 m_Divec2;
		static GPUivec3 m_Divec3;
		static GPUivec4 m_Divec4;

		static GPUmat2 m_Dmat2;
		static GPUmat3 m_Dmat3;
		static GPUmat4 m_Dmat4;

		static GPUdmat2 m_Ddmat2;
		static GPUdmat3 m_Ddmat3;
		static GPUdmat4 m_Ddmat4;

		static GPUbool m_Dbool;


		static GPUsampler m_Dsampler;		
		static GPUimage m_Dimage;
};





#endif
