/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/file.h>
#include <bx/os.h>
#include <bx/readerwriter.h>

#if BX_PLATFORM_WINDOWS
extern "C" __declspec(dllimport) uint32_t __stdcall GetTempPathA(uint32_t _max, char* _ptr);
#endif // BX_PLATFORM_WINDOWS

namespace bx
{
	static bool isPathSeparator(char _ch)
	{
		return false
			|| '/'  == _ch
			|| '\\' == _ch
			;
	}

	static int32_t normalizeFilePath(char* _dst, int32_t _dstSize, const char* _src, int32_t _num)
	{
		// Reference: Lexical File Names in Plan 9 or Getting Dot-Dot Right
		// https://9p.io/sys/doc/lexnames.html

		const int32_t num = strLen(_src, _num);

		if (0 == num)
		{
			return strCopy(_dst, _dstSize, ".");
		}

		int32_t size = 0;

		StaticMemoryBlockWriter writer(_dst, _dstSize);
		Error err;

		int32_t idx      = 0;
		int32_t dotdot   = 0;

		if (2 <= num
		&&  ':' == _src[1])
		{
			size += write(&writer, toUpper(_src[idx]), &err);
			size += write(&writer, ':', &err);
			idx  += 2;
			dotdot = size;
		}

		const int32_t slashIdx = idx;

		bool rooted = isPathSeparator(_src[idx]);
		if (rooted)
		{
			size += write(&writer, '/', &err);
			++idx;
			dotdot = size;
		}

		while (idx < num && err.isOk() )
		{
			switch (_src[idx])
			{
			case '/':
			case '\\':
				++idx;
				break;

			case '.':
				if (idx+1 == num
				||  isPathSeparator(_src[idx+1]) )
				{
					++idx;
					break;
				}

				if ('.' == _src[idx+1]
				&& (idx+2 == num || isPathSeparator(_src[idx+2]) ) )
				{
					idx += 2;

					if (dotdot < size)
					{
						for (--size
							; dotdot < size && !isPathSeparator(_dst[size])
							; --size)
						{
						}
						seek(&writer, size, Whence::Begin);
					}
					else if (!rooted)
					{
						if (0 < size)
						{
							size += write(&writer, '/', &err);
						}

						size += write(&writer, "..", &err);
						dotdot = size;
					}

					break;
				}

				BX_FALLTHROUGH;

			default:
				if ( ( rooted && slashIdx+1 != size)
				||   (!rooted &&          0 != size) )
				{
					size += write(&writer, '/', &err);
				}

				for (; idx < num && !isPathSeparator(_src[idx]); ++idx)
				{
					size += write(&writer, _src[idx], &err);
				}

				break;
			}
		}

		if (0 == size)
		{
			size += write(&writer, '.', &err);
		}

		write(&writer, '\0', &err);

		return size;
	}

	static bool getTempPath(char* _out, uint32_t* _inOutSize)
	{
#if BX_PLATFORM_WINDOWS
		uint32_t len = ::GetTempPathA(*_inOutSize, _out);
		bool result = len != 0 && len < *_inOutSize;
		*_inOutSize = len;
		return result;
#else
		static const char* s_tmp[] =
		{
			"TMPDIR",
			"TMP",
			"TEMP",
			"TEMPDIR",

			NULL
		};

		for (const char** tmp = s_tmp; *tmp != NULL; ++tmp)
		{
			uint32_t len = *_inOutSize;
			*_out = '\0';
			bool result = getenv(*tmp, _out, &len);

			if (result
			&&  len != 0
			&&  len < *_inOutSize)
			{
				*_inOutSize = len;
				return result;
			}
		}

		FileInfo fi;
		if (stat("/tmp", fi)
		&&  FileInfo::Directory == fi.m_type)
		{
			strCopy(_out, *_inOutSize, "/tmp");
			*_inOutSize = 4;
			return true;
		}

		return false;
#endif // BX_PLATFORM_*
	}

	FilePath::FilePath()
	{
		set("");
	}

	FilePath::FilePath(TempDir::Enum)
	{
		set(TempDir::Tag);
	}

	FilePath::FilePath(const char* _rhs)
	{
		set(_rhs);
	}

	FilePath::FilePath(const StringView& _filePath)
	{
		set(_filePath);
	}

	FilePath& FilePath::operator=(const StringView& _rhs)
	{
		set(_rhs);
		return *this;
	}

	void FilePath::set(TempDir::Enum)
	{
		char tmp[kMaxFilePath];
		uint32_t len = BX_COUNTOF(tmp);
		getTempPath(tmp, &len);
		set(StringView(tmp, len) );
	}

	void FilePath::set(const StringView& _filePath)
	{
		normalizeFilePath(
			  m_filePath
			, BX_COUNTOF(m_filePath)
			, _filePath.getPtr()
			, _filePath.getLength()
			);
	}

	void FilePath::join(const StringView& _str)
	{
		char tmp[kMaxFilePath];
		strCopy(tmp, BX_COUNTOF(tmp), m_filePath);
		strCat(tmp, BX_COUNTOF(tmp), "/");
		strCat(tmp, BX_COUNTOF(tmp), _str);
		set(tmp);
	}

	const char* FilePath::get() const
	{
		return m_filePath;
	}

	const StringView FilePath::getPath() const
	{
		const char* end = strRFind(m_filePath, '/');
		if (NULL != end)
		{
			return StringView(m_filePath, end+1);
		}

		return StringView();
	}

	const StringView FilePath::getFileName() const
	{
		const char* fileName = strRFind(m_filePath, '/');
		if (NULL != fileName)
		{
			return StringView(fileName+1);
		}

		return get();
	}

	const StringView FilePath::getBaseName() const
	{
		const StringView fileName = getFileName();
		if (!fileName.isEmpty() )
		{
			const char* ext = strFind(fileName.getPtr(), '.', fileName.getLength() );
			if (ext != NULL)
			{
				return StringView(fileName.getPtr(), ext);
			}
		}

		return StringView();
	}

	const StringView FilePath::getExt() const
	{
		const StringView fileName = getFileName();
		if (!fileName.isEmpty() )
		{
			const char* ext = strFind(fileName.getPtr(), '.', fileName.getLength() );
			return StringView(ext);
		}

		return StringView();
	}

	bool FilePath::isAbsolute() const
	{
		return  '/' == m_filePath[0] // no drive letter
			|| (':' == m_filePath[1] && '/' == m_filePath[2]) // with drive letter
			;
	}

} // namespace bx
