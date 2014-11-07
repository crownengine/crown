/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#if defined(CROWN_DEBUG)
	#include "console_server.h"
	#define CE_LOGI(msg, ...) crown::console_server_globals::console().log_to_all(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
	#define CE_LOGD(msg, ...) crown::console_server_globals::console().log_to_all(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
	#define CE_LOGE(msg, ...) crown::console_server_globals::console().log_to_all(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
	#define CE_LOGW(msg, ...) crown::console_server_globals::console().log_to_all(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
#else
	#define CE_LOGI(msg, ...) ((void)0)
	#define CE_LOGD(msg, ...) ((void)0)
	#define CE_LOGE(msg, ...) ((void)0)
	#define CE_LOGW(msg, ...) ((void)0)
#endif
