#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include <cstdint>

#include "ASTDecl.h"

namespace cmm {

namespace AST {

class Visitor
{
public:	
	virtual void visit(StmtSequence& stmtSequence) = 0;
	virtual void visit(TableInitializer& tableInit) = 0;	
	virtual void visit(FunctionDefinition& functionDef) = 0;

	virtual void visit(CompoundStmt& compoundStmt) = 0;
	virtual void visit(ForStmt& forStmt) = 0;
	virtual void visit(WhileStmt& whileStmt) = 0;
	virtual void visit(DoWhileStmt& doWhileStmt) = 0;
	virtual void visit(IfElseStmt& ifElseStmt) = 0;	
	virtual void visit(ReturnStmt& returnStmt) = 0;
	virtual void visit(JumpStmt& jumpStmt) = 0;
	virtual void visit(VariableStmt& variableStmt) = 0;
	virtual void visit(ExpressionStmt& expressionStmt) = 0;

	virtual void visit(UnaryExpr& unaryExpr) = 0;
	virtual void visit(BinaryExpr& binaryExpr) = 0;
	virtual void visit(TrinaryExpr& trinaryExpr) = 0;
	virtual void visit(TerminalExpr& terminalExpr) = 0;
	virtual void visit(CallExpr& callExpr) = 0;
	virtual void visit(FunctionExpr& functionExpr) = 0;
	virtual void visit(TableExpr& tableExpr) = 0;
};

} // namespace "AST"

} // namespace "cmm"

#endif

