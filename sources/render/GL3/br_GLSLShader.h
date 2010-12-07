#ifndef _BR_GLSLSHADER_H_
#define _BR_GLSLSHADER_H_

#include "render/br_RenderCommon.h"

class Data;

namespace brUGE
{
	namespace render
	{

		class brGLSLShader{
		public:
			brGLSLShader();
			~brGLSLShader();

			BRvoid bind();
			BRvoid unbind();

			BRbool loadShaders(const brStr &_vertexFileName,
				const brStr &_fragmentFileName) throw(brRenderException);
			BRbool loadShaders(Data *_vertexShaderData,
				Data *_fragmentShaderData) throw(brRenderException);

			BRvoid clear() throw(brRenderException);

			BRbool setUniformVector4(const brStr &_name, const vec4f &_value);
			BRbool setUniformVector4(BRint _loc, const vec4f &_value);
			BRbool setUniformVector3(const brStr &_name, const vec3f &_value);
			BRbool setUniformVector3(BRint _loc, const vec3f &_value);
			BRbool setUniformVector2(const brStr &_name, const vec2f &_value);
			BRbool setUniformVector2(BRint _loc, const vec2f &_value);
			BRbool setUniformFloat(const brStr &_name, BRfloat _value);
			BRbool setUniformFloat(BRint _loc, BRfloat _value);
			BRbool setUniformMatrix4(const brStr &_name, const mat4X4 &_value);
			BRbool setUniformMatrix3(const brStr &_name, const mat3X3 &_value);
			BRbool setUniformMatrix4(const brStr &_name, BRfloat _value[16]);
			BRbool setUniformInt(const brStr &_name, BRint _value);
			BRbool setUniformInt(BRint _loc, BRint _value);
			vec4f  getUniformVector(const brStr &_name);
			vec4f  getUniformVector(BRint _loc);
			BRint  locForUniformName(const brStr &_name);

			// attribute variables handling methods
			BRbool setAttribute(const brStr &_name, const vec4f &_value);
			BRbool setAttribute(BRint _index, const vec4f &_value);
			vec4f  getAttribute(const brStr &_name);
			vec4f  getAttribute(BRint _index);
			BRint  indexForAttrName(const brStr &_name);
			BRbool bindAttributeTo(BRint _loc, const brStr &_name);

			BRbool setTexture(const brStr &_name, BRint _texUnit);
			BRbool setTexture(BRint _loc, BRint _texUnit);

			static  BRbool isSupported();
			static  brStr version();

			// some limitations on program
			static  BRint maxVertexUniformComponents();
			static  BRint maxVertexAttribs();
			static  BRint maxFragmentTextureUnits();
			static  BRint maxVertexTextureUnits();
			static  BRint maxCombinedTextureUnits();
			static  BRint maxVaryingFloats();
			static  BRint maxFragmentUniformComponents();
			static  BRint maxTextureCoords();
		protected:
			BRbool loadShader(GLhandleARB _shader, Data *_data);
			BRvoid loadLog(GLhandleARB _object);
		private:
			GLhandleARB		program;
			GLhandleARB		vertexShader;
			GLhandleARB		fragmentShader;
			brStr		errorString;
			BRbool			successful;
		};
		
	}/*end namespace render*/
}/*end namespace brUGE*/

#endif/*_BR_GLSLSHADER_H_*/