#include "StdAfx.h"
#include "Parser.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <cwchar>

#include "AST.h"
#include "Token.h"
#include "Error.h"

namespace cmm
{

namespace
{

inline const bool isAssignOp(Token& t)
{
	return (t.type() >= Token::ASSIGN) && (t.type() <= Token::ASSIGN_XOR);
}

inline const bool isEqualityOp(Token& t)
{
	return (t.type() == Token::LOGIC_EQ) || (t.type() == Token::LOGIC_NOTEQ);
}

inline const bool isRelationalOp(Token& t)
{
	return (t.type() >= Token::LOGIC_GREATER) && (t.type() <= Token::LOGIC_LE);
}

inline const bool isShiftOp(Token& t)
{
	return (t.type() == Token::BIT_SL) || (t.type() == Token::BIT_SR);
}

inline const bool isAdditiveOp(Token& t)
{
	return (t.type() == Token::ARITH_ADD) || (t.type() == Token::ARITH_SUB);
}

inline const bool isMultiplicativeOp(Token& t)
{
	return (t.type() == Token::ARITH_MUL) || (t.type() == Token::ARITH_DIV) ||
	       (t.type() == Token::ARITH_MOD);
}

inline const bool isUnaryOp(Token& t)
{
	return (t.type() == Token::ARITH_ADD) || (t.type() == Token::ARITH_SUB) ||
	       (t.type() == Token::ARITH_INC) || (t.type() == Token::ARITH_DEC) ||
	       (t.type() == Token::BIT_NOT) || (t.type() == Token::LOGIC_NOT);
}

inline const bool isPostfixOp(Token& t)
{
	return (t.type() == Token::LEFTBRACKET) || (t.type() == Token::LEFTPAREN) ||
	       (t.type() == Token::ARITH_INC) || (t.type() == Token::ARITH_DEC);
}


const AST::BinaryExpr::Operator TokenToBinaryOp(Token& t)
{
	switch (t.type())
	{
	case Token::ARITH_ADD:     return AST::BinaryExpr::ARITH_ADD;
	case Token::ARITH_SUB:     return AST::BinaryExpr::ARITH_SUB;
	case Token::ARITH_MUL:     return AST::BinaryExpr::ARITH_MUL;
	case Token::ARITH_DIV:     return AST::BinaryExpr::ARITH_DIV;
	case Token::ARITH_MOD:     return AST::BinaryExpr::ARITH_MOD;
	case Token::ASSIGN:        return AST::BinaryExpr::ASSIGN;
	case Token::ASSIGN_ADD:    return AST::BinaryExpr::ASSIGN_ADD;
	case Token::ASSIGN_SUB:    return AST::BinaryExpr::ASSIGN_SUB;
	case Token::ASSIGN_MUL:    return AST::BinaryExpr::ASSIGN_MUL;
	case Token::ASSIGN_DIV:    return AST::BinaryExpr::ASSIGN_DIV;
	case Token::ASSIGN_MOD:    return AST::BinaryExpr::ASSIGN_MOD;
	case Token::ASSIGN_SL:     return AST::BinaryExpr::ASSIGN_SL;
	case Token::ASSIGN_SR:     return AST::BinaryExpr::ASSIGN_SR;
	case Token::ASSIGN_AND:    return AST::BinaryExpr::ASSIGN_AND;
	case Token::ASSIGN_OR:     return AST::BinaryExpr::ASSIGN_OR;
	case Token::ASSIGN_XOR:    return AST::BinaryExpr::ASSIGN_XOR;
	case Token::LOGIC_OR:      return AST::BinaryExpr::LOGIC_OR;
	case Token::LOGIC_AND:     return AST::BinaryExpr::LOGIC_AND;
	case Token::LOGIC_EQ:      return AST::BinaryExpr::LOGIC_EQ;
	case Token::LOGIC_NOTEQ:   return AST::BinaryExpr::LOGIC_NOTEQ;
	case Token::LOGIC_GREATER: return AST::BinaryExpr::LOGIC_GREATER;
	case Token::LOGIC_GE:      return AST::BinaryExpr::LOGIC_GE;
	case Token::LOGIC_LESS:    return AST::BinaryExpr::LOGIC_LESS;
	case Token::LOGIC_LE:      return AST::BinaryExpr::LOGIC_LE;
	default: assert(false);    return AST::BinaryExpr::ERR;		
	}
}

const AST::UnaryExpr::Operator TokenToPrefixOp(Token& t)
{
	switch (t.type())
	{
	case Token::ARITH_ADD:     return AST::UnaryExpr::PLUS;
	case Token::ARITH_SUB:     return AST::UnaryExpr::MINUS;
	case Token::ARITH_INC:     return AST::UnaryExpr::PREFIX_INC;
	case Token::ARITH_DEC:     return AST::UnaryExpr::PREFIX_DEC;
	case Token::LOGIC_NOT:     return AST::UnaryExpr::LOGIC_NOT;
	case Token::BIT_NOT:       return AST::UnaryExpr::BIT_NOT;
	default: assert(false);    return AST::UnaryExpr::ERR;		
	}
}

} // anonymous name space that contains utility functions only for parser class

// Function definitions

Parser::Parser(const wchar_t code[])
: scanner_()
{
	scanner_.load(code);
}

Parser::~Parser()
{
}

FunctionDefPtr Parser::parse()
{
	currentToken_ = scanner_.scan();

	return parseProgram_();
}


inline void Parser::processCurrentToken_()
{
	currentToken_ = scanner_.scan();	
}

void Parser::processCurrentToken_(Token::Type expected)
{
	if (currentToken_.type() == expected) {
		currentToken_ = scanner_.scan();
	} else {
		throw Error(L"%d: Token '%s' is expected near '%s'",
		            currentToken_.pos().startLine, Token::lexeme(expected).c_str(), currentToken_.lexeme().c_str());
	}
}

void Parser::savePosition_()
{
	tempPosition_ = currentToken_.pos();
}

/*
 * Program :== Statement*
 */
FunctionDefPtr Parser::parseProgram_()
{
	StmtSequencePtr program(parseStmtSequence_(Token::END));
	StmtSequencePtr emptyArg(new AST::StmtSequence);
	FunctionDefPtr rootFunction(new AST::FunctionDefinition(std::move(emptyArg), std::move(program)));

	return rootFunction;
}


/*
 * Statement :== CompoundStatement |
 *               ExpressionStatement |
 *               IfStatement |
 *               WhileStatement |
 *               DoWhileStatement |
 *               ForStatement |
 *               JumpStatement |
 *               FunctionStatement |
 *               VariableStatement |
 *
 * JumpStatement :== "return" expression? ";" |
 *                   "yield" expression? ";" |
 *                   "break" ";" | "continue" ";"
 */
StatementPtr Parser::parseStatement_()
{
	switch(currentToken_.type()) {
	case Token::LEFTBRACE:        return parseCompoundStatement_();
	case Token::KEYWORD_IF:       return parseIfStatement_();
	case Token::KEYWORD_WHILE:    return parseWhileStatement_();
	case Token::KEYWORD_DO:       return parseDoWhileStatement_();
	case Token::KEYWORD_FOR:      return parseForStatement_();
	case Token::KEYWORD_FUNCTION: return parseFunctionStatement_();
	case Token::KEYWORD_LOCAL:    return parseVariableStatement_();
	case Token::KEYWORD_RETURN:   
	case Token::KEYWORD_YIELD:    return parseReturnStatement_();
	case Token::KEYWORD_BREAK:
		processCurrentToken_();
		processCurrentToken_(Token::SEMICOLON);
		return JumpStmtPtr(new AST::JumpStmt(AST::JumpStmt::BREAK));
	case Token::KEYWORD_CONTINUE:
		processCurrentToken_();
		processCurrentToken_(Token::SEMICOLON);
		return JumpStmtPtr(new AST::JumpStmt(AST::JumpStmt::CONTINUE));
	default:		              return parseExpressionStatement_();
	}
}


/*
 * CompoundStatement :== "{" Statement* "}"
 */
StatementPtr Parser::parseCompoundStatement_()
{
	assert(currentToken_.type() == Token::LEFTBRACE);
	processCurrentToken_(Token::LEFTBRACE);

	return CompoundStmtPtr(new AST::CompoundStmt(parseStmtSequence_(Token::RIGHTBRACE)));
}


/*
 * ExpressionStatement :== Expression? ";"
 */
StatementPtr Parser::parseExpressionStatement_()
{
	StatementPtr expr;
	if (currentToken_.type() != Token::SEMICOLON) {
		expr = ExpressionStmtPtr(new AST::ExpressionStmt(parseExpression_()));
	}
	processCurrentToken_(Token::SEMICOLON);

	return expr;
}


/*
 * IfStatement :== "if" "(" Expression ")" Statement ( "else" Statement )?
 */
StatementPtr Parser::parseIfStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_IF);
	processCurrentToken_();
	processCurrentToken_(Token::LEFTPAREN);

	ExpressionPtr condition(parseExpression_());
	processCurrentToken_(Token::RIGHTPAREN);

	StatementPtr ifStmt(parseStatement_());
	StatementPtr elseStmt;

	if (currentToken_.type() == Token::KEYWORD_ELSE) {
		processCurrentToken_();
		elseStmt = parseStatement_();
	}

	return StatementPtr(new AST::IfElseStmt(std::move(condition), std::move(ifStmt), std::move(elseStmt)));
}


