#ifndef ANALYZER_H
#define ANALYZER_H

#include <cstdint>
#include <vector>
#include <string>

#include "ASTVisitor.h"

namespace cmm 
{

class Error;

class ScopeManager
{
public:
	explicit                  ScopeManager();
	                          ~ScopeManager();

	void                      openFunctionScope(AST::FunctionDefinition& functionDef);
	void                      closeFunctionScope();	
	void                      openScope();
	void                      closeScope();
	void                      openLoop(AST::LoopStmt& loopStmt);
	void                      closeLoop();
	void                      registerVariable(AST::VariableStmt& variableStmt);
	AST::FunctionDefinition&  currentFunction();
	AST::LoopStmt*            retrieveNearestLoop();
	AST::VariableStmt*        retrieveVariable(const std::wstring& identifier);

	const uint32_t            functionLevel() const;
	const uint32_t            numVariable() const;
	const uint32_t            loopLevel() const;
	const uint32_t            scopeLevel() const;

private:
	                          ScopeManager(const ScopeManager&);
	const ScopeManager&       operator=(const ScopeManager&);

	typedef std::vector<AST::LoopStmt*> LoopStack_;	
	typedef std::vector<AST::VariableStmt*> VariableStack_;

	struct Function_
	{
		Function_(AST::FunctionDefinition* funcDef) : functionDef(funcDef), numVariable(0) {}

		uint32_t                  numVariable;
		LoopStack_                loopStack;
		AST::FunctionDefinition*  functionDef;
	};

	typedef std::vector<Function_> FunctionStack_;

	uint32_t                  scopeLevel_;
	FunctionStack_            functionStack_;
	VariableStack_            variableStack_;
};


class Analyzer : public AST::Visitor
{
public:
	explicit            Analyzer();
	                    ~Analyzer();

	void                analyze(AST::FunctionDefinition& rootFunction);

	virtual void        visit(AST::StmtSequence& stmtSequence);
	virtual void        visit(AST::TableInitializer& tableInit);	
	virtual void        visit(AST::FunctionDefinition& functionDef);

	virtual void        visit(AST::CompoundStmt& compoundStmt);
	virtual void        visit(AST::ForStmt& forStmt);
	virtual void        visit(AST::WhileStmt& whileStmt);
	virtual void        visit(AST::DoWhileStmt& doWhileStmt);
	virtual void        visit(AST::IfElseStmt& ifElseStmt);	
	virtual void        visit(AST::ReturnStmt& returnStmt);
	virtual void        visit(AST::JumpStmt& jumpStmt);
	virtual void        visit(AST::VariableStmt& variableStmt);
	virtual void        visit(AST::ExpressionStmt& expressionStmt);

	virtual void        visit(AST::UnaryExpr& unaryExpr);
	virtual void        visit(AST::BinaryExpr& binaryExpr);
	virtual void        visit(AST::TrinaryExpr& trinaryExpr);
	virtual void        visit(AST::TerminalExpr& terminalExpr);
	virtual void        visit(AST::CallExpr& callExpr);
	virtual void        visit(AST::FunctionExpr& functionExpr);
	virtual void        visit(AST::TableExpr& tableExpr);
	
private:
	typedef std::vector<AST::Statement*> LoopStack;
	
	                    Analyzer(const Analyzer&);
	const Analyzer&     operator=(const Analyzer&);

	const bool          safeVisit_(AST::Base* host);

	ScopeManager        scopeManager_;
};

} // The end of namespace "cmm"

#endif