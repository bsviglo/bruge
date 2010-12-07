#ifndef _BR_OGLFRAGMENTPROGRAM_H_
#define _BR_OGLFRAGMENTPROGRAM_H_

#include "Utils/br_String.h"
#include "Utils/br_ParamProxy.h"

//forward declaration
class Data;

namespace brUGE{

	using namespace utils;

	namespace render{
		
		class brOGLFragmentProgram{
		public:
			brOGLFragmentProgram();
			brOGLFragmentProgram(const char *_fileName);
			~brOGLFragmentProgram();

			BRuint getId() const{
				return id;
			}

			brStr getErrorString() const{
				return errorString;
			}

			BRbool	hasError() const{
				return (errorCode ? true : false);
			}

			BRvoid bind();
			BRvoid unbind();
			BRvoid enable();
			BRvoid disable();
			BRbool load(Data *_data);
			BRbool load(const char *_fileName);

		public:
			brParamArray		local;
			BRuint				id;
			BRint				errorCode;
			brStr			errorString;
			static brParamArray env;
			static BRuint		activeProgramId;
		};

	}/*end namespace render*/
}/*end namespace brUGE*/

#endif/*_BR_OGLFRAGMENTPROGRAM_H_*/