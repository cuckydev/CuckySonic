#pragma once
#include <stdint.h>
#include <stdio.h>

template <typename T> struct LL_NODE
{
	T node_entry;
	LL_NODE<T> *next;
	LL_NODE<T> *prev;
};

template <typename T> class LINKEDLIST
{
	public:
		LL_NODE<T> *head = nullptr;
		LL_NODE<T> *tail = nullptr;
		size_t llSize = 0;
		
	public:
		//Constructor and destructor
		LINKEDLIST() { return; }
		~LINKEDLIST() { clear(); }
		
		//Linking functions
		inline LL_NODE<T> *link_front(T push)
		{
			//Allocate a new node and link to the head
			LL_NODE<T> *newNode = new LL_NODE<T>;
			newNode->node_entry = push;
			newNode->next = head;
			
			//If the tail is null, we must be also at the end of the list, set tail to our new node
			if (tail == nullptr)
				tail = newNode;
			
			//Link us behind the previous head
			if (head != nullptr)
				head->prev = newNode;
			head = newNode;
			
			llSize++;
			return newNode;
		}
		
		inline LL_NODE<T> *link_back(T push)
		{
			//Allocate a new node and link to the tail
			LL_NODE<T> *newNode = new LL_NODE<T>;
			newNode->node_entry = push;
			newNode->prev = tail;
			newNode->next = nullptr;
			
			//If the head is null, we must be also at the beginning of the list, set head to our new node
			if (head == nullptr)
				head = newNode;
			
			//Link us in front of the previous tail
			if (tail != nullptr)
				tail->next = newNode;
			tail = newNode;
			
			llSize++;
			return newNode;
		}
		
		//Position identification
		inline size_t pos_of_node(LL_NODE<T> want)
		{
			size_t i = 0;
			for (LL_NODE<T> *node = head; node != want; node = node->next)
				i++;
			return i;
		}
		
		inline size_t pos_of_val(T want)
		{
			size_t i = 0;
			for (LL_NODE<T> *node = head; node->node_entry != want; node = node->next)
				i++;
			return i;
		}
		
		//Random access and size
		inline LL_NODE<T> *node_at(size_t index)
		{
			//Find the node at the given index (from head)
			size_t i = 0;
			for (LL_NODE<T> *node = head; node != nullptr; node = node->next)
				if (i++ == index)
					return node;
			return nullptr;
		}
		
		inline T at(size_t index)
		{
			//Find the given entry at the given index (from head)
			return node_at(index)->node_entry;
		}

		inline size_t size() { return llSize; }
		
		//Erasers
		inline void erase(size_t index)
		{
			//Find the node at the given index (from head) and destroy it
			erase_node(node_at(index));
		}
		
		inline void erase(size_t from, size_t to)
		{
			//Erase entries from "from" to "to"
			for (size_t i = from; i < to; i++)
				erase_node(node_at(from));
		}
		
		inline void erase_node(LL_NODE<T> *node)
		{
			//Adjust linked list correctly and destroy node
			if (node->prev != nullptr)
				node->prev->next = node->next;
			else
				head = node->next;
			if (node->next != nullptr)
				node->next->prev = node->prev;
			else
				tail = node->prev;
			llSize--;
			delete node;
		}
		
		inline void clear()
		{
			//Destroy every node until empty
			while (head)
				erase_node(head);
			head = nullptr;
			tail = nullptr;
		}
		
		//Access operator
		T operator[](size_t index) { return at(index); };
};

#define CLEAR_INSTANCE_LINKEDLIST(linkedList)	while (linkedList.head)	\
												{	\
													delete linkedList.head->node_entry;	\
													linkedList.erase_node(linkedList.head);	\
												}
