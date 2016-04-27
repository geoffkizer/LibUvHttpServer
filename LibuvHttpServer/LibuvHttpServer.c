// LibuvHttpServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "core.h"
#include "http_handlers.h"
#include "consumer.h"
#include "pipe_dispatcher.h"

bool PIPE_DISPATCH = false;

int main()
{
	// declare
	uv_loop_t* main_loop;
	uv_tcp_t main_server;
	struct sockaddr_in listen_address;
	uv_barrier_t* listeners_created_barrier;
	uv_pipe_t pipe;
	struct dispatcher_ctx dispatcher;	
	uv_async_t* service_handle;

	// initialize
	if (PIPE_DISPATCH)
	{
		dispatcher.consumer_count = THREADS;
		dispatcher.server_handle = &main_server;
		pipe.data = &dispatcher;
	}

	main_loop = uv_default_loop();

	listeners_created_barrier = malloc(sizeof(uv_barrier_t));
	uv_barrier_init(listeners_created_barrier, THREADS + 1);

	service_handle = malloc(sizeof(uv_async_t));
	uv_async_init(main_loop, service_handle, NULL);

	struct consumer_server_ctx* consumers;
	consumers = calloc(THREADS, sizeof(consumers[0]));
	for (int i = 0; i < THREADS; i++)
	{
		struct consumer_server_ctx* ctx = consumers + i;
		ctx->index = i;
		ctx->created_barrier = listeners_created_barrier;
		ctx->loop = uv_loop_new();
		ctx->main_server = (uv_stream_t*)&main_server;
		ctx->buffers_pool = NULL;
		ctx->writes_pool = NULL;
		ctx->bytes_pool = NULL;
		ctx->clients_pool = NULL;
		ctx->pipe_dispatch = PIPE_DISPATCH;
		uv_sem_init(&ctx->semaphore, 0);
		uv_thread_create(&ctx->thread_id, connection_consumer_start, ctx);
	}
	uv_barrier_wait(listeners_created_barrier);
	
	uv_tcp_init(main_loop, &main_server);
	uv_ip4_addr("0.0.0.0", 8000, &listen_address);
	main_server.data = consumers;
	uv_tcp_bind(&main_server, (const struct sockaddr*)&listen_address, 0);

	if (PIPE_DISPATCH)
	{
		uv_pipe_init(main_loop, &pipe, 0);
		uv_pipe_bind(&pipe, PIPE_NAME);
		uv_listen((uv_stream_t*)&pipe, SOMAXCONN, dispatch_connection_cb);
	}

	for (int i = 0; i < THREADS; i++)
	{
		uv_sem_post(&consumers[i].semaphore);
	}

	// run
	if (PIPE_DISPATCH)
	{
		uv_run(main_loop, UV_RUN_DEFAULT);
		uv_close((uv_handle_t*)&main_server, NULL);
		uv_run(main_loop, UV_RUN_DEFAULT);
	}
	else
	{
		uv_listen((uv_stream_t*)&main_server, SOMAXCONN, get_connection_cb(false));
		uv_run(main_loop, UV_RUN_DEFAULT);
	}
    return 0;
}



