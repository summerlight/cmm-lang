#include "StdAfx.h"
#include "Object.h"

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include "Memory.h"
#include "Utility.h"

namespace cmm
{

Object::Object(ObjectManager* manager)
: refCount_(0), GCFlag_(GCFLAG_UNMARKED)
{
	if (manager != nullptr) {
		manager->registerObject(this);
	}
}

Object::~Object()
{
}

void Object::release() const
{
	assert(refCount_ > 0);

	refCount_--;
	
	if (refCount_ == 0) {
		node_.pickOut();
		
		if (!(GCFlag_ & GCFLAG_INVALID)) {
			delete this;
		}
	}
}

void Object::addRef() const
{
	assert(refCount_ >= 0);

	refCount_++;
}

const uint32_t Object::refCount() const
{
	return refCount_;
}



ObjectManager::ObjectManager()
{
	head_.listHeadInit();
}

ObjectManager::~ObjectManager()
{
	clearObjects_();
}


void ObjectManager::registerObject(Object* object)
{
	head_.insertBack(&object->node_);
}

void ObjectManager::clearObjects_()
{
	while (head_.next != &head_) {
		Object& obj = *objectPtr_(head_.next);
		
		obj.GCFlag_ |= GCFLAG_INVALID;
		obj.refCount_ = 1; // if the reference count is not 1, then the object will not be freed properly
		obj.release();
	}

	head_.listHeadInit();
}

void ObjectManager::garbageCollect(Object& rootSet)
{
	unmarkAllObjects_();

	Node workingSet;
	Node markedSet;

	workingSet.listHeadInit();
	markedSet.listHeadInit();

	std::function<void(const Object&)> marking = [&workingSet](const Object& object) {
		if (!(object.GCFlag_ & GCFLAG_MARKED)) {
			object.GCFlag_ = GCFLAG_MARKED;
			object.node_.pickOut();
			workingSet.insertFront(&object.node_);
		}
	};
	
	rootSet.node_.pickOut();
	workingSet.insertBack(&rootSet.node_);

	while (workingSet.next != &workingSet) {
		Node* node = workingSet.next;
		Object& obj = *objectPtr_(node);

		obj.forEachObject_(marking);

		node->pickOut();
		markedSet.insertFront(node);
	}

	clearObjects_();

	Node *headNode = markedSet.next;
	markedSet.pickOut();
	headNode->insertFront(&head_);
}


void ObjectManager::unmarkAllObjects_()
{
	Node *iter = head_.next;
	
	while (iter != &head_) {
		Object& obj = *objectPtr_(iter);

		obj.GCFlag_ = GCFLAG_UNMARKED;
		iter = iter->next;
	}
}


inline Object* ObjectManager::objectPtr_(Node* node)
{
	// WARNING : Below routine is verified only on win32/x86 platform.
	return reinterpret_cast<Object*>(reinterpret_cast<int8_t*>(node) - offsetof(Object, node_));
}


} // The end of namespace "cmm"