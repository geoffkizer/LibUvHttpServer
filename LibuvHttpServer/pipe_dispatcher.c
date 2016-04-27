#include "stdafx.h"
#include "core.h"
#include "pipe_dispatcher.h"

void dispatch_close_cb(uv_handle_t* handle)
{
	void* write_req = handle->data;
	free(handle);
	free(write_req);
}

void dispatch_write_cb(uv_write_t* req, int status)
{
	uv_close((uv_handle_t*)req->data, dispatch_close_cb);
}

void dispatch_connection_cb(uv_stream_t* dispatch_pipe, int status)
{
	uv_pipe_t* peer_pipe;
	uv_loop_t* loop;
	uv_write_t* write_req;
	uv_buf_t buf;
	struct dispatcher_ctx* dispatcher = dispatch_pipe->data;

	loop = dispatch_pipe->loop;
	buf = uv_buf_init("PING", 4);
	peer_pipe = malloc(sizeof(uv_pipe_t));
	write_req = malloc(sizeof(uv_write_t));
	write_req->data = peer_pipe;
	peer_pipe->data = write_req;
	uv_pipe_init(loop, peer_pipe, 1);

	uv_accept(dispatch_pipe, (uv_stream_t*)peer_pipe);
	uv_write2(write_req, (uv_stream_t*)peer_pipe, &buf, 1, (uv_stream_t*)dispatcher->server_handle, dispatch_write_cb);

	dispatcher->consumer_count--;
	if (dispatcher->consumer_count == 0)
		uv_close((uv_handle_t*)dispatch_pipe, NULL);
}