/*
 * WhileStatement :== "while" "(" Expression ")" Statement
 */
StatementPtr Parser::parseWhileStatement_()
{
	ExpressionPtr condition;
	StatementPtr contents;
	
	assert(currentToken_.type() == Token::KEYWORD_WHILE);
	processCurrentToken_();
	processCurrentToken_(Token::LEFTPAREN);
	condition = parseExpression_();
	processCurrentToken_(Token::RIGHTPAREN);
	contents = parseStatement_();

	return WhileStmtPtr(new AST::WhileStmt(std::move(condition), std::move(contents)));
}

/*
 * DoWhileStatement :== "do" Statement "while" "(" Expression ")" ";"
 */
StatementPtr Parser::parseDoWhileStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_DO);
	processCurrentToken_();
	StatementPtr contents(parseStatement_());
	processCurrentToken_(Token::KEYWORD_WHILE);
	processCurrentToken_(Token::LEFTPAREN);
	ExpressionPtr condition(parseExpression_());
	processCurrentToken_(Token::RIGHTPAREN);
	processCurrentToken_(Token::SEMICOLON);

	return DoWhileStmtPtr(new AST::DoWhileStmt(std::move(condition), std::move(contents)));
}

/*
 * ForStatement :== "for" "(" Expression? ";" Expression? ";" Expression? ")" Statement |
 *                  "for" "(" VariableStatement Expression? ";" Expression? ")" Statement
 */
