#pragma once

#include <vector>

namespace brUGE
{
namespace utils
{

	//----------------------------------------------------------------------------------------------
	template<typename CHAR, typename VALUE>
	class TernaryTree
	{
	public:
		typedef int NodeIndex;

	public:
		TernaryTree() {}
		
		//------------------------------------------
		int numNodes() const { return (int)m_nodes.size(); }

		// does no insertion if item already exists. 
		//------------------------------------------
		void insert(const CHAR* s, const VALUE& value)
		{
			_insert(0, s, value);
		}

		// returns true or false if not found.
		//------------------------------------------
		bool search(const CHAR* s, VALUE*& value)
		{   
			NodeIndex nId = 0; // root index;
			while (nId >= 0 && nId < numNodes()) 
			{
				Node& node = m_nodes[nId];
				if (*s < node.splitchar)
				{
					nId = node.lokid;
				}
				else if(*s > node.splitchar)
				{
					nId = node.hikid;
				}
				else //(*s == node.splitchar) 
				{
					if (*s++ == 0)
					{
						value = &node.val;
						return true;
					}
					nId = node.eqkid;
				} 
			}
			return false;
		}

	private:
		struct Node 
		{
			Node(CHAR s) : splitchar(s), lokid(-1), eqkid(-1), hikid(-1) {}

			CHAR      splitchar;
			NodeIndex lokid, eqkid, hikid;
			VALUE	  val;
		};

		std::vector<Node> m_nodes;
		
		//------------------------------------------
		NodeIndex _insert(NodeIndex nId, const CHAR* s, const VALUE& iVal)
		{ 
			if (nId < 0 || nId >= numNodes()) 
			{
				m_nodes.push_back(Node(*s));
				nId = m_nodes.size() - 1; 
			}
			const Node& node = m_nodes[nId];

			if (*s < node.splitchar)
			{
				// Note: we use m_nodes[nId] instead of just a node, declared above, because
				// m_nodes vector can be reallocated, and previous defined node was not more
				// a valid reference. This is because we haven got a recursion.
				//m_nodes[nId].lokid = _insert(node.lokid, s, iVal);
				NodeIndex _index = _insert(node.lokid, s, iVal);
				m_nodes[nId].lokid = _index;
			}
			else if (*s > node.splitchar)
			{
				//m_nodes[nId].hikid = _insert(node.hikid, s, iVal);
				NodeIndex _index = _insert(node.hikid, s, iVal);
				m_nodes[nId].hikid = _index;
			}
			else //*s == node.splitchar 
			{
				if (*s == 0)
				{
					// We'll exploit the fact that a node with a null splitchar cannot have
					// an eqkid, and store the data in that field.
					m_nodes[nId].val = iVal;
				}
				else
				{
					//m_nodes[nId].eqkid = _insert(node.eqkid, ++s, iVal);
					NodeIndex _index = _insert(node.eqkid, ++s, iVal);
					m_nodes[nId].eqkid = _index;
				}
			}
			return nId;
		}
	};

} // utils
} // brUGE