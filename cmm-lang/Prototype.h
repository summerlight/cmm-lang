#ifndef PROGRAM_H
#define PROGRAM_H

#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>

#include "Instruction.h"
#include "DataType.h"
#include "Object.h"

namespace cmm
{

class ObjectManager;

class Prototype : public Object
{
	friend class CodeGenerator;

public:
	uint32_t              functionLevel() const;
	uint32_t              numArgs() const;
	uint32_t              localSize() const;
                         
	uint32_t              numPrototype() const;
	uint32_t              numConstant() const;
	uint32_t              numInstruction() const;
                         
	Ref<Prototype>        localPrototype(const uint32_t index) const;
	const Variable&       constant(const uint32_t index) const;
	const Instruction&    instruction(const uint32_t offset) const;
	
private:
	virtual void          forEachObject_(const std::function<void(const Object&)>& func);

	explicit              Prototype(ObjectManager* objectManager = nullptr);
	                      ~Prototype();

	                      Prototype(const Prototype&) = delete;
	const Prototype&      operator=(const Prototype&) = delete;

	typedef std::vector<Ref<Prototype>> PrototypeVector_;
	typedef std::vector<Variable> VariableVector_;
	typedef std::vector<Instruction> InstructionVector_;

	PrototypeVector_      localPrototypes_;
	VariableVector_       constants_;
	InstructionVector_    code_;

	uint32_t              localSize_;
	uint32_t              functionLevel_;
	uint32_t              numArgs_;
};

inline Prototype::Prototype(ObjectManager* objectManager)
: Object(objectManager)
{
}


inline uint32_t Prototype::functionLevel() const
{
	return functionLevel_;
}

inline uint32_t Prototype::numArgs() const
{
	return numArgs_;
}

inline uint32_t Prototype::localSize() const
{
	return localSize_;
}

inline uint32_t Prototype::numPrototype() const
{
	return localPrototypes_.size();
}

inline uint32_t Prototype::numConstant() const
{
	return constants_.size();
}

inline uint32_t Prototype::numInstruction() const
{
	return code_.size();
}

inline Ref<Prototype> Prototype::localPrototype(const uint32_t index) const
{
	return localPrototypes_[index];
}

inline const Variable& Prototype::constant(const uint32_t index) const
{
	return constants_[index];
}

inline const Instruction& Prototype::instruction(const uint32_t offset) const
{
	assert(offset >= 0 && offset < code_.size());
	return code_[offset];
}

} // namespace "cmm"

#endif