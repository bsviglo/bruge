#pragma once

#include "prerequisites.hpp"

namespace brUGE
{

	//-- Base class for all of the possible components in the engine.
	//-- Note: IComponent should provide default (empty) constructor to be able to create IComponent in the default state.
	//------------------------------------------------------------------------------------------------------------------
	class IComponent
	{
	public:
		//-- Structure that holds unique ID of a IComponent derived class.
		//-- Note: it is unique only across single execution of one application.It is NOT suitable for serialization.
		//--------------------------------------------------------------------------------------------------------------
		class TypeID final
		{
		public:
			static const uint32 C_MAX_COMPONENT_TYPES = 16;

		public:
			TypeID();

			uint32 id() const		{ return m_id; }
			operator uint32() const { return m_id; }

		private:
			uint32 m_id;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Handle final
		{
		public:
			Handle(TypeID typeID, ISystem::TypeID systemTypeID, ::Handle handle) : m_typeID(typeID), m_systemTypeID(systemTypeID), m_handle(handle) { }

			uint32		typeID() const			{ return m_typeID; }
			uint32		systemTypeID() const	{ return m_systemTypeID; }
			::Handle	handle() const			{ return m_handle; }

			//-- ToDo: provide other operations

		private:
			uint32		m_typeID		: 16;
			uint32		m_systemTypeID	: 16;
			::Handle	m_handle;
		};

	public:
		IComponent(::Handle owner = CONST_INVALID_HANDLE)	: m_owner(owner) { }
		virtual ~IComponent() = 0 { }

		virtual void		serialize(pugi::xml_node& oData) const = 0;
		virtual void		deserialize(const pugi::xml_node& iData) = 0;

		//-- handle on the GameObject
		inline ::Handle		owner() const	{ return m_owner; } 

	protected:
		::Handle m_owner;
	};
}