StatementPtr Parser::parseForStatement_()
{
	StatementPtr init;
	ExpressionPtr condition;
	StatementPtr iteration;
	
	assert(currentToken_.type() == Token::KEYWORD_FOR);
	processCurrentToken_();
	processCurrentToken_(Token::LEFTPAREN);

	if (currentToken_.type() == Token::KEYWORD_LOCAL) {
		init = parseVariableStatement_();
	} else {
		if (currentToken_.type() != Token::SEMICOLON) {
			init = ExpressionStmtPtr(new AST::ExpressionStmt(parseExpression_()));
		}
		processCurrentToken_(Token::SEMICOLON);
	}

	if (currentToken_.type() != Token::SEMICOLON) {
		condition = parseExpression_();
	}
	processCurrentToken_(Token::SEMICOLON);
	
	if (currentToken_.type() != Token::SEMICOLON) {
		iteration = ExpressionStmtPtr(new AST::ExpressionStmt(parseExpression_()));
	}

	processCurrentToken_(Token::RIGHTPAREN);
	StatementPtr contents(parseStatement_());

	return ForStmtPtr(new AST::ForStmt(std::move(init), std::move(condition),
	                                   std::move(iteration), std::move(contents)));
}


/*
 * JumpStatement :== "return" expression? ";"
 *                   "yield" expression? ";"
 */
