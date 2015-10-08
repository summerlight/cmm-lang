#ifndef AST_DRAWER_H
#define AST_DRAWER_H

#include <cstdint>
#include <string>

#include "ASTVisitor.h"

namespace cmm 
{

class ASTDrawer : public AST::Visitor
{
public:
	explicit      ASTDrawer();
	              ~ASTDrawer();

	std::wstring& draw(AST::FunctionDefinition& rootFunction);	

	virtual void  visit(AST::StmtSequence& stmtSequence) override;
	virtual void  visit(AST::TableInitializer& tableInit) override;
	virtual void  visit(AST::FunctionDefinition& functionDef) override;

	virtual void  visit(AST::CompoundStmt& compoundStmt) override;
	virtual void  visit(AST::ForStmt& forStmt) override;
	virtual void  visit(AST::WhileStmt& whileStmt) override;
	virtual void  visit(AST::DoWhileStmt& doWhileStmt) override;
	virtual void  visit(AST::IfElseStmt& ifElseStmt) override;
	virtual void  visit(AST::ReturnStmt& returnStmt) override;
	virtual void  visit(AST::JumpStmt& jumpStmt) override;
	virtual void  visit(AST::VariableStmt& variableStmt) override;
	virtual void  visit(AST::ExpressionStmt& expressionStmt) override;

	virtual void  visit(AST::UnaryExpr& unaryExpr) override;
	virtual void  visit(AST::BinaryExpr& binaryExpr) override;
	virtual void  visit(AST::TrinaryExpr& trinaryExpr) override;
	virtual void  visit(AST::TerminalExpr& terminalExpr) override;
	virtual void  visit(AST::CallExpr& callExpr) override;
	virtual void  visit(AST::FunctionExpr& functionExpr) override;
	virtual void  visit(AST::TableExpr& tableExpr) override;
	
private:
	bool          safeVisit_(AST::Base* host);
	void          append_(const wchar_t format[], ...);
	void          appendTreeLine_();
	void          appendFlagInfo_(const AST::Base& node);
	void          appendBaseInfo_(const wchar_t nodeName[], const AST::Base& node);
	void          appendExpressionInfo_(const AST::Expression& node);
	void          appendNumberInfo_(const wchar_t format[], uint32_t number);
	void          appendStringInfo_(const wchar_t format[], const wchar_t string[]);
	void          appendNewline_(const wchar_t string[]);
	void          appendNewline_();

	void          pushTreeLine_(bool isLine);
	void          updateTreeLine_();
	void          popTreeLine_();
	void          turnOnBranchFlag_(bool holding);

	std::wstring  output_;
	std::wstring  treeLine_;
	uint32_t      indention_;
	bool          branching_;
	bool          holding_;
};

} // namespace "cmm"

#endif