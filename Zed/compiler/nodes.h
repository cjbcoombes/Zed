#pragma once
#include "../utils/code_location.h"
#include "../utils/bytecode.h"
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <list>

namespace compiler {
	namespace tokens {
		struct Token;
		class TokenData;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Types

	// "Primitive" expression types
	enum class PrimType {
		ERR, VOID, INT, FLOAT, BOOL, CHAR
	};
	constexpr int primTypeCount = 7;

	class TypeData {
		friend struct ExprType;

		struct Type {
			std::vector<const Type*> subtypes;
			std::optional<std::string*> name;

			Type();
			explicit Type(std::vector<const Type*>&& subtypes);
			Type(std::vector<const Type*>&& subtypes, std::string* name);
		};

		std::list<Type> types;
		Type* prims[primTypeCount];

	public:
		TypeData();

		static [[nodiscard]] ExprType none();
		[[nodiscard]] ExprType err() const;
		[[nodiscard]] ExprType prim(PrimType primType) const;
		[[nodiscard]] ExprType tuple(std::vector<const Type*>&& subtypes);

		[[nodiscard]] static bool isNoneType(const ExprType& t);
		[[nodiscard]] bool sameType(const ExprType& a, const ExprType& b) const;
		[[nodiscard]] bool sameType(const ExprType& a, PrimType primType) const;

		void printType(const ExprType& type, std::ostream& stream) const;
	};

	// A type in the type system
	// Currently not fleshed-out. Just primitives
	struct ExprType {
		friend class TypeData;

	private:
		TypeData::Type* type;
		// no type is nullptr
		
		explicit ExprType(TypeData::Type* type);
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Nodes

	enum class NodeType {
		UNIMPL,
		TOKEN,
		BLOCK,
		ARITH_BINOP,
		LITERAL,
		MACRO,
		TYPE
	};

	// Parent Node
	struct Node {
		NodeType type;
		ExprType exprType;
		code_location loc;

		Node(const NodeType type, const code_location loc);
		Node(const NodeType type, const ExprType exprType, const code_location loc);
		virtual ~Node() = default;

		virtual void print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const;
		virtual void printSimple(const TypeData& typeData, std::ostream& stream) const;
		void printType(const TypeData& typeData, std::ostream& stream) const;
	};

	// A Node for currently unimplemented cases
	struct UnimplNode : Node {
		std::string msg;
		std::vector<Node*> nodes;

		UnimplNode(const char* const msg, const code_location loc);

		void print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for a block (just a group of other Nodes)
	struct BlockNode : Node {
		std::vector<Node*> nodes;

		explicit BlockNode(const code_location loc);

		void print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for a token
	struct TokenNode : Node {
		const tokens::Token* token;

		explicit TokenNode(const tokens::Token* const token);

		void printSimple(const TypeData& typeData, std::ostream& stream) const override;
	};

	// A Node for a binary arithmetic operation
	struct ArithBinopNode : Node {
		enum class Type {
			ADD, SUB, MUL, DIV
		};

		Node* left;
		Node* right;
		Type opType;

		ArithBinopNode(const Type opType, const ExprType exprType, Node* const left, Node* const right, const code_location loc);

		void print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for literal values
	struct LiteralNode : Node {
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

		LiteralNode(const tokens::Token* const token, const TypeData& typeData, const code_location loc);

		void printSimple(const TypeData& typeData, std::ostream& stream) const override;
	};

	// A Node for Macros
	struct MacroNode : Node {
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

		MacroNode(const Type macroType, const ExprType exprType, Node* const arg, const code_location loc);

		void print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	struct TypeNode : Node {
		ExprType repType;

		TypeNode(const ExprType repType, const code_location loc);

		void printSimple(const TypeData& typeData, std::ostream& stream) const override;
	};
}