StatementPtr Parser::parseReturnStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_RETURN ||
	       currentToken_.type() == Token::KEYWORD_YIELD);
	
	// TODO: implement coroutine/yield
	if (currentToken_.type() == Token::KEYWORD_YIELD) {
		throw Error(L"currently coroutine/yield is not supported");
	}

	AST::ReturnStmt::Type type =
		(currentToken_.type() == Token::KEYWORD_RETURN) ? AST::ReturnStmt::RETURN : AST::ReturnStmt::YIELD;

	processCurrentToken_();

	ExpressionPtr expr;	
	if (currentToken_.type() != Token::SEMICOLON) {
		expr = parseExpression_();
	}

	processCurrentToken_(Token::SEMICOLON);

	return ReturnStmtPtr(new AST::ReturnStmt(type, std::move(expr)));
}

/*
 * LocalStatement :== "local" (FunctionStatement | VariableStatement)
 */
StatementPtr Parser::parseLocalStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_LOCAL);

	processCurrentToken_();

	if (currentToken_.type() == Token::KEYWORD_FUNCTION) {
		processCurrentToken_();

		VariableStmtPtr functionVar(new AST::VariableStmt(currentToken_.lexeme(), nullptr));
		processCurrentToken_(Token::IDENTIFIER);

		functionVar->init = ExpressionPtr(new AST::FunctionExpr(parseFunctionDef_()));

		return StatementPtr(std::move(functionVar));
	} else if (currentToken_.type() == Token::IDENTIFIER) {
		return parseVariableStatement_();
	} else {
		throw Error(L"%d: keyword 'function' or identifier is expected after 'local'", currentToken_.pos().startLine);
	}
}


/*
 * FunctionStatement :== "function" IDENTIFIER FunctionDef
 */
StatementPtr Parser::parseFunctionStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_FUNCTION);

	processCurrentToken_();

	TerminalExprPtr functionName(new AST::TerminalExpr(AST::TerminalExpr::IDENTIFIER, currentToken_.lexeme()));
	processCurrentToken_(Token::IDENTIFIER);

	FunctionExprPtr functionExpr(new AST::FunctionExpr(parseFunctionDef_()));
	ExpressionPtr assignExpr(new AST::BinaryExpr(AST::BinaryExpr::ASSIGN, std::move(functionExpr), std::move(functionName)));

	return StatementPtr(new AST::ExpressionStmt(std::move(assignExpr)));
}


/*
 * FunctionDef :==  "(" ArgList ")" CompoundStatement
 */
FunctionDefPtr Parser::parseFunctionDef_()
{
	assert(currentToken_.type() == Token::LEFTPAREN);

	StmtSequencePtr argumentList(new AST::StmtSequence);	

	processCurrentToken_(Token::LEFTPAREN);
	if (currentToken_.type() != Token::RIGHTPAREN) {
		AST::StmtSequence::StatementVector &argList = argumentList->statementList;

		argList.push_back(StatementPtr(new AST::VariableStmt(currentToken_.lexeme(), nullptr)));
		processCurrentToken_();
		
		while (currentToken_.type() == Token::COMMA) {
			processCurrentToken_();			
			argList.push_back(StatementPtr(new AST::VariableStmt(currentToken_.lexeme(), nullptr)));
			processCurrentToken_();
		}
	}
	processCurrentToken_(Token::RIGHTPAREN);
	processCurrentToken_(Token::LEFTBRACE);
	StmtSequencePtr contents(parseStmtSequence_(Token::RIGHTBRACE));

	return FunctionDefPtr(new AST::FunctionDefinition(std::move(argumentList), std::move(contents)));
}


/*
 * VariableStatement :== "local" Variable ("," Variable)* ";"
 */
