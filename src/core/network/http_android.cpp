/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID && !CROWN_HTTP_NOOP
#include "core/memory/memory.inl"
#include "core/network/http_internal.h"
#include "core/thread/condition_variable.h"
#include "core/thread/scoped_mutex.inl"
#include "core/thread/thread.h"
#include "device/types.h"
#include <jni.h>
#include <string.h>

namespace crown
{
extern PlatformData g_platform_data;

struct HttpAndroidRequest;

struct HttpAndroid
{
	Allocator *_allocator;
	JavaVM *_vm;
	Mutex _mutex;
	ConditionVariable _condition;
	Thread _thread;
	HttpAndroidRequest *_requests[HTTP_MAX_REQUESTS];
	u32 _num_requests;
	bool _ready;
	bool _failed;
	bool _exit;

	explicit HttpAndroid(Allocator &a)
		: _allocator(&a)
		, _vm((JavaVM *)g_platform_data._android_java_vm)
		, _num_requests(0)
		, _ready(false)
		, _failed(false)
		, _exit(false)
	{
	}
};

struct HttpAndroidRequest
{
	HttpRequest request;
};

static bool http_android_exception(JNIEnv *env, HttpRequest &request)
{
	if (!env->ExceptionCheck())
		return false;

	jthrowable exception = env->ExceptionOccurred();
	env->ExceptionClear();
	jclass cls = env->FindClass("java/lang/Throwable");
	jmethodID to_string = cls == NULL ? NULL : env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
	jstring message = to_string == NULL ? NULL : (jstring)env->CallObjectMethod(exception, to_string);
	if (env->ExceptionCheck()) {
		env->ExceptionClear();
		message = NULL;
	}

	const char *text = message == NULL ? NULL : env->GetStringUTFChars(message, NULL);
	http_request::fail(request.id, text == NULL ? "Android HTTP operation failed" : text);
	if (text != NULL)
		env->ReleaseStringUTFChars(message, text);
	env->DeleteLocalRef(message);
	env->DeleteLocalRef(cls);
	env->DeleteLocalRef(exception);
	env->ExceptionClear();
	return true;
}

static bool http_android_active(HttpRequest &request)
{
	if (!http_request::active(request))
		return false;

	if (http_request::expired(request)) {
		http_request::fail(request.id, "HTTP request timed out");
		return false;
	}

	return true;
}

static bool http_android_timeouts(JNIEnv *env
	, HttpRequest &request
	, jobject connection
	, jmethodID connect_timeout
	, jmethodID read_timeout
	)
{
	if (!http_request::active(request))
		return false;

	const s32 remaining = http_request::remaining_ms(request);
	if (remaining < 0) {
		http_request::fail(request.id, "HTTP request timed out");
		return false;
	}
	env->CallVoidMethod(connection, connect_timeout, remaining);
	if (http_android_exception(env, request))
		return false;

	env->CallVoidMethod(connection, read_timeout, remaining);
	return !http_android_exception(env, request);
}

static s32 http_android_run(void *user_data)
{
	HttpAndroid &backend = *(HttpAndroid *)user_data;
	JNIEnv *env = NULL;
	const bool attached = backend._vm->AttachCurrentThread(&env, NULL) == JNI_OK;
	backend._mutex.lock();
	backend._failed = !attached;
	backend._ready = true;
	backend._condition.signal();
	backend._mutex.unlock();
	if (!attached)
		return 0;

	jclass url_class = env->FindClass("java/net/URL");
	jclass connection_class = env->FindClass("java/net/HttpURLConnection");
	jclass input_class = env->FindClass("java/io/InputStream");
	jclass output_class = env->FindClass("java/io/OutputStream");
#define HTTP_JNI_METHOD(var, cls, name, signature)          \
	jmethodID var = env->GetMethodID(cls, name, signature); \
	CE_ENSURE(var != NULL)
	HTTP_JNI_METHOD(url_init, url_class, "<init>", "(Ljava/lang/String;)V");
	HTTP_JNI_METHOD(open, url_class, "openConnection", "()Ljava/net/URLConnection;");
	HTTP_JNI_METHOD(connect_timeout, connection_class, "setConnectTimeout", "(I)V");
	HTTP_JNI_METHOD(read_timeout, connection_class, "setReadTimeout", "(I)V");
	HTTP_JNI_METHOD(set_method, connection_class, "setRequestMethod", "(Ljava/lang/String;)V");
	HTTP_JNI_METHOD(set_header, connection_class, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
	HTTP_JNI_METHOD(set_output, connection_class, "setDoOutput", "(Z)V");
	HTTP_JNI_METHOD(get_output, connection_class, "getOutputStream", "()Ljava/io/OutputStream;");
	HTTP_JNI_METHOD(get_input, connection_class, "getInputStream", "()Ljava/io/InputStream;");
	HTTP_JNI_METHOD(get_error, connection_class, "getErrorStream", "()Ljava/io/InputStream;");
	HTTP_JNI_METHOD(get_code, connection_class, "getResponseCode", "()I");
	HTTP_JNI_METHOD(get_reason, connection_class, "getResponseMessage", "()Ljava/lang/String;");
	HTTP_JNI_METHOD(get_header_name, connection_class, "getHeaderFieldKey", "(I)Ljava/lang/String;");
	HTTP_JNI_METHOD(get_header_value, connection_class, "getHeaderField", "(I)Ljava/lang/String;");
	HTTP_JNI_METHOD(read, input_class, "read", "([B)I");
	HTTP_JNI_METHOD(close_input, input_class, "close", "()V");
	HTTP_JNI_METHOD(write, output_class, "write", "([BII)V");
	HTTP_JNI_METHOD(close_output, output_class, "close", "()V");
	HTTP_JNI_METHOD(disconnect, connection_class, "disconnect", "()V");
#undef HTTP_JNI_METHOD
	jstring post = env->NewStringUTF("POST");
	jstring content_type = env->NewStringUTF("Content-Type");
	jbyteArray bytes = env->NewByteArray(64 * 1024);
	CE_ENSURE(!env->ExceptionCheck());

	for (;;) {
		backend._mutex.lock();
		while (backend._num_requests == 0 && !backend._exit)
			backend._condition.wait(backend._mutex);
		if (backend._num_requests == 0) {
			backend._mutex.unlock();
			break;
		}

		HttpAndroidRequest *rr = backend._requests[0];
		memmove(backend._requests, backend._requests + 1, --backend._num_requests * sizeof(rr));
		backend._mutex.unlock();
		HttpRequest &request = rr->request;
		if (!http_android_active(request)) {
			http_request::release(&request);
			continue;
		}
		if (env->PushLocalFrame(64) != JNI_OK) {
			http_android_exception(env, request);
			http_request::release(&request);
			continue;
		}

		request.response = http_response::create(*request.allocator);
		jobject connection = NULL;
		{
			jstring url_text = env->NewStringUTF(request.url.data());
			if (http_android_exception(env, request) || url_text == NULL)
				goto request_done;

			jobject url = env->NewObject(url_class, url_init, url_text);
			if (http_android_exception(env, request) || url == NULL)
				goto request_done;

			connection = env->CallObjectMethod(url, open);
			if (http_android_exception(env, request) || connection == NULL)
				goto request_done;

			if (!http_android_active(request))
				goto request_done;

			if (request.post) {
				env->CallVoidMethod(connection, set_method, post);
				if (http_android_exception(env, request))
					goto request_done;

				jstring value = env->NewStringUTF(request.content_type.data());
				if (http_android_exception(env, request))
					goto request_done;

				env->CallVoidMethod(connection, set_header, content_type, value);
				if (http_android_exception(env, request))
					goto request_done;

				env->CallVoidMethod(connection, set_output, JNI_TRUE);
				if (http_android_exception(env, request)
					|| !http_android_timeouts(env, request, connection, connect_timeout, read_timeout))
					goto request_done;

				jobject output = env->CallObjectMethod(connection, get_output);
				if (http_android_exception(env, request) || output == NULL)
					goto request_done;

				for (u32 offset = 0; offset < request.upload.length();) {
					if (!http_android_active(request))
						goto request_done;

					const u32 count = min(request.upload.length() - offset, 64u * 1024);
					env->SetByteArrayRegion(bytes
						, 0
						, (jint)count
						, (const jbyte *)request.upload.data() + offset
						);
					env->CallVoidMethod(output, write, bytes, (jint)0, (jint)count);
					if (http_android_exception(env, request))
						goto request_done;

					offset += count;
				}

				if (!http_android_active(request))
					goto request_done;

				env->CallVoidMethod(output, close_output);
				if (http_android_exception(env, request))
					goto request_done;

				env->DeleteLocalRef(output);
			}

			HttpResponse *&response = request.response;
			if (!http_android_timeouts(env, request, connection, connect_timeout, read_timeout))
				goto request_done;

			const jint code = env->CallIntMethod(connection, get_code);
			if (http_android_exception(env, request) || !http_android_active(request))
				goto request_done;

			if (code < 100) {
				http_request::fail(request.id, "Invalid Android HTTP response");
				goto request_done;
			}

			response->status_code = (u32)code;
			jstring reason = (jstring)env->CallObjectMethod(connection, get_reason);
			if (http_android_exception(env, request))
				goto request_done;

			if (reason != NULL) {
				const char *text = env->GetStringUTFChars(reason, NULL);
				if (http_android_exception(env, request) || text == NULL)
					goto request_done;

				const bool ok = http_response::reason(response, *request.allocator, text, strlen32(text));
				env->ReleaseStringUTFChars(reason, text);
				env->DeleteLocalRef(reason);
				if (!ok) {
					http_request::fail(request.id, "HTTP response is too large");
					goto request_done;
				}
			}

			for (jint ii = 0; http_android_active(request); ++ii) {
				jstring name = (jstring)env->CallObjectMethod(connection, get_header_name, ii);
				if (http_android_exception(env, request))
					goto request_done;

				jstring value = (jstring)env->CallObjectMethod(connection, get_header_value, ii);
				if (http_android_exception(env, request))
					goto request_done;

				if (name == NULL && value == NULL)
					break;

				if (name != NULL && value != NULL) {
					const char *key = env->GetStringUTFChars(name, NULL);
					if (http_android_exception(env, request) || key == NULL)
						goto request_done;

					const char *val = env->GetStringUTFChars(value, NULL);
					if (http_android_exception(env, request) || val == NULL) {
						env->ReleaseStringUTFChars(name, key);
						goto request_done;
					}

					const bool ok = http_response::header(response
						, *request.allocator
						, key
						, strlen32(key)
						, val
						, strlen32(val)
						);
					env->ReleaseStringUTFChars(name, key);
					env->ReleaseStringUTFChars(value, val);
					if (!ok) {
						http_request::fail(request.id, "HTTP response headers are too large");
						goto request_done;
					}
				}

				env->DeleteLocalRef(name);
				env->DeleteLocalRef(value);
			}

			if (!http_android_active(request))
				goto request_done;

			jobject input = env->CallObjectMethod(connection, code >= 400 ? get_error : get_input);
			if (http_android_exception(env, request))
				goto request_done;

			if (input != NULL) {
				char buffer[64 * 1024];
				while (http_request::active(request)) {
					const s32 remaining = http_request::remaining_ms(request);
					if (remaining < 0) {
						http_request::fail(request.id, "HTTP request timed out");
						goto request_done;
					}
					if (request.timeout_ms != 0) {
						env->CallVoidMethod(connection, read_timeout, (jint)remaining);
						if (http_android_exception(env, request))
							goto request_done;
					}

					const jint count = env->CallIntMethod(input, read, bytes);
					if (http_android_exception(env, request))
						goto request_done;

					if (count < 0)
						break;

					env->GetByteArrayRegion(bytes, 0, count, (jbyte *)buffer);

					if (!http_request::body(request, buffer, (u32)count)) {
						http_request::fail(request.id, "HTTP response is too large");
						goto request_done;
					}
				}

				env->CallVoidMethod(input, close_input);
				if (http_android_exception(env, request))
					goto request_done;
			}

			if (http_android_active(request)) {
				http_request::complete(request);
			}
		}
	request_done:
		if (connection != NULL)
			env->CallVoidMethod(connection, disconnect);
		env->ExceptionClear();
		env->PopLocalFrame(NULL);
		if (http_request::active(request))
			http_request::fail(request.id, "Android HTTP request failed");

		http_request::release(&rr->request);
	}

	backend._vm->DetachCurrentThread();
	return 0;
}

namespace http_backend
{
	void *init(Allocator &a)
	{
		HttpAndroid *backend = CE_NEW(a, HttpAndroid)(a);
		backend->_thread.start(http_android_run, backend);
		ScopedMutex sm(backend->_mutex);
		while (!backend->_ready)
			backend->_condition.wait(backend->_mutex);

		return backend;
	}

	void shutdown(void *context)
	{
		HttpAndroid &backend = *(HttpAndroid *)context;
		backend._mutex.lock();
		backend._exit = true;
		backend._condition.signal();
		backend._mutex.unlock();
		backend._thread.stop();
		CE_DELETE(*backend._allocator, &backend);
	}

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		return http_request::create<HttpAndroidRequest>(a, url, content_type, body);
	}

	void start(void *context, HttpRequest *request)
	{
		HttpAndroid &backend = *(HttpAndroid *)context;
		if (backend._failed) {
			http_request::fail(request->id, "Unable to attach Android HTTP worker to Java VM");
			http_request::release(request);
			return;
		}

		HttpAndroidRequest *rr = (HttpAndroidRequest *)request;
		ScopedMutex sm(backend._mutex);
		backend._requests[backend._num_requests++] = rr;
		backend._condition.signal();
	}

	void cancel(void *context)
	{
		CE_UNUSED(context);
	}

} // namespace http_backend

} // namespace crown
#endif // if CROWN_PLATFORM_ANDROID && !CROWN_HTTP_NOOP
