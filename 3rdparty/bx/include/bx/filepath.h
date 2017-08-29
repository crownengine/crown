/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FILEPATH_H_HEADER_GUARD
#define BX_FILEPATH_H_HEADER_GUARD

#include "string.h"

namespace bx
{
	const int32_t kMaxFilePath = 1024;
	struct TempDir { enum Enum { Tag }; };

	/// FilePath parser and helper.
	///
	/// /abv/gd/555/333/pod.mac
	/// ppppppppppppppppbbbeeee
	/// ^               ^  ^
	/// +-path     base-+  +-ext
	///                 ^^^^^^^
	///                 +-filename
	///
	class FilePath
	{
	public:
		///
		FilePath();

		///
		FilePath(TempDir::Enum);

		///
		FilePath(const char* _str);

		///
		FilePath(const StringView& _str);

		///
		FilePath& operator=(const StringView& _rhs);

		///
		void set(TempDir::Enum);

		///
		void set(const StringView& _str);

		///
		void join(const StringView& _str);

		///
		const char* get() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `/abv/gd/555/333/`.
		///
		const StringView getPath() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod.mac`.
		///
		const StringView getFileName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod`.
		///
		const StringView getBaseName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `.mac`.
		///
		const StringView getExt() const;

		///
		bool isAbsolute() const;

	private:
		char m_filePath[kMaxFilePath];
	};

} // namespace bx

#endif // BX_FILEPATH_H_HEADER_GUARD
