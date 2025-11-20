/*
 * Copyright 2010-2025 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_RINGBUFFER_H_HEADER_GUARD
#define BX_RINGBUFFER_H_HEADER_GUARD

#include "bx.h"
#include "cpu.h"
#include "uint32_t.h"

namespace bx
{
	/// Ring buffer control structure. Tracking "read", "write", and "current" head.
	///
	/// This is not container, and data control represents is user defined. Read/write/current are
	/// just indices.
	///
	/// @notice One slot is always reseved. When creating ring buffer of N slots, N-1 slots can be
	///   used.
	///
	class RingBufferControl
	{
		BX_CLASS(RingBufferControl
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		/// Constructor.
		///
		/// @param[in] _size Maximum number of slots.
		///
		RingBufferControl(uint32_t _size);

		/// Destructor.
		~RingBufferControl();

		/// Returns number of used slots.
		///
		/// @returns Number of used slots.
		///
		uint32_t available() const;

		/// Returns 'true' if ring buffer is empty.
		///
		/// @returns Returns 'true' if ring buffer is empty.
		///
		bool isEmpty() const;

		/// Returns total size of ring buffer.
		///
		/// @returns Total size of ring buffer.
		///
		uint32_t getSize() const;

		/// Returns number of empty slots.
		///
		/// @returns Number of empty slots.
		///
		uint32_t getNumEmpty() const;

		/// Returns number of used slots.
		///
		/// @returns Number of used slots.
		///
		uint32_t getNumUsed() const;

		/// Returns number of reserved slots.
		///
		/// @returns Number of reserved slots.
		///
		uint32_t getNumReserved() const;

		/// Resize ring buffer. Resize happens at write head, read and current head will be moved
		/// forward or backward if write head is behind them.
		///
		/// @param[in] _size Amount to resize. Value can be positive when growing size or negative
		/// when shrinking size of buffer.
		///
		void resize(int32_t _size);

		/// Consume slots, makes slots free to be reserved. Moves "read" head forward.
		///
		/// @returns Number of reserved slots reserved.
		///
		uint32_t consume(uint32_t _size); // consumer only

		/// Reserve slots, makes slots non-free, but ready to be used yet. Moves "write" head forward.
		///
		/// @param[in] _size Number of slots.
		/// @param[in] _mustSucceed If argument is true it will not reseve any slots unless `_size`
		///   of slots is reseved.
		///
		/// @returns Number of reserved slots reserved.
		///
		uint32_t reserve(uint32_t _size, bool _mustSucceed = false); // producer only

		/// Commit slots, makes slots used, and ready to be consumed. Moves "current" head forward.
		///
		/// @param[in] _size Number of commited slots.
		///
		uint32_t commit(uint32_t _size); // producer only

		/// Calculate distance between two slots. Function takes wrapping into account.
		///
		/// @param[in] _from From.
		/// @param[in] _to To.
		///
		/// @returns Distance between slots.
		///
		uint32_t distance(uint32_t _from, uint32_t _to) const; // both

		/// Invalidate ring buffer.
		///
		void reset();

		uint32_t m_size;    //!< Size of ring buffer.
		uint32_t m_current; //!< Currently operated area start.
		uint32_t m_write;   //!< Write head.
		uint32_t m_read;    //!< Read head.
	};

	///
	class SpScRingBufferControl
	{
		BX_CLASS(SpScRingBufferControl
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		SpScRingBufferControl(uint32_t _size);

		///
		~SpScRingBufferControl();

		///
		uint32_t available() const;

		///
		uint32_t consume(uint32_t _size); // consumer only

		///
		uint32_t reserve(uint32_t _size); // producer only

		///
		uint32_t commit(uint32_t _size); // producer only

		///
		uint32_t distance(uint32_t _from, uint32_t _to) const; // both

		///
		void reset();

		const uint32_t m_size;
		uint32_t m_current;
		uint32_t m_write;
		uint32_t m_read;
	};

	///
	template <typename ControlT>
	class ReadRingBufferT
	{
		BX_CLASS(ReadRingBufferT
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		ReadRingBufferT(ControlT& _control, const char* _buffer, uint32_t _size);

		///
		~ReadRingBufferT();

		///
		void end();

		///
		void read(char* _data, uint32_t _len);

		///
		void skip(uint32_t _len);

	private:
		template <typename Ty>
		friend class WriteRingBufferT;

		ControlT& m_control;
		uint32_t m_read;
		uint32_t m_end;
		const uint32_t m_size;
		const char* m_buffer;
	};

	///
	typedef ReadRingBufferT<RingBufferControl> ReadRingBuffer;

	///
	typedef ReadRingBufferT<SpScRingBufferControl> SpScReadRingBuffer;

	///
	template <typename ControlT>
	class WriteRingBufferT
	{
		BX_CLASS(WriteRingBufferT
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		WriteRingBufferT(ControlT& _control, char* _buffer, uint32_t _size);

		///
		~WriteRingBufferT();

		///
		void end();

		///
		void write(const char* _data, uint32_t _len);

		///
		void write(ReadRingBufferT<ControlT>& _read, uint32_t _len);

		///
		void skip(uint32_t _len);

	private:
		ControlT& m_control;
		uint32_t m_write;
		uint32_t m_end;
		const uint32_t m_size;
		char* m_buffer;
	};

	///
	typedef WriteRingBufferT<RingBufferControl> WriteRingBuffer;

	///
	typedef WriteRingBufferT<SpScRingBufferControl> SpScWriteRingBuffer;

} // namespace bx

#include "inline/ringbuffer.inl"

#endif // BX_RINGBUFFER_H_HEADER_GUARD
