#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree Printing

#define TREE_BRANCH_MID "+-- "
#define TREE_BRANCH_END "\\-- "
#define TREE_SPACE "    "
#define TREE_PASS "|   "

void compiler::ast::Node::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	printSimple(tokenData, stream);
}

void compiler::ast::Node::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "??? Node ??? \n";
}

void compiler::ast::NodeToken::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Token ( ";
	if (token) {
		printToken(*token, tokenData, stream);
	} else {
		stream << "NO TOKEN";
	}
	stream << " )\n";
}

void compiler::ast::NodeInt::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Int " << val << '\n';
}

void compiler::ast::NodeFloat::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Float " << val << '\n';
}

void compiler::ast::NodeChar::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Char \'" << val << "\'\n";
}

void compiler::ast::NodeString::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "String \"" << tokenData.strList[strIndex] << "\"\n";
}

void compiler::ast::NodeGroup::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Group ";
	if (type == NodeType::ROOT_GROUP) stream << "--root--";
	else if (type == NodeType::PAREN_GROUP) stream << "()";
	else if (type == NodeType::SQUARE_GROUP) stream << "[]";
	else if (type == NodeType::CURLY_GROUP) stream << "{}";
	else if (type == NodeType::ANGLE_GROUP) stream << "<>";
	else stream << "??";
	stream << '\n';

	for (auto i = elems.cbegin(); i < elems.cend(); i++) {
		(*i)->print(tokenData, stream, indent + (last ? TREE_SPACE : TREE_PASS), i + 1 == elems.end());
	}
}

void compiler::ast::Tree::print(TokenData& tokenData, std::ostream& stream) {
	if (root) root->print(tokenData, stream, "", true);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AST Tree Functions

template<class N>
N* compiler::ast::Tree::addNode(std::unique_ptr<N> n) {
	N* temp = n.get();
	nodes.push_back(std::move(n));
	return temp;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AST Functions

int compiler::initAST(compiler::ast::Tree& astTree, TokenData& tokenData, CompilerSettings& settings, std::ostream& stream) {
	using namespace ast;

	// Flags/settings
	const bool isDebug = settings.flags.hasFlags(Flags::FLAG_DEBUG);

	// Stores the current "stack" of parenthesis/bracket/etc. groups that we're inside of
	std::stack<NodeGroup*> groups;

	// The "root" node is a group
	NodeGroup* root = astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(nullptr, NodeType::ROOT_GROUP));
	astTree.root = root;
	groups.push(root);

	NodeGroup* temp;
	int topLine = -1;
	int topCol = -1;

	// For each token, opening braces push a new group to the stack, closing braces pop a group but check that
	// it's the right type, and other tokens are just added to the top group
	for (auto i = tokenData.tokens.begin(); i != tokenData.tokens.end(); i++) {
		Token& token = *i;
		switch (token.type) {
			case TokenType::LEFT_PAREN:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(&token, NodeType::PAREN_GROUP)));
				topLine = token.line;
				topCol = token.column;
				break;
			case TokenType::LEFT_SQUARE:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(&token, NodeType::SQUARE_GROUP)));
				topLine = token.line;
				topCol = token.column;
				break;
			case TokenType::LEFT_CURLY:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(&token, NodeType::CURLY_GROUP)));
				topLine = token.line;
				topCol = token.column;
				break;
				// case TokenType::LEFT_ANGLE:
					// break;
			case TokenType::RIGHT_PAREN:
				if (groups.top()->type == NodeType::PAREN_GROUP) {
					temp = groups.top();
					groups.pop();
					groups.top()->elems.push_back(temp);
				} else {
					throw CompilerException(CompilerException::ErrorType::INVALID_CLOSING_PAREN, token.line, token.column);
				}
				break;
			case TokenType::RIGHT_SQUARE:
				if (groups.top()->type == NodeType::SQUARE_GROUP) {
					temp = groups.top();
					groups.pop();
					groups.top()->elems.push_back(temp);
				} else {
					throw CompilerException(CompilerException::ErrorType::INVALID_CLOSING_SQUARE, token.line, token.column);
				}
				break;
			case TokenType::RIGHT_CURLY:
				if (groups.top()->type == NodeType::CURLY_GROUP) {
					temp = groups.top();
					groups.pop();
					groups.top()->elems.push_back(temp);
				} else {
					throw CompilerException(CompilerException::ErrorType::INVALID_CLOSING_CURLY, token.line, token.column);
				}
				break;
				// case TokenType::RIGHT_ANGLE:
					// break;
			case TokenType::NUM_INT:
				groups.top()->elems.push_back(astTree.addNode<NodeInt>(std::make_unique<NodeInt>(&token, token.int_)));
				break;
			case TokenType::NUM_FLOAT:
				groups.top()->elems.push_back(astTree.addNode<NodeFloat>(std::make_unique<NodeFloat>(&token, token.float_)));
				break;
			case TokenType::CHAR:
				groups.top()->elems.push_back(astTree.addNode<NodeChar>(std::make_unique<NodeChar>(&token, token.char_)));
				break;
			case TokenType::STRING:
				groups.top()->elems.push_back(astTree.addNode<NodeString>(std::make_unique<NodeString>(&token, token.strIndex)));
				break;


			default:
				groups.top()->elems.push_back(astTree.addNode<NodeToken>(std::make_unique<NodeToken>(&token)));
				break;
		}
	}

	// If there's anything other then the root group at the end, then something wasn't closed
	if (groups.size() != 1) {
		switch (groups.top()->type) {
			case NodeType::PAREN_GROUP:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_PAREN, topLine, topCol);

			case NodeType::SQUARE_GROUP:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_SQUARE, topLine, topCol);

			case NodeType::CURLY_GROUP:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_CURLY, topLine, topCol);

			case NodeType::ANGLE_GROUP:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_ANGLE, topLine, topCol);
		}
	}

	return 0;
}

// Check if a type is a group
bool isNodeGroup(compiler::ast::NodeType type) {
	using compiler::ast::NodeType;
	return type == NodeType::ROOT_GROUP
		|| type == NodeType::PAREN_GROUP
		|| type == NodeType::SQUARE_GROUP
		|| type == NodeType::CURLY_GROUP;
}

// Fully process a node group into a tree
int reduceNodeGroup(compiler::ast::NodeGroup* group, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	// Pass 1: Reduce child groups and replace primitives
	for (Node*& n : group->elems) {
		if (isNodeGroup(n->type)) {
			reduceNodeGroup(dynamic_cast<NodeGroup*>(n), astTree, settings, stream);
			// Maybe do other stuff
			// Like if the result is a number remove the group wrapper
		}

		// Token int -> Node int
		// Token float -> Node float
		// etc..
	}

	return 0;
}

// Construct the AST
int compiler::constructAST(ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	return reduceNodeGroup(dynamic_cast<NodeGroup*>(astTree.root), astTree, settings, stream);
}