StatementPtr Parser::parseVariableStatement_()
{
	assert(currentToken_.type() == Token::KEYWORD_LOCAL);
	StmtSequencePtr variableList(new AST::StmtSequence);
	AST::StmtSequence::StatementVector &varList = variableList->statementList;
	
	processCurrentToken_(Token::KEYWORD_LOCAL);
	varList.push_back(parseVariable_());
	while (currentToken_.type() != Token::SEMICOLON) {
		processCurrentToken_(Token::COMMA);
		varList.push_back(parseVariable_());
	}
	processCurrentToken_();

	return StatementPtr(std::move(variableList));
}


/*
 * Variable :== IDENTIFIER ("=" AssignmentExpr)?
 */
StatementPtr Parser::parseVariable_()
{
	VariableStmtPtr variable(new AST::VariableStmt(currentToken_.lexeme(), nullptr));
	processCurrentToken_(Token::IDENTIFIER);

	if (currentToken_.type() == Token::ASSIGN) {
		processCurrentToken_();
		variable->init = parseAssignmentExpr_();
	}

	return StatementPtr(std::move(variable));
}


StmtSequencePtr Parser::parseStmtSequence_(const Token::Type delimiter)
{
	StmtSequencePtr stmtSeq(new AST::StmtSequence);

	if (currentToken_.type() != delimiter) {	
		AST::StmtSequence::StatementVector &stmtList = stmtSeq->statementList;
		
		StatementPtr statement(parseStatement_());
		stmtList.push_back(std::move(statement));

		while (currentToken_.type() != delimiter) {
			statement = parseStatement_();
			stmtList.push_back(std::move(statement));
		}
	}

	processCurrentToken_();

	return stmtSeq;
}


/*
 * Expression :== AssignmentExpr
 *
 * NOTE: Currently C-- does not support "," operator to reserve this operator
 *       for other use (ex. multiple assignment) in the future
 */
ExpressionPtr Parser::parseExpression_()
{
	return parseAssignmentExpr_();
}


/*
 * AssignExrp :== ConditionalExpr (AssignmentOp ConditionalExpr)*
 * AssignmentOp :== "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "<<=" | ">>=" | "&=" | "|=" | "^="
 */
ExpressionPtr Parser::parseAssignmentExpr_()
{
	ExpressionPtr expr(parseConditionalExpr_());

	if (isAssignOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();

		expr = ExpressionPtr(new AST::BinaryExpr(op, parseConditionalExpr_(), std::move(expr)));
		AST::BinaryExpr* bottomAssignExpr = static_cast<AST::BinaryExpr*>(expr.get());

		while (isAssignOp(currentToken_)) {
			AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
			processCurrentToken_();
			bottomAssignExpr->first = ExpressionPtr(new AST::BinaryExpr(op, parseConditionalExpr_(),
			                                                            std::move(bottomAssignExpr->first)));
			bottomAssignExpr = static_cast<AST::BinaryExpr*>(bottomAssignExpr->first.get());
		}
	}
	/*
	while (isAssignOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(op, parseConditionalExpr_(), std::move(expr)));
	}*/

	return expr;
}


/*
 * ConditionalExrp :== LogicalOrExpr ("?" Expression : ConditionalExpr)?
 */
ExpressionPtr Parser::parseConditionalExpr_()
{
	ExpressionPtr ifExpr(parseLogicalOrExpr_());

	if (currentToken_.type() == Token::LOGIC_IF) {
		processCurrentToken_();
		ExpressionPtr thenExpr(parseExpression_());
		processCurrentToken_(Token::COLON);
		ExpressionPtr elseExpr(parseConditionalExpr_());
		return ExpressionPtr(new AST::TrinaryExpr(AST::TrinaryExpr::BRANCH, std::move(ifExpr),
		                                          std::move(thenExpr), std::move(elseExpr)));
	} else {
		return ifExpr;
	}
}


/*
 * LogicalOrExrp :== LogicalAndExpr ("||" LogicalAndExpr)*
 */
