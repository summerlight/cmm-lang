#include "StdAfx.h"
#include "Context.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <algorithm>

#include "Compiler.h"
#include "Context.h"
#include "Prototype.h"
#include "Utility.h"
#include "DataType.h"
#include "Error.h"

namespace cmm 
{

namespace // Anonymous namespace for utility functions only for the Context class
{

// TODO: Currently calculation routine is quite bit inefficient - it should be optimized

template <typename CompOp>
inline const Variable CompareOp(Variable &rhs1, Variable &rhs2)
{
	switch (rhs1.t) {
	case TypeNull:
		return CompOp::op(rhs1.t, rhs2.t);
	case TypeInt:
		if (rhs2.t == TypeInt) {
			return CompOp::op(rhs1.v.i, rhs2.v.i);
		} else if (rhs2.t = TypeFloat) {
			return CompOp::op(rhs1.v.i, rhs2.v.f);
		} else {
			return CompOp::op(rhs1.t, rhs2.t);
		}
	case TypeFloat:
		if (rhs2.t == TypeInt) {
			CompOp::op(rhs1.v.f, rhs2.v.i);
		} else if (rhs2.t = TypeFloat) {
			CompOp::op(rhs1.v.f, rhs2.v.f);
		} else {
			CompOp::op(rhs1.t, rhs2.t);
		}
	case TypeString:		
		if (rhs2.t == TypeString) {
			String& rhs1Str = static_cast<String&>(*rhs1.v.obj);
			String& rhs2Str = static_cast<String&>(*rhs2.v.obj);
			return CompOp::op(rhs1.v.obj, rhs2.v.obj);
		} else {
			return CompOp::op(rhs1.t, rhs2.t);
		}
		break;
	default: 
		return CompOp::op(rhs1.v.obj, rhs2.v.obj);
		break;
	}
}

template <typename BinaryOp>
inline const Variable NumericOp(Variable &rhs1, Variable &rhs2)
{
	if (!rhs1.isNumber() || !rhs2.isNumber()) {
		throw Error(L"wrong attempt to perform arithmetic on non-numeric value.");
	}

	switch (rhs1.t) {
		case TypeInt:
			if (rhs2.t == TypeInt) {
				return BinaryOp::op(rhs1.v.i, rhs2.v.i);
			} else {
				return BinaryOp::op(rhs1.v.i, rhs2.v.f);
			}			
		case TypeFloat: 
			return (rhs2.t == TypeInt) ? BinaryOp::op(rhs1.v.f, rhs2.v.i) : BinaryOp::op(rhs1.v.f, rhs2.v.f);
	}

	assert(false);
	return Variable(TypeNull); // This return statement may not be executed
}

template <typename UnaryOp>
inline const Variable NumericOp(Variable &rhs, const bool checkNumber)
{
	if (checkNumber == true && (!rhs.isNumber())) {
		throw Error(L"wrong attempt to perform an arithmetic operation on non-numeric value.");
	}

	switch (rhs.t) {
		case TypeInt:   return UnaryOp::op(rhs.v.i);
		case TypeFloat: return UnaryOp::op(rhs.v.f);
	};

	assert(false);
	return Variable(TypeNull); // This return statement may not be executed
}

template <typename BinaryOp>
inline const Variable IntegerOp(Variable &rhs1, Variable &rhs2)
{
	if (rhs1.t != TypeInt || rhs2.t != TypeInt) {
		throw Error(L"wrong attempt to perform an integer operation on non-integer value.");
	}

	return BinaryOp::op(rhs1.v.i, rhs2.v.i);
}

template <typename UnaryOp>
inline const Variable IntegerOp(Variable &rhs)
{
	if (rhs.t != TypeInt) {
		throw Error(L"wrong attempt to perform an integer operation on non-integer value.");
	}

	return UnaryOp::op(rhs.v.i);
}

template <typename BinaryOp>
inline const int32_t LogicalOp(Variable &rhs1, Variable &rhs2)
{
	return BinaryOp::op(toBool(rhs1), toBool(rhs2));
}

template <typename UnaryOp>
inline const int32_t LogicalOp(Variable &rhs1)
{
	return UnaryOp::op(toBool(rhs1));
}

inline const int32_t toBool(const Variable &var)
{
	switch (var.t) {
		case TypeNull:    return 0; // 0 = false
		case TypeInt:     return var.v.i ? 1 : 0;
		case TypeFloat:   return var.v.f ? 1 : 0;
		default:          return 1; // 1 = true
	}
}

} // The end of anomymous namespace




Context::Context()
: objectManager_(), global_(new Table(&objectManager_)), buffer_(100, TypeNull), reentrant_(false)
{
}

Context::~Context()
{
}


void Context::run(const uint32_t numArgs, const uint32_t numRets)
{
	if (reentrant_ == true) {
		throw Error(L"currently C-- does not support to reentrant active C-- context");
	}

	if (bufferSize_ != numArgs+1) {
		throw Error(L"the number of argument is not according to the size of stack");
	}

	if (buffer_[0].t != TypeFunc) {
		throw Error(L"wrong attempt to call non-function value");
	}

	functionCall_(&buffer_[0], numArgs, numRets);
	try {
		loop_();
		bufferSize_ = numRets;
	} catch (...) {
		callStack_.clear();
		throw;
	}
}

void Context::load(const wchar_t code[])
{
	Compiler compiler(objectManager_);

	Ref<Prototype> prototype = compiler.compile(code, false, false);

	buffer_[0] = Variable(TypeFunc, new Function(prototype, nullptr, &objectManager_));
	bufferSize_ = 1;
}

void Context::registerCfunction(const wchar_t name[], CFunction func)
{
	Ref<String> string = new String(name, &objectManager_);
	global_->setValue(Variable(TypeString, string.get()), Variable(func));
}

void Context::garbageCollect()
{
	objectManager_.garbageCollect(*global_);
}




void Context::loop_()
{
	for (;;) {
		Function &currentFunction = *callStack_.back().function;
		Closure &currentClosure = *callStack_.back().closure;
		const Instruction &inst = currentFunction.prototype()->instruction(callStack_.back().programCounter);
		int32_t jumpDistance = 1;

#define operand(x) (currentClosure.local(inst.operand##x))
		switch(inst.opcode) {
			// Assign instructions
			case Instruction::ASSIGN:
				operand(1) = operand(2); break;
			case Instruction::GETCONST:
				operand(1) = currentFunction.prototype()->constant(inst.operand2); break;
			case Instruction::GETGLOBAL: {
				const Variable &constant = currentFunction.prototype()->constant(inst.operand2);
				operand(1) = global_->getValue(constant);
				break;
			}
			case Instruction::GETUPVAL:
				operand(1) = currentClosure.upValue(inst.operand3, inst.operand2); break;
			case Instruction::GETTABLE: {
				Variable &lhs = operand(1);
				Variable &container = operand(2);
				Variable &key = operand(3);

				switch (container.t) {
					case TypeTable:
						lhs = static_cast<Table*>(container.v.obj)->getValue(key);
						break;
					case TypeArray:
						if (key.t == TypeInt) {
							lhs = static_cast<Array*>(container.v.obj)->getValue(key.v.i);
						} else {
							throw Error(L"non-integer value for index value on array type");
						}
						break;
					default:
						throw Error(L"wrong type for index operation");
						break;
				};
				break;
			}
			case Instruction::SETGLOBAL: {
				const Variable &constant = currentFunction.prototype()->constant(inst.operand1);
				global_->setValue(constant, operand(2));
				break;
			}
			case Instruction::SETUPVAL:
				currentClosure.upValue(inst.operand3, inst.operand1) = operand(2); break;
			case Instruction::SETTABLE: {
				Variable &container = operand(1);
				Variable &value = operand(2);
				Variable &key = operand(3);

				switch (container.t) {
					case TypeTable:
						static_cast<Table*>(container.v.obj)->setValue(key, value);
						break;
					case TypeArray:
						if (key.t == TypeInt) {
							static_cast<Array*>(container.v.obj)->setValue(key.v.i, value);
						} else {
							throw Error(L"non-integer value for index value on array type");
						}
						break;
					default:
						throw Error(L"wrong type for index operation");
						break;
				};
				break;
			}

			// Object creation instructions
			case Instruction::NEWTABLE: {
				Table* newTable = new Table(&objectManager_);
				operand(1) = Variable(TypeTable, newTable); break;
			}
			case Instruction::NEWARRAY: {
				Array* newArray = new Array(&objectManager_);
				operand(1) = Variable(TypeArray, newArray); break;
			}
			case Instruction::NEWFUNC: {
				Prototype& currentPrototype = *currentFunction.prototype();
				Function *newFunction = new Function(currentPrototype.localPrototype(inst.operand2), &currentClosure, &objectManager_);
				
				operand(1) = Variable(TypeFunc, newFunction);			
				break;
			}

			// Arithmetic operation instructions
			case Instruction::ADD: {			
				if (operand(2).t == TypeString && operand(3).t == TypeString) {
					String& rhs1 = static_cast<String&>(*operand(2).v.obj);
					String& rhs2 = static_cast<String&>(*operand(3).v.obj);
					String* result = new String(rhs1.value() + rhs2.value(), &objectManager_);
					operand(1) = Variable(TypeString, result);
				} else {
					operand(1) = NumericOp<OpAdd>(operand(2), operand(3));
				}			
				break;
			}
			case Instruction::SUB:    operand(1) = NumericOp<OpSubtract>(operand(2), operand(3)); break;
			case Instruction::MUL:    operand(1) = NumericOp<OpMultiply>(operand(2), operand(3)); break;
			case Instruction::DIV:
				if (operand(2).t == TypeInt && operand(3).t == TypeInt && operand(3).v.i == 0) {
					throw Error(L"attempt to divide an integer by zero");
				} else {
					operand(1) = NumericOp<OpDivide>(operand(2), operand(3));
				}
				break;
			case Instruction::MOD:
				if (operand(2).t == TypeInt && operand(3).t == TypeInt && operand(3).v.i == 0) {
					throw Error(L"attempt to divide an integer by zero");
				} else {
					operand(1) = IntegerOp<OpModular>(operand(2), operand(3));
				}
				break;
			case Instruction::UNM:    operand(1) = NumericOp<OpNeg>(operand(2), true);

			// Bitwise operation instructions
			case Instruction::BITNOT: operand(1) = IntegerOp<OpBitNot>(operand(2)); break; 
			case Instruction::BITAND: operand(1) = IntegerOp<OpBitAnd>(operand(2), operand(3)); break;
			case Instruction::BITOR:  operand(1) = IntegerOp<OpBitOr>(operand(2), operand(3)); break; 
			case Instruction::BITXOR: operand(1) = IntegerOp<OpBitXor>(operand(2), operand(3)); break; 
			case Instruction::SL:     operand(1) = IntegerOp<OpShiftLeft>(operand(2), operand(3)); break;
			case Instruction::SR:     operand(1) = IntegerOp<OpShiftRight>(operand(2), operand(3)); break;
			
			// Logical operation instructions
			case Instruction::NOT:    operand(1) = LogicalOp<OpLogicNot>(operand(2)); break;
			case Instruction::EQ:     operand(1) = CompareOp<OpEqual>(operand(2), operand(3)); break;
			case Instruction::NOTEQ:  operand(1) = CompareOp<OpNotEqual>(operand(2), operand(3)); break;
			case Instruction::LT:     operand(1) = NumericOp<OpLess>(operand(2), operand(3)); break;
			case Instruction::LE:     operand(1) = NumericOp<OpLessEqual>(operand(2), operand(3)); break;
			
			// Call and Jump instructions
			case Instruction::JUMP:  jumpDistance = inst.operand1; break;
			case Instruction::BRANCH: {
				if (toBool(operand(1))) {
					jumpDistance = inst.operand2;
				}
				break;
			}
			 case Instruction::BRANCHNOT: {
				if (!toBool(operand(1))) {
					jumpDistance = inst.operand2;
				}
				break;
			}
			case Instruction::CALL: {
				if (operand(1).t == TypeFunc) {
					functionCall_(&operand(1), inst.operand2, inst.operand3);
					jumpDistance = 0;
				} else if (operand(1).t == TypeCFunc) {
					CfunctionCall_(&operand(1), inst.operand2, inst.operand3);
				} else {
					throw Error(L"wrong attempt to call non-function value");
				}
				break;
			}
			case Instruction::RETURN: {
				if (inst.operand2 == 0) {
					callStack_.pop_back();
				} else {
					functionReturn_(&currentClosure.local(inst.operand1), inst.operand2);
				}
				if (callStack_.empty() == true) { return; }
				break;
			}
			case Instruction::YIELD: {
				throw Error(L"currently coroutine/yield is not supported");
			}
		}
#undef local

		callStack_.back().programCounter += jumpDistance;
	}
}

void Context::functionCall_(Variable argValues[], const uint32_t numArgs, const uint32_t numRets)
{
	Ref<Function> callee(static_cast<Function*>(argValues[0].v.obj));
	Ref<Closure> closure(new Closure(callee->prototype(), callee->upperClosure(), &objectManager_));
	
	uint32_t size = std::min(numArgs, callee->prototype()->numArgs());

	for (uint32_t i = 0; i < size; i++) { 
		closure->local(i) = argValues[i+1];
	}
	for (uint32_t i = numArgs; i < callee->prototype()->numArgs(); i++) {
		closure->local(i) = TypeNull;
	}

	callStack_.push_back(CallInfo_(std::move(callee), std::move(closure), &argValues[0], numRets, 0));
}

void Context::CfunctionCall_(Variable argValues[], const uint32_t numArgs, const uint32_t numRets)
{
	bufferSize_ = numArgs;
	for (uint32_t i = 0; i < numArgs; i++) {
		buffer_[i] = argValues[i+1];
	}
	reentrant_ = true;
	argValues[0].v.func(*this);
	reentrant_ = false;

	uint32_t size = std::min(numRets, bufferSize_);
	for (uint32_t i = 0; i < size; i++) {
		argValues[i] = buffer_[i];
	}
	for (uint32_t i = size; i < numRets; i++) {
		argValues[i] = TypeNull;
	}
	bufferSize_ = 0;
}

void Context::functionReturn_(Variable retValues[], const uint32_t numRets)
{
	assert(callStack_.empty() == false);
	
	uint32_t size = std::min(numRets, callStack_.back().numRets);
	Variable* returnTo = callStack_.back().returnTo;

	for (uint32_t i = 0; i < size; i++) {
		returnTo[i] = retValues[i];
	}
	for (uint32_t i = numRets; i < callStack_.back().numRets; i++) {
		returnTo[i] = TypeNull;
	}

	callStack_.pop_back();
}

const uint32_t Context::stackSize()
{
	return bufferSize_;
}

const Type Context::type(const uint32_t index) const
{
	checkStackRange_(index);

	return buffer_[index].t;
}

void Context::pop(const uint32_t number)
{
	for (uint32_t i = 0; i < number; i++) {
		buffer_[--bufferSize_] = TypeNull;
	}
}

void Context::clear()
{
	for (uint32_t i = 0; i < bufferSize_; i++) {
		buffer_[--bufferSize_] = TypeNull;
	}
}

void Context::pushNull()
{
	checkStackOverflow_();
	buffer_[bufferSize_++] = TypeNull;
}

void Context::pushInt(const int32_t value)
{
	checkStackOverflow_();
	buffer_[bufferSize_++] = value;
}

const uint32_t Context::getInt(const uint32_t index) const
{
	checkStack_(index, TypeInt, L"integer");

	return buffer_[index].v.i;
}

void Context::pushFloat(const float value)
{
	checkStackOverflow_();
	buffer_[bufferSize_++] = value;
}

const float Context::getFloat(const uint32_t index) const
{
	checkStack_(index, TypeFloat, L"float");
	
	return buffer_[index].v.f;
}

void Context::pushString(const wchar_t value[])
{
	checkStackOverflow_();
	String* newString = new String(value, &objectManager_);

	buffer_[bufferSize_++] = Variable(TypeString, newString);
}

const wchar_t* Context::getString(const uint32_t index) const
{
	checkStack_(index, TypeString, L"string");

	String &str = static_cast<String&>(*buffer_[index].v.obj);
	return str.value().c_str();
}

void Context::pushNewTable()
{
	checkStackOverflow_();
	Table* newTable = new Table(&objectManager_);

	buffer_[bufferSize_++] = Variable(TypeTable, newTable);
}

void Context::pushTableValue(const uint32_t tablePos)
{
	checkStack_(tablePos, TypeTable, L"table");

	Table &table = static_cast<Table&>(*buffer_[tablePos].v.obj);
	buffer_.back() = table.getValue(buffer_.back());
}

void Context::setTableValue(const uint32_t tablePos)
{
	checkStack_(tablePos, TypeTable, L"table");

	Table &table = static_cast<Table&>(*buffer_[tablePos].v.obj);
	Variable &value = buffer_[--bufferSize_];
	Variable &key = buffer_[--bufferSize_];

	table.setValue(key, value);
}

const uint32_t Context::tableSize(const uint32_t tablePos) const
{
	checkStack_(tablePos, TypeTable, L"table");

	Table &table = static_cast<Table&>(*buffer_[tablePos].v.obj);
	return table.size();
}

void Context::pushNewArray()
{
	checkStackOverflow_();
	Array* newArray = new Array(&objectManager_);

	buffer_[bufferSize_++] = Variable(TypeTable, newArray);
}

void Context::pushArrayValue(const uint32_t arrayPos, const uint32_t arrayIndex)
{
	checkStack_(arrayPos, TypeArray, L"array");

	Array &array = static_cast<Array&>(*buffer_[arrayPos].v.obj);
	buffer_[bufferSize_++] = array.getValue(arrayIndex);
}

void Context::setArrayValue(const uint32_t arrayPos, const uint32_t arrayIndex)
{
	checkStack_(arrayPos, TypeArray, L"array");

	Array &array = static_cast<Array&>(*buffer_[arrayPos].v.obj);
	array.setValue(arrayIndex, buffer_[--bufferSize_]);
}

const uint32_t Context::arraySize(const uint32_t arrayPos) const
{
	checkStack_(arrayPos, TypeArray, L"array");

	Array &array = static_cast<Array&>(*buffer_[arrayPos].v.obj);
	return array.size();
}

void Context::setGlobal(const uint32_t index, const wchar_t globalName[])
{
	checkStackRange_(index);

	String *newString = new String(globalName, &objectManager_);
	global_->setValue(Variable(TypeString, newString), buffer_[index]);
}

void Context::getGlobal(const wchar_t globalName[])
{
	String *newString = new String(globalName, &objectManager_);
	buffer_[bufferSize_++] = global_->getValue(Variable(TypeString, newString));
}

void Context::checkStack_(const uint32_t index, const Type type, const wchar_t typeName[]) const
{
	checkStackRange_(index);

	if (buffer_[index].t != type) {
		throw Error(L"Communication stack index [%d] does not contains %s value.", index, typeName);
	}
}

void Context::checkStackRange_(const uint32_t index) const
{
	if (index >= bufferSize_) {
		throw Error(L"Communication stack index [%d] is out of range.", index);
	}
}

void Context::checkStackOverflow_() const
{
	if (bufferSize_ > 99) {
		// TODO: this is temporary implementatation - maximum stack size should be dynamically changable in near future
		throw Error(L"Communication stack overflow.");
	}
}

} // The end of namespace "cmm"