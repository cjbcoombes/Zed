#include "compiler.h"

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

int compiler::initAST(compiler::ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace ast;
	TokenData& tokenData = astTree.tokenData;

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
		auto next = i + 1;
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
			case TokenType::LEFT_ANGLE:
				// TODO: this might not be the correct way to determine if a left angle is specifying a type
				if (next != tokenData.tokens.end() && ((*next).type == TokenType::LEFT_ANGLE || isTypeToken((*next).type))) {
					groups.push(astTree.addNode<NodeGroup>(std::make_unique<NodeGroup>(&token, NodeType::ANGLE_GROUP)));
					topLine = token.line;
					topCol = token.column;
				} else {
					goto doDefaultCase;
				}
				break;
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
			case TokenType::RIGHT_ANGLE:
				if (groups.top()->type == NodeType::ANGLE_GROUP) {
					temp = groups.top();
					groups.pop();
					groups.top()->elems.push_back(temp);
				} else {
					goto doDefaultCase;
				}
				break;
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
			case TokenType::TRUE:
				groups.top()->elems.push_back(astTree.addNode<NodeBool>(std::make_unique<NodeBool>(&token, true)));
				break;
			case TokenType::FALSE:
				groups.top()->elems.push_back(astTree.addNode<NodeBool>(std::make_unique<NodeBool>(&token, false)));
				break;
			case TokenType::IDENTIFIER:
				groups.top()->elems.push_back(astTree.addNode<NodeIdentifier>(std::make_unique<NodeIdentifier>(&token, token.strIndex)));
				break;
			case TokenType::TYPE_INT:
				groups.top()->elems.push_back(astTree.addNode<NodeTypeSpec>(std::make_unique<NodeTypeSpec>(&token, TypeData::intIndex)));
				break;
			case TokenType::TYPE_FLOAT:
				groups.top()->elems.push_back(astTree.addNode<NodeTypeSpec>(std::make_unique<NodeTypeSpec>(&token, TypeData::floatIndex)));
				break;
			case TokenType::TYPE_CHAR:
				groups.top()->elems.push_back(astTree.addNode<NodeTypeSpec>(std::make_unique<NodeTypeSpec>(&token, TypeData::charIndex)));
				break;
			case TokenType::TYPE_BOOL:
				groups.top()->elems.push_back(astTree.addNode<NodeTypeSpec>(std::make_unique<NodeTypeSpec>(&token, TypeData::boolIndex)));
				break;

			doDefaultCase:
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