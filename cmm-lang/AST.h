#ifndef AST_H
#define AST_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "Position.h"
#include "ASTDecl.h"

namespace cmm
{

namespace AST
{

constexpr uint32_t FLAG_NOTHING   = 0x00000000; // There is no information to be returned
constexpr uint32_t FLAG_ERROR     = 0x00000001; // An error has occurred in semantic analysis process
constexpr uint32_t FLAG_LVALUE    = 0x00000002; // An expression is not l-value, then it is r-value
constexpr uint32_t FLAG_STORE     = 0x00000004; // An expression is modified by assign, inc or dec operation if it is l-value
constexpr uint32_t FLAG_LOAD      = 0x00000008; // The result value needs to be loaded to temporary register even if it has non temporary register
constexpr uint32_t FLAG_NOLOAD    = 0x00000010; // The result value does not need to be loaded because it is lhs in assign operation.
constexpr uint32_t FLAG_TABLE     = 0x00000020; // An expression is a table value
constexpr uint32_t FLAG_GLOBAL    = 0x00000040; // An expression is a global variable
constexpr uint32_t FLAG_UPVALUE   = 0x00000080; // An expression is an upvalue variable
constexpr uint32_t FLAG_INTVALUE  = 0x00000100; // An expression is an integer terminal
constexpr uint32_t FLAG_ARRAY     = 0x00000200; // A table initializer does not has a key or its key is an integer terminal
constexpr uint32_t FLAG_TEMP      = 0x00000400; // The result of an expression is located on temporary register
constexpr uint32_t FLAG_TEMPTABLE = 0x00000800; // The table and key value of an expression is located on temporary register

class Visitor;

struct Base
{
	explicit             Base();
	virtual              ~Base();
	virtual void         accept(Visitor& visitor) = 0;
	
	Position             position;
	uint32_t             flag;
};


struct Statement : public Base
{
	explicit             Statement();
	virtual              ~Statement() override;
	virtual void         accept(Visitor& visitor) = 0;
};


struct StmtSequence : public Statement
{
	explicit             StmtSequence();
	virtual              ~StmtSequence() override;
	virtual void         accept(Visitor& visitor) override;

	typedef std::vector<StatementPtr> StatementVector;

	StatementVector      statementList;
};


struct Expression : public Base
{
	explicit             Expression();
	virtual              ~Expression() override;
	virtual void         accept(Visitor& visitor) = 0;

	uint32_t             registerOffset;

	uint32_t             lvalue1;
	uint32_t             lvalue2;
	// If an expression is table value then offset of register that stores table and key are assigned to lvalue1 and lvalue2
	// If an expression is constant then index of corresponding constant is assigned to lvalue1
	// If an expression is upvalue then offset and function level of corresponding upvalue are assigned to lvalue1 and lvalue2
};


struct TableInitializer : public Base
{
	explicit             TableInitializer(ExpressionPtr key, ExpressionPtr value);
	virtual              ~TableInitializer() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        key;
	ExpressionPtr        value;

	uint32_t             tableOffset;
};


struct FunctionDefinition : public Base
{
	explicit             FunctionDefinition(StmtSequencePtr args, StmtSequencePtr contents);
	virtual              ~FunctionDefinition() override;
	virtual void         accept(Visitor& visitor) override;

	typedef std::vector<VariableStmt*> VariableVector;

	VariableVector       upValues;
	StmtSequencePtr      arguments;
	StmtSequencePtr      contents;
	uint32_t             numVariable;
	uint32_t             functionLevel;
	uint32_t             functionNum;
};


struct CompoundStmt : public Statement
{
	explicit             CompoundStmt(StmtSequencePtr contents);
	virtual              ~CompoundStmt() override;
	virtual void         accept(Visitor& visitor) override;

	StmtSequencePtr      contents;
	uint32_t             scopeLevel;
	uint32_t             numVariable;
};


struct LoopStmt : public Statement
{
	explicit             LoopStmt();
	virtual              ~LoopStmt() override;
	virtual void         accept(Visitor& visitor) = 0;

	uint32_t             continueLabel;
	uint32_t             breakLabel;
};


struct ForStmt : public LoopStmt
{
	explicit             ForStmt(StatementPtr init, ExpressionPtr cond, StatementPtr iter, StatementPtr contents);
	virtual              ~ForStmt() override;
	virtual void         accept(Visitor& visitor) override;

	StatementPtr         initial;
	ExpressionPtr        condition;
	StatementPtr         iteration;
	StatementPtr         contents;
};


struct WhileStmt : public LoopStmt
{
	explicit             WhileStmt(ExpressionPtr cond, StatementPtr contents);
	virtual              ~WhileStmt() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        condition;
	StatementPtr         contents;
};


struct DoWhileStmt : public LoopStmt
{
	explicit             DoWhileStmt(ExpressionPtr cond, StatementPtr contents);
	virtual              ~DoWhileStmt() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        condition;
	StatementPtr         contents;
};


struct IfElseStmt : public Statement
{
	explicit             IfElseStmt(ExpressionPtr cond, StatementPtr ifContents, StatementPtr elseContents);
	virtual              ~IfElseStmt() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        condition;
	StatementPtr         ifContents;
	StatementPtr         elseContents;
};


struct ReturnStmt : public Statement
{
	enum Type {
		RETURN,
		YIELD
	};
	explicit             ReturnStmt(Type t, ExpressionPtr returnExpr);
	virtual              ~ReturnStmt() override;
	virtual void         accept(Visitor& visitor) override;

