/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"
#include "core/containers/types.h"
#include "core/thread/condition_variable.h"
#include "core/thread/mutex.h"
#include "core/thread/thread.h"
#include <atomic>

/// @defgroup Thread Thread
/// @ingroup Core
namespace crown
{
#define MAX_TASKS 32768
#define TASK_INDEX_MASK (MAX_TASKS - 1)
#define NEW_TASK_ID_ADD MAX_TASKS
CE_STATIC_ASSERT(MAX_TASKS - 1 <= u16(-1));

typedef void (*TaskFunction)(u32 task_id, void *data);

struct Task
{
	CE_ALIGN_DECL(CROWN_CACHE_LINE_SIZE, TaskFunction func);
	std::atomic_int pending;
	u16 parent_index;      // A parent remains allocated until its children complete.
	u16 data_external : 1; // pad contains a pointer instead of copied data.
	CE_ALIGN_DECL(16, u8 pad[CROWN_CACHE_LINE_SIZE - 16]);
};

CE_STATIC_ASSERT(sizeof(Task) == CROWN_CACHE_LINE_SIZE);
CE_STATIC_ASSERT(alignof(Task) == CROWN_CACHE_LINE_SIZE);
CE_STATIC_ASSERT(sizeof(Task::pad) == CROWN_CACHE_LINE_SIZE - 16);

struct TaskData8
{
	TaskFunction func;
	char data[8];
};

struct TaskData16
{
	TaskFunction func;
	char data[16];
};

struct TaskData32
{
	TaskFunction func;
	char data[32];
};

struct TaskData48
{
	TaskFunction func;
	char data[48];
};
CE_STATIC_ASSERT(sizeof(TaskData48::data) <= sizeof(Task::pad));

/// TaskManager.
///
/// @ingroup Thread.
struct TaskManager
{
	struct Index
	{
		u32 id;
		union
		{
			u32 next;       // While free.
			u32 dependency; // While allocated.
		};
	};

	Allocator *_allocator;
	Thread *_workers;
	u32 _num_workers;

	Queue<u32> _queue;
	Mutex _tasks_mutex;
	ConditionVariable _tasks_condition;
	Mutex _execute_mutex;
	ThreadFunction _execute_function;
	void *_execute_data;
	u32 _execute_epoch;
	u32 _execute_pending;

	Index _indices[MAX_TASKS];
	Task _objects[MAX_TASKS];
	u32 _freelist_dequeue;
	u32 _freelist_enqueue;

	std::atomic_bool _exit;

	///
	TaskManager(Allocator &a);

	///
	~TaskManager();

	///
	TaskManager(const TaskManager &) = delete;

	///
	TaskManager &operator=(const TaskManager &) = delete;

	/// Creates and queues a task. If @a parent_id is 0 the task is a root task and cannot complete
	/// until all of its children also complete; call begin_add(parent_id=root) to add a child to a
	/// root task. If @a dependency is not 0, the task is not allowed to run until the specified
	/// dependency task completes. Call finish_add() on every root task when no more children will
	/// be added.
	u32 begin_add(TaskFunction func, void *data, u32 data_size = 0, u32 parent_id = 0, u32 dependency_id = 0);

	/// Like begin_add() but the data is guaranteed to be copied internally.
	u32 begin_add(TaskData8 task, u32 parent_id = 0, u32 dependency_id = 0);

	/// Like begin_add() but the data is guaranteed to be copied internally.
	u32 begin_add(TaskData16 task, u32 parent_id = 0, u32 dependency_id = 0);

	/// Like begin_add() but the data is guaranteed to be copied internally.
	u32 begin_add(TaskData32 task, u32 parent_id = 0, u32 dependency_id = 0);

	/// Like begin_add() but the data is guaranteed to be copied internally.
	u32 begin_add(TaskData48 task, u32 parent_id = 0, u32 dependency_id = 0);

	/// Creates and queues an empty task. Empty tasks can be useful when you have a task with
	/// multiple dependencies:
	///
	/// u32 root = tasks.begin_add_empty();
	/// tasks.begin_add_empty(root, a);
	/// tasks.begin_add_empty(root, b);
	/// tasks.finish_add(root);
	/// u32 task = tasks.begin_add(work, &data, 0, 0, root);
	/// tasks.finish_add(task);
	///
	/// Here 'root' completes after both children, and each child waits for its own dependency. So
	/// 'task' runs only after both 'a' and 'b' complete.
	u32 begin_add_empty(u32 parent_id = 0, u32 dependency_id = 0);

	/// Signals that no more children will be added to a root task.
	void finish_add(u32 task_id);

	/// Waits for @a task_id to complete, running other tasks while waiting.
	void wait(u32 task_id);

	/// Calls @a func once on each worker and waits for all calls to finish.
	/// Must be called from outside the worker threads.
	void execute_on_workers(ThreadFunction func, void *data);

	///
	s32 do_work();
};

typedef void (*ParallelForFunction)(void *items, u32 num_items);

/// Enqueues @a func for up to @a num_jobs ranges and returns a task ID that completes after all
/// ranges. If @a dependency_id is nonzero, the ranges start after that task completes. The caller
/// must keep @a items alive until the task completes.
u32 parallel_for(void *items, u32 item_size, u32 num_items, u32 num_jobs, ParallelForFunction func, u32 dependency_id = 0);

/// Returns the task manager.
TaskManager &task_manager();

namespace task_manager_globals
{
	///
	void init();

	///
	void shutdown();

} // namespace task_manager_globals

} // namespace crown