ExpressionPtr Parser::parseLogicalOrExpr_()
{
	ExpressionPtr expr(parseLogicalAndExpr_());

	while (currentToken_.type() == Token::LOGIC_OR) {
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::LOGIC_OR,
		                                         std::move(expr), parseLogicalAndExpr_()));
	}

	return expr;
}


/*
 * LogicalAndExrp :== BitwiseOrExpr ("&&" BitwiseOrExpr)*
 */
ExpressionPtr Parser::parseLogicalAndExpr_()
{
	ExpressionPtr expr(parseBitwiseOrExpr_());

	while (currentToken_.type() == Token::LOGIC_AND) {
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::LOGIC_AND,
		                                         std::move(expr), parseBitwiseOrExpr_()));
	}

	return expr;
}


/*
 * BitwiseOrExpr :== BitwiseXorExpr ("|" BitwiseXorExpr)*
 */
ExpressionPtr Parser::parseBitwiseOrExpr_()
{
	ExpressionPtr expr(parseBitwiseXorExpr_());

	while (currentToken_.type() == Token::BIT_OR) {
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::BIT_OR,
		                                         std::move(expr), parseBitwiseXorExpr_()));
	}

	return expr;
}


/*
 * BitwiseXorExpr :== BitwiseAndExpr ("^" BitwiseAndExpr)*
 */
ExpressionPtr Parser::parseBitwiseXorExpr_()
{
	ExpressionPtr expr(parseBitwiseAndExpr_());

	while (currentToken_.type() == Token::BIT_XOR) {
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::BIT_XOR,
		                                         std::move(expr), parseBitwiseAndExpr_()));
	}

	return expr;
}


/*
 * BitwiseAndExpr :== EqualityExpr ("&" EqualityExpr)*
 */
ExpressionPtr Parser::parseBitwiseAndExpr_()
{
	ExpressionPtr expr(parseEqualityExpr_());

	while (currentToken_.type() == Token::BIT_AND) {
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::BIT_AND,
		                                         std::move(expr), parseEqualityExpr_()));
	}

	return expr;
}


/*
 * EqualityExpr :== RelationalExpr (EqualityOp RelationalExpr)*
 * EqualityOp :== "==" | "!="
 */
ExpressionPtr Parser::parseEqualityExpr_()
{
	ExpressionPtr expr(parseRelationalExpr_());

	while (isEqualityOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(op, std::move(expr), parseRelationalExpr_()));
	}

	return expr;
}


/*
 * RelationalExpr :== ShiftExpr (RelationalOp ShiftExpr)*
 * RelationalOp :== "<" | "<=" | ">" | ">="
 */
ExpressionPtr Parser::parseRelationalExpr_()
{
	ExpressionPtr expr(parseShiftExpr_());

	while (isRelationalOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();
		expr = ExpressionPtr(new AST::BinaryExpr(op, std::move(expr), parseShiftExpr_()));
	}

	return expr;
}


/*
 * ShiftExpr :== AdditiveExpr (ShiftOp AdditiveExpr)*
 * ShiftOp :== "<<" | ">>"
 */
ExpressionPtr Parser::parseShiftExpr_()
{
	ExpressionPtr expr(parseAdditiveExpr_());

	while (isShiftOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();		
		expr = ExpressionPtr(new AST::BinaryExpr(op, std::move(expr), parseAdditiveExpr_()));
	}

	return expr;
}


/*
 * AdditiveExpr :== MultiplicativeExpr (AdditiveOp MultiplicativeExpr)*
 * AdditiveOp :== "+" | "-"
 */
ExpressionPtr Parser::parseAdditiveExpr_()
{
	ExpressionPtr expr(parseMultiplicativeExpr_());

	while (isAdditiveOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();		
		expr = ExpressionPtr(new AST::BinaryExpr(op, std::move(expr), parseMultiplicativeExpr_()));
	}

	return expr;
}


/*
 * MultiplicativeExpr :== UnaryExpr (MultiplicativeOp UnaryExpr)*
 * MultiplicativeOp :== "*" | "/"
 */
