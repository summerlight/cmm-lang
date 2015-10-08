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
	explicit          ASTDrawer();
	                  ~ASTDrawer();

	std::wstring&     draw(AST::FunctionDefinition& rootFunction);	

	virtual void      visit(AST::StmtSequence& stmtSequence);
	virtual void      visit(AST::TableInitializer& tableInit);	
	virtual void      visit(AST::FunctionDefinition& functionDef);

	virtual void      visit(AST::CompoundStmt& compoundStmt);
	virtual void      visit(AST::ForStmt& forStmt);
	virtual void      visit(AST::WhileStmt& whileStmt);
	virtual void      visit(AST::DoWhileStmt& doWhileStmt);
	virtual void      visit(AST::IfElseStmt& ifElseStmt);	
	virtual void      visit(AST::ReturnStmt& returnStmt);
	virtual void      visit(AST::JumpStmt& jumpStmt);
	virtual void      visit(AST::VariableStmt& variableStmt);
	virtual void      visit(AST::ExpressionStmt& expressionStmt);

	virtual void      visit(AST::UnaryExpr& unaryExpr);
	virtual void      visit(AST::BinaryExpr& binaryExpr);
	virtual void      visit(AST::TrinaryExpr& trinaryExpr);
	virtual void      visit(AST::TerminalExpr& terminalExpr);
	virtual void      visit(AST::CallExpr& callExpr);
	virtual void      visit(AST::FunctionExpr& functionExpr);
	virtual void      visit(AST::TableExpr& tableExpr);
	
private:
	const bool        safeVisit_(AST::Base* host);
	void              append_(const wchar_t format[], ...);
	void              appendTreeLine_();
	void              appendFlagInfo_(const AST::Base& node);
	void              appendBaseInfo_(const wchar_t nodeName[], const AST::Base& node);
	void              appendExpressionInfo_(const AST::Expression& node);
	void              appendNumberInfo_(const wchar_t format[], const uint32_t number);
	void              appendStringInfo_(const wchar_t format[], const wchar_t string[]);
	void              appendNewline_(const wchar_t string[]);
	void              appendNewline_();

	void              pushTreeLine_(const bool isLine);
	void              updateTreeLine_();
	void              popTreeLine_();
	void              turnOnBranchFlag_(const bool holding);

	std::wstring      output_;
	std::wstring      treeLine_;
	uint32_t          indention_;
	bool              branching_;
	bool              holding_;
};

} // The end of namespace "cmm"

#endif