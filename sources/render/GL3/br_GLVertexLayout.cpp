#include "br_GLVertexLayout.h"

namespace brUGE
{
namespace render
{
	//------------------------------------------
	brGLVertexLayout::brGLVertexLayout(
		const brVertexDesc *vd,
		uint count,
		const Ptr<ibrShadingProgram>& /* shader */
		)
		:	m_vertexSize(0)
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			m_strides[i] = 0;

		m_activated = std::vector<bool>(MAX_VERTEX_ATTRIBS, false);

		for (uint i = 0; i < count; ++i)
		{
			const brVertexDesc& desc = vd[i];
			_addVertexAttribute(
				desc.inputSlot,
				desc.attribute,
				_convertFormat(desc.type),
				desc.componentsCount, 
				desc.normalized
				);
		}

		_calculateStrides();
	}
	
	//------------------------------------------		
	brGLVertexLayout::~brGLVertexLayout()
	{

	}
	
	//------------------------------------------
	uint brGLVertexLayout::getVertexSize() const
	{
		return m_vertexSize;
	}
	
	//------------------------------------------
	GLenum brGLVertexLayout::_convertFormat(ebrTypeFormat format)
	{
		switch (format) 
		{
		case TF_BYTE:		return GL_BYTE;
		case TF_UBYTE:		return GL_UNSIGNED_BYTE;
		case TF_SHORT:		return GL_SHORT;
		case TF_USHORT:		return GL_UNSIGNED_SHORT;
		case TF_INT:		return GL_INT;
		case TF_UINT:		return GL_UNSIGNED_INT;
		case TF_FLOAT:		return GL_FLOAT;
		default:
			BR_EXCEPT("brOGLVertexLayout::_convertFormat", "Invalid type format.");
		}
	}
	
	//------------------------------------------
	GLsizei brGLVertexLayout::_typeSizeOf(GLenum type)
	{
		switch (type) 
		{
		case GL_BYTE:				return sizeof(GLubyte);
		case GL_UNSIGNED_BYTE:		return sizeof(GLubyte);
		case GL_SHORT:				return sizeof(GLushort);
		case GL_UNSIGNED_SHORT:		return sizeof(GLushort);
		case GL_INT:				return sizeof(GLuint);
		case GL_UNSIGNED_INT:		return sizeof(GLuint);
		case GL_FLOAT:				return sizeof(GLfloat);
		default:
			BR_EXCEPT("Invalid OpenGL type.");
		}
	}
	
	//------------------------------------------
	void brGLVertexLayout::_addVertexAttribute(
		uint inputSlot,
		ebrVertexAttribute vertAttr,
		GLenum type,
		uint componentsCount,
		bool normalized)
	{
		GLuint index = vertAttr - VA_ATTR0;

		if (inputSlot > MAX_VERTEX_INPUT_SLOTS)
			BR_EXCEPT("inputSlot is very big. It must be less then MAX_VERTEX_INPUT_SLOTS.");

		if (index > static_cast<GLuint>(MAX_VERTEX_ATTRIBS))
			BR_EXCEPT("Invalid 'vertAttr' parameter.");

		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			if (m_inputSlots[i].find(index) != m_inputSlots[i].end())
				BR_EXCEPT_F("Input vertex attribute index already exist in %d input slot.", i);

		if (componentsCount <= 0 || componentsCount > 4)
			BR_EXCEPT_F("Invalid 'componentsCount' attribute. It must be 1, 2, 3 or 4. Current value = %d.", componentsCount);
		
		brInternalVertexDesc desc;
		desc.type		= type;
		desc.size		= componentsCount;
		desc.normalized = normalized;
		desc.offset		= 0;

		m_inputSlots[inputSlot][index] = desc;
		m_activated[index] = true;
	}

	//------------------------------------------
	void brGLVertexLayout::_calculateStrides()
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
		{
			if (m_inputSlots[i].empty())
				continue;

			GLint bytes = 0;

			for (AttribMapIter iter = m_inputSlots[i].begin(); iter != m_inputSlots[i].end(); ++iter)
			{
				iter->second.offset = reinterpret_cast<GLubyte*>(bytes);
				bytes += iter->second.size * _typeSizeOf(iter->second.type);
			}

			m_vertexSize += bytes;
			m_strides[i] = (m_inputSlots[i].size() == 1) ? 0 : bytes;
		}

		if (m_vertexSize == 0)
			BR_EXCEPT("Vertex structure description is empty.");
	}

} // render
} // brUGE