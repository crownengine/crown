/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "device/export_transport.h"
#include <stddef.h>

#if defined(__linux__)
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int socket_address(struct sockaddr_un *addr, const char *path)
{
	size_t len = strlen(path);
	if (len == 0 || len >= sizeof(addr->sun_path))
		return -1;
	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	memcpy(addr->sun_path, path, len + 1);
	return 0;
}

void *export_bind(const char *path)
{
	struct sockaddr_un addr;
	if (socket_address(&addr, path) != 0)
		return (void *)(intptr_t)-1;
	int sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (sock < 0)
		return (void *)(intptr_t)-1;
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		close(sock);
		return (void *)(intptr_t)-1;
	}
	return (void *)(intptr_t)sock;
}

int export_receive(CrownExportPacket *packet, void *source, void *receiver)
{
	int *fd = (int *)source;
	int sock = (int)(intptr_t)receiver;
	char control[CMSG_SPACE(sizeof(int))];
	struct iovec iov = { packet, sizeof(*packet) };
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);
	*fd = -1;
	ssize_t size = recvmsg(sock, &msg, MSG_DONTWAIT | MSG_CMSG_CLOEXEC);
	if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	if (size < 0)
		return -1;
	for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET
			&& cmsg->cmsg_type == SCM_RIGHTS
			&& cmsg->cmsg_len >= CMSG_LEN(sizeof(int))
			) {
			memcpy(fd, CMSG_DATA(cmsg), sizeof(int));
			break;
		}
	}
	if (size != sizeof(*packet) || (msg.msg_flags & (MSG_TRUNC | MSG_CTRUNC))
		|| packet->magic != CROWN_EXPORT_MAGIC || packet->version != CROWN_EXPORT_VERSION) {
		if (*fd >= 0)
			close(*fd);
		*fd = -1;
		return -1;
	}
	return 1;
}

int export_send(const char *path, const CrownExportPacket *packet, int fd)
{
	struct sockaddr_un addr;
	if (socket_address(&addr, path) != 0)
		return -1;
	int sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (sock < 0)
		return -1;
	struct iovec iov = { (void *)packet, sizeof(*packet) };
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	char control[CMSG_SPACE(sizeof(int))];
	if (fd >= 0) {
		memset(control, 0, sizeof(control));
		msg.msg_control = control;
		msg.msg_controllen = sizeof(control);
		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
	}
	int result = sendmsg(sock, &msg, MSG_NOSIGNAL);
	close(sock);
	return result == sizeof(*packet) ? 0 : -1;
}

void export_close(void *receiver, const char *path)
{
	int sock = (int)(intptr_t)receiver;
	if (sock >= 0)
		close(sock);
	if (path != NULL)
		unlink(path);
}

#elif defined(_WIN32)
#include <windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ExportPipe
{
	HANDLE handle;
	int connected;
} ExportPipe;

void *export_bind(const char *path)
{
	ExportPipe *pipe = (ExportPipe *)malloc(sizeof(*pipe));
	if (pipe == NULL)
		return NULL;
	pipe->handle = CreateNamedPipeA(path
		, PIPE_ACCESS_INBOUND
		, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT
		, 1
		, sizeof(CrownExportPacket) * 4
		, sizeof(CrownExportPacket) * 4
		, 0
		, NULL
		);
	if (pipe->handle == INVALID_HANDLE_VALUE) {
		free(pipe);
		return NULL;
	}
	pipe->connected = 0;
	return pipe;
}

int export_receive(CrownExportPacket *packet, void *source, void *server)
{
	uint32_t *producer_pid = (uint32_t *)source;
	ExportPipe *pipe = (ExportPipe *)server;
	if (pipe == NULL)
		return -1;
	if (!pipe->connected) {
		if (!ConnectNamedPipe(pipe->handle, NULL)) {
			DWORD error = GetLastError();
			if (error == ERROR_PIPE_LISTENING)
				return 0;
			if (error != ERROR_PIPE_CONNECTED)
				return -1;
		}
		pipe->connected = 1;
	}
	DWORD size = 0;
	if (!ReadFile(pipe->handle, packet, sizeof(*packet), &size, NULL)) {
		DWORD error = GetLastError();
		if (error == ERROR_NO_DATA)
			return 0;
		if (error == ERROR_BROKEN_PIPE) {
			DisconnectNamedPipe(pipe->handle);
			pipe->connected = 0;
			return 0;
		}
		return -1;
	}
	if (size != sizeof(*packet)
		|| packet->magic != CROWN_EXPORT_MAGIC
		|| packet->version != CROWN_EXPORT_VERSION
		)
		return -1;
	ULONG pid = 0;
	if (!GetNamedPipeClientProcessId(pipe->handle, &pid))
		return -1;
	*producer_pid = pid;
	return 1;
}

int export_send(const char *path, const CrownExportPacket *packet, int fd)
{
	(void)fd;
	static HANDLE pipe = INVALID_HANDLE_VALUE;
	static char current_path[256];
	if (pipe != INVALID_HANDLE_VALUE && strcmp(path, current_path) != 0) {
		CloseHandle(pipe);
		pipe = INVALID_HANDLE_VALUE;
	}
	if (pipe == INVALID_HANDLE_VALUE) {
		pipe = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (pipe == INVALID_HANDLE_VALUE)
			return -1;
		if (strlen(path) >= sizeof(current_path)) {
			CloseHandle(pipe);
			pipe = INVALID_HANDLE_VALUE;
			return -1;
		}
		strcpy(current_path, path);
	}
	DWORD size = 0;
	if (!WriteFile(pipe, packet, sizeof(*packet), &size, NULL) || size != sizeof(*packet)) {
		CloseHandle(pipe);
		pipe = INVALID_HANDLE_VALUE;
		return -1;
	}
	return 0;
}

void export_close(void *server, const char *path)
{
	(void)path;
	ExportPipe *pipe = (ExportPipe *)server;
	if (pipe == NULL)
		return;
	DisconnectNamedPipe(pipe->handle);
	CloseHandle(pipe->handle);
	free(pipe);
}

#else
int export_send(const char *path, const CrownExportPacket *packet, int fd)
{
	(void)fd;
	(void)path;
	(void)packet;
	return -1;
}

#endif /* if defined(__linux__) */

int export_is_open(void *receiver)
{
#if defined(_WIN32)
	return receiver != NULL;
#else
	return (intptr_t)receiver >= 0;
#endif
}
