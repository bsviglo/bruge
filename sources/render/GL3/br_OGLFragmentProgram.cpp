#include "Renderer/gl/br_OGLFragmentProgram.h"
#include "Renderer/gl/libext.h"
//#include "Renderer/gl/br_OGLRender.h"
#include "Utils/data.h"

namespace brUGE{
	namespace render{

		brParamArray brOGLFragmentProgram::env(GL_FRAGMENT_PROGRAM_ARB);
		BRuint		 brOGLFragmentProgram::activeProgramId = 0;	
		
		brOGLFragmentProgram::brOGLFragmentProgram()
			: local(GL_FRAGMENT_PROGRAM_ARB), id(0), errorString(""), errorCode(0)
		{

		}

		brOGLFragmentProgram::brOGLFragmentProgram(const char *_fileName)
			: local(GL_FRAGMENT_PROGRAM_ARB), id(0), errorString(""), errorCode(0)
		{
			load(_fileName);
		}

		brOGLFragmentProgram::~brOGLFragmentProgram(){

		}

		BRvoid brOGLFragmentProgram::bind(){
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, activeProgramId = id);
		}

		BRvoid brOGLFragmentProgram::unbind(){
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, activeProgramId = 0);
		}

		BRvoid brOGLFragmentProgram::enable(){
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
		}

		BRvoid brOGLFragmentProgram::disable(){
			glDisable(GL_FRAGMENT_PROGRAM_ARB);

		}

		BRbool brOGLFragmentProgram::load(Data *_data){
			if(id == 0){
				glGenProgramsARB(1, &id);
			}
			if(id == 0){
				FATAL_F("[Fragment Shader] creating fragment program failed.");
				return false;
			}
			
			local.setPointers(glGetProgramLocalParameterfvARB, glProgramLocalParameter4fvARB);
			local.setId(id);
			env.setPointers(glGetProgramEnvParameterfvARB, glProgramEnvParameter4fvARB);

			if (!_data->isOk() || _data->isEmpty()){
				FATAL_F("[Fragment Shader] creating fragment program failed.");
				return false;
			}

			bind ();

			glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
				_data->getLength(), _data->getPtr());

			if(glGetError() == GL_INVALID_OPERATION)
			{
				glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorCode);

				errorString = (const char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
				FATAL_F("[Fragment Shader] creating fragment program failed.");
				return false;
			}

			errorCode   = 0;
			errorString = "";
			LOG_F("[Fragment Shader] Fragment program (FP) #%d: created.", id);
			return true;
		}

		BRbool brOGLFragmentProgram::load(const char *_fileName){
			Data data(_fileName);
			return load(&data);
		}

	}/*end namespace render*/
}/*end namespace brUGE*/