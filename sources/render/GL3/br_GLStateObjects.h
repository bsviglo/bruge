#ifndef _BR_GLSTATEOBJECTS_H_
#define _BR_GLSTATEOBJECTS_H_

#include "br_GLCommon.h"
#include "render/ibr_StateObjects.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------	
	class brGLRasterizerState : public ibrRasterizerState
	{
	public:
		// Внутренние структуры представления данных.
		struct Desc_s
		{
			GLenum fillMode;
			GLboolean cullEnable;
			GLenum cullMode;
			GLenum frontFaceMode;

			// TODO: Дополнить остальные поля. Все они имеют дефолтвые значения.

		};

	public:
		brGLRasterizerState(const ibrRasterizerState::Desc_s& desc);
		virtual ~brGLRasterizerState();

		void apply(const Desc_s* prevState = NULL) const;

	private:
		void _compileState(const ibrRasterizerState::Desc_s& desc);

		Desc_s m_glDesc;		
	};


	//------------------------------------------
	class brGLDepthStencilState : public ibrDepthStencilState
	{
	public:
		// Внутренние структуры представления данных.
		struct Face_s
		{
			GLenum stencilFailOp;
			GLenum stencilPassDepthFailOp;
			GLenum stencilPassDepthPassOp;
			GLenum stencilFunc;
		};

		struct Desc_s
		{
			GLboolean depthEnable;
			GLboolean depthWriteMask;
			GLenum depthFunc;
			GLboolean stencilEnable;
			GLuint stencilWriteMask;
			GLuint stencilReadMask;
			Face_s frontFace;
			Face_s backFace;
		};

	public:
		brGLDepthStencilState(const ibrDepthStencilState::Desc_s& desc);
		virtual ~brGLDepthStencilState();

		void apply(
			uint stencilRef,
			const Desc_s* prevState = NULL
			) const;

	private:
		void _compileState(const ibrDepthStencilState::Desc_s& desc);
		static GLenum _getGLStencilOp(ebrStencilOp op);
		static GLenum _getGLComparisonFunc(ebrComparisonFunc func);

		Desc_s m_glDesc;
	};
	

	//------------------------------------------
	class brGLBlendState : public ibrBlendState
	{
	public:
		// Внутренние структуры представления данных.
		struct Desc_s
		{
			GLboolean blendEnable[MAX_DRAW_BUFFERS];
			GLenum srcBlend;
			GLenum destBlend;
			GLenum blendOp;
			GLenum srcAlpha;
			GLenum destAlpha;
			GLenum blendAlphaOp;
			GLboolean colorWriteMask[MAX_DRAW_BUFFERS][4];
		};

	public:
		brGLBlendState(const ibrBlendState::Desc_s& desc);
		virtual ~brGLBlendState();

		void apply(
			const Desc_s* prevState = NULL
			) const;

	private:
		void _complileState(const ibrBlendState::Desc_s& desc);
		static GLenum _getGLBlendFactor(ebrBlendFactor blend);
		static GLenum _getGLBlendOp(ebrBlendOp op);

		Desc_s m_glDesc;
	};
	

	//------------------------------------------
	class brGLSamplerState : public ibrSamplerState
	{
	public:
		// Внутренние структуры представления данных.
		struct Desc_s
		{
			GLenum minFilter;
			GLenum magFilter;
			GLenum wrapS;
			GLenum wrapT;
			GLenum wrapR;
			float loadBias;
			uint maxAnisotropy;
			GLenum compareFunc;
			float borderColour[4];
			float minLoad;
			float maxLoad;
		};

	public:
		brGLSamplerState(const ibrSamplerState::Desc_s& desc);
		virtual ~brGLSamplerState();

		void apply(
			GLenum targetType,
			GLuint texId,
			const Desc_s* prevState = NULL
			) const;

	private:
		void _complileState(const ibrSamplerState::Desc_s& desc);
		static void _getGLTexFilter(ebrTexFilter filter, GLenum& minFilter, GLenum& magFilter);
		static GLenum _getGLTexAddressMode(ebrTexAddressMode mode);
		static GLenum _getGLTexCompareFunc(ebrTexCompareFunc func);

		Desc_s m_glDesc;
	};

} // render
} // brUGE

#endif /*_BR_GLSTATEOBJECTS_H_*/