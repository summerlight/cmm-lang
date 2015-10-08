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


String::String(ObjectManager* manager)
: Object(manager), value_(), hashCode_(calculateHashCode())
{
}

String::String(const wchar_t value[], ObjectManager* manager)
: Object(manager), value_(value), hashCode_(calculateHashCode())
{
}

String::String(const std::wstring& value, ObjectManager* manager)
: Object(manager), value_(value), hashCode_(calculateHashCode())
{
}

String::~String()
{
}

const uint32_t String::hashCode() const
{
	return hashCode_;
}

const std::wstring& String::value() const
{
	return value_;
}

const uint32_t String::calculateHashCode()
{
	uint32_t hashCode = 0;
	uint32_t hashLength = (value_.length() > 8) ? 8 : value_.length();

	for (uint32_t i = 0; i < hashLength; i++) {   
		hashCode = 31*hashCode + value_[i];   
	}

	return hashCode;
}

void String::forEachObject_(const std::function<void(const Object&)>& func)
{
	func; // By intention - a string object does not refer any other object
}



Array::Array(ObjectManager* manager)
: Object(manager)
{
	array_.reserve(16); // TODO: This number is subject to change
}

Array::~Array()
{
	// Unref: if a value is an object	
}

const Variable Array::getValue(const int32_t key) const
{
	if (key < 0 || static_cast<uint32_t>(key) >= array_.size()) {
		return Variable(TypeNull);
	}

	return array_[key];
}

const bool Array::setValue(const int32_t key, const Variable& value)
{
	// Ref: if a value is an object
	if (key < 0) { return false; }

	if (static_cast<uint32_t>(key) >= array_.size()) {
		array_.resize(key+1, Variable(TypeNull));
	}

	array_[key] = value;
	return true;
}

const uint32_t Array::size()
{
	return array_.size();
}

void Array::forEachObject_(const std::function<void(const Object&)>& func)
{
	std::for_each(array_.begin(), array_.end(), 
		[&func](decltype(*array_.begin()) i) { if (i.isObject()) { func(*i.v.obj); } }
	);
}




Table::Table(ObjectManager* manager)
: Object(manager)
{
	table_.bucket_size(17); // TODO: This number is subject to change
}

Table::~Table()
{
	// Unref : if a key or a value is an object
}

const Variable Table::getValue(const Variable& key) const
{
	auto iter = table_.find(key);

	if (iter != table_.end()) {
		return iter->second;
	} else {
		return Variable(TypeNull);
	}
}

void Table::setValue(const Variable& key, const Variable& value)
{
	// Ref: if a key or a value is an object
	auto iter = table_.find(key);

	if (iter != table_.end()) {
		if (key.t != TypeNull) {
			iter->second = value;
		} else {
			table_.erase(iter);
		}
	} else {
		table_.insert(std::make_pair(key, value));
	}
}

const uint32_t Table::size()
{
	return table_.size();
}

void Table::forEachObject_(const std::function<void(const Object&)>& func)
{
	std::for_each(table_.begin(), table_.end(), 
		[&func] (decltype(*table_.begin()) i) {
			if (i.first.isObject()) {
				Object& key = *i.first.v.obj;
				func(key);
			}

			if (i.second.isObject()) {
				Object& value = *i.second.v.obj;
				func(value);
			}
		}
	);
}

} // The end of namespace "cmm"