#pragma once
#include <stdint.h>

template <typename T> struct LL_NODE
{
	T nodeEntry;
	LL_NODE<T> *next;
	LL_NODE<T> *prev;
};

template <typename T> class LINKEDLIST
{
	public:
		LL_NODE<T> *head;
		LL_NODE<T> *tail;
		
	public:
		LINKEDLIST();
		~LINKEDLIST();
		
		inline LL_NODE<T> *link_front(T push);	//Link to head
		inline LL_NODE<T> *link_back(T push);	//Link to tail
		
		inline LL_NODE<T> *nodeAt(size_t index);
		inline T at(size_t index);
		inline size_t size();
		
		inline void erase(size_t index);
		inline void erase(size_t from, size_t to);
		inline void eraseNode(LL_NODE<T> *node);
		inline void clear();
		
		LINKEDLIST& operator[](size_t index) { return at(index); };
};

#define CLEAR_INSTANCE_LINKEDLIST(linkedList)	for (size_t i = 0; i < linkedList.size(); i++)	\
													delete linkedList[i];	\
												linkedList.clear();
