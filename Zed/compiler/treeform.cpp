#include "treeform.h"
#include "../utils/string_lookup.h"
#include "nodes.h"
#include "tokenizer.h"
#include "compiler.h"
#include "patterns.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Types

compiler::ast::TypeData::Type::Type() : subtypes(), name() {}

compiler::ast::TypeData::Type::Type(std::vector<const Type*>&& subtypes)
	: subtypes(std::move(subtypes)), name() {
}

compiler::ast::TypeData::Type::Type(std::vector<const Type*>&& subtypes, std::string* name)
	: subtypes(std::move(subtypes)), name(name) {
}

compiler::ast::TypeData::Type::Type(const Type& other, std::string* newname)
	: subtypes(other.subtypes), name(newname) {}

compiler::ast::ExprType::ExprType(TypeData::Type* type) : type(type) {}

compiler::ast::TypeData::TypeData() : types(), prims() {
	for (auto& prim : prims) {
		types.emplace_back();
		prim = &types.back();
	}
}

compiler::ast::ExprType compiler::ast::TypeData::none() {
	return ExprType(nullptr);
}

compiler::ast::ExprType compiler::ast::TypeData::err() const {
	return ExprType(prims[static_cast<int>(PrimType::ERR)]);
}

compiler::ast::ExprType compiler::ast::TypeData::prim(PrimType primType) const {
	return ExprType(prims[static_cast<int>(primType)]);
}

// TODO: this has to take in ExprType, not Type. And maybe as an iterable instead of a vector?
compiler::ast::ExprType compiler::ast::TypeData::tuple(std::vector<const Type*>&& subtypes) {
	types.emplace_back(std::move(subtypes));
	return ExprType(&types.back());
}

compiler::ast::ExprType compiler::ast::TypeData::annotate(const ExprType& t, std::string* newname) {
	types.emplace_back(*t.type, newname);
	return ExprType(&types.back());
}

bool compiler::ast::TypeData::isNoneType(const ExprType& t) {
	return t.type == nullptr;
}


bool compiler::ast::TypeData::sameType(const ExprType& a, const ExprType& b) const {
	return a.type == b.type;
}

