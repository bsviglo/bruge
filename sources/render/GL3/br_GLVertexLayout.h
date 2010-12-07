#ifndef _BR_GLVERTEXLAYOUT_H_
#define _BR_GLVERTEXLAYOUT_H_

#include "br_GLCommon.h"
#include "render/ibr_VertexLayout.h"

#include <map>
#include <vector>

// TODO: ��������� GL_ARB_instanced_arrays & GL_ARB_draw_instanced ��� D3D10.

namespace brUGE
{
namespace render
{
	// ���������� layout-a ��� OGL.
	// ��������� �� ���������� ������ ������ � ��� ��� ������ �� ���������� �������
	// ����� �������� �� ������� ��������� ���������� �������. � OpenGL ����� ���������
	// ������ ��������� ����, ��� ������� ���� � D3D9 � � ������ ���� �� ���������� �
	// D3D10. ��� ��������� ������� ��������� ���������� ������� ��� ��������� � ������, �
	// ���� ��� ����� �� ����� ��������� ������ � ������� �����.
	// OpenGL - ���������� ���������� �������� ������, ��� ��� �������� ������� ���� ��������
	// �� ��������������� ���������. ����� � �������� ����� ��� D3D-style ������� ���������� ��� 
	// ������, � ����� �������� �� OpenGL 3.0 ��� ��� � ��� ���� ��������� �����������
	// ���������������� ��������� � ������������� ���������.
	//------------------------------------------
	class brGLVertexLayout : public ibrVertexLayout
	{

		friend class brGLRenderDevice;

	public:
		brGLVertexLayout(
			const brVertexDesc *vd,
			uint count,
			const Ptr<ibrShadingProgram>& shader
			);
		~brGLVertexLayout();

		uint getVertexSize() const;

	private:
		static GLenum	_convertFormat(ebrTypeFormat format);
		static GLsizei	_typeSizeOf(GLenum type);

		void _addVertexAttribute(
			uint inputSlot,
			ebrVertexAttribute va,
			GLenum type,
			uint componentsCount,
			bool normalized
			);
		void _calculateStrides();

	private:
		enum
		{
			MAX_VERTEX_INPUT_SLOTS	= 4,
			MAX_VERTEX_ATTRIBS		= 16 // TODO: ����� ��� ���� �� ������ ������ ��� OpenGL.
		};

		struct brInternalVertexDesc
		{
			GLsizei		size;
			GLenum		type;
			GLboolean	normalized;
			GLubyte*	offset;
		};

		typedef std::map<uint, brInternalVertexDesc> AttribMap; // ����� �������� : ��������� ���������.
		typedef std::map<uint, brInternalVertexDesc>::iterator AttribMapIter; 
		typedef std::map<uint, brInternalVertexDesc>::const_iterator AttribMapConstIter; 

		AttribMap			m_inputSlots[MAX_VERTEX_INPUT_SLOTS];			
		GLsizei				m_strides[MAX_VERTEX_INPUT_SLOTS];
		std::vector<bool>	m_activated;
		uint				m_vertexSize;
	};

} // render
} // brUGE

#endif /*_BR_GLVERTEXLAYOUT_H_*/