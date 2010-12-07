#include "br_GLSLShader.h"
#include "Utils/data.h"
#include "Math/br_Matrix_ops.h"


namespace brUGE{
	namespace render{

		using namespace math;
		using namespace utils;
		
		brGLSLShader::brGLSLShader() : errorString(""), successful(false), program(0),
			vertexShader(0), fragmentShader(0)
		{

		}

		brGLSLShader::~brGLSLShader(){

		}

		BRbool brGLSLShader::loadShaders(const brStr &_vertexFileName,
			const brStr &_fragmentFileName)
		{
			successful = false;
			Data vertexData(_vertexFileName.c_str());

			if(!vertexData.isOk() || vertexData.isEmpty()){
				FATAL("[GLSL Shader] Error loading vertex shader");
				throw brRenderException("[GLSL Shader] Error loading vertex shader");
			}

			Data fragmentData(_fragmentFileName.c_str());

			if(!fragmentData.isOk() || fragmentData.isEmpty()){
					FATAL("[GLSL Shader] Error loading fragment shader");
					throw brRenderException("[GLSL Shader] Error loading fragment shader");
			}

			bool result = loadShaders(&vertexData, &fragmentData);
			return result;
		}

		BRbool brGLSLShader::loadShaders(Data *_vertexShaderData,
			Data *_fragmentShaderData)
		{
			successful = false;
			// check whether we should create program object
			if(program == 0){
				program = glCreateProgramObjectARB();
			}

			// check for errors
			if(!::checkGlError(errorString)){
				return false;
			}

			// create a vertex shader object and a fragment shader object
			if(_vertexShaderData != NULL)
			{
				vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

				LOG("[GLSL Shader] Loading vertex shader");

				if(!loadShader(vertexShader, _vertexShaderData)){
					return false;
				}

				// attach shaders to program object
				glAttachObjectARB(program, vertexShader);
			}

			if(_fragmentShaderData != NULL)
			{
				fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

				LOG("[GLSL Shader] Loading fragment shader");

				if(!loadShader(fragmentShader, _fragmentShaderData))
					return false;
				// attach shaders to program object
				glAttachObjectARB(program, fragmentShader);
			}

			GLint   linked;

			LOG("[GLSL Shader] Linking programs");

			// link the program object and print out the info log
			glLinkProgramARB(program);

			if(!::checkGlError(errorString)){     // check for errors
				return false;
			}

			glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);

			loadLog(program);

			if(!linked){
				return false;
			}

			return successful = true;
		}

		BRbool brGLSLShader::loadShader(GLhandleARB _shader, Data *_data){
			const char * body = (const char *)_data->getPtr(0);
			BRint		 len  = _data->getLength();
			GLint        compileStatus;

			glShaderSourceARB(_shader, 1, &body,  &len);

			// compile the particle vertex shader, and print out
			glCompileShaderARB(_shader);

			if(!::checkGlError(errorString)){              // check for OpenGL errors
				return false;
			}

			glGetObjectParameterivARB(_shader, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);

			loadLog(_shader);

			return compileStatus != 0;
		}

