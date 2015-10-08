#include "StdAfx.h"
#include "DataType.h"

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <utility>

#include "Memory.h"
#include "Object.h"
#include "Prototype.h"

namespace cmm
{

Function::Function(const Ref<Prototype> prototype, Ref<Closure> upper, ObjectManager* manager)
: Object(manager), prototype_(prototype), upperClosure_(upper)
{
	assert(prototype.get() != nullptr);
}

Function::~Function()
{
}

const Ref<Prototype> Function::prototype() const
{
	return prototype_;
}


void Function::forEachObject_(const std::function<void(const Object&)>& func)
{
	if (prototype_.get() != nullptr) { func(*prototype_); }
	if (upperClosure_.get() != nullptr) { func(*upperClosure_); }
}


Closure::Closure(const Ref<Prototype> prototype, Ref<Closure> upperClosure, ObjectManager* objectManager)
: prototype_(prototype), Object(objectManager), upperClosure_(upperClosure), local_(prototype->localSize(), TypeNull)
{
	assert(prototype.get());
}

Closure::~Closure()
{
}



Variable& Closure::upValue(const uint32_t functionLevel, const uint32_t offset)
{
	Ref<Closure> targetClosure = this;

	// TODO : change this closure routine to more efficient version
	for (uint32_t i = prototype_->functionLevel(); i != functionLevel; i--) {
		assert(targetClosure.get() != nullptr);
		targetClosure = targetClosure->upperClosure();
	}

	return targetClosure->local(offset);
}

void Closure::forEachObject_(const std::function<void(const Object&)>& func)
{
	std::for_each(local_.begin(), local_.end(),
		[&func](decltype(*local_.begin()) i) {
			if (i.isObject()) {
				func(*i.v.obj);
			}
		}
	);

	if (upperClosure_.get() != nullptr) {
		func(*upperClosure_.get());
	}
}



} // The end of namespace "cmm"