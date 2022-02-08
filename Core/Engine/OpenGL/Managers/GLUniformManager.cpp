#include "GLUniformManager.h"
#include <iostream>
#include "Engine/OpenGL/v4/GLProgram.h"


GPUfloat GLUniformManager::m_Dfloat = GPUfloat();
GPUvec2 GLUniformManager::m_Dvec2 = GPUvec2();
GPUvec3 GLUniformManager::m_Dvec3 = GPUvec3();
GPUvec4 GLUniformManager::m_Dvec4 = GPUvec4();

GPUdouble GLUniformManager::m_Ddouble = GPUdouble();
GPUdvec2 GLUniformManager::m_Ddvec2 = GPUdvec2();
GPUdvec3 GLUniformManager::m_Ddvec3 = GPUdvec3();
GPUdvec4 GLUniformManager::m_Ddvec4 = GPUdvec4();

GPUint GLUniformManager::m_Dint = GPUint();
GPUivec2 GLUniformManager::m_Divec2 = GPUivec2();
GPUivec3 GLUniformManager::m_Divec3 = GPUivec3();
GPUivec4 GLUniformManager::m_Divec4 = GPUivec4();

GPUmat2 GLUniformManager::m_Dmat2 = GPUmat2();
GPUmat3 GLUniformManager::m_Dmat3 = GPUmat3();
GPUmat4 GLUniformManager::m_Dmat4 = GPUmat4();

GPUdmat2 GLUniformManager::m_Ddmat2 = GPUdmat2();
GPUdmat3 GLUniformManager::m_Ddmat3 = GPUdmat3();
GPUdmat4 GLUniformManager::m_Ddmat4 = GPUdmat4();

GPUbool GLUniformManager::m_Dbool = GPUbool();


GPUsampler GLUniformManager::m_Dsampler = GPUsampler();
GPUimage GLUniformManager::m_Dimage = GPUimage();

GLUniformManager::GLUniformManager(GLProgram *program)
{
	m_Program = program;
	
	parseUniforms();
}
GLUniformManager::~GLUniformManager()
{
	for (unsigned int i = 0;i < listBlocks.size();i++)
		delete listBlocks[i];

}
GPUVariableBase* GLUniformManager::getUniformBase(string name)
{

	GPUVariableBase *to_ret = NULL;
	map<string , GPUVariableBase* >::iterator it = m_Uniforms.find(name);
	if (it != m_Uniforms.end())	to_ret = (*it).second;
	else
	{
		LOG(WARNING)  << m_Program->getName() <<":  There is no active uniform named " << name << endl;
		to_ret = NULL;
	}


	return to_ret;
}