	Type                 type;
	ExpressionPtr        returnExpr;
};


struct JumpStmt : public Statement
{
	enum Type {
		CONTINUE,
		BREAK
	};
	explicit             JumpStmt(Type t);
	virtual              ~JumpStmt() override;
	virtual void         accept(Visitor& visitor) override;

	Type                 type;
	LoopStmt*            correspondingLoop;
};


struct VariableStmt : public Statement
{
	explicit             VariableStmt(const std::wstring& name, ExpressionPtr init);
	virtual              ~VariableStmt() override;
	virtual void         accept(Visitor& visitor) override;

	std::wstring         name;
	ExpressionPtr        init;
	uint32_t             registerOffset;
	uint32_t             scopeLevel;
	uint32_t             functionLevel;
};


struct ExpressionStmt : public Statement
{
	explicit             ExpressionStmt(ExpressionPtr expr);
	virtual              ~ExpressionStmt() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        expression;
};


struct TrinaryExpr : public Expression
{
	enum Operator {		
		BRANCH         // cond ? branch1 : branch2
	};
	explicit             TrinaryExpr(Operator op, ExpressionPtr first, ExpressionPtr second, ExpressionPtr third);
	virtual              ~TrinaryExpr() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        first;
	ExpressionPtr        second;
	ExpressionPtr        third;
	Operator             op;
};


struct BinaryExpr : public Expression
{
	enum Operator {
		ARITH_ADD,      // +
		ARITH_SUB,      // -
		ARITH_MUL,      // *
		ARITH_DIV,      // /
		ARITH_MOD,      // %
		BIT_AND,        // &
		BIT_OR,         // |
		BIT_XOR,        // ^
		BIT_SL,         // <<
		BIT_SR,         // >>
		ASSIGN,         // =
		ASSIGN_ADD,     // +=
		ASSIGN_SUB,     // -=
		ASSIGN_MUL,     // *=
		ASSIGN_DIV,     // /=
		ASSIGN_MOD,     // %=
		ASSIGN_SL,      // <<=
		ASSIGN_SR,      // >>=
		ASSIGN_AND,     // &=
		ASSIGN_OR,      // !=
		ASSIGN_XOR,     // ^=
		LOGIC_OR,       // ||
		LOGIC_AND,      // &&		
		LOGIC_EQ,       // ==
		LOGIC_NOTEQ,    // !=
		LOGIC_GREATER,  // >
		LOGIC_GE,       // >=
		LOGIC_LESS,     // <
		LOGIC_LE,       // <=
		INDEX,          // [ ]
		ERR
	};
	explicit             BinaryExpr(Operator op, ExpressionPtr first, ExpressionPtr second);
	virtual              ~BinaryExpr() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        first;
	ExpressionPtr        second;
	Operator             op;
};

struct UnaryExpr : public Expression
{
	enum Operator {		
		PLUS,        // +
		MINUS,       // -
		PREFIX_INC,  // ++
		PREFIX_DEC,  // --
		POSTFIX_INC, // ++
		POSTFIX_DEC, // --
		BIT_NOT,     // ~
		LOGIC_NOT,   // !
		ERR
	};
	explicit             UnaryExpr(Operator op, ExpressionPtr first);
	virtual              ~UnaryExpr() override;
	virtual void         accept(Visitor& visitor) override;

	ExpressionPtr        first;
	Operator             op;
};


struct TerminalExpr : public Expression
{
	enum Type {
		NULLTYPE,
		INTEGER,
		HEX,
		FLOAT,
		STRING,
		IDENTIFIER
	};
	explicit             TerminalExpr(Type t, const std::wstring& lexeme);
	virtual              ~TerminalExpr() override;
	virtual void         accept(Visitor& visitor) override;

	Type                 type;
	std::wstring         lexeme;
	VariableStmt*        correspondingVar;
};


struct CallExpr : public Expression
{
	explicit             CallExpr(ExpressionPtr function);
	virtual              ~CallExpr() override;
	virtual void         accept(Visitor& visitor) override;

	typedef std::vector<ExpressionPtr> ArgumentVector;

	ExpressionPtr        function;
	ArgumentVector       argumentList;
};


struct FunctionExpr : public Expression
{
	explicit             FunctionExpr(FunctionDefPtr funcDef);
	virtual              ~FunctionExpr() override;
	virtual void         accept(Visitor& visitor) override;

	FunctionDefPtr       functionDef;
};


struct TableExpr : public Expression
{
	enum Type {
		TABLE,
		ARRAY,
		UNKNOWN
	};
	explicit             TableExpr(Type t);
	virtual              ~TableExpr() override;
	virtual void         accept(Visitor& visitor) override;

	typedef std::vector<TableInitPtr> InitializerVector;

	Type                 type;
	InitializerVector    initializerList;//
};


} // namespace "AST"


} // namespace "cmm"

#endif