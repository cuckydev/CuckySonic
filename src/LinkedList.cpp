#include "LinkedList.h"

//Constructor and destructor
template <typename T> LINKEDLIST<T>::LINKEDLIST()
{
	//Reset memory
	memset(this, 0, sizeof(LINKEDLIST));
}

template <typename T> LINKEDLIST<T>::~LINKEDLIST()
{
	//Destroy all nodes
	clear();
}

//Linking functions
template <typename T> inline LL_NODE<T> *LINKEDLIST<T>::link_front(T push)
{
	//Allocate a new node and link to the head
	LL_NODE<T> *newNode = new LL_NODE<T>;
	newNode->nodeEntry = push;
	newNode->next = head;
	newNode->prev = nullptr;
	head->prev = newNode;
	head = newNode;
}

template <typename T> inline LL_NODE<T> *LINKEDLIST<T>::link_back(T push)
{
	//Allocate a new node and link to the tail
	LL_NODE<T> *newNode = new LL_NODE<T>;
	newNode->nodeEntry = push;
	newNode->prev = tail;
	newNode->next = nullptr;
	tail->next = newNode;
	tail = newNode;
}

//Random access and size
template <typename T> inline LL_NODE<T> *LINKEDLIST<T>::nodeAt(size_t index)
{
	//Find the node at the given index (from head)
	size_t i = 0;
	for (LL_NODE<T> *node = head; node != nullptr; node = node->next)
		if (i++ == index)
			return node;
	return nullptr;
}

template <typename T> inline T LINKEDLIST<T>::at(size_t index)
{
	//Find the given entry at the given index (from head)
	return nodeAt(index)->nodeEntry;
}

template <typename T> inline size_t LINKEDLIST<T>::size()
{
	//Count all our nodes and return the size
	size_t i = 0;
	for (LL_NODE<T> *node = head; node != nullptr; node = node->next)
		i++;
	return i;
}

//Erasers
template <typename T> inline void LINKEDLIST<T>::erase(size_t index)
{
	//Find the node at the given index (from head) and destroy it
	eraseNode(nodeAt(index));
}

template <typename T> inline void LINKEDLIST<T>::erase(size_t from, size_t to)
{
	//Erase entries from "from" to "to"
	for (size_t i = from; i < to; i++)
		eraseNode(nodeAt(from));
}

template <typename T> inline void LINKEDLIST<T>::eraseNode(LL_NODE<T> *node)
{
	//Adjust linked list correctly and destroy node
	if (node->prev != nullptr)
		node->prev->next = node->next;
	if (node->next != nullptr)
		node->next->prev = node->prev;
	delete node;
}

template <typename T> inline void LINKEDLIST<T>::clear()
{
	//Destroy every node until empty
	while (head != tail)
		eraseNode(head);
}
