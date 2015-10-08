#include "StdAfx.h"
#include "Memory.h"

#include <cstddef>

#include "DataType.h"

namespace cmm
{

std::size_t Variable::Hash::operator() (const Variable& arg) const {
	switch (arg.t) {
		case TypeString: {
			String *s = static_cast<String*>(arg.v.obj);

			return s->hashCode();
		}
		default:
			// TODO: Implement hash function
			return arg.v.i;
	}
}

bool Variable::StrictEqual::operator() (const Variable& rhs, const Variable& lhs) const {
	if (rhs.t != lhs.t) {
		return false;
	}

	switch (rhs.t) {
		case TypeNull:   return true;
		case TypeInt:    return rhs.v.i == lhs.v.i;
		case TypeFloat:  return rhs.v.f == lhs.v.f;
		case TypeString: {
			String& rhsStr = static_cast<String&>(*rhs.v.obj);
			String& lhsStr = static_cast<String&>(*lhs.v.obj);
			return rhsStr == lhsStr;
		}
		default:         return rhs.v.obj == lhs.v.obj;
	}
}

} // The end of namespace "cmm"