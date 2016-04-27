#pragma once
#include "stdafx.h"

uv_connection_cb get_connection_cb(bool pipe_dispatch);
void http_start_read(uv_stream_t* stream);
