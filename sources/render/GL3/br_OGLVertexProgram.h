#ifndef _BR_OGL_VERTEXPROGRAM_H_
#define _BR_OGL_VERTEXPROGRAM_H_

#include "br_Types.h"
#include "Utils/br_String.h"
#include "Utils/br_ParamProxy.h"

//forward declaration
class Data;

namespace brUGE{
	namespace render{

	using namespace brUGE::utils;

		class brOGLVertexProgram{
		public:
			/**
			 *	local variables for given vertex shader
			 *  Uses for parametrize given vertex shader
			 */
			brParamArray local;
			/**
			 *	global variables for all vertex programs
			 *  environment's variables is general for all vertex shader's 
			 *	Use for parametrize all vertex shader's in given application
			 */
			static brParamArray env;

			//functions 

			/**
			 *	Get current binding vertex program in given application and OGL context
			 */
			static BRuint getCurrentProgram();

			/**
			 *	Get max vertex attributes available in vertex program
			 */
			static BRint  getMaxVertexAttributes();

			/**
			 *	Get max vertex local parameters available in vertex program
			 */
			static BRint  getMaxLocalParameters();

			/**
			 *	Get max vertex environment parameters available in all
			 *  vertex programs given application
			 */
			static BRint  getMaxEnvParameters();

			/**
			 *	Get max matrices available in vertex program
			 */
			static BRint  getMaxMatrices();

			/**
			 *	Get max temporaries variable available in vertex program
			 */
			static BRint  getMaxTemporaries();

			/**
			 *	Get max parameters available in vertex program
			 */
			static BRint  getMaxParameters();

			/**
			 *	Get  max address registers available in vertex program
			 */
			static BRint  getMaxAdressRegisters();

			/**
			 *	Check for support vertex program
			 */
			static BRbool isSupported();

			brOGLVertexProgram();
			brOGLVertexProgram(const char *_shaderFileName);
			~brOGLVertexProgram();

			BRuint getId() const{
				return id;
			}

			brStr getErrorString() const{
				return errorString;
			}

			BRbool hasError() const{
				return (errorCode ? true : false);
			}

			/**
			 *	Bind given vertex program as current
			 */
			BRvoid bind();

			/**
			 *	Unbind given vertex program as current
			 */
			BRvoid unbind();

			/**
			 *	Enable vertex program
			 */
			BRvoid enable();
			
			/**
			 *	Disable vertex program
			 */
			BRvoid disable();

			/**
			 *	Load vertex program from memory
			 */
			BRbool load(Data *_data);

			/**
			 * Load vertex program from text file
			 */
			BRbool load(const char *_shaderFileName);

		protected:
			static		BRuint activeProgramId;
			BRuint		id;
			BRint		errorCode;
			brStr	errorString;
		    
		};

	}/*end namespace render*/
}/*end namespace brUGE*/

#endif /*_BR_OGL_VERTEXPROGRAM_H_*/
/*
	* 25/10/2007
		created.
*/