		BRvoid brGLSLShader::loadLog(GLhandleARB _object){
			BRint       logLength     = 0;
			BRint       charsWritten  = 0;
			GLcharARB   buffer [2048];
			GLcharARB * infoLog;

			glGetObjectParameterivARB(_object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
			if(!checkGlError(errorString)){          // check for OpenGL errors
				return;
			}

			if(logLength < 1){
				return;
			}

			// try to avoid allocating buffer
			if(logLength > sizeof(buffer)){
				infoLog = (GLcharARB*)malloc(logLength);
				if(infoLog == NULL){
					WARNING("[GLSL Shader] Could not allocate log buffer.");
					return;
				}
			}else{
				infoLog = buffer;
			}

			glGetInfoLogARB(_object, logLength, &charsWritten, infoLog);
			errorString = infoLog;
			if(infoLog != buffer){
				free(infoLog);
			}
		}

		BRvoid brGLSLShader::bind(){
			glUseProgramObjectARB(program);
		}

		BRvoid brGLSLShader::unbind(){
			glUseProgramObjectARB(0);
		}

		BRvoid brGLSLShader::clear(){
			glDeleteObjectARB(program);                   // it will also detach shaders
			glDeleteObjectARB(vertexShader);
			glDeleteObjectARB(fragmentShader);

			program        = 0;
			vertexShader   = 0;
			fragmentShader = 0;
			successful     = false;
		}

		BRbool brGLSLShader::setUniformVector4(const brStr &_name, const vec4f &_value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniform4fvARB(loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformVector4(BRint _loc, const vec4f &_value){
			glUniform4fvARB(_loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformVector3(const brStr &_name, const vec3f &_value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniform3fvARB(loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformVector3(BRint _loc, const vec3f &_value){
			glUniform3fvARB(_loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformVector2(const brStr &_name, const vec2f &_value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniform2fvARB(loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformVector2(BRint _loc, const vec2f &_value){
			glUniform2fvARB(_loc, 1, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformFloat(const brStr &_name, BRfloat _value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniform1fARB(loc, _value);
			return true;
		}

		BRbool brGLSLShader::setUniformFloat(BRint _loc, BRfloat _value){
			glUniform1fARB(_loc, _value);
			return true;
		}

		BRbool brGLSLShader::setUniformInt(const brStr &_name, BRint _value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniform1iARB(loc, _value);
			return true;
		}

		BRbool brGLSLShader::setUniformInt(BRint _loc, BRint _value){
			glUniform1iARB(_loc, _value);
			return true;
		}

		BRbool brGLSLShader::setUniformMatrix4(const brStr &_name, const mat4X4 &_value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniformMatrix4fvARB(loc, 1, GL_FALSE, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformMatrix3(const brStr &_name, const mat3X3 &_value){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniformMatrix3fvARB(loc, 1, GL_FALSE, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setUniformMatrix4(const brStr &_name, BRfloat _value[16]){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				return false;
			}

			glUniformMatrix4fvARB(loc, 1, GL_FALSE, _value);
			return true;
		}
		
		vec4f  brGLSLShader::getUniformVector(const brStr &_name){
			BRfloat values[4];
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc < 0){
				vec4f vec;
				set_zero(vec);
				return vec;
			}

			glGetUniformfvARB(program, loc, values);
			return init_vec4(values[0], values[1], values[2], values[3]);
		}

		vec4f  brGLSLShader::getUniformVector(BRint _loc){
			BRfloat values[4];
			if(_loc < 0){
				vec4f vec;
				set_zero(vec);
				return vec;
			}

			glGetUniformfvARB(program, _loc, values);
			return init_vec4(values[0], values[1], values[2], values[3]);
		}

		BRint  brGLSLShader::locForUniformName(const brStr &_name){
			return glGetUniformLocationARB(program, _name.c_str());
		}

		// attribute variables handling methods
		BRbool brGLSLShader::setAttribute(const brStr &_name, const vec4f &_value){
			BRint index = glGetAttribLocationARB(program, _name.c_str());
			if(index < 0){
				return false;
			}

			glVertexAttrib4fvARB(index, _value.pointer());
			return true;
		}

		BRbool brGLSLShader::setAttribute(BRint _index, const vec4f &_value){
			glVertexAttrib4fvARB(_index, _value.pointer());
			return true;
		}

		vec4f  brGLSLShader::getAttribute(const brStr &_name){
			BRint index = glGetAttribLocationARB(program, _name.c_str());
			if(index < 0){
				return init_vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}

			BRfloat buf[4];
			glGetVertexAttribfvARB(index, GL_CURRENT_VERTEX_ATTRIB_ARB, buf);
			return init_vec4(buf[0], buf[1], buf[2], buf[3]);
		}

		vec4f  brGLSLShader::getAttribute(BRint _index){
			BRfloat buf[4];
			glGetVertexAttribfvARB(_index, GL_CURRENT_VERTEX_ATTRIB_ARB, buf);
			return init_vec4(buf[0], buf[1], buf[2], buf[3]);
		}

		BRint  brGLSLShader::indexForAttrName(const brStr &_name){
			return glGetAttribLocationARB(program, _name.c_str());
		}

		BRbool brGLSLShader::bindAttributeTo(BRint _loc, const brStr &_name){
			glBindAttribLocationARB(program, _loc, _name.c_str());
			return true;
		}

		BRbool brGLSLShader::setTexture(const brStr &_name, BRint _texUnit){
			BRint loc = glGetUniformLocationARB(program, _name.c_str());
			if(loc == -1){
				return false;
			}

			glUniform1iARB(loc, _texUnit);
			return true;
		}

		BRbool brGLSLShader::setTexture(BRint _loc, BRint _texUnit){
			if(_loc < 0){
				return false;
			}

			glUniform1iARB(_loc, _texUnit);
			return true;
		}

		BRbool brGLSLShader::isSupported(){
			return  isExtensionSupported("GL_ARB_shading_language_100") &&
					isExtensionSupported("GL_ARB_shader_objects"      ) &&
					isExtensionSupported("GL_ARB_vertex_shader"       ) &&
					isExtensionSupported("GL_ARB_fragment_shader"     );
		}

		brStr brGLSLShader::version(){
			const char * slVer = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
			if(glGetError() != GL_NO_ERROR){
				return "1.051";
			}
			return brStr(slVer);
		}

		// some limitations on program
		BRint brGLSLShader::maxVertexUniformComponents(){
			BRint maxVertexUniformComponents;
			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, &maxVertexUniformComponents);
			return maxVertexUniformComponents;
		}

		BRint brGLSLShader::maxVertexAttribs(){
			BRint maxVertexAttribs;
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &maxVertexAttribs);
			return maxVertexAttribs;
		}

		BRint brGLSLShader::maxFragmentTextureUnits(){
			BRint maxFragmentTextureUnits;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &maxFragmentTextureUnits);
			return maxFragmentTextureUnits;
		}

		BRint brGLSLShader::maxVertexTextureUnits(){
			BRint maxVertexTextureUnits;
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &maxVertexTextureUnits);
			return maxVertexTextureUnits;
		}

		BRint brGLSLShader::maxCombinedTextureUnits(){
			BRint maxCombinedTextureUnits;
			glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &maxCombinedTextureUnits);
			return maxCombinedTextureUnits;
		}

		BRint brGLSLShader::maxVaryingFloats(){
			BRint maxVaryingFloats;
			glGetIntegerv(GL_MAX_VARYING_FLOATS_ARB, &maxVaryingFloats);
			return maxVaryingFloats;
		}

		BRint brGLSLShader::maxFragmentUniformComponents(){
			BRint maxFragmentUniformComponents;
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB, &maxFragmentUniformComponents);
			return maxFragmentUniformComponents;
		}

		BRint brGLSLShader::maxTextureCoords(){
			BRint maxTextureCoords;
			glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &maxTextureCoords);
			return maxTextureCoords;
		}
	}/*end namespace render*/
}/*end namespace brUGE*/
