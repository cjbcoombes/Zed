#include "..\utils\utils.h"

namespace compiler {
	namespace expr {
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Expression Types

		enum class PrimType {
			INT,
			FLOAT,
			CHAR,
			// STRING?
			Count
		};

		constexpr int primTypeCount = static_cast<int>(PrimType::Count);

		struct TypeInfo {
			std::vector<int> subtypes;

			TypeInfo() : subtypes(0) {}
		};

		// <int, float, char>
		// <int, <float, char>, <char, <int, float, char>>>
		// 
		// [
		//  0: int
		//  1: float
		//  2: char
		//  3: [0,1,2]
		//  4: [1,2]
		//  5: [2,3]
		//  6: [1,4,5]
		// 
		// 
		// ]


		class TypeSet {
			std::vector<TypeInfo> types;
		};

	}
	namespace ast {

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Nodes

		// An enum of node types
		enum class NodeType {
			TOKEN,
			INT,
			FLOAT,
			CHAR,
			STRING,
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
			Token* token;

			Node(Token* token, NodeType type) : type(type), token(token) {}
			virtual ~Node() {}
			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		class Expr : public Node {
		public:
			// ExprType -- Some storage of type
			Expr(Token* token, NodeType type) : Node(token, type) {}
		};

		// A node wrapping a token
		class NodeToken : public Node {
		public:

			NodeToken(Token* token) : Node(token, NodeType::TOKEN) {}
			void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A node wrapping an int
		class NodeInt : public Expr {
		public:
			int val;

			NodeInt(Token* token, int val) : Expr(token, NodeType::INT), val(val) {}
			void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A node wrapping a float
		class NodeFloat : public Expr {
		public:
			float val;

			NodeFloat(Token* token, float val) : Expr(token, NodeType::FLOAT), val(val) {}
			void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A node wrapping a char
		class NodeChar : public Expr {
		public:
			char val;

			NodeChar(Token* token, char val) : Expr(token, NodeType::CHAR), val(val) {}
			void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A node wrapping a string
		class NodeString : public Expr {
		public:
			int strIndex;

			NodeString(Token* token, int strIndex) : Expr(token, NodeType::STRING), strIndex(strIndex) {}
			void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A node containing a list of other nodes
		// Used for groups in parens, brackets, etc.
		class NodeGroup : public Node {
		public:
			std::vector<Node*> elems;

			NodeGroup(Token* token, NodeType type) : Node(token, type) {
				assert(type == NodeType::ANGLE_GROUP ||
					   type == NodeType::CURLY_GROUP ||
					   type == NodeType::PAREN_GROUP ||
					   type == NodeType::ROOT_GROUP ||
					   type == NodeType::SQUARE_GROUP);
			}
			void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
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

			void print(TokenData& tokenData, std::ostream& stream);

			// Adds a node to the list and returns a pointer to the node
			template<class N>
			N* addNode(std::unique_ptr<N> n);
		};


	}
}