ExpressionPtr Parser::parseMultiplicativeExpr_()
{
	ExpressionPtr expr(parseUnaryExpr_());

	while (isMultiplicativeOp(currentToken_)) {
		AST::BinaryExpr::Operator op = TokenToBinaryOp(currentToken_);
		processCurrentToken_();		
		expr = ExpressionPtr(new AST::BinaryExpr(op, std::move(expr), parseUnaryExpr_()));
	}

	return expr;
}


/*
 * UnaryExpr :== PostfixExpr | UnaryOp UnaryExpr
 * UnaryOp :== "+" | "-" | "++" | "--" | "~" | "!"
 */
ExpressionPtr Parser::parseUnaryExpr_()
{
	if (isUnaryOp(currentToken_)) {
		AST::UnaryExpr::Operator op = TokenToPrefixOp(currentToken_);
		processCurrentToken_();		
		return ExpressionPtr(new AST::UnaryExpr(op, parseUnaryExpr_()));
	} else {
		return parsePostfixExpr_();
	}
}


/*
 * PostfixExpr :== PrimaryExpr ("[" Expression "]" | "(" ExprList ")" | "++" | "--")*
 */
ExpressionPtr Parser::parsePostfixExpr_()
{
	ExpressionPtr expr(parsePrimaryExpr_());

	while (isPostfixOp(currentToken_)) {
		switch (currentToken_.type()) {
		case Token::LEFTBRACKET:
			processCurrentToken_(); 
			expr = ExpressionPtr(new AST::BinaryExpr(AST::BinaryExpr::INDEX,
			                                         std::move(expr), parseExpression_()));
			processCurrentToken_(Token::RIGHTBRACKET); 
			break;
		case Token::LEFTPAREN: {
			processCurrentToken_();
			CallExprPtr callExpr(new AST::CallExpr(std::move(expr)));
			if (currentToken_.type() != Token::RIGHTPAREN) {
				parseCallExpr_(callExpr.get());
			}
			expr = std::move(callExpr);
			processCurrentToken_(Token::RIGHTPAREN);			
			break;
		}
		case Token::ARITH_INC:
			processCurrentToken_();
			expr = ExpressionPtr(new AST::UnaryExpr(AST::UnaryExpr::POSTFIX_INC, std::move(expr)));
			break;
		case Token::ARITH_DEC:
			processCurrentToken_();
			expr = ExpressionPtr(new AST::UnaryExpr(AST::UnaryExpr::POSTFIX_DEC, std::move(expr)));
			break;
		}
	}

	return expr;
}


/*
 * PrimaryExpr :== Identifier |
 *                 Constant |
 *                 String |
 *                 "(" Expression ")" |
 *                 TableExpr |
 *                 FunctionExpr
 */
ExpressionPtr Parser::parsePrimaryExpr_()
{
	ExpressionPtr terminal;

	switch (currentToken_.type()) {
	case Token::IDENTIFIER:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::IDENTIFIER, currentToken_.lexeme()));
		processCurrentToken_();
		return terminal;
	case Token::LITERAL_INT:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::INTEGER, currentToken_.lexeme()));
		processCurrentToken_();
		return terminal;
	case Token::LITERAL_HEX:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::HEX, currentToken_.lexeme()));
		processCurrentToken_();
		return terminal;
	case Token::LITERAL_FLOAT:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::FLOAT, currentToken_.lexeme()));
		processCurrentToken_();
		return terminal;
	case Token::LITERAL_STRING:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::STRING, currentToken_.lexeme()));
		processCurrentToken_();
		return terminal;
	case Token::KEYWORD_TRUE:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::INTEGER, L"1"));
		processCurrentToken_();
		return terminal;
	case Token::KEYWORD_FALSE:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::INTEGER, L"0"));
		processCurrentToken_();
		return terminal;
	case Token::KEYWORD_NULL:
		terminal = ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::NULLTYPE, L""));
		processCurrentToken_();
		return terminal;
	case Token::LEFTPAREN:
	{
		processCurrentToken_();
		ExpressionPtr expr(parseExpression_());
		processCurrentToken_(Token::RIGHTPAREN);
		return expr;
	}
	case Token::LEFTBRACE:
	case Token::KEYWORD_TABLE:
	case Token::KEYWORD_ARRAY:
		return parseTableExpr_();
	case Token::KEYWORD_FUNCTION:
		return parseFunctionExpr_();
	default:
		throw Error(L"%d: %s is not a primary expression", currentToken_.pos().startLine, currentToken_.lexeme().c_str());
	}
}

