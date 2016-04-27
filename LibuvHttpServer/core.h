#pragma once
#include "stdafx.h"
#include "object_pools.h"

#define RESPONSE "HTTP/1.1 200 OK\r\nServer: Libuv/TechEmpowerPlain\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nhello world\r\n"
#define RESPONSE_LENGTH (sizeof(RESPONSE) - 1)		// Don't include null terminator
#define PIPELINED_RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE RESPONSE
#define PIPELINED_RESPONSE_LENGTH (sizeof(PIPELINED_RESPONSE) - 1)		// Don't include null terminator
#define THREADS 48
#define PIPELINE_DEPTH 16
#define PIPE_NAME "\\\\.\\pipe\\TEST_PIPENAME"

struct consumer_server_ctx
{
	int index;
	bool pipe_dispatch;
	uv_async_t async_handle;
	uv_thread_t thread_id;
	uv_sem_t semaphore;
	uv_barrier_t* created_barrier;
	uv_tcp_t server_handle;
	uv_loop_t* loop;
	uv_stream_t* main_server;
	struct buffer_node* buffers_pool;
	struct write_node* writes_pool;
	struct bytes_node* bytes_pool;
	struct client_node* clients_pool;
};

struct consumer_client_ctx
{
	uv_connect_t connect_req;
	uv_tcp_t* server_handle;
	uv_pipe_t pipe;
	char scratch[16];
};

struct dispatcher_ctx
{
	uv_tcp_t* server_handle;
	int consumer_count;
};

