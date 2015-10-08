#include "StdAfx.h"
#include "AST.h"

#include <vector>
#include <memory>

#include "ASTVisitor.h"

// NOTE :
// The intention of several unsigned integer variables initialization by UINT32_MAX is that
// the variable is not initialized by semantic analyzer or code generator - so we can assert
// that the variablee should be UINT32_MAX before corresponding module and shouldn't be after that.


namespace cmm
{

namespace AST
{

Base::Base()
: flag(0)
{
}

Base::~Base()
{
}



Statement::Statement()
{
}

Statement::~Statement()
{
}



StmtSequence::StmtSequence()
{
}

StmtSequence::~StmtSequence()
{
}

void StmtSequence::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



Expression::Expression()
: registerOffset(UINT32_MAX), lvalue1(UINT32_MAX), lvalue2(UINT32_MAX)
{
}

Expression::~Expression()
{
}



TableInitializer::TableInitializer(ExpressionPtr key, ExpressionPtr value)
: key(std::move(key)), value(std::move(value)), tableOffset(UINT32_MAX)
{
}

TableInitializer::~TableInitializer()
{
}

void TableInitializer::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



FunctionDefinition::FunctionDefinition(StmtSequencePtr args, StmtSequencePtr contents)
: arguments(std::move(args)), contents(std::move(contents)), numVariable(UINT32_MAX),
  functionLevel(UINT32_MAX), functionNum(UINT32_MAX)
{
}

FunctionDefinition::~FunctionDefinition()
{
}

void FunctionDefinition::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}


CompoundStmt::CompoundStmt(StmtSequencePtr contents)
: contents(std::move(contents)), scopeLevel(UINT32_MAX), numVariable(UINT32_MAX)
{
}

CompoundStmt::~CompoundStmt()
{
}

void CompoundStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



LoopStmt::LoopStmt()
: continueLabel(UINT32_MAX), breakLabel(UINT32_MAX)
{
}

LoopStmt::~LoopStmt()
{
}



ForStmt::ForStmt(StatementPtr init, ExpressionPtr cond, StatementPtr iter, StatementPtr contents)
: initial(std::move(init)), condition(std::move(cond)), iteration(std::move(iter)), contents(std::move(contents))
{
}

ForStmt::~ForStmt()
{
}

void ForStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



WhileStmt::WhileStmt(ExpressionPtr cond, StatementPtr contents)
: condition(std::move(cond)), contents(std::move(contents))
{
}

WhileStmt::~WhileStmt()
{
}

void WhileStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



DoWhileStmt::DoWhileStmt(ExpressionPtr cond, StatementPtr contents)
: condition(std::move(cond)), contents(std::move(contents))
{
}

DoWhileStmt::~DoWhileStmt()
{
}

void DoWhileStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



IfElseStmt::IfElseStmt(ExpressionPtr cond, StatementPtr ifContents, StatementPtr elseContents)
: condition(std::move(cond)), ifContents(std::move(ifContents)), elseContents(std::move(elseContents))
{
}

IfElseStmt::~IfElseStmt()
{
}

void IfElseStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



ReturnStmt::ReturnStmt(const Type t, ExpressionPtr returnExpr)
: type(t), returnExpr(std::move(returnExpr))
{
}

ReturnStmt::~ReturnStmt()
{
}

void ReturnStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



JumpStmt::JumpStmt(const Type t)
: type(t), correspondingLoop(nullptr)
{
}

JumpStmt::~JumpStmt()
{
}

void JumpStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}




VariableStmt::VariableStmt(const std::wstring& name, ExpressionPtr init)
: name(name), init(std::move(init)), registerOffset(UINT32_MAX), scopeLevel(UINT32_MAX), functionLevel(UINT32_MAX)
{
}

VariableStmt::~VariableStmt()
{
}

void VariableStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



ExpressionStmt::ExpressionStmt(ExpressionPtr expr)
: expression(std::move(expr))
{
}

ExpressionStmt::~ExpressionStmt()
{
}

void ExpressionStmt::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



UnaryExpr::UnaryExpr(const Operator op, ExpressionPtr first)
: op(op), first(std::move(first))
{
}

UnaryExpr::~UnaryExpr()
{
}

void UnaryExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



BinaryExpr::BinaryExpr(const Operator op, ExpressionPtr first, ExpressionPtr second)
: op(op), first(std::move(first)), second(std::move(second))
{
}

BinaryExpr::~BinaryExpr()
{
}

void BinaryExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



TrinaryExpr::TrinaryExpr(const Operator op, ExpressionPtr first, ExpressionPtr second, ExpressionPtr third)
: op(op), first(std::move(first)), second(std::move(second)), third(std::move(third))
{
}

TrinaryExpr::~TrinaryExpr()
{
}

void TrinaryExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



TerminalExpr::TerminalExpr(const Type t, const std::wstring& lexeme)
: type(t), lexeme(lexeme), correspondingVar(nullptr)
{
}

TerminalExpr::~TerminalExpr()
{
}

void TerminalExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



CallExpr::CallExpr(ExpressionPtr function)
: function(std::move(function))
{
}

CallExpr::~CallExpr()
{
}

void CallExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



FunctionExpr::FunctionExpr(FunctionDefPtr funcDef)
: functionDef(std::move(funcDef))
{
}

FunctionExpr::~FunctionExpr()
{
}

void FunctionExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}



TableExpr::TableExpr(const Type t)
: type(t)
{
}

TableExpr::~TableExpr()
{
}

void TableExpr::accept(Visitor& visitor)
{
	return visitor.visit(*this);
}


} // The end of namespace AST


} // The end of namespace "cmm"