ExpressionPtr Parser::newIntegerTerminal_(const uint32_t integer)
{
	wchar_t buffer[10];
#pragma warning (push)
#pragma warning (disable:4996)
	swprintf(buffer, L"%d", integer);
#pragma warning (pop)
	return ExpressionPtr(new AST::TerminalExpr(AST::TerminalExpr::INTEGER, buffer));
}

/*
 * TableExpr :== "table" ("{" FieldList? "}")? |
 *               "array" ("{" FieldList? "}")? |
 *               "{" FieldList? "}"
 * FieldList :== Field ("," Field)*
 */
ExpressionPtr Parser::parseTableExpr_()
{
	assert(currentToken_.type() == Token::KEYWORD_TABLE ||
	       currentToken_.type() == Token::KEYWORD_ARRAY ||
	       currentToken_.type() == Token::LEFTBRACE);

	AST::TableExpr::Type tableType;

	if (currentToken_.type() == Token::KEYWORD_TABLE) {
		tableType = AST::TableExpr::TABLE;
		processCurrentToken_();
	} else if (currentToken_.type() == Token::KEYWORD_ARRAY) {
		tableType = AST::TableExpr::ARRAY;
		processCurrentToken_();
	} else {
		tableType = AST::TableExpr::UNKNOWN;
	}

	TableExprPtr tableExpr(new AST::TableExpr(tableType));

	if (currentToken_.type() == Token::LEFTBRACE) {
		processCurrentToken_();
		uint32_t arrayIndex = 0;
		
		if (currentToken_.type() != Token::RIGHTBRACE) {
			AST::TableExpr::InitializerVector &initList = tableExpr->initializerList;
			
			initList.push_back(parseField_());
			if (initList.back().get()->key == nullptr) {
				initList.back().get()->key = newIntegerTerminal_(arrayIndex++);
			}
			while (currentToken_.type() == Token::COMMA) {
				processCurrentToken_();
				initList.push_back(parseField_());
				if (initList.back().get()->key == nullptr) {
					initList.back().get()->key = newIntegerTerminal_(arrayIndex++);
				}
			}
		}
		processCurrentToken_(Token::RIGHTBRACE);
	}

	return ExpressionPtr(std::move(tableExpr));
}


/*
 * Field :== Expression (":" Expression)?
 */
TableInitPtr Parser::parseField_()
{
	ExpressionPtr key(parseExpression_());
	ExpressionPtr value;

	if (currentToken_.type() == Token::COLON) {
		processCurrentToken_();
		value = parseExpression_();
	} else {
		value = ExpressionPtr(std::move(key));
	}

	return TableInitPtr(new AST::TableInitializer(std::move(key), std::move(value)));
}

/*
 * FunctionExpr :== "function" FunctionDef
 */
ExpressionPtr Parser::parseFunctionExpr_()
{
	assert(currentToken_.type() == Token::KEYWORD_FUNCTION);

	processCurrentToken_();

	return ExpressionPtr(new AST::FunctionExpr(parseFunctionDef_()));
}


/*
 * ExprList :== ConditionalExpression ("," ConditionalExpression)*
 */
void Parser::parseCallExpr_(AST::CallExpr* callExpr)
{
	AST::CallExpr::ArgumentVector &argList = callExpr->argumentList;
	
	argList.push_back(parseConditionalExpr_());

	while (currentToken_.type() == Token::COMMA) {
		processCurrentToken_();
		argList.push_back(parseConditionalExpr_());
	}
}


} // The end of namespace "cmm"