#include "br_GLStateObjects.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLRasterizerState::brGLRasterizerState(
		const ibrRasterizerState::Desc_s &desc
		)
	{
		_compileState(desc);
	}

	//------------------------------------------
	brGLRasterizerState::~brGLRasterizerState()
	{

	}
	
	//------------------------------------------
	void brGLRasterizerState::_compileState(
		const ibrRasterizerState::Desc_s& desc)
	{
		if (desc.fillMode != FM_WIREFRAME && desc.fillMode != FM_SOLID)
			BR_EXCEPT("brGLRasterizedState::_compileState", "Invalid fill mode parameter.");
		
		// тип заполнения полигона.
		if (desc.fillMode == FM_WIREFRAME)
			m_glDesc.fillMode = GL_LINE;
		else
			m_glDesc.fillMode = GL_FILL;
		
		if (desc.cullMode != CM_NOTHING && desc.cullMode != CM_FRONT && desc.cullMode != CM_BACK)
			BR_EXCEPT("brGLRasterizedState::_compileState", "Invalid cull mode parameter.");
		
		// отсечение полигонов.
		m_glDesc.cullEnable = GL_TRUE;
		if (desc.cullMode == CM_NOTHING)
			m_glDesc.cullEnable = GL_FALSE;
		else if (desc.cullMode == CM_FRONT)
			m_glDesc.cullMode = GL_FRONT;
		else
			m_glDesc.cullMode = GL_BACK;
		
		// определение переденей стороны полигона.
		if (desc.frontCounterClockwise)
			m_glDesc.frontFaceMode = GL_CCW;
		else
			m_glDesc.frontFaceMode = GL_CW;
		
		// TODO: Остальное будет добавлено по мере необходимости.

	}
	
	//------------------------------------------
	void brGLRasterizerState::apply(
		const Desc_s* /* prevState = NULL */) const
	{
		GL_CLEAR_ERRORS

		// TODO: организовать проверку и переключение только тех параметров стейта, которые изменились.

		glFrontFace(m_glDesc.frontFaceMode);
		
		if (!m_glDesc.cullEnable)
			glDisable(GL_CULL_FACE);
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(m_glDesc.cullMode);
		}

		glPolygonMode(GL_FRONT_AND_BACK, m_glDesc.fillMode);

		GL_THROW_ERRORS
	}

	
	//------------------------------------------
	brGLDepthStencilState::brGLDepthStencilState(
		const ibrDepthStencilState::Desc_s& desc
		)
	{
		_compileState(desc);
	}
	
	//------------------------------------------
	brGLDepthStencilState::~brGLDepthStencilState()
	{

	}
	
	//------------------------------------------
	void brGLDepthStencilState::_compileState(
		const ibrDepthStencilState::Desc_s& desc)
	{
		if (!desc.depthEnable)
			m_glDesc.depthEnable = GL_FALSE;
		else
		{
			m_glDesc.depthWriteMask = desc.depthWriteMask;
			m_glDesc.depthFunc = _getGLComparisonFunc(desc.depthFunc);
		}

		if (!desc.stencilEnable)
			m_glDesc.stencilEnable = GL_FALSE;
		else
		{
			m_glDesc.stencilReadMask = desc.stencilReadMask;
			m_glDesc.stencilWriteMask = desc.stencilWriteMask;
			
			Face_s& front = m_glDesc.frontFace;
			front.stencilFailOp				= _getGLStencilOp(desc.frontFace.stencilFailOp);
			front.stencilPassDepthFailOp	= _getGLStencilOp(desc.frontFace.stencilPassDepthFailOp);
			front.stencilPassDepthPassOp	= _getGLStencilOp(desc.frontFace.stencilPassDepthPassOp);
			front.stencilFunc				= _getGLComparisonFunc(desc.frontFace.stencilFunc);

			Face_s& back = m_glDesc.backFace;
			back.stencilFailOp				= _getGLStencilOp(desc.backFace.stencilFailOp);
			back.stencilPassDepthFailOp		= _getGLStencilOp(desc.backFace.stencilPassDepthFailOp);
			back.stencilPassDepthPassOp		= _getGLStencilOp(desc.backFace.stencilPassDepthPassOp);
			back.stencilFunc				= _getGLComparisonFunc(desc.backFace.stencilFunc);
		}
	}
	
	//------------------------------------------
	void brGLDepthStencilState::apply(
		uint stencilRef,
		const Desc_s* /* prevState = NULL */) const
	{
		GL_CLEAR_ERRORS

		if (!m_glDesc.depthEnable)
			glDisable(GL_DEPTH_TEST);
		else
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(m_glDesc.depthWriteMask);
			glDepthFunc(m_glDesc.depthFunc);
		}

		if (!m_glDesc.stencilEnable)
			glDisable(GL_STENCIL_TEST);
		else
		{
			glEnable(GL_STENCIL_TEST);

			const Face_s& front = m_glDesc.frontFace;
			glStencilOpSeparate(
				GL_FRONT,
				front.stencilFailOp,
				front.stencilPassDepthFailOp,
				front.stencilPassDepthPassOp
				);

			glStencilFuncSeparate(
				GL_FRONT,
				front.stencilFunc,
				stencilRef,
				m_glDesc.stencilReadMask
				);
			
			const Face_s& back = m_glDesc.backFace;
			glStencilOpSeparate(
				GL_BACK,
				back.stencilFailOp,
				back.stencilPassDepthFailOp,
				back.stencilPassDepthPassOp
				);

			glStencilFuncSeparate(
				GL_BACK,
				back.stencilFunc,
				stencilRef,
				m_glDesc.stencilReadMask
				);

			glStencilMask(m_glDesc.stencilWriteMask);

		}

		GL_THROW_ERRORS
	}

	//------------------------------------------
	/*static*/ GLenum brGLDepthStencilState::_getGLStencilOp(
		ebrStencilOp op)
	{
		switch (op)
		{
		case SO_KEEP:		return GL_KEEP;
		case SO_ZERO:		return GL_ZERO;
		case SO_REPLACE:	return GL_REPLACE;
		case SO_INCR:		return GL_INCR;
		case SO_DECR:		return GL_DECR;
		case SO_INVERT:		return GL_INVERT;
		case SO_INCR_WRAP:	return GL_INCR_WRAP;
		case SO_DECR_WRAP:	return GL_DECR_WRAP;
		default:
			BR_EXCEPT("brGLDepthStencilState::_getGLStencilOp", "Invalid stencil operation.");
		}
	}

	//------------------------------------------
	/*static*/ GLenum brGLDepthStencilState::_getGLComparisonFunc(
		ebrComparisonFunc func)
	{
		switch (func)
		{
		case CF_NEVER:			return GL_NEVER;
		case CF_ALWAYS:			return GL_ALWAYS;
		case CF_EQUAL:			return GL_EQUAL;
		case CF_NOT_EQUAL:		return GL_NOTEQUAL;
		case CF_LESS:			return GL_LESS;
		case CF_LESS_EQUAL:		return GL_LEQUAL;
		case CF_GREATER:		return GL_GREATER;
		case CF_GREATER_EQUAL:	return GL_GEQUAL;
		default:
			BR_EXCEPT("brGLDepthStencilState::_getGLComparisonFunc", "Invalid stencil comparison function.");
		}
	}
	
	//------------------------------------------
	brGLBlendState::brGLBlendState(
		const ibrBlendState::Desc_s &desc
		)
	{
		_complileState(desc);
	}
	
	//------------------------------------------
	brGLBlendState::~brGLBlendState()
	{

	}
	
	//------------------------------------------
	void brGLBlendState::apply(
		const Desc_s* /* prevState = NULL  */) const
	{
		GL_CLEAR_ERRORS

		for (uint i = 0; i < MAX_DRAW_BUFFERS; ++i)
		{
			glColorMaski(
				i,
				m_glDesc.colorWriteMask[i][0],
				m_glDesc.colorWriteMask[i][1],
				m_glDesc.colorWriteMask[i][2],
				m_glDesc.colorWriteMask[i][3]
			);

			if (m_glDesc.blendEnable[i])
				glEnablei(GL_BLEND, i);
			else
				glDisablei(GL_BLEND, i);
		}

		glBlendFuncSeparate(
			m_glDesc.srcBlend,
			m_glDesc.destBlend,
			m_glDesc.srcAlpha,
			m_glDesc.destAlpha
			);

		glBlendEquationSeparate(m_glDesc.blendOp, m_glDesc.blendAlphaOp);	

		GL_THROW_ERRORS
	}
	
	//------------------------------------------
	void brGLBlendState::_complileState(
		const ibrBlendState::Desc_s& desc)
	{
		for (uint i = 0; i < MAX_DRAW_BUFFERS; ++i)		
		{
			m_glDesc.blendEnable[i] = desc.blendEnable[i];
			m_glDesc.colorWriteMask[i][0] = desc.colorWriteMask[i][0];
			m_glDesc.colorWriteMask[i][1] = desc.colorWriteMask[i][1];
			m_glDesc.colorWriteMask[i][2] = desc.colorWriteMask[i][2];
			m_glDesc.colorWriteMask[i][3] = desc.colorWriteMask[i][3];
		}

		m_glDesc.srcBlend		= _getGLBlendFactor(desc.srcBlend);
		m_glDesc.destBlend		= _getGLBlendFactor(desc.destBlend);
		m_glDesc.srcAlpha		= _getGLBlendFactor(desc.srcBlendAlpha);
		m_glDesc.destAlpha		= _getGLBlendFactor(desc.destBlendAlpha);
		m_glDesc.blendOp		= _getGLBlendOp(desc.blendOp);
		m_glDesc.blendAlphaOp	= _getGLBlendOp(desc.blendAlphaOp);
	}

	//------------------------------------------
	/*static*/ GLenum brGLBlendState::_getGLBlendFactor(
		ebrBlendFactor blend)
	{
		switch (blend)
		{
		case BF_ZERO:				return GL_ZERO;
		case BF_ONE:				return GL_ONE;
		case BF_SRC_COLOR:			return GL_SRC_COLOR;
		case BF_INV_SRC_COLOR:		return GL_ONE_MINUS_SRC_COLOR;
		case BF_SRC_ALPHA:			return GL_SRC_ALPHA;
		case BF_INV_SRC_ALPHA:		return GL_ONE_MINUS_SRC_ALPHA;
		case BF_DEST_ALPHA:			return GL_DST_ALPHA;
		case BF_INV_DEST_ALPHA:		return GL_ONE_MINUS_DST_ALPHA;
		case BF_DEST_COLOR:			return GL_DST_COLOR;
		case BF_INV_DEST_COLOR:		return GL_ONE_MINUS_DST_COLOR;
		default:
			BR_EXCEPT("brGLBlendState::_getGLBlendFactor", "Invalid blend factor.");
		}
	}

	//------------------------------------------
	/*static*/ GLenum brGLBlendState::_getGLBlendOp(
		ebrBlendOp op)
	{
		switch (op)
		{
		case BO_ADD:			return GL_FUNC_ADD;
		case BO_SUBTRACT:		return GL_FUNC_SUBTRACT;
		case BO_REV_SUBTRACT:	return GL_FUNC_REVERSE_SUBTRACT;
		case BO_MIN:			return GL_MIN;
		case BO_MAX:			return GL_MAX;
		default:
			BR_EXCEPT("brGLBlendState::_getGLBlendOp", "Invalid blending operation.");
		}
	}
	
	
	//------------------------------------------
	brGLSamplerState::brGLSamplerState(
		const ibrSamplerState::Desc_s& desc
		)
	{
		_complileState(desc);
	}
	
	//------------------------------------------
	brGLSamplerState::~brGLSamplerState()
	{

	}

	//------------------------------------------
	void brGLSamplerState::_complileState(
		const ibrSamplerState::Desc_s& desc)
	{
		_getGLTexFilter(desc.minMagFilter, m_glDesc.minFilter, m_glDesc.magFilter);

		m_glDesc.wrapS = _getGLTexAddressMode(desc.wrapS);
		m_glDesc.wrapR = _getGLTexAddressMode(desc.wrapR);
		m_glDesc.wrapT = _getGLTexAddressMode(desc.wrapT);

		m_glDesc.loadBias		= desc.loadBias;
		m_glDesc.maxAnisotropy	= desc.maxAnisotropy;
		m_glDesc.compareFunc	= _getGLTexCompareFunc(desc.compareFunc);

		for (uint i = 0; i < 4; ++i)
			m_glDesc.borderColour[i] = desc.borderColour.color[i];

		m_glDesc.minLoad	= desc.minLoad;
		m_glDesc.maxLoad	= desc.maxLoad;
	}
	
	//------------------------------------------
	void brGLSamplerState::apply(
		GLenum texTarget,
		GLuint texId,
		const Desc_s* /* prevState = NULL  */) const
	{
		// TODO: Продумать возможность изменения не всего стейта,
		//		 а только того что изменилось с прошлого prevState стейта.
		
		GL_CLEAR_ERRORS

		glBindTexture(texTarget, texId);

		glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, m_glDesc.magFilter);
		glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, m_glDesc.minFilter);
		glTexParameterf(texTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_glDesc.maxAnisotropy);

		glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, m_glDesc.wrapS);
		if (texTarget == GL_TEXTURE_2D ||
			texTarget == GL_TEXTURE_2D_ARRAY ||
			texTarget == GL_TEXTURE_CUBE_MAP ||
			texTarget == GL_TEXTURE_3D)
		{
			glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, m_glDesc.wrapT);
			if (texTarget == GL_TEXTURE_CUBE_MAP || texTarget == GL_TEXTURE_3D)
				glTexParameteri(texTarget, GL_TEXTURE_WRAP_R, m_glDesc.wrapR);
		}

		glTexParameterf(texTarget, GL_TEXTURE_LOD_BIAS, m_glDesc.loadBias);
		glTexParameteri(texTarget, GL_TEXTURE_COMPARE_FUNC, m_glDesc.compareFunc);
		glTexParameterfv(texTarget, GL_TEXTURE_BORDER_COLOR, m_glDesc.borderColour);
		glTexParameterf(texTarget, GL_TEXTURE_MIN_LOD, m_glDesc.minLoad);
		glTexParameterf(texTarget, GL_TEXTURE_MAX_LOD, m_glDesc.maxLoad);

		GL_THROW_ERRORS
	}
	
	//------------------------------------------	
	/*static*/ void brGLSamplerState::_getGLTexFilter(
		ebrTexFilter filter,
		GLenum& minFilter,
		GLenum& magFilter)
	{
		switch (filter)
		{
		case TF_MIN_MAG_MIP_POINT:					minFilter = GL_NEAREST_MIPMAP_NEAREST;	magFilter = GL_NEAREST; break;
		case TF_MIN_MAG_POINT_MIP_LINEAR:			minFilter = GL_NEAREST_MIPMAP_LINEAR;	magFilter = GL_NEAREST; break;
		case TF_MIN_POINT_MAG_LINEAR_MIP_POINT:		minFilter = GL_NEAREST_MIPMAP_NEAREST;	magFilter = GL_LINEAR;	break;
		case TF_MIN_POINT_MAG_MIP_LINEAR:			minFilter = GL_NEAREST_MIPMAP_LINEAR;	magFilter = GL_LINEAR;	break;
		case TF_MIN_LINEAR_MAG_MIP_POINT:			minFilter = GL_LINEAR_MIPMAP_NEAREST;	magFilter = GL_NEAREST; break;
		case TF_MIN_LINEAR_MAG_POINT_MIP_LINEAR:	minFilter = GL_LINEAR_MIPMAP_LINEAR;	magFilter = GL_NEAREST; break;
		case TF_MIN_MAG_LINEAR_MIP_POINT:			minFilter = GL_LINEAR_MIPMAP_NEAREST;	magFilter = GL_LINEAR;	break;
		case TF_MIN_MAG_MIP_LINEAR:					minFilter = GL_LINEAR_MIPMAP_LINEAR;	magFilter = GL_LINEAR;	break;
		case TF_ANISOTROPIC:						minFilter = GL_LINEAR_MIPMAP_LINEAR;	magFilter = GL_LINEAR;	break;
		default:
			BR_EXCEPT("brGLSamplerState::_getGLTexFilter", "Invalid texture filter type.");
		}
	}
	
	//------------------------------------------
	/*static*/ GLenum brGLSamplerState::_getGLTexAddressMode(ebrTexAddressMode mode)
	{
		switch (mode)
		{
		case TAM_WRAP:			return GL_REPEAT;
		case TAM_MIRROR:		return GL_MIRRORED_REPEAT;
		case TAM_CLAMP:			return GL_CLAMP;
		case TAM_BORDER:		return GL_CLAMP_TO_BORDER;
		case TAM_MIRROR_ONCE:	return GL_MIRROR_CLAMP_EXT;
		default:
			BR_EXCEPT("brGLSamplerState::_getGLTexAddressMode", "Invalid texture address mode.");
		}
	}
	
	//------------------------------------------
	/*static*/ GLenum brGLSamplerState::_getGLTexCompareFunc(ebrTexCompareFunc func)
	{
		switch (func)
		{
		case TCF_LEQUAL:	return GL_LEQUAL;	
		case TCF_GEQUAL:	return GL_GEQUAL;
		case TCF_LESS:		return GL_LESS;
		case TCF_GREATER:	return GL_GREATER;
		case TCF_EQUAL:		return GL_EQUAL;
		case TCF_NOTEQUAL:	return GL_NOTEQUAL;
		case TCF_ALWAYS:	return GL_ALWAYS;
		case TCF_NEVER:		return GL_NEVER;
		default:
			BR_EXCEPT("brGLSamplerState::_getGLTexCompareFunc", "Invalid texture comparison function.");
		}
	}


} // render
} // brUGE