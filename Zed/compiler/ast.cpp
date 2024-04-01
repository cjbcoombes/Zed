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

compiler::ast::Node* constructTypespec(compiler::ast::NodeGroup* group, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	// TODO: this later

	// Reduce to a flat group of only allowed nodes
	/*for (auto i = group->elems.begin(); i != group->elems.end(); i++) {
		if ((*i)->type == NodeType::ANGLE_GROUP) {
			(*i) = constructTypespec(dynamic_cast<NodeGroup*>(*i), astTree, settings, stream);
			continue;
		}
		if ((*i)->type == NodeType::IDENTIFIER || (*i)->type == NodeType::TYPESPEC) continue;
		if ((*i)->type == NodeType::TOKEN) {
			if ((*i)->token->type == TokenType::PERIOD || (*i)->token->type == TokenType::COMMA) continue;
		}
		throw CompilerException(CompilerException::ErrorType::INVALID_TYPE_TOKEN, (*i)->token->line, (*i)->token->column);
	}*/

	return group;
}

void slideWindow2(compiler::ast::Node* (*windowFunc)(compiler::ast::Node*, compiler::ast::Node*, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream),
				  compiler::ast::NodeGroup* group, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	std::list<Node*>& nodes = group->elems;

	if (nodes.size() < 2) return;

	std::list<Node*>::iterator rangeStart = nodes.begin();
	std::list<Node*>::iterator rangeEnd = std::next(rangeStart);

	while (rangeEnd != nodes.end()) {
		Node* ret = windowFunc(*rangeStart, *rangeEnd, astTree, settings, stream);
		if (ret != nullptr) {
			nodes.insert(std::next(rangeEnd), ret);
			rangeStart = nodes.erase(rangeStart, std::next(rangeEnd));
			rangeEnd = std::next(rangeStart);
		} else {
			rangeStart++;
			rangeEnd++;
		}
	}
}

void slideWindow3(compiler::ast::Node* (*windowFunc)(compiler::ast::Node*, compiler::ast::Node*, compiler::ast::Node*, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream),
				  compiler::ast::NodeGroup* group, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	std::list<Node*>& nodes = group->elems;

	if (nodes.size() < 3) return;

	std::list<Node*>::iterator rangeStart = nodes.begin();
	std::list<Node*>::iterator rangeEnd = std::next(rangeStart, 2);

	while (rangeEnd != nodes.end()) {
		Node* ret = windowFunc(*rangeStart, *std::next(rangeStart), *rangeEnd, astTree, settings, stream);
		if (ret != nullptr) {
			nodes.insert(std::next(rangeEnd), ret);
			rangeStart = nodes.erase(rangeStart, std::next(rangeEnd));
			rangeEnd = std::next(rangeStart);
			if (rangeEnd == nodes.end()) break;
			rangeEnd = std::next(rangeEnd);
		} else {
			rangeStart++;
			rangeEnd++;
		}
	}
}

compiler::ast::Node* macroWindow3(compiler::ast::Node* first, compiler::ast::Node* second, compiler::ast::Node* third, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	if (first->type == NodeType::TOKEN && first->token->type == TokenType::HASH) {
		if (second->type == NodeType::IDENTIFIER && third->isExpr) {
			int idx = lookupString(astTree.tokenData.strList[dynamic_cast<NodeIdentifier*>(second)->strIndex].c_str(), macroStrings, macroCount);

			MacroType type = static_cast<MacroType>(idx);
			if ((type == MacroType::PRINTI && dynamic_cast<Expr*>(third)->typeIndex != TypeData::intIndex) ||
				(type == MacroType::PRINTF && dynamic_cast<Expr*>(third)->typeIndex != TypeData::floatIndex)) {
				throw CompilerException(CompilerException::ErrorType::BAD_TYPE_MACRO, third->token->line, third->token->column);
			}
			if (idx >= 0) {
				return astTree.addNode(std::make_unique<NodeMacro>(first->token, type, dynamic_cast<Expr*>(third)));
			}
		}
		throw CompilerException(CompilerException::ErrorType::INVALID_MACRO, first->token->line, first->token->column);
	}

	return nullptr;
}

