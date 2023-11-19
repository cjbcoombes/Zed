#include "..\utils\utils.h"

namespace compiler {
	namespace ast {

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Nodes

		// An enum of node types
		enum class NodeType {
			TOKEN,
			ROOT_GROUP,
			PAREN_GROUP,
			SQUARE_GROUP,
			CURLY_GROUP,
			ANGLE_GROUP
		};

		// Root node class
		class Node {
		public:
			NodeType type;

			Node(NodeType type) : type(type) {}
			virtual ~Node() {}
			virtual void print(std::ostream& stream, std::string&& indent, bool last);
		};

		class Expr : public Node {
			// ExprType -- Some storage of type
		};

		// A node wrapping a token
		class NodeToken : public Node {
		public:
			Token token;

			NodeToken(Token token) : Node(NodeType::TOKEN), token(token) {}
			void print(std::ostream& stream, std::string&& indent, bool last);
		};

		// A node containing a list of other nodes
		// Used for groups in parens, brackets, etc.
		class NodeGroup : public Node {
		public:
			std::vector<Node*> elems;

			NodeGroup(NodeType type) : Node(type) {}
			void print(std::ostream& stream, std::string&& indent, bool last);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Tree

		class Tree {
		public:
			// Is responsible for the memory management of all nodes
			std::vector<std::unique_ptr<Node>> nodes;
			// Contains the root of the tree
			Node* root;

			Tree() : root(nullptr), nodes() {}

			void print(std::ostream& stream);

			// Adds a node to the list and returns a pointer to the node
			template<class N>
			N* addNode(std::unique_ptr<N> n) {
				N* temp = n.get();
				nodes.push_back(std::move(n));
				return temp;
			}
		};


	}
}