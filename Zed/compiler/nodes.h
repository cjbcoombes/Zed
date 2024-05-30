#include "../utils/utils.h"

namespace compiler {
	namespace ast {
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Types

		// A type in the type system
		// Currently not fleshed-out. Just primitives
		struct ExprType {
			enum class PrimType {
				NONE, ERR, VOID, INT, FLOAT, BOOL, CHAR
			};

			static const ExprType primNoType;
			static const ExprType primErrType;
			static const ExprType primVoid;
			static const ExprType primInt;
			static const ExprType primFloat;
			static const ExprType primBool;
			static const ExprType primChar;

			PrimType type;

			ExprType() : ExprType(ExprType::primNoType) {}
			ExprType(PrimType type) : type(type) {}

			void printSimple(std::ostream& stream);
		};

		bool sameType(const ExprType& a, const ExprType& b);

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Nodes

		enum class NodeType {
			UNIMPL,
			TOKEN,
			BLOCK,
			ARITH_BINOP,
			LITERAL,
			MACRO
		};

		// Parent Node
		class Node {
		public:
			NodeType type;
			ExprType exprType;
			code_location loc;

			Node(NodeType type, code_location loc) : type(type), loc(loc), exprType(ExprType::primNoType) {}
			Node(NodeType type, ExprType exprType, code_location loc) : type(type), loc(loc), exprType(exprType) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
			void printType(std::ostream& stream);
		};

		// A Node for currently unimplemented cases
		class UnimplNode : public Node {
		public:
			std::string msg;
			std::vector<Node*> nodes;

			UnimplNode(const char* msg, code_location loc) : Node(NodeType::UNIMPL, loc), msg(msg), nodes() {}
			//UnimplNode(std::string msg, code_location loc) : Node(NodeType::UNIMPL, loc), msg(msg) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// A Node for a block (just a group of other Nodes)
		class BlockNode : public Node {
		public:
			std::vector<Node*> nodes;

			BlockNode(code_location loc) : Node(NodeType::BLOCK, loc) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// A Node for a token
		class TokenNode : public Node {
		public:
			Token* token;

			TokenNode(Token* token) : Node(NodeType::TOKEN, token->loc), token(token) {}

			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A Node for a binary arithmetic operaton
		class ArithBinopNode : public Node {
		public:
			enum class Type {
				ADD, SUB, MUL, DIV
			};

			Node* left;
			Node* right;
			Type opType;

			ArithBinopNode(Type opType, ExprType exprType, Node* left, Node* right, code_location loc)
				: Node(NodeType::ARITH_BINOP, exprType, loc), opType(opType), left(left), right(right) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// A Node for literal values
		class LiteralNode : public Node {
		public:
			enum class Type {
				INT, FLOAT, BOOL, CHAR
			};

			union {
				bytecode::types::float_t float_;
				bytecode::types::int_t int_;

				bytecode::types::bool_t bool_; // Might not be necessary
				bytecode::types::char_t char_;
			};

			Type litType;

			LiteralNode(Token* token);

			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A Node for Macros
		class MacroNode : public Node {
		public:
			enum class Type {
				HELLO_WORLD,
				PRINT_I,

				Count,
				UNKNOWN
			};

			static constexpr int macroCount = static_cast<int>(Type::Count);
			static constexpr const char* const macroStrings[] = {
				"hello_world",
				"printi"
			};
			static constexpr std::span<const char* const> macroSpan{ macroStrings, macroCount };
			static_assert(sizeof(macroStrings) / sizeof(const char*) == macroCount,
						  "Number of macros does not match list of macros");

			Type macroType;
			Node* arg;

			MacroNode(Type macroType, ExprType exprType, Node* arg, code_location loc) 
				: Node(NodeType::MACRO, exprType, loc), macroType(macroType), arg(arg) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Tree

		class Tree {
		public:
			std::list<std::unique_ptr<Node>> nodes;
			Node* root;

			Tree() : nodes(), root(nullptr) {}

			template<class N>
			N* add(std::unique_ptr<N> node);

			template<class N, class... Args>
			N* make(Args&&... args);

			void print(TokenData& tokenData, std::ostream& stream);
		};
	}
}