compiler::ast::Node* mulDivWindow3(compiler::ast::Node* first, compiler::ast::Node* second, compiler::ast::Node* third, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	TokenType op = second->token->type;
	if (second->type == NodeType::TOKEN && (op == TokenType::STAR || op == TokenType::SLASH)) {
		// TODO: non-int types
		if (first->isExpr && third->isExpr) {
			Expr* left = dynamic_cast<Expr*>(first);
			Expr* right = dynamic_cast<Expr*>(third);

			if (left->typeIndex == TypeData::intIndex && right->typeIndex == TypeData::intIndex ||
				left->typeIndex == TypeData::floatIndex && right->typeIndex == TypeData::floatIndex) {
				return astTree.addNode(std::make_unique<NodeArithBinop>(second->token, left, right, op == TokenType::STAR ? NodeArithBinop::OpType::MUL : NodeArithBinop::OpType::DIV, left->typeIndex));
			}
			// else fall through to error
		}
		throw CompilerException(op == TokenType::STAR ? CompilerException::ErrorType::BAD_TYPE_MUL : CompilerException::ErrorType::BAD_TYPE_DIV, second->token->line, second->token->column);
	}

	return nullptr;
}

compiler::ast::Node* addSubWindow3(compiler::ast::Node* first, compiler::ast::Node* second, compiler::ast::Node* third, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	TokenType op = second->token->type;
	if (second->type == NodeType::TOKEN && (op == TokenType::PLUS || op == TokenType::DASH)) {
		// TODO: non-int types
		if (first->isExpr && third->isExpr) {
			Expr* left = dynamic_cast<Expr*>(first);
			Expr* right = dynamic_cast<Expr*>(third);

			if (left->typeIndex == TypeData::intIndex && right->typeIndex == TypeData::intIndex ||
				left->typeIndex == TypeData::floatIndex && right->typeIndex == TypeData::floatIndex) {
				return astTree.addNode(std::make_unique<NodeArithBinop>(second->token, left, right, op == TokenType::PLUS ? NodeArithBinop::OpType::ADD : NodeArithBinop::OpType::SUB, left->typeIndex));
			}
			// else fall through to error
		}
		throw CompilerException(op == TokenType::PLUS ? CompilerException::ErrorType::BAD_TYPE_ADD : CompilerException::ErrorType::BAD_TYPE_SUB, second->token->line, second->token->column);
	}

	return nullptr;
}

// Fully process a node group into a tree
compiler::ast::Node* reduceNodeGroup(compiler::ast::NodeGroup* group, compiler::ast::Tree& astTree, compiler::CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	std::list<Node*>& nodes = group->elems;

	if (nodes.size() == 0) return group;

	// Pass 1: Reduce child groups
	for (Node*& n : nodes) {
		if (n->type == NodeType::ANGLE_GROUP) {
			n = constructTypespec(dynamic_cast<NodeGroup*>(n), astTree, settings, stream);
			continue;
		}
		if (isNodeGroup(n->type)) {
			n = reduceNodeGroup(dynamic_cast<NodeGroup*>(n), astTree, settings, stream);
			continue;
		}
	}

	// Pass 2: Macros
	slideWindow3(macroWindow3, group, astTree, settings, stream);

	// Pass 3: Multiplication and Division
	slideWindow3(mulDivWindow3, group, astTree, settings, stream);

	// Pass 4: Addition and Subtraction
	slideWindow3(addSubWindow3, group, astTree, settings, stream);

	if (group->type == NodeType::PAREN_GROUP && nodes.size() == 1 && nodes.front()->isExpr) {
		// TODO: are we losing important context here?
		return nodes.front();
	}
	return group;
}

// Construct the AST
int compiler::constructAST(ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	astTree.root = reduceNodeGroup(dynamic_cast<NodeGroup*>(astTree.root), astTree, settings, stream);

	return 0;
}