#include "StdAfx.h"
#include "Analyzer.h"

#include <cassert>
#include <cstdint>
#include <algorithm>

#include "Error.h"
#include "AST.h"

namespace cmm {


Analyzer::Analyzer()
: scopeManager_()
{
}

Analyzer::~Analyzer()
{
}

void Analyzer::analyze(AST::FunctionDefinition& rootFunction)
{
	rootFunction.accept(*this);
}


inline bool Analyzer::safeVisit_(AST::Base* host)
{
	if (host == nullptr) {
		return false;
	} else {
		host->accept(*this);
		return true;
	}
}

void Analyzer::visit(AST::StmtSequence& stmtSequence)
{
	AST::StmtSequence::StatementVector &stmtList = stmtSequence.statementList;

	std::for_each(stmtList.begin(), stmtList.end(),
		[this](decltype(*stmtList.begin()) i) { safeVisit_(i.get()); } );
}


void Analyzer::visit(AST::TableInitializer& tableInit)
{
	safeVisit_(tableInit.key.get());
	safeVisit_(tableInit.value.get());

	if (tableInit.key->flag & AST::FLAG_INTVALUE) {
		tableInit.flag = AST::FLAG_ARRAY;
	}
}


void Analyzer::visit(AST::FunctionDefinition& functionDef)
{
	scopeManager_.openFunctionScope(functionDef);
	scopeManager_.openScope();
	safeVisit_(functionDef.arguments.get());
	safeVisit_(functionDef.contents.get());
	scopeManager_.closeScope();
	functionDef.numVariable = scopeManager_.numVariable();
	functionDef.functionLevel = scopeManager_.functionLevel();
	scopeManager_.closeFunctionScope();
}

void Analyzer::visit(AST::CompoundStmt& compoundStmt)
{ 
	scopeManager_.openScope();	
	safeVisit_(compoundStmt.contents.get());
	scopeManager_.closeScope();
}

void Analyzer::visit(AST::ForStmt& forStmt)
{
	scopeManager_.openScope();
	safeVisit_(forStmt.initial.get());
	safeVisit_(forStmt.condition.get());
	safeVisit_(forStmt.iteration.get());
	scopeManager_.openLoop(forStmt);
	safeVisit_(forStmt.contents.get());
	scopeManager_.closeLoop();
	scopeManager_.closeScope();
}

void Analyzer::visit(AST::WhileStmt& whileStmt)
{
	scopeManager_.openScope();
	safeVisit_(whileStmt.condition.get());
	scopeManager_.openLoop(whileStmt);
	safeVisit_(whileStmt.contents.get());
	scopeManager_.closeLoop();
	scopeManager_.closeScope();
}

void Analyzer::visit(AST::DoWhileStmt& doWhileStmt)
{
	scopeManager_.openScope();
	safeVisit_(doWhileStmt.condition.get());
	scopeManager_.openLoop(doWhileStmt);
	safeVisit_(doWhileStmt.contents.get());
	scopeManager_.closeLoop();
	scopeManager_.closeScope();
}

void Analyzer::visit(AST::IfElseStmt& ifElseStmt)
{
	safeVisit_(ifElseStmt.condition.get());
	scopeManager_.openScope();
	safeVisit_(ifElseStmt.ifContents.get());
	scopeManager_.closeScope();
	scopeManager_.openScope();
	safeVisit_(ifElseStmt.elseContents.get());
	scopeManager_.closeScope();
}
	
void Analyzer::visit(AST::ReturnStmt& returnStmt)
{
	safeVisit_(returnStmt.returnExpr.get());
}

void Analyzer::visit(AST::JumpStmt& jumpStmt)
{
	if (scopeManager_.loopLevel() == 0) {
		throw Error(L"Jump statement cannot be used outside of loop statement");
	} else {
		jumpStmt.correspondingLoop = scopeManager_.retrieveNearestLoop();
	}
}

void Analyzer::visit(AST::VariableStmt& variableStmt)
{
	scopeManager_.registerVariable(variableStmt);
	variableStmt.scopeLevel = scopeManager_.scopeLevel();
	variableStmt.functionLevel = scopeManager_.functionLevel();

	safeVisit_(variableStmt.init.get());
}


void Analyzer::visit(AST::ExpressionStmt& expressionStmt)
{
	safeVisit_(expressionStmt.expression.get());
}


void Analyzer::visit(AST::UnaryExpr& unaryExpr)
{
	AST::Expression &firstExpr = *unaryExpr.first;
	safeVisit_(unaryExpr.first.get());

	switch(unaryExpr.op)
	{
	case AST::UnaryExpr::PREFIX_INC:
	case AST::UnaryExpr::PREFIX_DEC: // NOTE: In C--, prefix expression is not evaluted as l-value
	case AST::UnaryExpr::POSTFIX_INC:
	case AST::UnaryExpr::POSTFIX_DEC:
		if (firstExpr.flag & AST::FLAG_LVALUE) {
			firstExpr.flag |= AST::FLAG_STORE;			
		} else {
			throw Error(L"Operand of increment/decrement must be non-conditional l-value");			
		}
		return;
	default: return;
	}
}

void Analyzer::visit(AST::BinaryExpr& binaryExpr)
{
	AST::Expression &secondExpr = *binaryExpr.second;

	safeVisit_(binaryExpr.first.get());
	safeVisit_(binaryExpr.second.get());

	switch(binaryExpr.op) {
	case AST::BinaryExpr::INDEX:
		binaryExpr.flag = AST::FLAG_LVALUE | AST::FLAG_TABLE;
		return;
	case AST::BinaryExpr::ASSIGN:
	case AST::BinaryExpr::ASSIGN_ADD:
	case AST::BinaryExpr::ASSIGN_SUB:
	case AST::BinaryExpr::ASSIGN_MUL:
	case AST::BinaryExpr::ASSIGN_DIV:
	case AST::BinaryExpr::ASSIGN_MOD:
	case AST::BinaryExpr::ASSIGN_SL:
	case AST::BinaryExpr::ASSIGN_SR:
	case AST::BinaryExpr::ASSIGN_AND:
	case AST::BinaryExpr::ASSIGN_OR:
	case AST::BinaryExpr::ASSIGN_XOR: // NOTE: In C--, assignment expression is not evaluted as l-value
		if (secondExpr.flag & AST::FLAG_LVALUE) { // second expression in assign op is lhs (which is modified value)
			secondExpr.flag |= AST::FLAG_STORE;
			if (binaryExpr.op == AST::BinaryExpr::ASSIGN) {
				secondExpr.flag |= AST::FLAG_NOLOAD;
			}
		} else {
			throw Error(L"left operand of assign operator must be non-conditional l-value");
		}
		return;
	default:
		return;
	}
}

// NOTE: Currently, conditional expression is not evaluated as l-value even if both operands are l-value
// TODO: Support for conditional l-value feature
void Analyzer::visit(AST::TrinaryExpr& trinaryExpr)
{
	safeVisit_(trinaryExpr.first.get());
	safeVisit_(trinaryExpr.second.get());
	safeVisit_(trinaryExpr.third.get());
}


void Analyzer::visit(AST::TerminalExpr& terminalExpr)
{
	if (terminalExpr.type == AST::TerminalExpr::IDENTIFIER) {
		terminalExpr.correspondingVar = scopeManager_.retrieveVariable(terminalExpr.lexeme);

		if (terminalExpr.correspondingVar == nullptr) {
			terminalExpr.flag = AST::FLAG_LVALUE | AST::FLAG_GLOBAL;
		} else if (terminalExpr.correspondingVar->functionLevel < scopeManager_.functionLevel()) {
			terminalExpr.flag = AST::FLAG_LVALUE | AST::FLAG_UPVALUE;
		} else {
			terminalExpr.flag = AST::FLAG_LVALUE;
		}
	} else if (terminalExpr.type == AST::TerminalExpr::INTEGER) {
		terminalExpr.flag |= AST::FLAG_INTVALUE;
	}
}


void Analyzer::visit(AST::CallExpr& callExpr)
{
	AST::CallExpr::ArgumentVector &argList = callExpr.argumentList;

	safeVisit_(callExpr.function.get());
	callExpr.function->flag |= AST::FLAG_LOAD;

	std::for_each(argList.begin(), argList.end(),
		[this](decltype(*argList.begin()) i) {
			safeVisit_(i.get());
			i->flag |= AST::FLAG_LOAD;
		}
	);
}


void Analyzer::visit(AST::FunctionExpr& functionExpr)
{
	safeVisit_(functionExpr.functionDef.get());
}


void Analyzer::visit(AST::TableExpr& tableExpr)
{
	AST::TableExpr::InitializerVector &initList = tableExpr.initializerList;

	for (auto i = initList.begin(); i != initList.end(); i++) {
		AST::TableInitializer *initializer = i->get();
		safeVisit_(initializer);

		if (!(initializer->flag & AST::FLAG_ARRAY)) {
			if (tableExpr.type == AST::TableExpr::ARRAY) {
				throw Error(L"Only an integer literal can be key value of an array");
			} else if (tableExpr.type == AST::TableExpr::UNKNOWN) {
				tableExpr.type = AST::TableExpr::TABLE;
			}
		}
	}

	if (tableExpr.type == AST::TableExpr::UNKNOWN) {
		tableExpr.type = AST::TableExpr::ARRAY;
	}
}







ScopeManager::ScopeManager()
: scopeLevel_(0)
{
}

ScopeManager::~ScopeManager()
{
}

void ScopeManager::openFunctionScope(AST::FunctionDefinition& functionDef)
{
	functionStack_.push_back(Function_(&functionDef));
}

void ScopeManager::closeFunctionScope()
{
	functionStack_.pop_back();
}

void ScopeManager::openScope()
{
	scopeLevel_++;
}

void ScopeManager::closeScope()
{
	scopeLevel_--;
	
	while (variableStack_.size() > 0 && variableStack_.back()->scopeLevel > scopeLevel_) {
		variableStack_.pop_back();
	}
}

void ScopeManager::openLoop(AST::LoopStmt& loopStmt)
{
	LoopStack_ &currentLoopStack = functionStack_.back().loopStack;

	currentLoopStack.push_back(&loopStmt);
}

void ScopeManager::closeLoop()
{
	LoopStack_ &currentLoopStack = functionStack_.back().loopStack;

	currentLoopStack.pop_back();
}

void ScopeManager::registerVariable(AST::VariableStmt& variableStmt)
{
	variableStack_.push_back(&variableStmt);
	functionStack_.back().numVariable++;
}

AST::FunctionDefinition& ScopeManager::currentFunction()
{
	return *functionStack_.back().functionDef;
}

AST::LoopStmt* ScopeManager::retrieveNearestLoop()
{
	LoopStack_ &currentLoopStack = functionStack_.back().loopStack;

	return currentLoopStack.back();
}

AST::VariableStmt* ScopeManager::retrieveVariable(const std::wstring& identifier)
{
	auto result = std::find_if(variableStack_.rbegin(), variableStack_.rend(), 
		[&identifier](decltype(*variableStack_.rbegin()) i) -> const bool {
			return i->name == identifier;
		}
	);

	if (result == variableStack_.rend()) {
		return nullptr;
	} else {
		return *result;
	}
}


uint32_t ScopeManager::functionLevel() const
{
	assert(functionStack_.size() > 0);
	return functionStack_.size() - 1;
}

uint32_t ScopeManager::numVariable() const
{
	return functionStack_.back().numVariable;
}

uint32_t ScopeManager::loopLevel() const
{
	const LoopStack_ &currentLoopStack = functionStack_.back().loopStack;
		
	return currentLoopStack.size();
}

uint32_t ScopeManager::scopeLevel() const
{
	assert(scopeLevel_ >= 0);
	return scopeLevel_;
}

} // namespace"cmm"