void GLUniformManager::parseUniforms()
{
	GLint activeUniforms = 0;
	GLint activeBlockUniforms = 0;

	glGetProgramiv(m_Program->getProgram(),GL_ACTIVE_UNIFORM_BLOCKS,&activeBlockUniforms);

	for(GLuint i=0; i< (GLuint) activeBlockUniforms; i++)
	{
		char name[256];
		glGetActiveUniformBlockName(m_Program->getProgram(),i, 256, NULL, name);
		GLint size;
		glGetActiveUniformBlockiv(m_Program->getProgram(), i,GL_UNIFORM_BLOCK_DATA_SIZE,&size);
		
		bool shared = false;
		if (std::string("Lighting").compare(name) == 0) shared = true;
		if (std::string("kernelBuffer").compare(name) == 0) shared = true;

		
		GLUniformBlock *B = new GLUniformBlock(string(name),m_Program,i,size,shared);
		int baseBindings = MISC_BINDING_START;

		// For now Lighting Binding BLock as reserved Goblim KeyWord
		
	    if (m_Program->getType() == GL_VERTEX_SHADER)
			baseBindings = VERTEX_BINDING_START;
		else if (m_Program->getType() == GL_TESS_CONTROL_SHADER)
			baseBindings = TESS_CONTROL_BINDING_START;
		else if (m_Program->getType() == GL_TESS_EVALUATION_SHADER)
			baseBindings =TESS_EVALUATION_BINDING_START;
		else if (m_Program->getType() == GL_GEOMETRY_SHADER)
			baseBindings = GEOMETRY_BINDING_START;
		else if (m_Program->getType() == GL_FRAGMENT_SHADER)
			baseBindings = FRAGMENT_BINDING_START;
		B->m_BindingPoint = baseBindings + i;
				
		if (std::string("Lighting").compare(name) == 0)
			B->m_BindingPoint = LIGHTING_BINDING;

		if (std::string("kernelBuffer").compare(name) == 0)
			B->m_BindingPoint = MISC_BINDING_START;

			listBlocks.push_back(B);

		// Default Binding solution for block indices
		glUniformBlockBinding( m_Program->getProgram(), B->m_BlockIndex,B->m_BindingPoint);

	}

	glGetProgramiv(m_Program->getProgram(),GL_ACTIVE_UNIFORMS,&activeUniforms);

	for (GLuint i = 0;i < (GLuint) activeUniforms;i++)
	{
		char name[256];
		GLenum type;
		GLint size;
		glGetActiveUniform(m_Program->getProgram(),i,256,NULL,&size,&type,name);
		GLint blockIndex;
		glGetActiveUniformsiv( m_Program->getProgram(), 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
		GLint offset;
		glGetActiveUniformsiv( m_Program->getProgram(), 1, &i, GL_UNIFORM_OFFSET, &offset);
		GLint loc = -1;
		GLint uBuffer = -1;
		if (blockIndex != -1 )
			uBuffer = listBlocks[blockIndex]->getUniformBuffer();
		else
			loc = glGetUniformLocation(m_Program->getProgram(),name);
		
		
		GPUVariableBase *b = createGPUVariable(type,string(name),m_Program->getProgram(),uBuffer,offset,loc,size);

		if (b != NULL) // Try inserting the new variable into Manager list
			m_Uniforms.insert(make_pair(name,b));

	}



}

void GLUniformManager::mapBufferToBlock(GPUBuffer* ubo,std::string blockName)
{
	for (unsigned int i = 0;i < listBlocks.size();i++)
		if (listBlocks[i]->getName().compare(blockName) == 0)
			listBlocks[i]->attachBuffer(ubo);
}



void GLUniformManager::bindUniformBuffers()
{
	for(unsigned int i = 0;i < listBlocks.size();i++)
		listBlocks[i]->bindBuffer();
}


GPUVariableBase* GLUniformManager::createGPUVariable( GLenum type,string name,GLint program,GLuint uBuffer,GLint offset,GLint loc,GLint size)
{
	GPUVariableBase *v = NULL;
	GPUsampler *bb ;	
	GPUimage *bi ;
	switch (type)
	{
		case GL_FLOAT:
			v= new GPUfloat(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_FLOAT_VEC2:
			v= new GPUvec2(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_FLOAT_VEC3:
			v= new GPUvec3(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_FLOAT_VEC4:
			v= new GPUvec4(name,program,uBuffer,offset,loc,size,type);
			break;

		case GL_DOUBLE:
			v= new GPUdouble(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_DOUBLE_VEC2:
			v= new GPUdvec2(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_DOUBLE_VEC3:
			v= new GPUdvec3(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_DOUBLE_VEC4:
			v= new GPUdvec4(name,program,uBuffer,offset,loc,size,type);
			break;

		case GL_INT:
			v= new GPUint(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_INT_VEC2:
			v= new GPUivec2(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_INT_VEC3:
			v= new GPUivec3(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_INT_VEC4:
			v= new GPUivec4(name,program,uBuffer,offset,loc,size,type);
			break;

		case GL_FLOAT_MAT2:
			v= new GPUmat2(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_FLOAT_MAT3:
			v= new GPUmat3(name,program,uBuffer,offset,loc,size,type);
			break;
		case GL_FLOAT_MAT4:
			v= new GPUmat4(name,program,uBuffer,offset,loc,size,type);
			break;

		case GL_BOOL:
			v= new GPUbool(name,program,uBuffer,offset,loc,size,type);
			break;

			/************************************************************************** TODO : All GPU types *******************************************/
			  case GL_SAMPLER_1D:
			  case GL_SAMPLER_2D:
			  case GL_SAMPLER_3D:
			  case  GL_SAMPLER_CUBE:                        
			  case GL_SAMPLER_1D_SHADOW:                        
			  case GL_SAMPLER_2D_SHADOW:                        
			  case GL_SAMPLER_1D_ARRAY:                        
			  case GL_SAMPLER_2D_ARRAY:                        
			  case GL_SAMPLER_1D_ARRAY_SHADOW:                        
			  case GL_SAMPLER_2D_ARRAY_SHADOW:                        
			  case GL_SAMPLER_2D_MULTISAMPLE:                        
			  case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:                        
			  case GL_SAMPLER_CUBE_SHADOW:                        
			  case GL_SAMPLER_BUFFER:                        
			  case GL_SAMPLER_2D_RECT:                        
			  case GL_SAMPLER_2D_RECT_SHADOW:                        
			  case GL_INT_SAMPLER_1D:                        
			  case GL_INT_SAMPLER_2D:                        
			  case GL_INT_SAMPLER_3D:                        
			  case GL_INT_SAMPLER_CUBE:                        
			  case GL_INT_SAMPLER_1D_ARRAY:                        
			  case GL_INT_SAMPLER_2D_ARRAY:                        
			  case GL_INT_SAMPLER_2D_MULTISAMPLE:                        
			  case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:                        
			  case GL_INT_SAMPLER_BUFFER:                        
			  case  GL_INT_SAMPLER_2D_RECT:                        
			  case GL_UNSIGNED_INT_SAMPLER_1D:                        
			  case  GL_UNSIGNED_INT_SAMPLER_2D:                        
			  case GL_UNSIGNED_INT_SAMPLER_3D:                        
			  case GL_UNSIGNED_INT_SAMPLER_CUBE:                        
			  case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:                        
			  case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:                        
			  case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:                        
			  case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:                        
			  case GL_UNSIGNED_INT_SAMPLER_BUFFER:                        
			  case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
                  
				  v = NULL;          
			  bb = new GPUsampler(name,program,uBuffer,offset,loc,size,type);
			  // directly include in corresponding sampler List
			  listSamplers.insert(make_pair(name,bb));
				  
			  break;
			  /************************************************************************** TODO : All GPU types *******************************************/
			  case GL_IMAGE_1D:				  
			  case GL_IMAGE_2D:
			  case GL_IMAGE_3D:
			  case GL_IMAGE_1D_ARRAY:
			  case GL_IMAGE_2D_ARRAY:
 			  case GL_INT_IMAGE_1D:				  
			  case GL_INT_IMAGE_2D:
			  case GL_INT_IMAGE_3D:
			  case GL_INT_IMAGE_1D_ARRAY:
			  case GL_INT_IMAGE_2D_ARRAY:       
			  case GL_UNSIGNED_INT_IMAGE_1D:				  
			  case GL_UNSIGNED_INT_IMAGE_2D:
			  case GL_UNSIGNED_INT_IMAGE_3D:
			  case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
			  case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:  
				  v = NULL;
                           
			  bi = new GPUimage(name,program,uBuffer,offset,loc,size,type);
			  // directly include in corresponding sampler List
			  listImages.insert(make_pair(name,bi));
				  
			  break;

		default :
			LOG(WARNING) << m_Program->getName() << " :    GPU type for variable " << name << " not yet implemented in Uniform Manager." << endl;
			v = NULL;
			break;
	}

	return v;
}



GPUfloat* GLUniformManager::getGPUfloat(string name)
{
	GPUfloat* to_ret = dynamic_cast<GPUfloat*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a float. Returning dummy variable. "<<endl;
		
		to_ret = &m_Dfloat;
	}

	return to_ret;

}
GPUvec2* GLUniformManager::getGPUvec2(string name)
{
	GPUvec2* to_ret = dynamic_cast<GPUvec2*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a vec2. Returning dummy variable. "<<endl;
		to_ret = &m_Dvec2;
	}

	return to_ret;

}
GPUvec3* GLUniformManager::getGPUvec3(string name)
{
	GPUvec3* to_ret = dynamic_cast<GPUvec3*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a vec3. Returning dummy variable. "<<endl;
		to_ret = &m_Dvec3;
	}

	return to_ret;

}
GPUvec4* GLUniformManager::getGPUvec4(string name)
{
	GPUvec4* to_ret = dynamic_cast<GPUvec4*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a vec4. Returning dummy variable. ";
		to_ret = &m_Dvec4;
	}

	return to_ret;

}

GPUdouble* GLUniformManager::getGPUdouble(string name)
{
	GPUdouble* to_ret = dynamic_cast<GPUdouble*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a double. Returning dummy variable. "<<endl;
		to_ret = &m_Ddouble;
	}

	return to_ret;

}
GPUdvec2* GLUniformManager::getGPUdvec2(string name)
{
	GPUdvec2* to_ret = dynamic_cast<GPUdvec2*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dvec2. Returning dummy variable. "<<endl;
		to_ret = &m_Ddvec2;
	}

	return to_ret;

}
GPUdvec3* GLUniformManager::getGPUdvec3(string name)
{
	GPUdvec3* to_ret = dynamic_cast<GPUdvec3*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dvec3. Returning dummy variable. "<<endl;
		to_ret = &m_Ddvec3;
	}

	return to_ret;

}
GPUdvec4* GLUniformManager::getGPUdvec4(string name)
{
	GPUdvec4* to_ret = dynamic_cast<GPUdvec4*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dvec4. Returning dummy variable. "<<endl;
		to_ret = &m_Ddvec4;
	}

	return to_ret;

}
GPUint* GLUniformManager::getGPUint(string name)
{
	GPUint* to_ret = dynamic_cast<GPUint*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a int. Returning dummy variable. "<<endl;
		to_ret = &m_Dint;
	}

	return to_ret;

}
GPUivec2* GLUniformManager::getGPUivec2(string name)
{
	GPUivec2* to_ret = dynamic_cast<GPUivec2*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a ivec2. Returning dummy variable. "<<endl;
		to_ret = &m_Divec2;
	}

	return to_ret;

}
GPUivec3* GLUniformManager::getGPUivec3(string name)
{
	GPUivec3* to_ret = dynamic_cast<GPUivec3*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a ivec3. Returning dummy variable. "<<endl;
		to_ret = &m_Divec3;
	}

	return to_ret;

}
GPUivec4* GLUniformManager::getGPUivec4(string name)
{
	GPUivec4* to_ret = dynamic_cast<GPUivec4*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a ivec4. Returning dummy variable. "<<endl;
		to_ret = &m_Divec4;
	}

	return to_ret;

}

