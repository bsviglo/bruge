#include "br_OGLVertexLayout.h"
#include "utils/br_String.h"

namespace brUGE
{
namespace render
{
	//------------------------------------------
	brOGLVertexLayout::brOGLVertexLayout(
		const brVertexDesc *vd,
		uint count,
		const brPtr<ibrShadingProgram>& /* shader */)
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			strides_[i] = 0;

		activated_ = std::vector<bool>(MAX_VERTEX_ATTRIBS, false);

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
	brOGLVertexLayout::~brOGLVertexLayout()
	{

	}
	
	//------------------------------------------
	uint brOGLVertexLayout::getVertexSize() const
	{
		return vertexSize_;
	}
	
	//------------------------------------------
	GLenum brOGLVertexLayout::_convertFormat(ebrTypeFormat format)
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
			throw brException("brOGLVertexLayout::_convertFormat",
				"Invalid 'format' attribute. No appropriate format to vertex attribute type found.");
		}
	}
	
	//------------------------------------------
	GLsizei brOGLVertexLayout::_typeSizeOf(GLenum type)
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
			throw brException("brOGLVertexLayout::_typeSizeOf", "Invalid type.");
		}
	}
	
	//------------------------------------------
	void brOGLVertexLayout::_addVertexAttribute(
		uint inputSlot,
		ebrVertexAttribute vertAttr,
		GLenum type,
		uint componentsCount,
		bool normalized)
	{
		if (inputSlot > MAX_VERTEX_INPUT_SLOTS)
			throw brException("brOGLVertexLayout::_addVertexAttribute",
				"inputSlot is very big. It must be less then MAX_VERTEX_INPUT_SLOTS.");

		GLuint index = vertAttr - VA_ATTR0;

		if (index > static_cast<GLuint>(MAX_VERTEX_ATTRIBS))
			throw brException("brOGLVertexLayout::_addVertexAttribute",
				"Invalid 'vertAttr' parameter."
				"Attribute index implementation-dependent of constant GL_MAX_VERTEX_ATTRIBS");

		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			if (inputSlots_[i].find(index) != inputSlots_[i].end())
				throw brException("brOGLVertexLayout::_addVertexAttribute",
					"Input vertex attribute index already exist in " + utils::brStr(i) + " input slot.");

		if (componentsCount <= 0 || componentsCount > 4)
			throw brException("brOGLVertexLayout::_addVertexAttribute",
				"Invalid 'componentsCount' attribute. It must be 1, 2, 3 or 4. Current value = " + brStr::toString(componentsCount));
		
		brInternalVertexDesc desc;
		desc.type		= type;
		desc.size		= componentsCount;
		desc.normalized = normalized;
		desc.offset		= 0;

		inputSlots_[inputSlot][index] = desc;
		activated_[index] = true;
	}

	//------------------------------------------
	void brOGLVertexLayout::_calculateStrides()
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
		{
			if (inputSlots_[i].empty())
				continue;

			GLint bytes = 0;

			for (AttribMapIter iter = inputSlots_[i].begin(); iter != inputSlots_[i].end(); ++iter)
			{
				iter->second.offset = reinterpret_cast<GLubyte*>(bytes);
				bytes += iter->second.size * _typeSizeOf(iter->second.type);
			}

			vertexSize_ += bytes;
			strides_[i] = (inputSlots_[i].size() == 1) ? 0 : bytes;
		}

		if (vertexSize_ == 0)
			throw brException("brOGLVertexLayout::_calculateStrides", "Vertex structure description is empty.");
	}
}
}