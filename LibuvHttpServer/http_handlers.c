#include "stdafx.h"
#include "http_handlers.h"
#include "core.h"

#define DO_PIPELINED_RESPONSE (true)

void http_on_close(uv_handle_t* handle)
{
	struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)handle->data;
	return_client(&ctx->clients_pool, (uv_tcp_t*)handle);
}

void http_after_write(uv_write_t* req, int status)
{
	struct consumer_server_ctx* ctx = req->handle->data;
	return_buffer(&ctx->buffers_pool, req->data);
	return_write_req(&ctx->writes_pool, req);
}

void http_handler(uv_stream_t* client)
{
	struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)client->data;
	uv_write_t* write_req = get_write_req(&ctx->writes_pool);
	uv_buf_t* buffer = get_buffer(&ctx->buffers_pool);
	buffer->base = RESPONSE;
	buffer->len = RESPONSE_LENGTH;
	write_req->data = buffer;
	uv_write(write_req, client, buffer, 1, http_after_write);
}

void http_handler_pipelined(uv_stream_t* client)
{
	struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)client->data;
	uv_write_t* write_req = get_write_req(&ctx->writes_pool);
	uv_buf_t* buffer = get_buffer(&ctx->buffers_pool);
	buffer->base = PIPELINED_RESPONSE;
	buffer->len = PIPELINED_RESPONSE_LENGTH;
	write_req->data = buffer;
	uv_write(write_req, client, buffer, 1, http_after_write);
}

void http_on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
	if (nread >= 0)
	{
		if (DO_PIPELINED_RESPONSE)
		{
			http_handler_pipelined(tcp);
		}
		else
		{
			for (int i = 0; i < PIPELINE_DEPTH; i++)
			{
				http_handler(tcp);
			}
		}
	}
	else
	{
		uv_close((uv_handle_t*)tcp, http_on_close);
	}

	if (buf->base != NULL)
	{
		struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)tcp->data;
		return_bytes(&ctx->bytes_pool, buf->base);
	}
}

void http_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf)
{
	struct consumer_server_ctx* ctx = (struct consumer_server_ctx*)client->data;
	char* bytes = get_bytes(&ctx->bytes_pool);
	*buf = uv_buf_init(bytes, (unsigned int)suggested_size);
}

void http_on_connect(uv_stream_t* stream, int status)
{
	static int index = -1;
	int next = (index + 1) % THREADS;
	index = next;
	struct consumer_server_ctx* consumer = ((struct consumer_server_ctx*)stream->data) + next;
	uv_async_send(&consumer->async_handle);
	uv_sem_wait(&consumer->semaphore);
}

void http_on_connect_pipe_dispatch(uv_stream_t* stream, int status)
{
	uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	client->data = stream->data;
	uv_tcp_init(stream->loop, client);
	uv_accept(stream, (uv_stream_t*)client);
	uv_read_start((uv_stream_t*)client, http_on_alloc, http_on_read);
}

void http_start_read(uv_stream_t* stream)
{
	uv_read_start(stream, http_on_alloc, http_on_read);
}

uv_connection_cb get_connection_cb(bool pipe_dispatch)
{
	return pipe_dispatch ? http_on_connect_pipe_dispatch : http_on_connect;
}