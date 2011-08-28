#pragma once

class PhysicsUI
{
public:
	PhysicsUI();
	~PhysicsUI();



private:


public:

	//----------------------------------------------------------------------------------------------
	struct Shape
	{
		Shape();
		virtual ~Shape() = 0;

		virtual void display() = 0;
		virtual void load(const brUGE::ROData& iData) = 0;
		virtual void save(brUGE::WOData& oData) = 0;
	};
	std::unique_ptr<Shape> ShapePtr;

	//----------------------------------------------------------------------------------------------
	struct RigidBody
	{
		void display();
		void load(const brUGE::ROData& iData);
		void save(brUGE::WOData& oData);

		std::string	m_name;
		std::string	m_node;
		float		m_mass;
		vec3f		m_offset;
		bool		m_isKinematic;
		ShapePtr	m_shape;
	};

	//----------------------------------------------------------------------------------------------
	struct PhysObj
	{
		void display();
		void load(const brUGE::ROData& iData);
		void save(brUGE::WOData& oData);

		std::map<std::string, RigidBody> m_bodies;
	};
};