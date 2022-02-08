/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#include "Engine/OpenGL/v4/GLProgramSource.h"
#include "Engine/OpenGL/Managers/textfile.h"
#include <string.h>

// for static init
map<string, const char*> GLProgramSource::m_IncludeSource;
bool GLProgramSource::noIncludes = true;

GLProgramSource::GLProgramSource(std::string name)
{

	m_Program = 0;
	info_text = "";
	loaded = false;
}
GLProgramSource::~GLProgramSource()
{

	glDeleteProgram(m_Program);
}

void GLProgramSource::createNamedString(string name, string filename)
{
	string pfn(filename.c_str());
	char *shadertxt = textFileRead((char*)filename.c_str());

	const char *shadertxtc = shadertxt;
	// besoin d'un stockage des sources dans un map <name,shaderSource> pour un eventuel remplacement dans les shaders
	if (GLEW_ARB_shading_language_include)
	{
		glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, name.c_str(), -1, shadertxtc);
	}
	else
	{
		m_IncludeSource.insert(make_pair(name, shadertxtc));
		noIncludes = false;
	}
}

void GLProgramSource::deleteNamedString(string name)
{
	if (GLEW_ARB_shading_language_include)
	{
		glDeleteNamedStringARB(-1, name.c_str());
	}
	else
	{
		if (!noIncludes)
		{
			// Pour le moment on désactive : erreur de mémoire à la fermeture de la fenêtre
			//if (GLProgramSource::m_IncludeSource.find(name) != GLProgramSource::m_IncludeSource.end())
			//{
			//	GLProgramSource::m_IncludeSource.erase(GLProgramSource::m_IncludeSource.find(name));
			//}
			//if (GLProgramSource::m_IncludeSource.size() == 0) noIncludes = true;
		}

	}
}

void GLProgramSource::parseShader(std::string parsedSource)
{
	string extensionTest = string("#extension GL_ARB_shading_language_include : enable");
	string include = string("#include");

	std::size_t found = 0;
	found = parsedSource.find(extensionTest);
	if(found != std::string::npos)
	{
		parsedSource.erase(found,extensionTest.size());
	}

	found = 0;
	std::size_t lastPosition = 0;
	bool parsing = true;
	while (parsing)
	{

		string* currentlyParsed = new string();
		string toSearch = string("#include \""); // searching the include
		found = parsedSource.find(toSearch, found);
		if (found != std::string::npos)
		{
			// storing the current string in array
			int currentLength = found - lastPosition - 1;
			if (currentLength != 0)
			{
				*currentlyParsed = (parsedSource.substr(lastPosition, currentLength));
				currentlyParsed->append("\0");
				listOfSources.push_back(currentlyParsed->c_str());
				listOfLength.push_back(currentlyParsed->length());
			}
			// Search the end of the current include
			std::size_t nextLineAt = parsedSource.find("\"", found + 11);
			string currentInclude = parsedSource.substr(found + 10, nextLineAt - (found + 10));
			std::map<string, const char*>::iterator currentIncludeIt = m_IncludeSource.find(currentInclude);
			if (currentIncludeIt != m_IncludeSource.end())
			{
				parseShader(string(currentIncludeIt->second));
				// storing the include's source in the source array
				//listOfSources.push_back(currentIncludeIt->second);
				//listOfLength.push_back(strlen(currentIncludeIt->second));
			}
			// nextline to parse
			lastPosition = parsedSource.find("\n", found);
			found = lastPosition;

		}
		else
		{
			// nothing else to parse : storing the rest of the source file
			*currentlyParsed = parsedSource.substr(lastPosition);
			currentlyParsed->append("\0");
			listOfSources.push_back(currentlyParsed->c_str());
			listOfLength.push_back(currentlyParsed->length());
			parsing = false;
		}
	}
}

bool GLProgramSource::createProgram(GLenum shaderType, std::string filename)
{
	setFilename(filename);
	this->shaderType = shaderType;
	char *shadertxt;
	string pfn(filename.c_str());
	shadertxt = textFileRead((char*)filename.c_str());
	shaderSource = string(shadertxt);
	if (shadertxt == NULL)
		throw std::logic_error(string("ERROR : GLProgram : Error Reading Source File\n") + filename + string("\n"));



	const GLuint shader = glCreateShader(shaderType);

	if (GLEW_ARB_shading_language_include)
	{
		const char *shadertxtc = shadertxt;
		glShaderSource(shader, 1, &shadertxtc, NULL);
		string rootPath = "/";
		const char *SourceString = rootPath.c_str();
		glCompileShaderIncludeARB(shader, 1, &SourceString, NULL);

	}
	else
	{
		parseShader(shaderSource);

		glShaderSource(shader, listOfLength.size(), &listOfSources[0], &listOfLength[0]);

		glCompileShader(shader);
	}

	m_Program = glCreateProgram();

	if (m_Program) {
		GLint compiled = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		glProgramParameteri(m_Program, GL_PROGRAM_SEPARABLE, GL_TRUE);
		if (compiled) {
			glAttachShader(m_Program, shader);
			glLinkProgram(m_Program);
			glDetachShader(m_Program, shader);
		}
		else
		{
			printShaderInfoLog(shader);
		}
	}
	glDeleteShader(shader);

	free(shadertxt);

	loaded = isValid();
	if (!loaded)
	{
		printProgramInfoLog();
		string err = printErrorString();
		throw std::logic_error(err);
	}
	return loaded;

}



void GLProgramSource::printProgramInfoLog()
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(m_Program, infologLength, &charsWritten, infoLog);
		string df(infoLog);
		info_text += df ;
		info_text += "\n";
		free(infoLog);
	}
}
void GLProgramSource::printShaderInfoLog(GLuint shader)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		string df(infoLog);
		info_text += df ;
		info_text += "\n";
		free(infoLog);
	}
}
string GLProgramSource::printErrorString()
{
	string err;
	err += "ERROR : GLProgramSource : Error Creating ";
	if (shaderType==GL_VERTEX_SHADER)
		err += "Vertex Shader ";
	else if (shaderType==GL_TESS_CONTROL_SHADER)
		err += "Tesselation Control Shader ";
	else if (shaderType==GL_TESS_EVALUATION_SHADER)
		err += "Tesselation Evaluation Shader ";
	else if (shaderType==GL_GEOMETRY_SHADER)
		err += "Geometry Shader ";
	else if (shaderType==GL_FRAGMENT_SHADER)
		err += "Fragment Shader ";
	else if (shaderType==GL_COMPUTE_SHADER)
		err += "Compute Shader ";
	else
		err += "Shader ";

	err += m_Filename ;
	err += " : \n";
	err += info_text;
	return err;
}


bool GLProgramSource::isValid()
{
	GLint status;
	glGetProgramiv(m_Program,GL_VALIDATE_STATUS,&status);
	return(status==GL_TRUE);
}
GLuint GLProgramSource::getProgram()
{
	return(m_Program);
}
void GLProgramSource::setFilename(string pathfilename)
{
	m_Filename = pathfilename;
}
