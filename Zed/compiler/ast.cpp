#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree Printing

#define TREE_BRANCH_MID "+-- "
#define TREE_BRANCH_END "\\-- "
#define TREE_SPACE "    "
#define TREE_PASS "|   "

void compiler::ast::Node::print(std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "??? Node ??? \n";
}

void compiler::ast::NodeToken::print(std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "NodeToken \n";
}

void compiler::ast::NodeGroup::print(std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "NodeGroup ";
	if (type == NodeType::ROOT_GROUP) stream << "--root--";
	else if (type == NodeType::PAREN_GROUP) stream << "()";
	else if (type == NodeType::SQUARE_GROUP) stream << "[]";
	else if (type == NodeType::CURLY_GROUP) stream << "{}";
	else if (type == NodeType::ANGLE_GROUP) stream << "<>";
	else stream << "??";
	stream << '\n';
	
	for (auto i = elems.begin(); i < elems.end(); i++) {
		(*i)->print(stream, indent + (last ? TREE_SPACE : TREE_PASS), i + 1 == elems.end());
	}
}


void compiler::ast::Tree::print(std::ostream& stream) {
	if (root) root->print(stream, "", true);
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
	NodeGroup* root = astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(NodeType::ROOT_GROUP));
	astTree.root = root;
	groups.push(root);

	NodeGroup* temp;
	int topLine = -1;
	int topCol = -1;

	// For each token, opening braces push a new group to the stack, closing braces pop a group but check that
	// it's the right type, and other tokens are just added to the top group
	for (const Token& token : tokenData.tokens) {
		switch (token.type) {
			case TokenType::LEFT_PAREN:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(NodeType::PAREN_GROUP)));
				topLine = token.line;
				topCol = token.column;
				break;
			case TokenType::LEFT_SQUARE:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(NodeType::SQUARE_GROUP)));
				topLine = token.line;
				topCol = token.column;
				break;
			case TokenType::LEFT_CURLY:
				groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(NodeType::CURLY_GROUP)));
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
			default:
				groups.top()->elems.push_back(astTree.addNode<NodeToken>(std::make_unique<NodeToken>(token)));
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