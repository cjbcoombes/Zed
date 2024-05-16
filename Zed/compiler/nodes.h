#include "..\utils\utils.h"

namespace compiler {
	namespace ast {
		
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Nodes

		enum class NodeType {
			UNIMPL,
			TOKEN,
			BLOCK
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

			UnimplNode(const char* msg, int line, int column) : Node(NodeType::UNIMPL, line, column), msg(msg) {}
			UnimplNode(std::string msg, int line, int column) : Node(NodeType::UNIMPL, line, column), msg(msg) {}

			virtual void printSimple(TokenData& tokenData, std::ostream& stream);
		};

		// A Node for a block (just a group of other Nodes)
		class BlockNode : public Node {
		public:
			std::list<Node*> nodes;

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