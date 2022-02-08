#ifndef _GPU_VARIABLE_
#define _GPU_VARIABLE_
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
using namespace std;


class GPUVariableBase
{
public:
	 GPUVariableBase(string name,GLint program,GLuint UBuffer,GLuint offset,GLuint loc,GLuint size,GLenum type)
	 {
		m_Name = name;
		m_Program = program;
		m_Offset = offset;
		m_UBuffer = UBuffer;
		m_Loc = loc;
		m_Size = size;
		m_Type = type;


	 }
	 GPUVariableBase(){};
	~GPUVariableBase(){};
	virtual void Set(void* ta){};
	GLenum getType() {return m_Type;};

	GLuint getLoc(){return m_Loc;};

	std::string getName(){return m_Name;};
	
protected:
	string m_Name;
	GLint m_Program;
	GLint m_Offset;
	GLint m_UBuffer;
	GLint m_Loc;
	GLint m_Size;
	void* m_Value;
	GLenum m_Type;
};

template <typename T> class GPUVariable : public GPUVariableBase	
{
public:
	  GPUVariable(string name,GLint program,GLuint UBuffer,GLuint offset,GLuint loc,GLuint size,GLenum type):
	  GPUVariableBase(name,program,UBuffer,offset,loc,size,type)
	 {		
	 }
	  GPUVariable(){};
	  
	  void Set(const T & ta) // optimisation nécessaire pour glm : référence vers un objet non modifiable
	  {
			T val = ta;
			m_Value = val;
			if (m_UBuffer != -1)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, m_UBuffer);
				glBufferSubData(GL_UNIFORM_BUFFER, m_Offset, sizeof(T),   &m_Value);
			}
	  };
	  void Setv(int num,T *ta)	// Set an array of num values
	  {
			T* val = ta;
			if (m_UBuffer != -1)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, m_UBuffer);
				glBufferSubData(GL_UNIFORM_BUFFER, m_Offset, sizeof(T)*num,  val);	
			}
			
	  };
	  void Set()
	  {

	  }
	
	T getValue()
	{
		return m_Value;
	}
private:
	T m_Value;
};

class GPUsampler : public GPUVariableBase	
{
public:
	  GPUsampler(string name,GLint program,GLuint UBuffer,GLuint offset,GLuint loc,GLuint size,GLenum type):
	  GPUVariableBase(name,program,UBuffer,offset,loc,size,type)
	 {		

		 // get default value in case of explicit binding of uniforms in shader

		 GLint param = -1;
		 if (m_UBuffer == -1)
		 {
		  	glGetUniformiv(program,loc,&param);
			m_Value = param;
		 }
	 }
	  GPUsampler(){};
	  
	  void Set(GLuint64 t)
	  {			 
	  
		if (m_UBuffer != -1)
		{	
			glBindBuffer(GL_UNIFORM_BUFFER, m_UBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, m_Offset, sizeof(t),   &t);
			m_Value = t;
		}
		else if (t > 100)
		{
#if !defined __APPLE__
			glProgramUniformHandleui64NV(m_Program, m_Loc, t);
			m_Value = t;
#endif
		}
		else
		{
			glProgramUniform1i(m_Program, m_Loc, (GLint)t);
			m_Value = (GLuint)t;
		}
	  };

	  void Set()
	  {			 
	  	if (m_Loc != -1)
	  	{
#if !defined __APPLE__
			if (m_Value > 30)
				glProgramUniformHandleui64NV(m_Program, m_Loc, m_Value);
			else
#endif
				glProgramUniform1i(m_Program, m_Loc, (GLint)m_Value);
		}
	  };
	GLuint64 getValue()
	{
		return m_Value;
	}
private:
	GLuint64 m_Value;	
};

class GPUimage : public GPUVariableBase	
{
public:
	  GPUimage(string name,GLint program,GLuint UBuffer,GLuint offset,GLuint loc,GLuint size,GLenum type):
	  GPUVariableBase(name,program,UBuffer,offset,loc,size,type)
	 {		
	 }
	  GPUimage(){};

	  void Set(GLuint ta)
	  {			
			m_Value = ta;			
	  };
	  void Setv(int num,GLuint *ta)	// Unused
	  {
						
	  };
	
	GLuint getValue()
	{
		return m_Value;
	}
private:
	GLuint m_Value;
};




typedef  GPUVariable<float> GPUfloat;
typedef  GPUVariable<glm::vec2> GPUvec2;
typedef  GPUVariable<glm::vec3> GPUvec3;
typedef  GPUVariable<glm::vec4> GPUvec4;

typedef  GPUVariable<double> GPUdouble;
typedef  GPUVariable<glm::dvec2> GPUdvec2;
typedef  GPUVariable<glm::dvec3> GPUdvec3;
typedef  GPUVariable<glm::dvec4> GPUdvec4;

typedef  GPUVariable<int> GPUint;
typedef  GPUVariable<glm::ivec2> GPUivec2;
typedef  GPUVariable<glm::ivec3> GPUivec3;
typedef  GPUVariable<glm::ivec2> GPUivec4;

typedef  GPUVariable<glm::mat2> GPUmat2;
typedef  GPUVariable<glm::mat3> GPUmat3;
typedef  GPUVariable<glm::mat4> GPUmat4;

typedef  GPUVariable<glm::dmat2> GPUdmat2;
typedef  GPUVariable<glm::dmat3> GPUdmat3;
typedef  GPUVariable<glm::dmat4> GPUdmat4;
typedef GPUVariable<int> GPUbool;
#endif
