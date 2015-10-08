#include "StdAfx.h"
#include "ASTDrawer.h"

#include <cstdarg>
#include <cstdint>
#include <iostream>

#include "AST.h"

namespace cmm {


ASTDrawer::ASTDrawer()
: indention_(0), branching_(false), holding_(true)
{
}

ASTDrawer::~ASTDrawer()
{
}

std::wstring& ASTDrawer::draw(AST::FunctionDefinition& rootFunction)
{
	rootFunction.accept(*this);

	return output_;
}



inline const bool ASTDrawer::safeVisit_(AST::Base* host)
{
	if (host == nullptr) {
		return false;
	} else {
		host->accept(*this);
		return true;
	}
}

void ASTDrawer::appendFlagInfo_(const AST::Base& node)
{
	if (node.flag == 0) {
		return;
	}
	appendTreeLine_();
	append_(L"FLAG : ");
	if (node.flag & AST::FLAG_ERROR)     { append_(L"ERROR "); }
	if (node.flag & AST::FLAG_LVALUE)    { append_(L"LVALUE "); }
	if (node.flag & AST::FLAG_STORE)     { append_(L"STORE "); }
	if (node.flag & AST::FLAG_NOLOAD)    { append_(L"NOLOAD "); }
	if (node.flag & AST::FLAG_TABLE)     { append_(L"TABLE "); }
	if (node.flag & AST::FLAG_GLOBAL)    { append_(L"GLOBAL "); }
	if (node.flag & AST::FLAG_UPVALUE)   { append_(L"UPVALUE "); }
	if (node.flag & AST::FLAG_INTVALUE)  { append_(L"INTVALUE "); }
	if (node.flag & AST::FLAG_ARRAY)     { append_(L"ARRAY "); }
	if (node.flag & AST::FLAG_TEMP)      { append_(L"TEMP "); }
	if (node.flag & AST::FLAG_TEMPTABLE) { append_(L"TEMPTABLE "); }
	append_(L"\n");
}

void ASTDrawer::appendBaseInfo_(const wchar_t nodeName[], const AST::Base& node)
{
	appendTreeLine_();
	append_(L"%s\n", nodeName);
	appendFlagInfo_(node);
}

void ASTDrawer::appendNumberInfo_(const wchar_t format[], const uint32_t number)
{
	if (number != UINT32_MAX) {
		appendTreeLine_();
		append_(format, number);
		append_(L"\n");
	}
}

void ASTDrawer::appendStringInfo_(const wchar_t format[], const wchar_t string[])
{
	appendTreeLine_();
	append_(format, string);
	append_(L"\n");
}

void ASTDrawer::appendNewline_(const wchar_t string[])
{
	appendTreeLine_();
	append_(string);
	append_(L"\n");
}

void ASTDrawer::appendNewline_()
{
	appendTreeLine_();
	append_(L"\n");
}

void ASTDrawer::appendExpressionInfo_(const AST::Expression& node)
{	
	if (node.registerOffset != UINT32_MAX) {
		appendTreeLine_();
		append_(L"Register : %d\n", node.registerOffset);
	}
	
	if (node.lvalue1 != UINT32_MAX) {
		appendTreeLine_();
		if (node.flag & AST::FLAG_TABLE) {
			append_(L"Table key : %d\n", node.lvalue1);
		} else if (node.flag & AST::FLAG_GLOBAL) {
			append_(L"Global key : %d\n", node.lvalue1);
		} else if (node.flag & AST::FLAG_UPVALUE) {
			append_(L"Function level : %d\n", node.lvalue1);
		}
	}
	if (node.lvalue2 != UINT32_MAX) {
		appendTreeLine_();
		if (node.flag & AST::FLAG_TABLE) {
			append_(L"Table value : %d\n", node.lvalue2);
		} else if (node.flag & AST::FLAG_UPVALUE) {
			append_(L"Register : %d\n", node.lvalue2);
		}
	}
}

void ASTDrawer::append_(const wchar_t format[], ...)
{
	va_list args;
	va_start(args, format);
	wchar_t buffer[256];

	vswprintf(buffer, 256, format, args);

	output_.append(buffer);
}


void ASTDrawer::appendTreeLine_()
{
	updateTreeLine_();

	output_.append(treeLine_);
}


void ASTDrawer::pushTreeLine_(const bool isLine)
{
	updateTreeLine_();

	if (isLine == true) {
		treeLine_.push_back(L'\x2502');
	} else {
		treeLine_.push_back(L'\x3000');
	}
}

void ASTDrawer::updateTreeLine_()
{
	if (treeLine_.size() == 0) {
		return;
	}
	
	if (treeLine_.back() == L'\x2514') { // '|_'
		treeLine_.back() = L'\x3000';
	} else if (treeLine_.back() == L'\x251C') { // '|-'
		treeLine_.back() = L'\x2502';
	} else if (treeLine_.back() == L'\x2502') { // '|'
		if (branching_ == true) {
			if (holding_ == true) {
				treeLine_.back() = L'\x251C';
			} else {
				treeLine_.back() = L'\x2514';
			}
		}
		branching_ = false;
		holding_ = true;
	}
}

void ASTDrawer::popTreeLine_()
{
	treeLine_.pop_back();
}

void ASTDrawer::turnOnBranchFlag_(const bool holding)
{
	branching_ = true;
	holding_ = holding;
}



void ASTDrawer::visit(AST::StmtSequence& stmtSequence)
{
	AST::StmtSequence::StatementVector &stmtList = stmtSequence.statementList;

	appendBaseInfo_(L"Statement Sequence", stmtSequence);
	pushTreeLine_(stmtList.size() > 0);
	appendNewline_();

	for (uint32_t i = 0; i < stmtList.size(); i++) {
		turnOnBranchFlag_(i != stmtList.size() - 1);
		safeVisit_(stmtList[i].get());
	}
	popTreeLine_();
}


void ASTDrawer::visit(AST::TableInitializer& tableInit)
{ 
	appendBaseInfo_(L"Table Initializer", tableInit);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(tableInit.key.get());

	turnOnBranchFlag_(false);
	safeVisit_(tableInit.value.get());
	popTreeLine_();
}


void ASTDrawer::visit(AST::FunctionDefinition& functionDef)
{
	appendBaseInfo_(L"Function Definition", functionDef);
	appendNumberInfo_(L"Function Level : %d", functionDef.functionLevel);
	appendNumberInfo_(L"Function Number : %d", functionDef.functionNum);
	appendNumberInfo_(L"Number of Variables : %d", functionDef.numVariable);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(functionDef.arguments.get());

	turnOnBranchFlag_(false);
	safeVisit_(functionDef.contents.get());
	popTreeLine_();
}

void ASTDrawer::visit(AST::CompoundStmt& compoundStmt)
{ 
	appendBaseInfo_(L"Compound Statement", compoundStmt);
	appendNumberInfo_(L"Scope Level : %d", compoundStmt.scopeLevel);
	appendNumberInfo_(L"Number of Variables : %d", compoundStmt.numVariable);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(false);
	safeVisit_(compoundStmt.contents.get());
	popTreeLine_();
}

void ASTDrawer::visit(AST::ForStmt& forStmt)
{
	appendBaseInfo_(L"For Statement", forStmt);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(forStmt.initial.get());

	turnOnBranchFlag_(true);
	safeVisit_(forStmt.condition.get());

	turnOnBranchFlag_(true);
	safeVisit_(forStmt.iteration.get());

	turnOnBranchFlag_(false);
	safeVisit_(forStmt.contents.get());
	
	popTreeLine_();
}

void ASTDrawer::visit(AST::WhileStmt& whileStmt)
{
	appendBaseInfo_(L"While Statement", whileStmt);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(whileStmt.condition.get());

	turnOnBranchFlag_(false);
	safeVisit_(whileStmt.contents.get());
	popTreeLine_();
}

void ASTDrawer::visit(AST::DoWhileStmt& doWhileStmt)
{
	appendBaseInfo_(L"Do While Statement", doWhileStmt);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(doWhileStmt.condition.get());

	turnOnBranchFlag_(false);
	safeVisit_(doWhileStmt.contents.get());
	popTreeLine_();
}

void ASTDrawer::visit(AST::IfElseStmt& ifElseStmt)
{
	appendBaseInfo_(L"If Else Statement", ifElseStmt);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(ifElseStmt.condition.get());

	turnOnBranchFlag_(ifElseStmt.elseContents.get() != nullptr);
	safeVisit_(ifElseStmt.ifContents.get());

	turnOnBranchFlag_(false);
	safeVisit_(ifElseStmt.elseContents.get());

	popTreeLine_();
}
	
void ASTDrawer::visit(AST::ReturnStmt& returnStmt)
{
	appendBaseInfo_(L"Return Statement", returnStmt);
	if (returnStmt.type == AST::ReturnStmt::RETURN) {
		appendNewline_(L"Type : RETURN");
	} else {
		appendNewline_(L"Type : YIELD");
	}
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(false);
	safeVisit_(returnStmt.returnExpr.get());

	popTreeLine_();
}

void ASTDrawer::visit(AST::JumpStmt& jumpStmt)
{
	appendBaseInfo_(L"Jump Statement", jumpStmt);
	if (jumpStmt.type == AST::JumpStmt::CONTINUE) {
		appendNewline_(L"Type : CONTINUE");
	} else {
		appendNewline_(L"Type : BREAK");
	}
	appendNewline_();
}

void ASTDrawer::visit(AST::VariableStmt& variableStmt)
{
	appendBaseInfo_(L"Variable Statement", variableStmt);
	appendStringInfo_(L"Name : %s", variableStmt.name.c_str());
	appendNumberInfo_(L"Function level: %d", variableStmt.functionLevel);
	appendNumberInfo_(L"Scope level: %d", variableStmt.scopeLevel);

	pushTreeLine_(variableStmt.init != nullptr);
	appendNewline_();
	
	turnOnBranchFlag_(false);
	safeVisit_(variableStmt.init.get());
	popTreeLine_();
}


void ASTDrawer::visit(AST::ExpressionStmt& expressionStmt)
{
	appendBaseInfo_(L"Expression Statement", expressionStmt);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(false);
	safeVisit_(expressionStmt.expression.get());
	
	popTreeLine_();
}


void ASTDrawer::visit(AST::UnaryExpr& unaryExpr)
{	
	appendBaseInfo_(L"Unary Expression", unaryExpr);
	appendExpressionInfo_(unaryExpr);
	
	switch(unaryExpr.op)
	{
	case AST::UnaryExpr::PREFIX_INC:  appendNewline_(L"Operator : ++(Prefix)"); break;
	case AST::UnaryExpr::PREFIX_DEC:  appendNewline_(L"Operator : --(Prefix)"); break;
	case AST::UnaryExpr::POSTFIX_INC: appendNewline_(L"Operator : ++(Postfix)"); break;
	case AST::UnaryExpr::POSTFIX_DEC: appendNewline_(L"Operator : --(Postfix)"); break;
	default: break;
	}

	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(false);
	safeVisit_(unaryExpr.first.get());

	popTreeLine_();
}

void ASTDrawer::visit(AST::BinaryExpr& binaryExpr)
{
	bool inverted = false;

	appendBaseInfo_(L"Binary Expression", binaryExpr);
	appendExpressionInfo_(binaryExpr);

	switch(binaryExpr.op) {
	case AST::BinaryExpr::ARITH_ADD:     appendNewline_(L"Operator : +"); break;
	case AST::BinaryExpr::ARITH_SUB:     appendNewline_(L"Operator : -"); break;
	case AST::BinaryExpr::ARITH_MUL:     appendNewline_(L"Operator : *"); break;
	case AST::BinaryExpr::ARITH_DIV:     appendNewline_(L"Operator : /"); break;
	case AST::BinaryExpr::ARITH_MOD:     appendNewline_(L"Operator : %"); break;
	case AST::BinaryExpr::BIT_AND:       appendNewline_(L"Operator : &"); break;
	case AST::BinaryExpr::BIT_OR:        appendNewline_(L"Operator : |"); break;
	case AST::BinaryExpr::BIT_XOR:       appendNewline_(L"Operator : ^"); break;
	case AST::BinaryExpr::BIT_SL:        appendNewline_(L"Operator : <<"); break;
	case AST::BinaryExpr::BIT_SR:        appendNewline_(L"Operator : >>"); break;
	case AST::BinaryExpr::ASSIGN:        appendNewline_(L"Operator : ="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_ADD:    appendNewline_(L"Operator : +="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_SUB:    appendNewline_(L"Operator : -="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_MUL:    appendNewline_(L"Operator : *="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_DIV:    appendNewline_(L"Operator : /="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_MOD:    appendNewline_(L"Operator : %="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_SL:     appendNewline_(L"Operator : <<="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_SR:     appendNewline_(L"Operator : >>="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_AND:    appendNewline_(L"Operator : &="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_OR:     appendNewline_(L"Operator : |="); inverted = true; break;
	case AST::BinaryExpr::ASSIGN_XOR:    appendNewline_(L"Operator : ^="); inverted = true; break;
	case AST::BinaryExpr::LOGIC_EQ:      appendNewline_(L"Operator : =="); break;
	case AST::BinaryExpr::LOGIC_NOTEQ:   appendNewline_(L"Operator : !="); break;
	case AST::BinaryExpr::LOGIC_GREATER: appendNewline_(L"Operator : >"); break;
	case AST::BinaryExpr::LOGIC_GE:      appendNewline_(L"Operator : >="); break;
	case AST::BinaryExpr::LOGIC_LESS:    appendNewline_(L"Operator : <"); break;
	case AST::BinaryExpr::LOGIC_LE:      appendNewline_(L"Operator : <="); break;
	case AST::BinaryExpr::LOGIC_AND:     appendNewline_(L"Operator : &&"); break;
	case AST::BinaryExpr::LOGIC_OR:      appendNewline_(L"Operator : ||"); break;
	case AST::BinaryExpr::INDEX:         appendNewline_(L"Operator : []"); break;
	default: break;		
	}

	pushTreeLine_(true);
	appendNewline_();
	
	if (inverted == false) {
		turnOnBranchFlag_(true);
		safeVisit_(binaryExpr.first.get());

		turnOnBranchFlag_(false);
		safeVisit_(binaryExpr.second.get());
	} else {
		turnOnBranchFlag_(true);
		safeVisit_(binaryExpr.second.get());

		turnOnBranchFlag_(false);
		safeVisit_(binaryExpr.first.get());
	}

	popTreeLine_();
}

void ASTDrawer::visit(AST::TrinaryExpr& trinaryExpr)
{
	appendBaseInfo_(L"Trinary Expression", trinaryExpr);
	appendExpressionInfo_(trinaryExpr);
	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(true);
	safeVisit_(trinaryExpr.first.get());

	turnOnBranchFlag_(true);
	safeVisit_(trinaryExpr.second.get());

	turnOnBranchFlag_(false);
	safeVisit_(trinaryExpr.third.get());

	popTreeLine_();
}


void ASTDrawer::visit(AST::TerminalExpr& terminalExpr)
{
	appendBaseInfo_(L"Terminal Expression", terminalExpr);
	appendExpressionInfo_(terminalExpr);
		
	switch(terminalExpr.type) {
	case AST::TerminalExpr::IDENTIFIER: appendNewline_(L"Type : Identifier"); break;
	case AST::TerminalExpr::INTEGER:    appendNewline_(L"Type : Integer"); break;
	case AST::TerminalExpr::HEX:        appendNewline_(L"Type : Hexadecimal"); break;
	case AST::TerminalExpr::FLOAT:      appendNewline_(L"Type : Floating point"); break;
	case AST::TerminalExpr::STRING:     appendNewline_(L"Type : String"); break;
	case AST::TerminalExpr::NULLTYPE:   appendNewline_(L"Type : Null"); break;
	default : break;
	}
	appendStringInfo_(L"Lexeme : %s", terminalExpr.lexeme.c_str());
	appendNewline_();
}


void ASTDrawer::visit(AST::CallExpr& callExpr)
{
	AST::CallExpr::ArgumentVector &argList = callExpr.argumentList;

	appendBaseInfo_(L"Call Expression", callExpr);
	appendExpressionInfo_(callExpr);
	appendNumberInfo_(L"Number of Arguments : %d", callExpr.argumentList.size());
	
	pushTreeLine_(true);
	appendNewline_();
	
	turnOnBranchFlag_(argList.size() != 0);
	safeVisit_(callExpr.function.get());

	for (uint32_t i = 0; i < argList.size(); i++) {
		turnOnBranchFlag_(i != argList.size() - 1);
		safeVisit_(argList[i].get());
	}

	popTreeLine_();
}


void ASTDrawer::visit(AST::FunctionExpr& functionExpr)
{
	appendBaseInfo_(L"Function Expression", functionExpr);
	appendExpressionInfo_(functionExpr);

	pushTreeLine_(true);
	appendNewline_();

	turnOnBranchFlag_(false);
	safeVisit_(functionExpr.functionDef.get());

	popTreeLine_();
}


void ASTDrawer::visit(AST::TableExpr& tableExpr)
{
	AST::TableExpr::InitializerVector &initList = tableExpr.initializerList;

	appendBaseInfo_(L"Table Expression", tableExpr);
	appendExpressionInfo_(tableExpr);

	if (tableExpr.type == AST::TableExpr::TABLE) {
		appendNewline_(L"Type : TABLE");
	} else if (tableExpr.type == AST::TableExpr::ARRAY) {
		appendNewline_(L"Type : ARRAY");
	} else {
		appendNewline_(L"Type : UNKNOWN");
	}

	pushTreeLine_(initList.size() != 0);
	appendNewline_();

	for (uint32_t i = 0; i < initList.size(); i++) {
		turnOnBranchFlag_(i != initList.size() - 1);
		safeVisit_(initList[i].get());
	}

	popTreeLine_();
}



} // The end of namespace "cmm"