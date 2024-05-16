#include "..\utils\utils.h"

namespace compiler {
	namespace ast {
		
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Nodes

		enum class NodeType {
			TOKEN,
			ROOT_GROUP,
			UNKNOWN_GROUP
		};

		class Node {
		public:
			NodeType type;
			int line;
			int column;

			Node(NodeType type, int line, int column) : type(type), line(line), column(column) {}
		};

		class GroupNode : public Node {
			std::list<Node*> nodes;

			GroupNode(NodeType type, int line, int column) : Node(type, line, column) {}
		};

		class TokenNode : public Node {
		public:
			Token* token;

			TokenNode(Token* token) : Node(NodeType::TOKEN, token->line, token->column), token(token) {}
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Tree

		class Tree {
		public:
			std::list<std::unique_ptr<Node>> nodes;
			Node* root;

			template<class N>
			N* add(std::unique_ptr<N> node);
		};
	}
}