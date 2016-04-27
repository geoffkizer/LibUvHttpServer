#include "stdafx.h"
#include "object_pools.h"


// object pools
uv_buf_t* get_buffer(struct buffer_node** head)
{
	uv_buf_t* ret;
	if (*head == NULL)
	{
		struct buffer_node* node = malloc(sizeof(struct buffer_node));
		memset(node, 0, sizeof(struct buffer_node));
		ret = &node->buffer;
	}
	else
	{
		struct buffer_node* temp = *head;
		*head = (*head)->next;
		temp->next = NULL;
		ret = &temp->buffer;
	}

	return ret;
}

void return_buffer(struct buffer_node** head, uv_buf_t* buffer)
{
	buffer->base = NULL;
	buffer->len = 0;
	struct buffer_node* node = (struct buffer_node*)buffer;
	node->next = *head;
	*head = node;
}

uv_write_t* get_write_req(struct write_node** head)
{
	uv_write_t* ret;
	if (*head == NULL)
	{
		struct write_node* node = malloc(sizeof(struct write_node));
		memset(node, 0, sizeof(struct write_node));
		ret = &node->write;
	}
	else
	{
		struct write_node* temp = *head;
		*head = (*head)->next;
		temp->next = NULL;
		ret = &temp->write;
	}

	return ret;
}

void return_write_req(struct write_node** head, uv_write_t* write)
{
	memset(write, 0, sizeof(uv_write_t));
	struct write_node* node = (struct write_node*)write;
	node->next = *head;
	*head = node;
}

char* get_bytes(struct bytes_node** head)
{
	char* ret;
	if (*head == NULL)
	{
		struct bytes_node* node = malloc(sizeof(struct bytes_node));
		memset(node, 0, sizeof(struct bytes_node));
		ret = node->bytes;
	}
	else
	{
		struct bytes_node* temp = *head;
		*head = (*head)->next;
		temp->next = NULL;
		ret = temp->bytes;
	}

	return ret;
}

void return_bytes(struct bytes_node** head, char* bytes)
{
	struct bytes_node* node = (struct bytes_node*)bytes;
	node->next = *head;
	*head = node;
}

uv_tcp_t* get_client(struct client_node** head)
{
	uv_tcp_t* ret;
	if (*head == NULL)
	{
		struct client_node* node = malloc(sizeof(struct client_node));
		memset(node, 0, sizeof(struct client_node));
		ret = &node->client;
	}
	else
	{
		struct client_node* temp = *head;
		*head = (*head)->next;
		temp->next = NULL;
		ret = &temp->client;
	}

	return ret;
}

void return_client(struct client_node** head, uv_tcp_t* client)
{
	memset(client, 0, sizeof(uv_tcp_t));
	struct client_node* node = (struct client_node*)client;
	node->next = *head;
	*head = node;
}