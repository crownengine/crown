/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/thread/task_manager.h"
#include "world/sound_world.h"
#include <stddef.h>

namespace crown
{
CE_STATIC_ASSERT(offsetof(SoundWorld, _marker) == 0);

struct SoundUpdateData
{
	SoundWorld *world;
};

static void update_sound_task(u32 task_id, void *data)
{
	CE_UNUSED(task_id);
	SoundUpdateData &task = *(SoundUpdateData *)data;
	task.world->update();
}

TaskData8 SoundWorld::update_task()
{
	struct SoundUpdateTaskData
	{
		TaskFunction func;
		SoundUpdateData data;
	};
	union SoundUpdateTask
	{
		TaskData8 task;
		SoundUpdateTaskData data;
	};
	CE_STATIC_ASSERT(sizeof(SoundUpdateTask) == sizeof(TaskData8));

	SoundUpdateTask task = {};
	task.data = { update_sound_task, { this } };
	return task.task;
}

} // namespace crown
