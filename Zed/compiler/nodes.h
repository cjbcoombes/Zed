#include "..\utils\utils.h"

namespace compiler {
	namespace ast {
		
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Nodes

		enum class NodeType {
			UNIMPL,
			TOKEN,
			BLOCK,
			ARITH_BINOP
		};

		// Parent Node
		class Node {
		public:
			NodeType type;
			int line;
			int column;

			Node(NodeType type, int line, int column) : type(type), line(line), column(column) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A Node for currently unimplemented cases
		class UnimplNode : public Node {
		public:
			std::string msg;
			std::vector<Node*> nodes;

			UnimplNode(const char* msg, int line, int column) : Node(NodeType::UNIMPL, line, column), msg(msg), nodes() {}
			//UnimplNode(std::string msg, int line, int column) : Node(NodeType::UNIMPL, line, column), msg(msg) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// A Node for a block (just a group of other Nodes)
		class BlockNode : public Node {
		public:
			std::vector<Node*> nodes;

			BlockNode(int line, int column) : Node(NodeType::BLOCK, line, column) {}

			virtual void print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last);
		};

		// A Node for a token
		class TokenNode : public Node {
		public:
			Token* token;

			TokenNode(Token* token) : Node(NodeType::TOKEN, token->line, token->column), token(token) {}

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

			ArithBinopNode(Type opType, Node* left, Node* right, int line, int column)
				: Node(NodeType::ARITH_BINOP, line, column), opType(opType), left(left), right(right) {}

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