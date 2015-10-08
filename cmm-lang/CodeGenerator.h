#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <cstdint>
#include <vector>

#include "Object.h"
#include "ASTVisitor.h"
#include "Instruction.h"
#include "Prototype.h"

namespace cmm
{

class Prototype;
class Program;
class ConstantTable;
class ByteCode;
class Error;
struct Variable;

class LabelManager
{
public:
	explicit            LabelManager();
	                    ~LabelManager();

                        LabelManager(const LabelManager&) = delete;
    const LabelManager& operator=(const LabelManager&) = delete;

	uint32_t            newLabel(uint32_t offset = UINT32_MAX);
	void                setOffset(uint32_t labelNum, uint32_t offset);
	uint32_t            getOffset(uint32_t labelNum);
	void                reset();

private:
	typedef std::vector<uint32_t> LabelVector;

	LabelVector         labelList_;
};


class Register
{
public:
	explicit    Register();
	            ~Register();

                Register(const Register&) = delete;
    Register&   operator=(const Register&) = delete;
    
	uint32_t    allocateVariable();
	uint32_t    allocateTempRegister();
	uint32_t    allocate();
	void        deallocate(uint32_t registerOffset);
	void        deallocate(const AST::Expression& expr);
	void        reset();
	uint32_t    maxSize();

private:
	uint32_t    numRegister_;
	uint32_t    maxNumRegister_;
};


class CodeGenerator : public AST::Visitor
{
public:
	explicit        CodeGenerator(ObjectManager& objectManager);
	                ~CodeGenerator();
                    CodeGenerator(const CodeGenerator&) = delete;
    CodeGenerator   operator=(const CodeGenerator&) = delete;

	Ref<Prototype>  createPrototype(AST::FunctionDefinition& functionDef);

	virtual void    visit(AST::StmtSequence& stmtSequence) override;
	virtual void    visit(AST::TableInitializer& tableInit) override;
	virtual void    visit(AST::FunctionDefinition& functionDef) override;

	virtual void    visit(AST::CompoundStmt& compoundStmt) override;
	virtual void    visit(AST::ForStmt& forStmt) override;
	virtual void    visit(AST::WhileStmt& whileStmt) override;
	virtual void    visit(AST::DoWhileStmt& doWhileStmt) override;
    virtual void    visit(AST::IfElseStmt& ifElseStmt) override;
	virtual void    visit(AST::ReturnStmt& returnStmt) override;
	virtual void    visit(AST::JumpStmt& jumpStmt) override;
	virtual void    visit(AST::VariableStmt& variableStmt) override;
	virtual void    visit(AST::ExpressionStmt& expressionStmt) override;

	virtual void    visit(AST::UnaryExpr& unaryExpr) override;
	virtual void    visit(AST::BinaryExpr& binaryExpr) override;
	virtual void    visit(AST::TrinaryExpr& trinaryExpr) override;
	virtual void    visit(AST::TerminalExpr& terminalExpr) override;
	virtual void    visit(AST::CallExpr& callExpr) override;
	virtual void    visit(AST::FunctionExpr& functionExpr) override;
	virtual void    visit(AST::TableExpr& tableExpr) override;

private:
    bool            safeVisit_(AST::Base* host);

	uint32_t        addConstant_(const Variable& constant);
	void            substitueLabelToOffset();

	void            appendCode_(Instruction::Opcode op, int32_t operand1,
	                            int32_t operand2 = 0, int32_t operand3 = 0);
	uint32_t        nextOffset_();

	void            appendUnaryOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op);
	void            appendPrefixOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op);
	void            appendPostfixOp_(AST::UnaryExpr& unaryExpr, Instruction::Opcode op);

	void            appendBinaryOp_(AST::BinaryExpr& binaryExpr, Instruction::Opcode op, bool inverted = false);
	void            appendAssignOp_(AST::BinaryExpr& binaryExpr, Instruction::Opcode Op);
	void            appendAssignOp_(AST::BinaryExpr& binaryExpr);
	void            appendShortCutLogic_(AST::BinaryExpr& binaryExpr);
	void            appendTableLoadOp_(AST::BinaryExpr& binaryExpr);

	void            appendStoreOp_(const AST::Expression& value, const AST::Expression& dest);
	void            appendStoreOp_(uint32_t valueRegister, const AST::Expression& dest);

	ObjectManager&  objectManager_;
	Ref<Prototype>  prototype_;
	LabelManager    labelManager_;
	Register        register_;
};

} // namespace "cmm"

#endif