#include "br_GLCgShadingProgram.h"
#include "br_GLCgContext.h"
#include "br_GLBuffer.h"
#include "br_GLTexture.h"
#include "br_GLStateObjects.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLCgShadingProgram::brGLCgShadingProgram()
		: m_combinedProgram(0)
	{	
		// Заполняем указатели нулями.
		for (int i = 0; i<3; ++i)
			m_shaderBase.push_back(NULL);
		
		// TODO: Сколько нужно резервировать?

		// Резервируем для 16 текстур.
		m_samplers.reserve(16);

		brGLCgContext::addRef();			
	}
	
	//------------------------------------------
	brGLCgShadingProgram::~brGLCgShadingProgram()
	{
		if (cgIsProgram(m_combinedProgram))
			cgDestroyProgram(m_combinedProgram);
		brGLCgContext::release();
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::setVertexShader(const Ptr<ibrVertexShader>& shader)
	{
		if (shader.isValid())
		{
			m_vertexShader = shader.castDown<brGLCgVertexShader>();
			m_shaderBase[0] = m_vertexShader.get();
		}
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::setFragmentShader(const Ptr<ibrFragmentShader>& shader)
	{
		if (shader.isValid())
		{
			m_fragmentShader = shader.castDown<brGLCgFragmentShader>();
			m_shaderBase[1] = m_fragmentShader.get();
		}
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::setGeometryShader(const Ptr<ibrGeometryShader>& shader)
	{
		if (shader.isValid())
		{
			m_geometricShader = shader.castDown<brGLCgGeometryShader>();
			m_shaderBase[2] = m_geometricShader.get();
		}
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::link()
	{
		if (cgIsProgram(m_combinedProgram))
			cgDestroyProgram(m_combinedProgram);

		std::vector<CGprogram> programs;

		for (int i = 0; i < 3; ++i)
			if (m_shaderBase[i])
			{
				programs.push_back(m_shaderBase[i]->getCgProgram());
				m_profiles.push_back(m_shaderBase[i]->getCgProfile());
			}

		m_combinedProgram = cgCombinePrograms(static_cast<int>(programs.size()), &programs[0]);
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("Failed to combine Cg program.");

		cgGLLoadProgram(m_combinedProgram);
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("Failed to load Cg program.");
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::enable() const
	{
		cgGLBindProgram(m_combinedProgram);
#ifdef _DEBUG
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("A Cg error has occurred.");
#endif

		// Включаем все профайлы.
		for (uint i = 0; i < m_profiles.size(); ++i)
			cgGLEnableProfile(m_profiles[i]);

		for (uint i = 0; i < m_samplers.size(); ++i)
		{
			m_samplers[i]->checkStatus();
			m_samplers[i]->enable();
		}
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::disable() const
	{
		// Выключаем все профайлы.
		for (uint i = 0; i < m_profiles.size(); ++i)
			cgGLDisableProfile(m_profiles[i]);

		for (uint i = 0; i < m_samplers.size(); ++i)
			m_samplers[i]->disable();
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::setUniformBuffer(
		ebrShaderType shaderType,
		const Ptr<ibrUniformBuffer>& buffer)
	{
#ifdef _DEBUG
		if (!buffer.castDown<brGLUniformBuffer>())
			BR_EXCEPT("dynamic_cast() return NULL.");
#endif // _DEBUG

		Ptr<brGLUniformBuffer> glBuff = buffer.castDown<brGLUniformBuffer>();
		cgSetProgramBuffer(_pickShaderType(shaderType), glBuff->getSlot(), glBuff->getCgBuffer());

#ifdef _DEBUG
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("Can't set uniform buffer.");
#endif // _DEBUG
	}
	
	//------------------------------------------
	void brGLCgShadingProgram::setSamplerUniform(
		ebrShaderType /* shaderType */,
		const Ptr<ibrSamplerUniform>& sampler)
	{
#ifdef _DEBUG
		if (!sampler.castDown<brGLCgSamplerUniform>())
			BR_EXCEPT("dynamic_cast() return NULL.");
#endif // _DEBUG
		
		m_samplers.push_back(sampler.castDown<brGLCgSamplerUniform>());
	}
	
	//------------------------------------------
	Ptr<ibrUniformBuffer> brGLCgShadingProgram::createUniformBuffer(
		ebrShaderType shaderType,
		const brStr &bufferName,
		uint size, 
		void *data /* = NULL */,
		ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DYNAMIC */,
		ibrBuffer::ebrCPUAccessFlag cpuAccess /* = ibrBuffer::CPU_AF_WRITE */) const
	{
		CGparameter param = cgGetNamedParameter(_pickShaderType(shaderType), bufferName.c_str());

#ifdef _DEBUG
		if (cgGetError() == CG_INVALID_PROGRAM_HANDLE_ERROR)
			BR_EXCEPT("Invalid program handle.");

		if (!param)
			BR_EXCEPT_F("Cannot find uniform buffer with name '%s'", bufferName.c_str());
#endif //_DEBUG

		uint slot = cgGetParameterBufferIndex(param);
		
#ifdef _DEBUG
		cgGetError();
		if (slot == -1)
			BR_EXCEPT_F("Cannot find uniform buffer with name '%s'", bufferName.c_str());
#endif //_DEBUG

		return new brGLUniformBuffer(slot, size, usage, cpuAccess, data);
	}
	
	//------------------------------------------
	Ptr<ibrSamplerUniform> brGLCgShadingProgram::createSamplerUniform(
		ebrShaderType shaderType,
		const brStr& name) const
	{
		CGparameter param = cgGetNamedParameter(_pickShaderType(shaderType), name.c_str());

#ifdef _DEBUG
		if (cgGetError() == CG_INVALID_PROGRAM_HANDLE_ERROR)
			BR_EXCEPT("Invalid program handle.");

		if (!param)
			BR_EXCEPT_F("Cannot find sampler uniform with name '%s'",  name.c_str());
#endif //_DEBUG

		return new brGLCgSamplerUniform(param);
	}

	//------------------------------------------	
	CGprogram brGLCgShadingProgram::_pickShaderType(
		ebrShaderType type) const
	{
		// pick shader program.
		switch (type)
		{
		case ST_VERTEX:		return m_vertexShader->getCgProgram();
		case ST_FRAGMENT:	return m_fragmentShader->getCgProgram();
		case ST_GEOMETRY:	return m_geometricShader->getCgProgram();
		default:
			BR_EXCEPT("Invalid 'shaderType' parameter.");
		}
	}
	

	//------------------------------------------
	brGLCgSamplerUniform::brGLCgSamplerUniform(
		CGparameter parameter
		)
		:	m_parameter(parameter),
			m_texture(NULL),
			m_samplerState(NULL),
			m_completeness(false)
	{

	}
	
	//------------------------------------------
	brGLCgSamplerUniform::~brGLCgSamplerUniform()
	{

	}
	
	//------------------------------------------
	void brGLCgSamplerUniform::setTexture(
		const Ptr<ibrTexture> &texture)
	{
#ifdef _DEBUG
		if (!texture.castDown<brGLTexture>())
			BR_EXCEPT("dynamic_cast() return NULL.");
#endif // _DEBUG

		m_texture = texture.castDown<brGLTexture>();
		cgGLSetTextureParameter(m_parameter, m_texture->getTexId());
		m_completeness = false;

#ifdef _DEBUG
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("A Cg error has occurred.");
#endif
	}
	
	//------------------------------------------
	void brGLCgSamplerUniform::setSamplerState(
		const Ptr<ibrSamplerState>& state)
	{
#ifdef _DEBUG
		if (!state.castDown<brGLSamplerState>())
			BR_EXCEPT("dynamic_cast() return NULL.");
#endif // _DEBUG

		m_samplerState = state.castDown<brGLSamplerState>();
		if (m_texture)
		{
			m_samplerState->apply(
				m_texture->getTexTarget(),
				m_texture->getTexId(),
				NULL
				);

			m_completeness = true;
		}
		else
			m_completeness = false;
	}

	//------------------------------------------
	void brGLCgSamplerUniform::checkStatus() const
	{
		if (!m_completeness)
		{
#ifdef _DEBUG
			if (!m_texture)
				BR_EXCEPT("Sampler uniform without texture object.");
#endif // _DEBUG

			m_samplerState->apply(
				m_texture->getTexTarget(),
				m_texture->getTexId(),
				NULL
				);

			m_completeness = true;
		}
	}
	
	//------------------------------------------
	void brGLCgSamplerUniform::enable() const
	{
		cgGLEnableTextureParameter(m_parameter);

#ifdef _DEBUG
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("A Cg error has occurred.");
#endif
	}
	
	//------------------------------------------
	void brGLCgSamplerUniform::disable() const
	{
		cgGLDisableTextureParameter(m_parameter);

#ifdef _DEBUG
		if (cgGetError() != CG_NO_ERROR)
			BR_EXCEPT("A Cg error has occurred.");
#endif
	}

} // render
} // brUGE