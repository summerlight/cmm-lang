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
                              ScopeManager(const ScopeManager&) = delete;
    const ScopeManager&       operator=(const ScopeManager&) = delete;

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

	uint32_t                  functionLevel() const;
	uint32_t                  numVariable() const;
	uint32_t                  loopLevel() const;
	uint32_t                  scopeLevel() const;

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
                        Analyzer(const Analyzer&) = delete;
    const Analyzer&     operator=(const Analyzer&) = delete;

	void                analyze(AST::FunctionDefinition& rootFunction);

	virtual void        visit(AST::StmtSequence& stmtSequence) override;
	virtual void        visit(AST::TableInitializer& tableInit) override;
	virtual void        visit(AST::FunctionDefinition& functionDef) override;

	virtual void        visit(AST::CompoundStmt& compoundStmt) override;
	virtual void        visit(AST::ForStmt& forStmt) override;
	virtual void        visit(AST::WhileStmt& whileStmt) override;
	virtual void        visit(AST::DoWhileStmt& doWhileStmt) override;
	virtual void        visit(AST::IfElseStmt& ifElseStmt) override;
	virtual void        visit(AST::ReturnStmt& returnStmt) override;
	virtual void        visit(AST::JumpStmt& jumpStmt) override;
	virtual void        visit(AST::VariableStmt& variableStmt) override;
	virtual void        visit(AST::ExpressionStmt& expressionStmt) override;

	virtual void        visit(AST::UnaryExpr& unaryExpr) override;
	virtual void        visit(AST::BinaryExpr& binaryExpr) override;
	virtual void        visit(AST::TrinaryExpr& trinaryExpr) override;
	virtual void        visit(AST::TerminalExpr& terminalExpr) override;
	virtual void        visit(AST::CallExpr& callExpr) override;
	virtual void        visit(AST::FunctionExpr& functionExpr) override;
	virtual void        visit(AST::TableExpr& tableExpr) override;
	
private:
	typedef std::vector<AST::Statement*> LoopStack;

	bool                safeVisit_(AST::Base* host);

	ScopeManager        scopeManager_;
};

} // namespace "cmm"

#endif