bool compiler::ast::TypeData::sameType(const ExprType& a, PrimType primType) const {
	return a.type == prims[static_cast<int>(primType)];
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Context

compiler::ast::TreeContext::TreeContext(const TreeContext& other) {
	// TODO: keep stuff that persists across levels
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tree

compiler::ast::Tree::Tree() : nodes(), root(nullptr), typeData() {}

void compiler::ast::Tree::setRoot(Node* const root) {
	if (this->root != nullptr) {
		throw std::logic_error("Tree root was set multiple times");
	} else {
		this->root = root;
	}
}

template<class N>
N* compiler::ast::Tree::add(std::unique_ptr<N> node) {
	N* temp = node.get();
	nodes.emplace_back(std::move(node));
	return temp;
}

template<class N, class... Args>
N* compiler::ast::Tree::addNode(Args&&... args) {
	return add<N>(std::make_unique<N>(std::forward<Args>(args)...));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node Constructors

compiler::ast::Node::Node(const NodeType type, const code_location loc) : type(type), exprType(TypeData::none()), loc(loc) {}
compiler::ast::Node::Node(const NodeType type, const ExprType exprType, const code_location loc) : type(type), exprType(exprType), loc(loc) {}

compiler::ast::UnimplNode::UnimplNode(const char* const msg, const code_location loc) : Node(NodeType::UNIMPL, loc), msg(msg), nodes() {}

compiler::ast::BlockNode::BlockNode(const code_location loc) : Node(NodeType::BLOCK, loc) {}

compiler::ast::TokenNode::TokenNode(const tokens::Token* const token) : Node(NodeType::TOKEN, token->loc), token(token) {}

compiler::ast::ArithBinopNode::ArithBinopNode(const Type opType, const ExprType exprType, Node* const left, Node* const right, const code_location loc)
	: Node(NodeType::ARITH_BINOP, exprType, loc), left(left), right(right), opType(opType) {
}

compiler::ast::MacroNode::MacroNode(const Type macroType, const ExprType exprType, Node* const arg, const code_location loc)
	: Node(NodeType::MACRO, exprType, loc), macroType(macroType), arg(arg) {
}

compiler::ast::TypeNode::TypeNode(const ExprType repType, const code_location loc) : Node(NodeType::TYPE, loc), repType(repType) {}

compiler::ast::LiteralNode::LiteralNode(const tokens::Token* const token, const TypeData& typeData, const code_location loc) : Node(NodeType::LITERAL, loc), int_(0) {
	using tokens::TokenType;

	switch (token->type) {
		case TokenType::NUM_INT:
			int_ = token->int_;
			litType = Type::INT;
			exprType = typeData.prim(PrimType::INT);
			break;
		case TokenType::NUM_FLOAT:
			float_ = token->float_;
			litType = Type::FLOAT;
			exprType = typeData.prim(PrimType::FLOAT);
			break;
		case TokenType::CHAR:
			char_ = token->char_;
			litType = Type::CHAR;
			exprType = typeData.prim(PrimType::CHAR);
			break;
		case TokenType::TRUE:
			bool_ = true;
			litType = Type::BOOL;
			exprType = typeData.prim(PrimType::BOOL);
			break;
		case TokenType::FALSE:
			bool_ = false;
			litType = Type::BOOL;
			exprType = typeData.prim(PrimType::BOOL);
			break;
		default:
			throw std::logic_error("Literal Node constructed from Token not representing a literal");
			break;
	}
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Match Tree Formation

compiler::ast::treeres_t compiler::ast::Match::formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG | compiler::FLAG_DEBUG_AST)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { tree.add(std::make_unique<UnimplNode>("Generic match", loc)), 0 };
}

compiler::ast::treeres_t compiler::ast::TokenMatch::formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	using tokens::TokenType;

	switch (token->type) {
		case TokenType::NUM_INT:
		case TokenType::NUM_FLOAT:
		case TokenType::CHAR:
		case TokenType::TRUE:
		case TokenType::FALSE:
			return { tree.add(std::make_unique<LiteralNode>(token, tree.typeData, loc)), 0 };

		case TokenType::TYPE_INT:
			return { tree.add(std::make_unique<TypeNode>(tree.typeData.prim(PrimType::INT), loc)), 0 };
		case TokenType::TYPE_FLOAT:
			return { tree.add(std::make_unique<TypeNode>(tree.typeData.prim(PrimType::FLOAT), loc)), 0 };
		case TokenType::TYPE_CHAR:
			return { tree.add(std::make_unique<TypeNode>(tree.typeData.prim(PrimType::CHAR), loc)), 0 };
		case TokenType::TYPE_BOOL:
			return { tree.add(std::make_unique<TypeNode>(tree.typeData.prim(PrimType::BOOL), loc)), 0 };

		default:
			return { tree.add(std::make_unique<TokenNode>(token)), 0 };
	}
}

static compiler::ast::treeres_t formTupleType(const compiler::ast::GroupMatch& match,
											  compiler::ast::Tree& tree,
											  const compiler::ast::TreeContext& context,
											  compiler::CompilerStatus& status,
											  const compiler::CompilerSettings& settings,
											  std::ostream& stream) {
	using namespace compiler::ast;

	treeres_t tempRes;
	TreeContext newContext(context);

	std::vector<Node*> nodes;
	bool parity = false;
	for (const Match* const submatch : match.matches) {
		tempRes = submatch->formTree(tree, newContext, status, settings, stream);
		if (tempRes.second) {
			return tempRes;
		}
		if (parity) {
			if (tempRes.first->type != NodeType::TOKEN || 
				dynamic_cast<TokenNode*>(tempRes.first)->token->type != compiler::tokens::TokenType::COMMA) {
				status.addIssue(compiler::CompilerIssue::Type::MISSING_COMMA_TYPE_LIST, tempRes.first->loc);
				return { tree.addNode<TypeNode>(tree.typeData.err(), match.loc), 0};
			}
		} else if (tempRes.first->type != NodeType::TOKEN) {
			status.addIssue(compiler::CompilerIssue::Type::INVALID_TYPE_TYPE_LIST, tempRes.first->loc);
			return { tree.addNode<TypeNode>(tree.typeData.err(), match.loc), 0 };
		} else {
			nodes.push_back(tempRes.first);
		}
		parity = !parity;
	}

	// TODO: construct tuple from the resTypes of the nodes

}

compiler::ast::treeres_t compiler::ast::GroupMatch::formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	// The parent match may decide to do something different with the group.

	compiler::ast::treeres_t tempRes;
	BlockNode* tempBlockNode;

	switch (type) {
		case MatchType::ROOT_GROUP:
		case MatchType::CURLY_GROUP:
			// block
			tempBlockNode = tree.add(std::make_unique<BlockNode>(loc));
			for (const Match* const match : matches) {
				tempRes = match->formTree(tree, context, status, settings, stream);
				if (tempRes.second) {
					return tempRes;
				}
				tempBlockNode->nodes.push_back(tempRes.first);
			}
			return { tempBlockNode, 0 };
		case MatchType::SQUARE_GROUP:
			return formTupleType(*this, tree, context, status, settings, stream);
		case MatchType::PAREN_GROUP:
			if (matches.size() == 1) {
				return matches.front()->formTree(tree, context, status, settings, stream);
			} else {
				return { tree.add(std::make_unique<UnimplNode>("Paren group with multiple internal matches", loc)), 0 };
			}
			break;
		default:
			throw std::logic_error("GroupMatch has unexpected MatchType");
	}
}

