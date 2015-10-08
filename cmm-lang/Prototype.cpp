#include "StdAfx.h"
#include "Prototype.h"


namespace cmm
{

Prototype::~Prototype()
{
}


void Prototype::forEachObject_(const std::function<void(const Object&)>& func)
{
	std::for_each(localPrototypes_.begin(), localPrototypes_.end(),
		[&func](decltype(*localPrototypes_.begin()) i) { func(*i); } );

	std::for_each(constants_.begin(), constants_.end(),
		[&func](decltype(*constants_.begin()) i) {
			if (i.isObject()) {
				func(*i.v.obj);
			}
		}
	);
}

} // The end of namespace "cmm"