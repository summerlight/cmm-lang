#ifndef PARSER_H
#define PARSER_H

#include <cstdint>
#include <memory>

#include "ASTDecl.h"
#include "Token.h"
#include "Scanner.h"

namespace cmm
{

class Error;

class Parser
{
public:
	explicit           Parser(const wchar_t code[]);
	                   ~Parser();

	FunctionDefPtr     parse();

private:
	                   Parser();
	                   Parser(const Parser&);
	const Parser&      operator=(const Parser&);

	void               processCurrentToken_();
	void               processCurrentToken_(Token::Type expected);
	void               savePosition_();

	FunctionDefPtr     parseProgram_();

	StatementPtr       parseStatement_();
	StatementPtr       parseCompoundStatement_();
	StatementPtr       parseExpressionStatement_();
	StatementPtr       parseIfStatement_();
	StatementPtr       parseWhileStatement_();
	StatementPtr       parseDoWhileStatement_();
	StatementPtr       parseForStatement_();
	StatementPtr       parseReturnStatement_();
	StatementPtr       parseLocalStatement_();
	StatementPtr       parseFunctionStatement_();
	StatementPtr       parseVariableStatement_();
	StatementPtr       parseVariable_();

	StmtSequencePtr    parseStmtSequence_(const Token::Type delimiter);

	ExpressionPtr      parseExpression_();
	ExpressionPtr      parseAssignmentExpr_();
	ExpressionPtr      parseConditionalExpr_();
	ExpressionPtr      parseLogicalOrExpr_();
	ExpressionPtr      parseLogicalAndExpr_();
	ExpressionPtr      parseBitwiseOrExpr_();
	ExpressionPtr      parseBitwiseXorExpr_();
	ExpressionPtr      parseBitwiseAndExpr_();
	ExpressionPtr      parseEqualityExpr_();
	ExpressionPtr      parseRelationalExpr_();
	ExpressionPtr      parseShiftExpr_();
	ExpressionPtr      parseAdditiveExpr_();
	ExpressionPtr      parseMultiplicativeExpr_();
	ExpressionPtr      parseUnaryExpr_();
	ExpressionPtr      parsePostfixExpr_();
	ExpressionPtr      parsePrimaryExpr_();

	ExpressionPtr      parseTableExpr_();
	TableInitPtr       parseField_();

	ExpressionPtr      parseFunctionExpr_();
	void               parseCallExpr_(AST::CallExpr* callExpr);
		
	FunctionDefPtr     parseFunctionDef_();

	ExpressionPtr      newIntegerTerminal_(const uint32_t integer);
	
	Scanner            scanner_;
	Token              currentToken_;
	Position           tempPosition_;
};

} // The end of namespace "cmm"

#endif