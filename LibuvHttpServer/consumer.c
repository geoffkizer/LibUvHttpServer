#include "stdafx.h"
#include "core.h"
#include "consumer.h"
#include "http_handlers.h"

void consumer_async(uv_async_t* handle)
{
	struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)handle->data;
	uv_stream_t* accept_socket = (uv_stream_t*)get_client(&ctx->clients_pool);
	accept_socket->data = ctx;
	uv_tcp_init(ctx->loop, (uv_tcp_t*)accept_socket);
	uv_accept(ctx->main_server, accept_socket);
	uv_sem_post(&ctx->semaphore);
	//uv_read_start(acceptSocket, http_on_alloc, http_on_read);
	http_start_read(accept_socket);
}

void consume_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	uv_loop_t* loop;
	uv_handle_type type;
	uv_pipe_t* pipe;
	struct consumer_client_ctx* ctx = handle->data;
	pipe = (uv_pipe_t*)handle;
	loop = pipe->loop;

	type = uv_pipe_pending_type(pipe);

	if (type == UV_TCP) {
		uv_tcp_init(loop, ctx->server_handle);
	}
	else if (type == UV_NAMED_PIPE)
	{
		assert(0);
	}

	uv_accept(handle, (uv_stream_t*)ctx->server_handle);
	uv_close((uv_handle_t*)&ctx->pipe, NULL);
}

void consume_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	struct consumer_client_ctx* ctx = handle->data;
	buf->base = ctx->scratch;
	buf->len = sizeof(ctx->scratch);
}

void consume_connect_cb(uv_connect_t* req, int status)
{
	struct consumer_client_ctx* ctx = req->data;
	uv_read_start((uv_stream_t*)&ctx->pipe, consume_alloc_cb, consume_read_cb);
}

void consume_get_listen_handle(uv_loop_t* loop, uv_tcp_t* server_handle, struct consumer_server_ctx* server_ctx)
{
	struct consumer_client_ctx ctx;
	ctx.pipe.data = &ctx;
	ctx.connect_req.data = &ctx;
	ctx.server_handle = server_handle;
	ctx.server_handle->data = server_ctx;

	uv_pipe_init(loop, &ctx.pipe, 1);
	uv_pipe_connect(&ctx.connect_req, &ctx.pipe, PIPE_NAME, consume_connect_cb);
	uv_run(loop, UV_RUN_DEFAULT);
}

void connection_consumer_start(void *arg)
{
	struct consumer_server_ctx* ctx;
	uv_loop_t* loop;

	ctx = arg;
	loop = ctx->loop;

	uv_barrier_wait(ctx->created_barrier);

	if (ctx->pipe_dispatch)
	{
		uv_sem_wait(&ctx->semaphore);
		consume_get_listen_handle(loop, &ctx->server_handle, ctx);
		uv_sem_post(&ctx->semaphore);

		uv_listen((uv_stream_t*)&ctx->server_handle, SOMAXCONN, get_connection_cb(true));
	}
	else
	{
		uv_async_init(loop, &ctx->async_handle, consumer_async);
		ctx->async_handle.data = ctx;
		uv_sem_wait(&ctx->semaphore);
	}

	uv_run(loop, UV_RUN_DEFAULT);

	uv_loop_delete(loop);
}









