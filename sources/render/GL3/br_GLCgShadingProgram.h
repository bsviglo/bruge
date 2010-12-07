#ifndef _BR_GLCGSHADINGPROGRAM_H_
#define _BR_GLCGSHADINGPROGRAM_H_

#include "br_GLCommon.h"
#include "br_GLCgShader.h"

#include <vector>

namespace brUGE
{
namespace render
{
	
	// 
	//------------------------------------------
	class brGLCgShadingProgram : public ibrShadingProgram
	{
	public:
		brGLCgShadingProgram();
		virtual ~brGLCgShadingProgram();

		// Установка в шейдинг программу соответствующих шейдеров.
		virtual void setVertexShader(const Ptr<ibrVertexShader>& shader);
		virtual void setFragmentShader(const Ptr<ibrFragmentShader>& shader);
		virtual void setGeometryShader(const Ptr<ibrGeometryShader>& shader);

		virtual void  setUniformBuffer(
			ebrShaderType shaderType,
			const Ptr<ibrUniformBuffer>& buffer
			);

		virtual void  setSamplerUniform(
			ebrShaderType shaderType,
			const Ptr<ibrSamplerUniform>& sampler
			);

		virtual Ptr<ibrUniformBuffer> createUniformBuffer(
			ebrShaderType shaderType,
			const brStr& bufferName,
			uint size,
			void* data = NULL,
			ibrBuffer::ebrUsageFlag usage = ibrBuffer::UF_DYNAMIC,
			ibrBuffer::ebrCPUAccessFlag	cpuAccess = ibrBuffer::CPU_AF_WRITE
			) const;

		virtual Ptr<ibrSamplerUniform> createSamplerUniform(
			ebrShaderType shaderType,
			const brStr& name
			) const;

		virtual void link();
		virtual void enable() const;
		virtual void disable() const;

	private:
		CGprogram _pickShaderType(ebrShaderType type) const;

	private:
		CGprogram									m_combinedProgram;
		Ptr<brGLCgVertexShader>					m_vertexShader;
		Ptr<brGLCgFragmentShader>					m_fragmentShader;
		Ptr<brGLCgGeometryShader>					m_geometricShader;
		std::vector<brGLCgShaderImpl*>				m_shaderBase;
		std::vector<CGprofile>						m_profiles;
		std::vector<Ptr<brGLCgSamplerUniform> >	m_samplers;
	};


	// 
	//------------------------------------------
	class brGLCgSamplerUniform : public ibrSamplerUniform
	{
	public:
		brGLCgSamplerUniform(CGparameter parameter);
		virtual ~brGLCgSamplerUniform();

		virtual void setTexture(const Ptr<ibrTexture>& texture);
		virtual void setSamplerState(const Ptr<ibrSamplerState>& state);

		void checkStatus() const;

		void enable() const;
		void disable() const;

	protected:
		mutable bool			m_completeness;
		CGparameter				m_parameter;
		Ptr<brGLTexture>		m_texture;
		Ptr<brGLSamplerState>	m_samplerState;
	};

} // render
} // brUGE

#endif /*_BR_GLCGSHADINGPROGRAM_H_*/