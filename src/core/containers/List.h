/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Allocator.h"
#include "Types.h"
#include "Assert.h"
#include <cstring>

namespace crown
{

/// Dynamic array of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
template <typename T>
class List
{
public:

						List(Allocator& allocator);

	/// Allocates capacity * sizeof(T) bytes.
						List(Allocator& allocator, uint32_t capacity);
						List(const List<T>& list);
						~List();

	/// Random access by index
	T&					operator[](uint32_t index);

	/// Random access by index
	const T&			operator[](uint32_t index) const;

	/// Returns whether the list is empty
	bool				empty() const;

	/// Returns the number of items in the list
	uint32_t			size() const;

	/// Returns the maximum number of items the array can hold
	uint32_t			capacity() const;

	/// Resizes the list to the given @size.
	/// @note
	/// Old items will be copied to the newly created list.
	/// If the new capacity is smaller than the previous one, the
	/// list will be truncated.
	void				resize(uint32_t size);

	/// Reserves space in the list for at least @capacity items.
	void				reserve(uint32_t capacity);

	/// Sets the list capacity
	void				set_capacity(uint32_t capacity);

	/// Grows the list to contain at least @min_capacity items
	void				grow(uint32_t min_capacity);

	/// Condenses the array so that the capacity matches the actual number
	/// of items in the list.
	void				condense();

	/// Appends an item to the list and returns its index.
	uint32_t			push_back(const T& item);

	/// Removes the last item from the list.
	void				pop_back();

	/// Appends @count @items to the list and returns the number
	/// of items in the list after the append operation.
	uint32_t			push(const T* items, uint32_t count);

	/// Clears the content of the list.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the list for efficiency.
	void				clear();

	/// Copies the content of the @other list into this one.
	const List<T>&		operator=(const List<T>& other);

	T*					begin();
	const T*			begin() const;
	T*					end();
	const T*			end() const;

	T&					front();
	const T&			front() const;
	T&					back();
	const T&			back() const;

private:

	Allocator&			m_allocator;
	uint32_t			m_capacity;
	uint32_t			m_size;
	T*					m_array;
};

//-----------------------------------------------------------------------------
template <typename T>
inline List<T>::List(Allocator& allocator) :
	m_allocator(allocator),
	m_capacity(0),
	m_size(0),
	m_array(NULL)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline List<T>::List(Allocator& allocator, uint32_t capacity) :
	m_allocator(allocator),
	m_capacity(0),
	m_size(0),
	m_array(NULL)
{
	resize(capacity);
}

//-----------------------------------------------------------------------------
template <typename T>
inline List<T>::List(const List<T>& list) :
	m_capacity(0),
	m_size(0),
	m_array(NULL)
{
	*this = list;
}

//-----------------------------------------------------------------------------
template <typename T>
inline List<T>::~List()
{
	if (m_array)
	{
		m_allocator.deallocate(m_array);
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& List<T>::operator[](uint32_t index)
{
	//ce_assert(index < m_size);

	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& List<T>::operator[](uint32_t index) const
{
	//ce_assert(index < m_size);

	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool List<T>::empty() const
{
	return m_size == 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t List<T>::size() const
{
	return m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t List<T>::capacity() const
{
	return m_capacity;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::resize(uint32_t size)
{
	if (size > m_capacity)
	{
		set_capacity(size);
	}

	m_size = size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::reserve(uint32_t capacity)
{
	if (capacity > m_capacity)
	{
		grow(capacity);
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::set_capacity(uint32_t capacity)
{
	if (capacity == m_capacity)
	{
		return;
	}

	if (capacity < m_size)
	{
		resize(capacity);
	}

	if (capacity > 0)
	{
		T* tmp = m_array;
		m_capacity = capacity;

		m_array = (T*)m_allocator.allocate(capacity * sizeof(T));

		memcpy(m_array, tmp, m_size * sizeof(T));

		if (tmp)
		{
			m_allocator.deallocate(tmp);
		}
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::grow(uint32_t min_capacity)
{
	uint32_t new_capacity = m_capacity * 2 + 1;

	if (new_capacity < min_capacity)
	{
		new_capacity = min_capacity;
	}

	set_capacity(new_capacity);
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::condense()
{
	resize(m_size);
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t List<T>::push_back(const T& item)
{
	if (m_capacity == m_size)
	{
		grow(0);
	}

	m_array[m_size] = item;

	return 	m_size++;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::pop_back()
{
	ce_assert(m_size > 0, "The list is empty");

	m_size--;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t List<T>::push(const T* items, uint32_t count)
{
	if (m_capacity <= m_size + count)
	{
		grow(m_size + count);
	}

	memcpy(&m_array[m_size], items, sizeof(T) * count);
	m_size += count;

	return m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void List<T>::clear()
{
	m_size = 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const List<T>& List<T>::operator=(const List<T>& other)
{
	if (m_array)
	{
		m_allocator.deallocate(m_array);
	}

	m_size = other.m_size;
	m_capacity = other.m_capacity;

	if (m_capacity)
	{
		m_array = (T*)m_allocator.allocate(m_capacity * sizeof(T));

		memcpy(m_array, other.m_array, m_size * sizeof(T));
	}

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::begin() const
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::begin()
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::end() const
{
	return m_array + m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::end()
{
	return m_array + m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& List<T>::front()
{
	ce_assert(m_size > 0, "The list is empty");

	return m_array[0];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& List<T>::front() const
{
	ce_assert(m_size > 0, "The list is empty");

	return m_array[0];
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& List<T>::back()
{
	ce_assert(m_size > 0, "The list is empty");

	return m_array[m_size - 1];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& List<T>::back() const
{
	ce_assert(m_size > 0, "The list is empty");

	return m_array[m_size - 1];
}

} // namespace crown

