/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
const string CROWN_VERSION = "0.59.0";

const string CROWN_WWW_URL = "https://www.crownengine.org";
const string CROWN_DOCS_URL = "https://docs.crownengine.org";
const string CROWN_LATEST_DOCS_URL = CROWN_DOCS_URL + "/html/v" + CROWN_VERSION;
const string CROWN_LATEST_CHANGELOG_URL = CROWN_LATEST_DOCS_URL + "/changelog.html#changelog";
const string CROWN_FUND_URL = CROWN_WWW_URL + "/fund";
const string CROWN_CREDITS_URL = CROWN_WWW_URL + "/about/credits";

#if CROWN_PLATFORM_LINUX
const string ENGINE_DIR = ".";
const string EXE_PREFIX = "./";
const string EXE_SUFFIX = "";
#elif CROWN_PLATFORM_WINDOWS
const string ENGINE_DIR = ".";
const string EXE_PREFIX = "";
const string EXE_SUFFIX = ".exe";
#endif
const string ENGINE_EXE = EXE_PREFIX
#if CROWN_DEBUG
	+ "crown-debug"
#else
	+ "crown-development"
#endif
	+ EXE_SUFFIX;

const string DATA_COMPILER_ADDRESS = "127.0.0.1";
const uint16 DATA_COMPILER_TCP_PORT = 10618;

const string EDITOR_ADDRESS = "127.0.0.1";
const uint16 EDITOR_TCP_PORT = 10001;

const string UNIT_PREVIEW_ADDRESS = "127.0.0.1";
const uint16 UNIT_PREVIEW_TCP_PORT = 10002;

const string GAME_ADDRESS = "127.0.0.1";
const uint16 GAME_TCP_PORT = 12345;

const string THUMBNAIL_ADDRESS = "127.0.0.1";
const uint16 THUMBNAIL_TCP_PORT = 54321;

const int DATA_COMPILER_CONNECTION_TRIES = 10;
const int DATA_COMPILER_CONNECTION_INTERVAL = 250;

const int EDITOR_CONNECTION_TRIES = 16;
const int EDITOR_CONNECTION_INTERVAL = 125;

const int GAME_CONNECTION_TRIES = 16;
const int GAME_CONNECTION_INTERVAL = 125;

const int THUMBNAIL_CONNECTION_TRIES = 16;
const int THUMBNAIL_CONNECTION_INTERVAL = 125;

const string LEVEL_EDITOR_BOOT_DIR = "core/editors/level_editor";
const string UNIT_PREVIEW_BOOT_DIR = "core/editors/unit_preview";
const string THUMBNAIL_BOOT_DIR = "core/editors/thumbnail";
const string LEVEL_NONE = "";
const string LEVEL_EMPTY = "core/editors/levels/empty";

} /* namespace Crown */
