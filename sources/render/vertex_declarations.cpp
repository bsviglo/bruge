#include "vertex_declarations.hpp"
#include "render_system.hpp"
#include "IRenderDevice.h"
#include "IShader.h"
#include "os/FileSystem.h"
#include "pugixml/pugixml.hpp"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::os;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const char* cfg = "resources/system/vertex_formats.xml";

	//----------------------------------------------------------------------------------------------
	EAttributeType getAttributeType(const std::string& str)
	{
		if		(str == "byte")		return TYPE_BYTE;
		else if (str == "short")	return TYPE_SHORT;
		else if (str == "int")		return TYPE_INT;
		else if (str == "uint")		return TYPE_UINT;
		else if (str == "float")	return TYPE_FLOAT;
		else						return TYPE_UNKNOWN;
	}

	//----------------------------------------------------------------------------------------------
	EAttributeSemantic getSemantic(const std::string& str)
	{
		if		(str == "POSITION")		return SEMANTIC_POSITION;
		else if (str == "TEXCOORD")		return SEMANTIC_TEXCOORD;	
		else if (str == "TEXCOORD0")	return SEMANTIC_TEXCOORD0;
		else if (str == "TEXCOORD1")	return SEMANTIC_TEXCOORD1;
		else if (str == "TEXCOORD2")	return SEMANTIC_TEXCOORD2;
		else if	(str == "NORMAL")		return SEMANTIC_NORMAL;
		else if (str == "TANGENT")		return SEMANTIC_TANGENT;	
		else if (str == "BINORMAL")		return SEMANTIC_BINORMAL;	
		else if (str == "COLOR")		return SEMANTIC_COLOR;
		else							return SEMANTIC_UNKNOWN;
	}

	typedef std::vector<VertexDesc>	VertexDclr;

	//-- load vertex format from xml data section.
	//----------------------------------------------------------------------------------------------
	bool loadVertexDesc(std::string& name, VertexDclr& vDlcr, const pugi::xml_node& section)
	{
		if (auto param = section.attribute("name"))
		{
			name = section.attribute("name").value();
		}
		else return false;

		//-- ToDo: more checking.
		for (auto attr = section.child("attr"); attr; attr = attr.next_sibling("attr"))
		{
			VertexDesc desc;
			desc.stream   = attr.attribute("stream").as_uint();
			desc.size     = attr.attribute("size").as_uint();
			desc.semantic = getSemantic(attr.attribute("semantic").value());
			desc.type     = getAttributeType(attr.attribute("type").value());

			vDlcr.push_back(desc);
		}

		return true;
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	VertexDeclarations::VertexDeclarations()
	{

	}

	//----------------------------------------------------------------------------------------------
	VertexDeclarations::~VertexDeclarations()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool VertexDeclarations::init()
	{
		RODataPtr data = FileSystem::instance().readFile(cfg);
		if (!data.get())
		{
			ERROR_MSG("Can't load vertex formats cfg file.");
			return false;
		}

		pugi::xml_document doc;
		if (!doc.load_buffer(data->ptr(), data->length()))
		{
			ERROR_MSG("Most likely loading file '%s' is corrupted.", cfg);
			return false;
		}

		pugi::xml_node cfgNode = doc.document_element();
		
		for (auto prop = cfgNode.child("format"); prop; prop = prop.next_sibling("format"))
		{
			std::string name;
			VertexDclr  vDclr;

			if (loadVertexDesc(name, vDclr, prop))
			{
				m_map[name] = vDclr;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	VertexLayoutID VertexDeclarations::get(const char* name, const IShader& shader)
	{
		auto iter = m_map.find(name);
		if (iter != m_map.end())
		{
			VertexDclr& vDclr = iter->second;
			return rd()->createVertexLayout(&vDclr[0], vDclr.size(), shader);
		}
		return CONST_INVALID_HANDLE;
	}

} //-- render
} //-- brUGE