#ifndef _BR_OGLVERTEXLAYOUT_H_
#define _BR_OGLVERTEXLAYOUT_H_

#include "br_OGLCommon.h"
#include "render/ibr_VertexLayout.h"
#include "render/ibr_ShadingProgram.h"
#include <map>
#include <vector>

// TODO: Продумать GL_ARB_instanced_arrays & GL_ARB_draw_instanced аля D3D10.

namespace brUGE
{
namespace render
{
	// Реализация layout-a для OGL.
	// Сохраняет во внутренний формат данные о том как данные из вершинного буффера
	// будут мапиться на входные параметры вершинного шейдера. В OpenGL явной поддержки
	// такого механизма нету, его зачатки есть в D3D9 а в полной мере он реализован в
	// D3D10. Это позволяет сверять аттрибуты вершинного шейдера что храняться в буфере, с
	// теми что хочет на входе вершинный шейдер в компайл тайме.
	// OpenGL - реализация игнорирует входящий шейдер, так как провести компайл тайм проверку
	// не предоставляется возможным. Класс в основном нужен для D3D-style задания аттрибутов для 
	// вершин, а также перехода на OpenGL 3.0 так как в нем была объявлена запрещенной
	// функциональность связанная с фиксированным конвеером.
	//------------------------------------------
	class brOGLVertexLayout : public ibrVertexLayout
	{

		friend class brOGLRenderDevice;

	public:
		brOGLVertexLayout(const brVertexDesc *vd, uint count, const brPtr<ibrShadingProgram>& shader);
		~brOGLVertexLayout();

		uint getVertexSize() const;

	private:
		static GLenum _convertFormat(ebrTypeFormat format);
		static GLsizei _typeSizeOf(GLenum type);
		void _addVertexAttribute(uint inputSlot, ebrVertexAttribute va, GLenum type, uint componentsCount, bool normalized);
		void _calculateStrides();

	private:
		enum
		{
			MAX_VERTEX_INPUT_SLOTS = 4,
			MAX_VERTEX_ATTRIBS = 16 // TODO: Брать эту инфу из списка капсов для OpenGL.
		};

		struct brInternalVertexDesc
		{
			GLsizei size;
			GLenum type;
			GLboolean normalized;
			GLubyte *offset;
		};

		typedef std::map<uint, brInternalVertexDesc> AttribMap; // номер атрибута : параметры аттрибута.
		typedef std::map<uint, brInternalVertexDesc>::iterator AttribMapIter; 
		typedef std::map<uint, brInternalVertexDesc>::const_iterator AttribMapConstIter; 

		AttribMap			inputSlots_[MAX_VERTEX_INPUT_SLOTS];			
		GLsizei				strides_[MAX_VERTEX_INPUT_SLOTS];
		std::vector<bool>	activated_;
		uint				vertexSize_;
	};
}
}

#endif /*_BR_OGLVERTEXLAYOUT_H_*/