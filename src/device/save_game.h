/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/types.h"
#include "core/types.h"

namespace crown
{
/// Save request error codes.
///
/// @ingroup Device
struct SaveError
{
	enum Enum
	{
		SUCCESS = 0,      ///< Operation completed successfully.
		INVALID_REQUEST,  ///< Token does not identify a live request.
		SAVE_DIR_UNSET,   ///< save_dir is not configured.
		MISSING,          ///< Save file does not exist.
		INVALID_FILENAME, ///< Filename is invalid.
		IO_ERROR,         ///< File read/write failed.
		CORRUPTED,        ///< Save data could not be parsed.
		UNKNOWN           ///< Request failed for an unknown reason.
	};
};

/// Save request status.
///
/// @ingroup Device
struct SaveStatus
{
	bool done;         ///< Request has finished.
	f32 progress;      ///< Completion progress in [0; 1] range.
	void *data;        ///< Loaded data, or NULL unless this is a successful load request.
	u32 size;          ///< Loaded data size in bytes, excluding any internal terminator.
	s32 error;         ///< SaveError code, SaveError::SUCCESS while pending or when no error occurred.
};

struct SaveGame;

namespace save_game
{
	/// Starts loading @a filename asynchronously and returns its request id.
	///
	/// @a filename must be a valid filename, not a path. The function returns
	/// before the file has been read. Call status() with the returned id to poll
	/// for completion. On success, SaveStatus::data points to the loaded bytes
	/// and SaveStatus::size contains their size.
	u32 load(const char *filename);

	/// Starts saving @a data to @a filename asynchronously and returns its request id.
	///
	/// @a filename must be a valid filename, not a path. @a data is copied before
	/// this function returns and may be NULL only when @a size is 0. The function
	/// returns before the file has been written. Call status() with the returned
	/// id to poll for completion.
	u32 save(const char *filename, const void *data, u32 size);

	/// Returns the status of @a save_request.
	///
	/// While the request is pending, SaveStatus::error is SaveError::SUCCESS.
	/// SaveStatus::data is owned by the request and remains valid until free()
	/// is called for @a save_request.
	SaveStatus status(u32 save_request);

	/// Frees resources allocated for @a save_request.
	///
	/// The request must be done. After this call, @a save_request is invalid.
	void free(u32 save_request);

} // namespace save_game

namespace save_game_globals
{
	/// Initializes the SaveGame singleton.
	void init(Allocator &a, const char *save_dir);

	/// Shuts down the SaveGame singleton.
	void shutdown();

	/// Returns the SaveGame singleton.
	SaveGame *save_game();

} // namespace save_game_globals

} // namespace crown