GPUmat2* GLUniformManager::getGPUmat2(string name)
{
	GPUmat2* to_ret = dynamic_cast<GPUmat2*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a mat2. Returning dummy variable. "<<endl;
		to_ret = &m_Dmat2;
	}

	return to_ret;

}
GPUmat3* GLUniformManager::getGPUmat3(string name)
{
	GPUmat3* to_ret = dynamic_cast<GPUmat3*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a mat3. Returning dummy variable. "<<endl;
		to_ret = &m_Dmat3;
	}

	return to_ret;

}
GPUmat4* GLUniformManager::getGPUmat4(string name)
{
	GPUmat4* to_ret = dynamic_cast<GPUmat4*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a mat4. Returning dummy variable. "<<endl;
		to_ret = &m_Dmat4;
	}

	return to_ret;

}
GPUdmat2* GLUniformManager::getGPUdmat2(string name)
{
	GPUdmat2* to_ret = dynamic_cast<GPUdmat2*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dmat2. Returning dummy variable. "<<endl;
		to_ret = &m_Ddmat2;
	}

	return to_ret;

}
GPUdmat3* GLUniformManager::getGPUdmat3(string name)
{
	GPUdmat3* to_ret = dynamic_cast<GPUdmat3*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dmat3. Returning dummy variable. "<<endl;
		to_ret = &m_Ddmat3;
	}

	return to_ret;

}
GPUdmat4* GLUniformManager::getGPUdmat4(string name)
{
	GPUdmat4* to_ret = dynamic_cast<GPUdmat4*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a dmat4. Returning dummy variable. "<<endl;
		to_ret = &m_Ddmat4;
	}

	return to_ret;

}
GPUbool* GLUniformManager::getGPUbool(string name)
{
	GPUbool* to_ret = dynamic_cast<GPUbool*> (getUniformBase(name));
	if (!to_ret)
	{
		LOG(WARNING) << m_Program->getName() << " :    Variable "<< name << " is not a bool. Returning dummy variable. "<<endl;
		to_ret = &m_Dbool;
	}

	return to_ret;

}




GPUsampler* GLUniformManager::getGPUsampler(string name)
{
	GPUsampler *to_ret = NULL;
	map<string , GPUsampler* >::iterator it = listSamplers.find(name);
	if (it != listSamplers.end())
		to_ret = (*it).second;
	else
	{
		LOG(WARNING) << m_Program->getName() << " :    There is no active sampler named " << name << ". Returning dummy sampler. " << endl;
		to_ret = &m_Dsampler;
	}


	return to_ret;

}
GPUimage* GLUniformManager::getGPUimage(string name)
{
	GPUimage *to_ret = NULL;
	map<string , GPUimage* >::iterator it = listImages.find(name);
	if (it != listImages.end())
		to_ret = (*it).second;
	else
	{
		LOG(WARNING) << m_Program->getName() << " :    There is no active sampler named " << name << ". Returning dummy sampler. " << endl;
		to_ret = &m_Dimage;
	}


	return to_ret;

}
