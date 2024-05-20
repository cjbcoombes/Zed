#include "patterns.h"

namespace compiler {
	namespace ast {
		
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Types
		
		// A type in the type system
		// Currently not fleshed-out. Just primitives
		struct ExprType {
			enum class PrimType {
				VOID, INT, FLOAT, BOOL, CHAR
			};

			static const ExprType primVoid;
			static const ExprType primInt;
			static const ExprType primFloat;
			static const ExprType primBool;
			static const ExprType primChar;

			PrimType type;

			ExprType(PrimType type) : type(type) {}
		};

		const ExprType ExprType::primVoid{ ExprType::PrimType::VOID };
		const ExprType ExprType::primInt{ ExprType::PrimType::INT };
		const ExprType ExprType::primFloat{ ExprType::PrimType::FLOAT };
		const ExprType ExprType::primBool{ ExprType::PrimType::BOOL };
		const ExprType ExprType::primChar{ ExprType::PrimType::CHAR };

		bool sameType(ExprType& a, ExprType& b);
	}
}