#include "Renderer/GL/br_OGLVertexProgram.h"
#include "Renderer/GL/libExt.h"
#include "Utils/Data.h"


namespace brUGE{
namespace render{

//static definition
brParamArray brOGLVertexProgram::env(GL_VERTEX_PROGRAM_ARB);
BRuint		 brOGLVertexProgram::activeProgramId = 0;

//
//Constructor
//
brOGLVertexProgram::brOGLVertexProgram() : local(GL_VERTEX_PROGRAM_ARB),
										   id(0),
										   errorCode(0),
										   errorString(""){
    
} 

//
//Constructor
//
brOGLVertexProgram::brOGLVertexProgram(const char *_shaderFileName) : local(GL_VERTEX_PROGRAM_ARB),
																	  id(0),
																	  errorCode(0),
																	  errorString(""){	
	load(_shaderFileName);
}

//
//Destructor
//
brOGLVertexProgram::~brOGLVertexProgram(){
	glDeleteProgramsARB(1, &id);
}

BRbool brOGLVertexProgram::load(const char *_shaderFileName){
	Data data(_shaderFileName);
	return load(&data);
}

BRbool brOGLVertexProgram::load(Data *_data){
	if(id == 0){
		glGenProgramsARB(1, &id);
	}
	if(id == 0){
		FATAL_F("[Vertex Shader] creating vertex program failed.");
		return false;
	}

	local.setPointers(glGetProgramLocalParameterfvARB, glProgramLocalParameter4fvARB);
	local.setId(id);
	env.setPointers(glGetProgramEnvParameterfvARB, glProgramEnvParameter4fvARB);

	if(!_data->isOk() && _data->isEmpty()){
		FATAL_F("[Vertex Shader] creating vertex program failed.");
		return false;
	}
	
	//bind current vertex program
	bind();

	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		_data->getLength(), _data->getPtr());

	if(glGetError() == GL_INVALID_OPERATION){
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorCode);

		errorString = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		FATAL_F("[Vertex Shader] creating vertex program failed.");
		return false;
	}

	errorCode = 0;
	errorString = "";
	LOG_F("[Vertex Shader] Vertex program (VP) #%d: created.", id);
    return true;
}

BRvoid brOGLVertexProgram::bind(){
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, activeProgramId = id);	
}

BRvoid brOGLVertexProgram::unbind(){
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, activeProgramId = 0);
}

BRvoid brOGLVertexProgram::enable(){
	glEnable(GL_VERTEX_PROGRAM_ARB);
}

BRvoid brOGLVertexProgram::disable(){
	glDisable(GL_VERTEX_PROGRAM_ARB);
}

BRuint brOGLVertexProgram::getCurrentProgram(){
    return activeProgramId;
}

BRint brOGLVertexProgram::getMaxVertexAttributes(){
	BRint maxVertAttr;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_VERTEX_ATTRIBS_ARB, &maxVertAttr);
	return maxVertAttr;
}

BRint brOGLVertexProgram::getMaxLocalParameters(){
	BRint maxLocalParams;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &maxLocalParams);
	return maxLocalParams;
}

BRint brOGLVertexProgram::getMaxEnvParameters(){
	BRint maxEnvParams;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &maxEnvParams);
	return maxEnvParams;
}

BRint brOGLVertexProgram::getMaxAdressRegisters(){
	BRint maxAddrRegists;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, &maxAddrRegists);
	return maxAddrRegists;
}

BRint brOGLVertexProgram::getMaxMatrices(){
	BRint maxMatrices;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_MATRICES_ARB, &maxMatrices);
	return maxMatrices;
}

BRint brOGLVertexProgram::getMaxParameters(){
	BRint maxParameters;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB, &maxParameters);		
	return maxParameters;
}

BRint brOGLVertexProgram::getMaxTemporaries(){
	BRint maxTemporaries;
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, &maxTemporaries);
	return maxTemporaries;
}

BRbool brOGLVertexProgram::isSupported(){
	return isExtensionSupported("GL_ARB_vertex_program");
}

}/*end namespace render*/
}/*end namespace brUGE*/