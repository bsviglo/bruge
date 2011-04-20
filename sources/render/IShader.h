#pragma once

#include "render_common.h"
#include "utils/TernaryTree.h"
#include "math/math_all.hpp"
#include <vector>

namespace brUGE
{
namespace render
{

	//-- shader macro struct.
	//----------------------------------------------------------------------------------------------
	struct ShaderMacro
	{
		const char* name;
		const char* value;
	};


	//-- shader include interface.
	//----------------------------------------------------------------------------------------------
	class IShaderInclude : public NonCopyable
	{
	public:
		IShaderInclude() { }
		virtual ~IShaderInclude() = 0 { }

		virtual bool open (const char* name, const void*& data, uint& size) = 0;
		virtual bool close(const void*& data) = 0;
	};


	//-- Note: By-defaults all the uniforms blocks in the different stages with the same name are shared.
	//--	   If you want to make the different uniform blocks or to update it independently You have
	//--	   to make their with the different names. 		
	//----------------------------------------------------------------------------------------------
	class IShader : public utils::RefCount, public NonCopyable
	{
	public:
		enum EShaderType
		{
			VS, GS, PS, SHADER_TYPES_COUNT
		};

	public:
		//-- retrieve handles by names.
		Handle	getHandleBool			(const char* name) const									{ return doGetHandleBool(name);	}
		Handle	getHandleFloat			(const char* name) const									{ return doGetHandleFloat(name); }
		Handle	getHandleInt			(const char* name) const									{ return doGetHandleInt(name);	}

		Handle	getHandleVec2f			(const char* name) const									{ return doGetHandleVec2f(name); }
		Handle	getHandleVec3f			(const char* name) const									{ return doGetHandleVec3f(name); }
		Handle	getHandleVec4f			(const char* name) const									{ return doGetHandleVec4f(name); }
		Handle	getHandleMat4f			(const char* name) const									{ return doGetHandleMat4f(name); }

		Handle	getHandleTexture		(const char* name) const									{ return doGetHandleTexture(name); }
		Handle	getHandleUniformBlock	(const char* name) const									{ return doGetHandleUniformBlock(name); }
		Handle	getHandleTextureBuffer	(const char* name) const									{ return doGetHandleTextureBuffer(name); }

		//-- set properties by handle.
		bool	setBool					(Handle id, bool val)										{ return doSetBool(id, val); }
		bool	setFloat				(Handle id, float val)										{ return doSetFloat(id, val); }
		bool	setInt					(Handle id, int val)										{ return doSetInt(id, val); }

		bool	setVec2f				(Handle id, const vec2f& val)								{ return doSetVec2f(id, val); }
		bool	setVec3f				(Handle id, const vec3f& val)								{ return doSetVec3f(id, val); }
		bool	setVec4f				(Handle id, const vec4f& val)								{ return doSetVec4f(id, val); }
		bool	setMat4f				(Handle id, const mat4f& val)								{ return doSetMat4f(id, val); }

		bool	setTexture				(Handle id, ITexture* texture, SamplerStateID state)		{ return doSetTexture(id, texture, state); }
		bool	setUniformBlock			(Handle id, const void* data, uint size)					{ return doSetUniformBlock(id, data, size); }	
		bool	setTextureBuffer		(Handle id, IBuffer* buffer)								{ return doSetTextureBuffer(id, buffer); }

		//-- set properties by name.
		bool	setBool					(const char* name, bool val)								{ Handle id = doGetHandleBool(name);  return doSetBool(id, val); }
		bool	setFloat				(const char* name, float val)								{ Handle id = doGetHandleFloat(name); return doSetFloat(id, val); }
		bool	setInt					(const char* name, int val)									{ Handle id = doGetHandleInt(name);	  return doSetInt(id, val); }

		bool	setVec2f				(const char* name, const vec2f& val)						{ Handle id = doGetHandleVec2f(name); return doSetVec2f(id, val); }
		bool	setVec3f				(const char* name, const vec3f& val)						{ Handle id = doGetHandleVec3f(name); return doSetVec3f(id, val); }
		bool	setVec4f				(const char* name, const vec4f& val)						{ Handle id = doGetHandleVec4f(name); return doSetVec4f(id, val); }
		bool	setMat4f				(const char* name, const mat4f& val)						{ Handle id = doGetHandleMat4f(name); return doSetMat4f(id, val); }

		bool	setTexture				(const char* name, ITexture* texture, SamplerStateID state) { Handle id = doGetHandleTexture(name); return doSetTexture(id, texture, state); }
		bool	setUniformBlock			(const char* name, const void* data, uint size)				{ Handle id = doGetHandleUniformBlock(name); return doSetUniformBlock(id, data, size); }
		bool	setTextureBuffer		(const char* name, IBuffer* buffer)							{ Handle id = doGetHandleTextureBuffer(name); return doSetTextureBuffer(id, buffer); }

		//-- the main usage of this function is to make this uniform block shared
		//-- between other shaders.
		bool	changeUniformBuffer		(Handle id, const Ptr<IBuffer>& newBuffer)					{ return doChangeUniformBuffer(id, newBuffer); }
		bool	changeUniformBuffer		(const char* name, const Ptr<IBuffer>& newBuffer)			{ Handle id = doGetHandleUniformBlock(name); return doChangeUniformBuffer(id, newBuffer); }

	protected:
		IShader() { }
		virtual ~IShader() = 0 { }

		virtual Handle	doGetHandleBool			(const char* name) const = 0;
		virtual Handle	doGetHandleFloat		(const char* name) const = 0;
		virtual Handle	doGetHandleInt			(const char* name) const = 0;

		virtual Handle	doGetHandleVec2f		(const char* name) const = 0;
		virtual Handle	doGetHandleVec3f		(const char* name) const = 0;
		virtual Handle	doGetHandleVec4f		(const char* name) const = 0;
		virtual Handle	doGetHandleMat4f		(const char* name) const = 0;

		virtual Handle	doGetHandleTexture		(const char* name) const = 0;
		virtual Handle	doGetHandleUniformBlock	(const char* name) const = 0;
		virtual Handle	doGetHandleTextureBuffer(const char* name) const = 0;

		virtual bool	doSetBool				(Handle id, bool val) = 0;
		virtual bool	doSetFloat				(Handle id, float val) = 0;
		virtual bool	doSetInt				(Handle id, int val) = 0;

		virtual bool	doSetVec2f				(Handle id, const vec2f& val) = 0;
		virtual bool	doSetVec3f				(Handle id, const vec3f& val) = 0;
		virtual bool	doSetVec4f				(Handle id, const vec4f& val) = 0;
		virtual bool	doSetMat4f				(Handle id, const mat4f& val) = 0;

		virtual bool	doSetTexture			(Handle id, ITexture* texture, SamplerStateID state) = 0;
		virtual bool	doSetUniformBlock		(Handle id, const void* data, uint size) = 0;
		virtual bool	doSetTextureBuffer		(Handle id, IBuffer* buffer) = 0;

		virtual bool	doChangeUniformBuffer	(Handle id, const Ptr<IBuffer>& newBuffer) = 0;
	};

} // render
} // brUGE
