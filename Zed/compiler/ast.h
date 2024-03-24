#include "..\utils\utils.h"

namespace compiler {
	namespace ast {

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Types

		// <int, float, char>
		// <int, <float, char>, <char, <int, float, char>>>
		// 
		// [
		//  0: void
		//  1: int
		//  2: float
		//  3: char
		//  4: bool
		//  5: [0,1,2]
		//  6: [1,2]
		//  7: [2,3]
		//  8: [1,4,5]
		// 
		// 
		// ]

		struct TypeInfo {
			std::vector<int> types;
			std::vector<int> typeStrIndices;
			int returnType;

			TypeInfo() : types(0), typeStrIndices(0), returnType(-1) {}
		};

		class TypeData {
		public:

			static constexpr int voidIndex = 0;
			static constexpr int intIndex = 1;
			static constexpr int floatIndex = 2;
			static constexpr int charIndex = 3;
			static constexpr int boolIndex = 4;

			std::vector<TypeInfo> types;

			TypeData() : types({ TypeInfo(), TypeInfo(), TypeInfo(), TypeInfo(), TypeInfo() }) {}
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Nodes

		// An enum of node types
		enum class NodeType {
			TOKEN,
			MACRO,
			TYPESPEC,

			INT,
			FLOAT,
			CHAR,
			BOOL,
			STRING,
			IDENTIFIER,

			ROOT_GROUP,
			PAREN_GROUP,
			SQUARE_GROUP,
			CURLY_GROUP,
			ANGLE_GROUP,

			ARITH_BINOP
		};

		// Root node class
		class Node {
		public:
			NodeType type;
			Token* token;
			bool isExpr;

			Node(Token* token, NodeType type) : type(type), token(token), isExpr(false) {}
			Node(Token* token, NodeType type, bool isExpr) : type(type), token(token), isExpr(isExpr) {}
			virtual ~Node() {}
			void printToken(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
			virtual void print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last);
			virtual void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		class Expr : public Node {
		public:
			int typeIndex;

			Expr(Token* token, NodeType type) : Node(token, type, true), typeIndex(-1) {}
			Expr(Token* token, NodeType type, int typeIndex) : Node(token, type, true), typeIndex(typeIndex) {}
		};

		// A node wrapping a token
		class NodeToken : public Node {
		public:

			NodeToken(Token* token) : Node(token, NodeType::TOKEN) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		class NodeMacro : public Node {
		public:
			int strIndex;

			NodeMacro(Token* token, int strIndex) : Node(token, NodeType::MACRO), strIndex(strIndex) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node representing a type declaration
		class NodeTypeSpec : public Node {
		public:
			int typeIndex;

			NodeTypeSpec(Token* token, int typeIndex) : Node(token, NodeType::TYPESPEC), typeIndex(typeIndex) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping an int
		class NodeInt : public Expr {
		public:
			int val;

			NodeInt(Token* token, int val) : Expr(token, NodeType::INT, TypeData::intIndex), val(val) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping a float
		class NodeFloat : public Expr {
		public:
			float val;

			NodeFloat(Token* token, float val) : Expr(token, NodeType::FLOAT, TypeData::floatIndex), val(val) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping a char
		class NodeChar : public Expr {
		public:
			char val;

			NodeChar(Token* token, char val) : Expr(token, NodeType::CHAR, TypeData::charIndex), val(val) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping a bool
		class NodeBool : public Expr {
		public:
			bool val;

			NodeBool(Token* token, bool val) : Expr(token, NodeType::BOOL, TypeData::boolIndex), val(val) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping a string
		class NodeString : public Expr {
		public:
			int strIndex;

			NodeString(Token* token, int strIndex) : Expr(token, NodeType::STRING, -1/* TODO : Type of strings? */), strIndex(strIndex) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node wrapping an identifier
		class NodeIdentifier : public Expr {
		public:
			int strIndex;
			int scopeIndex; // TODO : scopes?

			NodeIdentifier(Token* token, int strIndex) : Expr(token, NodeType::IDENTIFIER, -1), strIndex(strIndex), scopeIndex(-1) {}
			void printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream);
		};

		// A node containing a list of other nodes
		// Used for groups in parens, brackets, etc.
		class NodeGroup : public Node {
		public:
			std::list<Node*> elems;

			NodeGroup(Token* token, NodeType type) : Node(token, type) {
				assert(type == NodeType::ANGLE_GROUP ||
					   type == NodeType::CURLY_GROUP ||
					   type == NodeType::PAREN_GROUP ||
					   type == NodeType::ROOT_GROUP ||
					   type == NodeType::SQUARE_GROUP);
			}
			void print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last);
		};

		class NodeArithBinop : public Expr {
		public:
			enum class OpType {
				ADD,
				SUB,
				MUL,
				DIV
				// MOD?
			};

			Expr* left;
			Expr* right;
			OpType type;

			NodeArithBinop(Token* token, Expr* left, Expr* right, OpType type, int typeIndex) : Expr(token, NodeType::ARITH_BINOP, typeIndex), left(left), right(right), type(type) {}
			void print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Tree

		class Tree {
		public:
			// Is responsible for the memory management of all nodes
			std::vector<std::unique_ptr<Node>> nodes;
			// Contains the root of the tree
			Node* root;
			TokenData& tokenData;
			TypeData typeData;

			Tree(TokenData& tokenData) : root(nullptr), nodes(), tokenData(tokenData), typeData() {}

			void print(std::ostream& stream);

			// Adds a node to the list and returns a pointer to the node
			template<class N>
			N* addNode(std::unique_ptr<N> n);
		};


	}
}