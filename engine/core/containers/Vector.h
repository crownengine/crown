/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

namespace crown
{

/// Dynamic array of objects.
/// @note
/// Calls constructors and destructors, not suitable for performance-critical stuff.
/// If your data is POD, use List<T> instead.
template <typename T>
class Vector
{
public:

	/// Does not allocate memory.
						Vector(Allocator& allocator);

	/// Allocates capacity * sizeof(T) bytes.
						Vector(Allocator& allocator, uint32_t capacity);
						Vector(const Vector<T>& other);
						~Vector();

	/// Random access by index
	T&					operator[](uint32_t index);

	/// Random access by index
	const T&			operator[](uint32_t index) const;

	/// Returns whether the vector is empty
	bool				empty() const;

	/// Returns the number of items in the vector
	uint32_t			size() const;

	/// Returns the maximum number of items the array can hold
	uint32_t			capacity() const;

	/// Resizes the vector to the given @a size.
	/// @note
	/// Old items will be copied to the newly created vector.
	/// If the new capacity is smaller than the previous one, the
	/// vector will be truncated.
	void				resize(uint32_t size);

	/// Reserves space in the vector for at least @a capacity items.
	void				reserve(uint32_t capacity);

	/// Sets the vector capacity
	void				set_capacity(uint32_t capacity);

	/// Grows the vector to contain at least @a min_capacity items
	void				grow(uint32_t min_capacity);

	/// Condenses the array so that the capacity matches the actual number
	/// of items in the vector.
	void				condense();

	/// Appends an item to the vector and returns its index.
	uint32_t			push_back(const T& item);

	/// Removes the last item from the vector.
	void				pop_back();

	/// Appends @a count @a items to the vector and returns the number
	/// of items in the vector after the append operation.
	uint32_t			push(const T* items, uint32_t count);

	/// Clears the content of the vector.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the vector for efficiency.
	void				clear();

	/// Copies the content of the @a other vector into this one.
	const Vector<T>&		operator=(const Vector<T>& other);

	T*					begin();
	const T*			begin() const;
	T*					end();
	const T*			end() const;

	T&					front();
	const T&			front() const;
	T&					back();
	const T&			back() const;

private:

	Allocator*			m_allocator;
	uint32_t			m_capacity;
	uint32_t			m_size;
	T*					m_array;
};

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(Allocator& allocator)
	: m_allocator(allocator), m_capacity(0), m_size(0), m_array(NULL)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(Allocator& allocator, uint32_t capacity)
	: m_allocator(allocator), m_capacity(0), m_size(0), m_array(NULL)
{
	resize(capacity);
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(const Vector<T>& other)
	: m_allocator(other.m_allocator), m_capacity(0), m_size(0), m_array(NULL)
{
	*this = other;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::~Vector()
{
	if (m_array)
	{
		for (uint32_t i = 0; i < m_size; i++)
		{
			m_array[i].~T();
		}
		m_allocator->deallocate(m_array);
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Vector<T>::operator[](uint32_t index)
{
	CE_ASSERT(index < m_size, "Index out of bounds");

	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Vector<T>::operator[](uint32_t index) const
{
	CE_ASSERT(index < m_size, "Index out of bounds");

	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Vector<T>::empty() const
{
	return m_size == 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Vector<T>::size() const
{
	return m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Vector<T>::capacity() const
{
	return m_capacity;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::resize(uint32_t size)
{
	if (size > m_capacity)
	{
		set_capacity(size);
	}

	m_size = size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::reserve(uint32_t capacity)
{
	if (capacity > m_capacity)
	{
		grow(capacity);
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::set_capacity(uint32_t capacity)
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

		m_array = (T*)m_allocator->allocate(capacity * sizeof(T));

		for (uint32_t i = 0; i < m_size; i++)
		{
			new (m_array + i) T(tmp[i]);
		}

		if (tmp)
		{
			for (uint32_t i = 0; i < m_size; i++)
			{
				tmp[i].~T();
			}
			m_allocator->deallocate(tmp);
		}
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::grow(uint32_t min_capacity)
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
inline void Vector<T>::condense()
{
	resize(m_size);
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Vector<T>::push_back(const T& item)
{
	if (m_capacity == m_size)
	{
		grow(0);
	}

	new (m_array + m_size) T(item);

	return m_size++;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::pop_back()
{
	CE_ASSERT(m_size > 0, "The vector is empty");

	m_size--;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Vector<T>::push(const T* items, uint32_t count)
{
	if (m_capacity <= m_size + count)
	{
		grow(m_size + count);
	}

	T* arr = &m_array[m_size];
	for (uint32_t i = 0; i < count; i++)
	{
		arr[i] = items[i];
	}

	m_size += count;

	return m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Vector<T>::clear()
{
	for (uint32_t i = 0; i < m_size; i++)
	{
		m_array[i].~T();
	}

	m_size = 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	if (m_array)
	{
		m_allocator->deallocate(m_array);
	}

	m_size = other.m_size;
	m_capacity = other.m_capacity;

	if (m_capacity)
	{
		m_array = (T*)m_allocator->allocate(m_capacity * sizeof(T));

		for (uint32_t i = 0; i < m_size; i++)
		{
			m_array[i] = other.m_array[i];
		}
	}

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Vector<T>::begin() const
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Vector<T>::begin()
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Vector<T>::end() const
{
	return m_array + m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Vector<T>::end()
{
	return m_array + m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Vector<T>::front()
{
	CE_ASSERT(m_size > 0, "The vector is empty");

	return m_array[0];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Vector<T>::front() const
{
	CE_ASSERT(m_size > 0, "The vector is empty");

	return m_array[0];
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Vector<T>::back()
{
	CE_ASSERT(m_size > 0, "The vector is empty");

	return m_array[m_size - 1];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Vector<T>::back() const
{
	CE_ASSERT(m_size > 0, "The vector is empty");

	return m_array[m_size - 1];
}

} // namespace crown