static compiler::ast::treeres_t formArithBinop(const compiler::ast::FixedSizeMatch& match,
											   compiler::ast::Tree& tree,
											   const compiler::ast::TreeContext& context,
											   compiler::CompilerStatus& status,
											   const compiler::CompilerSettings& settings,
											   std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;
	using tokens::TokenType;

	if (match.matches.size() != 3 || match.matches[1]->type != MatchType::TOKEN) {
		throw std::logic_error("Binary Arithmetic Match doesn't have exactly 3 children, where the second is a token");
	}
	treeres_t tempRes1 = match.matches[0]->formTree(tree, context, status, settings, stream);
	if (tempRes1.second) {
		return tempRes1;
	}
	treeres_t tempRes2 = match.matches[2]->formTree(tree, context, status, settings, stream);
	if (tempRes2.second) {
		return tempRes2;
	}

	ArithBinopNode::Type opType;
	switch (dynamic_cast<const TokenMatch*>(match.matches[1])->token->type) {
		case TokenType::PLUS:
			opType = ArithBinopNode::Type::ADD;
			break;
		case TokenType::DASH:
			opType = ArithBinopNode::Type::SUB;
			break;
		case TokenType::STAR:
			opType = ArithBinopNode::Type::MUL;
			break;
		case TokenType::SLASH:
			opType = ArithBinopNode::Type::DIV;
			break;
		default:
			throw std::logic_error("Binary Arithmetic Match has invalid arithmetic operation token");
	}

	ExprType exprType = tree.typeData.none();
	if (tree.typeData.sameType(tempRes1.first->exprType, PrimType::INT) &&
		tree.typeData.sameType(tempRes2.first->exprType, PrimType::INT)) {
		exprType = tree.typeData.prim(PrimType::INT);
	} else if (tree.typeData.sameType(tempRes1.first->exprType, PrimType::FLOAT) &&
			   tree.typeData.sameType(tempRes2.first->exprType, PrimType::FLOAT)) {
		exprType = tree.typeData.prim(PrimType::FLOAT);
	} else {
		exprType = tree.typeData.err();
		status.addIssue(CompilerIssue::Type::INVALID_TYPE_ARITH_BINOP, match.loc);
	}

	return { tree.add(std::make_unique<ArithBinopNode>(opType, exprType, tempRes1.first, tempRes2.first, match.loc)), 0 };
}

static compiler::ast::treeres_t formMacro(const compiler::ast::FixedSizeMatch& match,
										  compiler::ast::Tree& tree,
										  const compiler::ast::TreeContext& context,
										  compiler::CompilerStatus& status,
										  const compiler::CompilerSettings& settings,
										  std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	if (match.matches.size() != 3
		|| match.matches[1]->type != MatchType::TOKEN
		|| dynamic_cast<const TokenMatch*>(match.matches[1])->token->type != tokens::TokenType::IDENTIFIER) {
		throw std::logic_error("Macro Match doesn't have exactly three children, the second of which is an identifier");
	}

	treeres_t res = match.matches[2]->formTree(tree, context, status, settings, stream);
	if (res.second) {
		return res;
	}

	const std::string& str = *dynamic_cast<const TokenMatch*>(match.matches[1])->token->str;
	int strIndex = lookupString(str.c_str(), MacroNode::macroSpan);

	if (strIndex == -1) {
		status.addIssue(CompilerIssue::Type::INVALID_MACRO, match.matches[0]->loc, str);
		return { tree.add(std::make_unique<MacroNode>(MacroNode::Type::UNKNOWN, tree.typeData.err(), res.first, match.matches[0]->loc)), 0 };
	}

	MacroNode::Type type = static_cast<MacroNode::Type>(strIndex);
	ExprType exprType = tree.typeData.none();

	switch (type) {
		case MacroNode::Type::HELLO_WORLD:
			// No type checking
			break;
		case MacroNode::Type::PRINT_I:
			if (!tree.typeData.sameType(res.first->exprType, PrimType::INT)) {
				status.addIssue(CompilerIssue::Type::INVALID_TYPE_MACRO, match.matches[2]->loc);
				exprType = tree.typeData.none();
			} else {
				exprType = tree.typeData.prim(PrimType::INT);
			}
			break;
		default:
			throw std::logic_error("Macro has impossible type");
	}

	return { tree.add(std::make_unique<MacroNode>(type, exprType, res.first, match.matches[0]->loc)), 0 };
}

compiler::ast::treeres_t compiler::ast::FixedSizeMatch::formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	treeres_t tempRes1;

	switch (type) {
		case MatchType::ARITH_BINOP:
			return formArithBinop(*this, tree, context, status, settings, stream);
			break;
		case MatchType::MACRO:
			return formMacro(*this, tree, context, status, settings, stream);
			break;
		default:
			UnimplNode* unimplNode = tree.add(std::make_unique<UnimplNode>("fixed size match with unhandled type", loc));
			for (const Match* match : matches) {
				tempRes1 = match->formTree(tree, context, status, settings, stream);
				if (tempRes1.second) {
					return tempRes1;
				}
				unimplNode->nodes.push_back(tempRes1.first);
			}
			return { unimplNode, 0 };
	}
}

int compiler::ast::formTree(ast::Tree& tree, const ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	auto res = matchData.getRoot()->formTree(tree, TreeContext(), status, settings, stream);
	if (!res.second) {
		tree.setRoot(res.first);
	}

	return res.second;
}