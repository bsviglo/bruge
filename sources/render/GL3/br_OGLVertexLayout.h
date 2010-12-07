#ifndef _BR_OGLVERTEXLAYOUT_H_
#define _BR_OGLVERTEXLAYOUT_H_

#include "br_OGLCommon.h"
#include "render/ibr_VertexLayout.h"
#include "render/ibr_ShadingProgram.h"
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
			MAX_VERTEX_ATTRIBS = 16 // TODO: ����� ��� ���� �� ������ ������ ��� OpenGL.
		};

		struct brInternalVertexDesc
		{
			GLsizei size;
			GLenum type;
			GLboolean normalized;
			GLubyte *offset;
		};

		typedef std::map<uint, brInternalVertexDesc> AttribMap; // ����� �������� : ��������� ���������.
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