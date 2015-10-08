#ifndef OBJECT_H
#define OBJECT_H

#include <cassert>
#include <cstdint>
#include <cassert>
#include <unordered_set>
#include <algorithm>
#include <functional>

#include "Utility.h"

namespace cmm
{

constexpr uint8_t GCFLAG_UNMARKED = 0x00;
constexpr uint8_t GCFLAG_MARKED = 0x01;
constexpr uint8_t GCFLAG_INVALID = 0x02;

struct Variable;

// Base class for garbage collection

// This class currently uses simple mark-and-sweep algorithm, and it should be updated to
// cycle detection algorithm with ref-counting.

// Note : reference count begins from 0.

class Object
{
	friend class ObjectManager;

public:
	void               addRef() const;
	void               release() const;
	uint32_t           refCount() const;

protected:
	explicit           Object(ObjectManager* manager);
	virtual            ~Object();

private:
	                   Object(const Object&);
	const Object&      operator=(const Object&);

	virtual void       forEachObject_(const std::function<void(const Object&)>& func) = 0;

	mutable Node       node_;
	mutable uint32_t   refCount_;
	mutable uint8_t    GCFlag_;
};


class ObjectManager
{
public:
	                     ObjectManager();
	                     ~ObjectManager();

	void                 registerObject(Object* object);
	void                 garbageCollect(Object& rootSet);

private:
		                 ObjectManager(const ObjectManager&);
	const ObjectManager& operator=(const ObjectManager&);

	void                 clearObjects_();
	void                 unmarkAllObjects_();
	static Object*       objectPtr_(Node* node);

	Node                 head_;
};


template<class T>
class Ref
{
public:
	Ref() : ptr_(nullptr) {}

	Ref(T* ptr) : ptr_(ptr)
	{
		addRefOp_();
	}
	
	Ref(const Ref &rhs): ptr_(rhs.ptr_)
	{
		addRefOp_();
	}

	~Ref()
	{
		releaseOp_();
	}

	Ref& operator=(const Ref &rhs)
	{
		Ref<T>(rhs).swap(*this);
		return *this;
	}

	Ref(Ref &&rhs)
	: ptr_(rhs.ptr_)
	{
		rhs.ptr_ = nullptr;
	}

	Ref& operator=(Ref &&rhs)
	{
		Ref<T>(std::move(rhs)).swap(*this);
		return *this;
	}

	void reset()
	{
		Ref<T>().swap(*this);
	}

	void reset(T* rhs)
	{
		Ref<T>(rhs).swap(*this);
	}

	T* get() const
	{
		return ptr_;
	}

	T& operator*() const
	{
		assert(ptr_ != nullptr);
		return *ptr_;
	}

	T* operator->() const
	{
		assert(ptr_ != nullptr);
		return ptr_;
	}

	void swap(Ref &rhs)
	{
		T* temp = ptr_;
		ptr_ = rhs.ptr_;
		rhs.ptr_ = temp;
	}

private:
	void addRefOp_() { if (ptr_ != nullptr) { ptr_->addRef(); } }
	void releaseOp_() { if (ptr_ != nullptr) { ptr_->release(); } }

	T* ptr_;
};

} // namespace"cmm"

#endif