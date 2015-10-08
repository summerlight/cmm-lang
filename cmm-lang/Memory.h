#ifndef MEMORY_H
#define MEMORY_H

#include <cassert>
#include <cstdint>

#include "Object.h"

namespace cmm
{

class Context;
typedef void(*CFunction)(Context&);

enum Type
{
	TypeInt,
	TypeFloat,
	// Above is number
	// CHECK : TypeNull and Below are also used as border line between primitive number variable and object internally
	TypeNull,	
	TypeCFunc,
	// Below is objects
	TypeString,
	TypeArray,
	TypeTable,
	TypeFunc,
	TypeEnd
};

union Value
{
	int32_t    i;
	float      f;
	Object*    obj;
	CFunction  func;

	Value() {}
	Value(const int32_t i_num) : i(i_num) {}
	Value(const float f_num)   : f(f_num) {}
	Value(Object* object)      : obj(object) {}
	Value(CFunction function)  : func(function) {}
};

// Struct "Variable" represents basic data which consists of a type info and an actual data in cmm,
// and provides simple utility functions each simplifies reference counting process.
// Since both copy constructor and assign operator is overloaded for reference counting purpose, 
// programmer using this structure may not concerns about reference counting.
// However, if a programmer wants to manage ref-counting by hand then each member variables
// still can be manipulated without automated ref-counting process.

struct Variable
{
	Type   t;
	Value  v;

	struct Hash : public std::unary_function<Variable, std::size_t>
	{
		std::size_t operator() (const Variable& arg) const;
	};

	struct StrictEqual : public std::binary_function<Variable, Variable, bool>
	{
		bool operator() (const Variable& rhs, const Variable& lhs) const;
	};

	Variable() {}

	// Below constructors are intended to be constructed by implicit type conversion
	Variable(const Type type)              : t(type) {}  // Responsibility of type checking is up to user.
	Variable(const Type type, Object* ptr) : t(type), v(ptr) { v.obj->addRef(); }
	Variable(const bool boolean)           : t(TypeInt), v(boolean) {} // Boolean value is represented by integer form (0, 1) in C--
	Variable(const int32_t i_num)          : t(TypeInt), v(i_num) {}
	Variable(const float f_num)            : t(TypeFloat), v(f_num) {}
	Variable(CFunction func)               : t(TypeCFunc), v(func) {}

	Variable(const Variable& rhs)          : t(rhs.t), v(rhs.v) { objectAddRef(); }
	Variable(Variable&& rhs)               : t(rhs.t), v(rhs.v) { rhs.t = TypeNull; }

	~Variable() { objectRelease(); }

	const Variable&   operator=(const Variable& rhs);
	const Variable&   operator=(Variable&& rhs);

	const bool        isObject() const;
	const bool        isNumber() const;
	const bool        isNull() const;
	void              objectAddRef() const;
	void              objectRelease() const;

	const bool operator==(const Variable& rhs) const;
};

inline const Variable& Variable::operator=(const Variable& rhs) {
	objectRelease();
	t = rhs.t; v = rhs.v;
	objectAddRef();
	
	return *this;
}

inline const Variable& Variable::operator=(Variable&& rhs) {
	t = rhs.t; v = rhs.v;
	rhs.t = TypeNull;
	
	return *this;
}

inline const bool Variable::isObject() const {
	assert(t >= 0 && t < TypeEnd);
	return (t > TypeCFunc);
	// CHECK : TypeNull is also used as border line between primitive number variable and object internally
}

inline const bool Variable::isNumber() const {
	assert(t >= 0 && t < TypeEnd);
	return (t < TypeNull);
	// CHECK : TypeNull is also used as border line between primitive number variable and object internally
}

inline const bool Variable::isNull() const {
	assert(t >= 0 && t < TypeEnd);
	return (t == TypeNull);
}

inline void Variable::objectAddRef() const {
	if (isObject()) {
		v.obj->addRef();
	}
}

inline void Variable::objectRelease() const {
	if (isObject()) {
		v.obj->release();
	}
}

} // The end of namespace "cmm"
#endif