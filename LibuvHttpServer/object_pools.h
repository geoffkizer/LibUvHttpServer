#pragma once
#include "stdafx.h"

struct buffer_node
{
	uv_buf_t buffer;
	struct buffer_node* next;
};
C_ASSERT(offsetof(struct buffer_node, buffer) == 0);

struct write_node
{
	uv_write_t write;
	struct write_node* next;
};
C_ASSERT(offsetof(struct write_node, write) == 0);

struct bytes_node
{
	char bytes[65536];
	struct bytes_node* next;
};
C_ASSERT(offsetof(struct bytes_node, bytes) == 0);

struct client_node
{
	uv_tcp_t client;
	struct client_node* next;
};
C_ASSERT(offsetof(struct client_node, client) == 0);

uv_buf_t* get_buffer(struct buffer_node** head);
void return_buffer(struct buffer_node** head, uv_buf_t* buffer);

uv_write_t* get_write_req(struct write_node** head);
void return_write_req(struct write_node** head, uv_write_t* write);

char* get_bytes(struct bytes_node** head);
void return_bytes(struct bytes_node** head, char* bytes);

uv_tcp_t* get_client(struct client_node** head);
void return_client(struct client_node** head, uv_tcp_t* bytes);