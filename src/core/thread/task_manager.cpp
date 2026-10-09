/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/thread/task_manager.h"
#include "core/containers/queue.inl"
#include "core/error/error.inl"
#include "core/math/math.h"
#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include <new>
#include <string.h> // memcpy

namespace crown
{
static s32 task_manager_worker(void *data)
{
	return ((TaskManager *)data)->do_work();
}

TaskManager::TaskManager(Allocator &a)
	: _allocator(&a)
	, _workers(NULL)
	, _num_workers(thread::num_logical_cpus() - 1)
	, _queue(a)
	, _execute_function(NULL)
	, _execute_data(NULL)
	, _execute_epoch(0)
	, _execute_pending(0)
	, _freelist_dequeue(1)
	, _freelist_enqueue(MAX_TASKS - 1)
	, _exit(false)
{
	// TaskID 0 is reserved for no parent/dependency.
	for (u32 i = 0; i < MAX_TASKS; ++i) {
		_indices[i].id = i;
		_indices[i].next = i + 1;
	}

	_workers = (Thread *)_allocator->allocate(_num_workers * sizeof(Thread), alignof(Thread));
	for (u32 i = 0; i < _num_workers; ++i) {
		new (&_workers[i]) Thread();
		_workers[i].start(task_manager_worker, this);
	}
}

TaskManager::~TaskManager()
{
	_tasks_mutex.lock();
	_exit = true;
	_tasks_condition.broadcast();
	_tasks_mutex.unlock();
	for (u32 i = 0; i < _num_workers; ++i)
		_workers[i].~Thread();

	_allocator->deallocate(_workers);
}

u32 TaskManager::begin_add(TaskFunction func, void *data, u32 data_size, u32 parent_id, u32 dependency_id)
{
	_tasks_mutex.lock();
	CE_ASSERT(_freelist_dequeue < MAX_TASKS, "Too many tasks");
	Index &index = _indices[_freelist_dequeue];
	_freelist_dequeue = index.next;
	index.id += NEW_TASK_ID_ADD;
	const u32 task_id = index.id;
	Task &task = _objects[task_id & TASK_INDEX_MASK];
	task.func = func;
	CE_ASSERT(data_size == 0 || data != NULL, "Invalid task data");
	if (data_size != 0 && data_size <= sizeof(task.pad)) {
		memcpy(task.pad, data, data_size);
		task.data_external = false;
	} else {
		memcpy(task.pad, &data, sizeof(data));
		task.data_external = true;
	}
	task.pending.store(parent_id == 0 ? 2 : 1);
	task.parent_index = parent_id & TASK_INDEX_MASK;
	index.dependency = dependency_id;
	if (dependency_id != 0)
		CE_ASSERT(dependency_id >= MAX_TASKS
			&& _indices[dependency_id & TASK_INDEX_MASK].id >= dependency_id
			, "Invalid dependency task ID"
			);
	if (parent_id != 0) {
		CE_ASSERT(_indices[parent_id & TASK_INDEX_MASK].id == parent_id, "Invalid parent task ID");
		Task &parent = _objects[parent_id & TASK_INDEX_MASK];
		CE_ASSERT(parent.parent_index == 0, "Task is not a parent");
		CE_ASSERT(parent.pending.load() > 0, "Parent task has completed");
		parent.pending.fetch_add(1);
	}
	queue::push_back(_queue, task_id);
	_tasks_condition.signal();
	_tasks_mutex.unlock();
	return task_id;
}

u32 TaskManager::begin_add(TaskData8 task, u32 parent_id, u32 dependency_id)
{
	return begin_add(task.func, task.data, sizeof(task.data), parent_id, dependency_id);
}

u32 TaskManager::begin_add(TaskData16 task, u32 parent_id, u32 dependency_id)
{
	return begin_add(task.func, task.data, sizeof(task.data), parent_id, dependency_id);
}

u32 TaskManager::begin_add(TaskData32 task, u32 parent_id, u32 dependency_id)
{
	return begin_add(task.func, task.data, sizeof(task.data), parent_id, dependency_id);
}

u32 TaskManager::begin_add(TaskData48 task, u32 parent_id, u32 dependency_id)
{
	return begin_add(task.func, task.data, sizeof(task.data), parent_id, dependency_id);
}

u32 TaskManager::begin_add_empty(u32 parent_id, u32 dependency_id)
{
	return begin_add(NULL, NULL, 0, parent_id, dependency_id);
}

// Called with the queue mutex locked.
static void recycle_task(TaskManager &manager, u32 task_id)
{
	TaskManager::Index &index = manager._indices[task_id & TASK_INDEX_MASK];
	index.next = MAX_TASKS;
	if (manager._freelist_dequeue == MAX_TASKS)
		manager._freelist_dequeue = task_id & TASK_INDEX_MASK;
	else
		manager._indices[manager._freelist_enqueue].next = task_id & TASK_INDEX_MASK;
	manager._freelist_enqueue = task_id & TASK_INDEX_MASK;
}

// Called with the queue mutex locked.
static void complete_task(TaskManager &manager, u32 task_id)
{
	Task &task = manager._objects[task_id & TASK_INDEX_MASK];
	if (task.pending.fetch_sub(1) == 1) {
		const u32 parent_index = task.parent_index;
		recycle_task(manager, task_id);
		if (parent_index != 0) {
			Task &parent = manager._objects[parent_index];
			if (parent.pending.fetch_sub(1) == 1)
				recycle_task(manager, parent_index);
		}
	}
	manager._tasks_condition.broadcast();
}

void TaskManager::finish_add(u32 task_id)
{
	_tasks_mutex.lock();
	CE_ASSERT(_indices[task_id & TASK_INDEX_MASK].id == task_id, "Invalid task ID");
	CE_ASSERT(_objects[task_id & TASK_INDEX_MASK].parent_index == 0, "Task is not a parent");
	complete_task(*this, task_id);
	_tasks_mutex.unlock();
}

// Called and returns with the queue mutex locked.
static bool run_next(TaskManager &manager)
{
	const u32 num_queued = queue::size(manager._queue);
	for (u32 i = 0; i < num_queued; ++i) {
		const u32 task_id = queue::back(manager._queue);
		queue::pop_back(manager._queue);
		Task &task = manager._objects[task_id & TASK_INDEX_MASK];
		const u32 dependency_id = manager._indices[task_id & TASK_INDEX_MASK].dependency;
		const bool dependency_pending = dependency_id != 0
			&& manager._indices[dependency_id & TASK_INDEX_MASK].id == dependency_id
			&& manager._objects[dependency_id & TASK_INDEX_MASK].pending.load() > 0;
		if (task.pending.load() == 0 || dependency_pending) {
			queue::push_front(manager._queue, task_id);
			continue;
		}
		manager._tasks_mutex.unlock();

		if (task.func != NULL) {
			void *data = task.pad;
			if (task.data_external)
				memcpy(&data, task.pad, sizeof(data));
			task.func(task_id, data);
		}
		manager._tasks_mutex.lock();
		complete_task(manager, task_id);
		return true;
	}

	return false;
}

void TaskManager::wait(u32 task_id)
{
	_tasks_mutex.lock();
	Index &index = _indices[task_id & TASK_INDEX_MASK];
	CE_ASSERT(task_id >= MAX_TASKS && index.id >= task_id, "Invalid task ID");
	while (index.id == task_id && _objects[task_id & TASK_INDEX_MASK].pending.load() > 0) {
		if (!run_next(*this))
			_tasks_condition.wait(_tasks_mutex);
	}
	_tasks_mutex.unlock();
}

void TaskManager::execute_on_workers(ThreadFunction func, void *data)
{
	CE_ASSERT(func != NULL, "Invalid worker function");
	_execute_mutex.lock();
	_tasks_mutex.lock();
	_execute_function = func;
	_execute_data = data;
	_execute_pending = _num_workers;
	++_execute_epoch;
	_tasks_condition.broadcast();
	while (_execute_pending != 0)
		_tasks_condition.wait(_tasks_mutex);
	_tasks_mutex.unlock();
	_execute_mutex.unlock();
}

s32 TaskManager::do_work()
{
	u32 execute_epoch = 0;
	_tasks_mutex.lock();
	while (!_exit.load()) {
		if (execute_epoch != _execute_epoch) {
			ThreadFunction func = _execute_function;
			void *data = _execute_data;
			execute_epoch = _execute_epoch;
			_tasks_mutex.unlock();
			func(data);
			_tasks_mutex.lock();
			--_execute_pending;
			_tasks_condition.broadcast();
			continue;
		}
		if (!run_next(*this))
			_tasks_condition.wait(_tasks_mutex);
	}
	_tasks_mutex.unlock();

	return 0;
}

struct ParallelForJob
{
	ParallelForFunction func;
	void *items;
	u32 item_size;
	u32 num_items;
	u32 num_jobs;
	u32 root_id;
};

static void parallel_for_job(u32 task_id, void *data)
{
	CE_UNUSED(task_id);
	ParallelForJob &job = *(ParallelForJob *)data;
	TaskManager &tasks = task_manager();
	while (job.num_jobs > 1) {
		const u32 left_jobs = job.num_jobs / 2;
		const u32 left_items = u32(u64(job.num_items) * left_jobs / job.num_jobs);
		ParallelForJob right = {
			job.func
			, (u8 *)job.items + size_t(left_items) * job.item_size
			, job.item_size
			, job.num_items - left_items
			, job.num_jobs - left_jobs
			, job.root_id
		};
		tasks.begin_add(parallel_for_job, &right, sizeof(right), job.root_id);
		job.num_items = left_items;
		job.num_jobs = left_jobs;
	}
	if (job.num_items != 0)
		job.func(job.items, job.num_items);
}

CE_STATIC_ASSERT(sizeof(ParallelForJob) <= sizeof(Task::pad));
CE_STATIC_ASSERT(alignof(ParallelForJob) <= 16);

struct ParallelForRoot
{
	ParallelForFunction func;
	void *items;
	u32 item_size;
	u32 num_items;
	u32 num_jobs;
};

static void parallel_for_root(u32 task_id, void *data)
{
	ParallelForRoot &root = *(ParallelForRoot *)data;
	ParallelForJob job = { root.func, root.items, root.item_size, root.num_items, root.num_jobs, task_id };
	parallel_for_job(task_id, &job);
}

u32 parallel_for(void *items, u32 item_size, u32 num_items, u32 num_jobs, ParallelForFunction func, u32 dependency_id)
{
	CE_ASSERT(num_jobs != 0 && num_jobs <= MAX_TASKS - 2u, "Invalid parallel_for job count");
	CE_ASSERT((items != NULL && item_size != 0) || num_items == 0, "Invalid parallel_for items");
	CE_ASSERT(func != NULL, "Invalid parallel_for function");
	TaskManager &tasks = task_manager();
	ParallelForRoot root_job = { func, items, item_size, num_items, min(num_items, num_jobs) };
	const u32 root = tasks.begin_add(parallel_for_root, &root_job, sizeof(root_job), 0, dependency_id);
	tasks.finish_add(root);
	return root;
}

namespace task_manager_globals
{
	static TaskManager *_task_manager;

	void init()
	{
		CE_ENSURE(_task_manager == NULL);
		_task_manager = CE_NEW(default_allocator(), TaskManager)(default_allocator());
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), _task_manager);
		_task_manager = NULL;
	}

} // namespace task_manager_globals

TaskManager &task_manager()
{
	return *task_manager_globals::_task_manager;
}

} // namespace crown
