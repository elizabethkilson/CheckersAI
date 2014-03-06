#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "checkers.c"
#include "checkers_comp.c"
#define PORT 8888
#define POSTBUFFERSIZE 512
#define GET 0
#define POST 1

#define PAGEMISSING "<html><head><title>File not found</title></head><body>File not found</body></html>"
#define uint32NOTSET 4294967295
#define dNOTSET -1
#define turnNOTSET 2
#define MIMETYPE "application/javascript"

struct connection_info_struct
{
	int connectiontype;
	uint32_t occupied;
	uint32_t player;
	uint32_t king;
	int k_val;
	int p_val;
	int prop_val;
	float time;
	uint8_t turn;
	bool ready;
	char *type;
	char *answer_string;
	struct MHD_PostProcessor *postprocessor;
};

static int
send_data (struct MHD_Connection *connection, const char *data)
{
	int ret;
	struct MHD_Response *response;
	response = MHD_create_response_from_buffer (strlen (data), (void *) data,
		MHD_RESPMEM_PERSISTENT);
	if (!response)
		return MHD_NO;
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
}

static int
send_file (struct MHD_Connection *connection, const char *url)
{
	struct MHD_Response *response;
	int fd;
	int ret;
	struct stat sbuf;

	if ( (-1 == (fd = open (++url, O_RDONLY))) ||
       (0 != fstat (fd, &sbuf)) )
    {
    	/* error accessing file */
    	return send_data (connection, PAGEMISSING);
    }
	response = MHD_create_response_from_fd (sbuf.st_size, fd);
	char * it = url;
	while (*it)
		it++;
	it = it - 3;
	if (0==strcmp(it, ".js"))
	{
		MHD_add_response_header (response, "Content-Type", MIMETYPE);
	}
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}

static int
iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
	const char *filename, const char *content_type,
	const char *transfer_encoding, const char *data, uint64_t off,
	size_t size)
{
	struct connection_info_struct *con_info = coninfo_cls;
	if (0 == strcmp (key, "occupied"))
	{
		if (size > 0)
		{
			uint32_t o = (uint32_t) strtoul(data, (char**)NULL, 2);
			con_info->occupied = o;
		}
	}
	else if (0 == strcmp (key, "player"))
	{
		if (size > 0)
		{
			uint32_t p = (uint32_t) strtoul(data, (char**)NULL, 2);
			con_info->player = p;
		}
	}
	else if (0 == strcmp (key, "king"))
	{
		if (size > 0)
		{
			uint32_t k = (uint32_t) strtoul(data, (char**)NULL, 2);
			con_info->king = k;
		}
	}
	else if (0 == strcmp (key, "k_val"))
	{
		if (size > 0)
		{
			int k = (int) strtol(data, (char**)NULL, 10);
			con_info->k_val = k;
		}
	}
	else if (0 == strcmp (key, "p_val"))
	{
		if (size > 0)
		{
			int k = (int) strtol(data, (char**)NULL, 10);
			con_info->p_val = k;
		}
	}
	else if (0 == strcmp (key, "prop_val"))
	{
		if (size > 0)
		{
			int k = (int) strtol(data, (char**)NULL, 10);
			con_info->prop_val = k;
		}
	}
	else if (0 == strcmp (key, "time"))
	{
		if (size > 0)
		{
			float t = strtof(data, (char**)NULL);
			con_info->time = t;
		}
	}
	else if (0 == strcmp (key, "turn"))
	{
		if (size > 0)
		{
			uint8_t t = (uint8_t) strtoul(data, (char**)NULL, 10);
			con_info->turn = t;
		}
	}
	else if (0 == strcmp (key, "type"))
	{
		if (size > 0)
		{
			char *type;
			type = malloc (MAXANSWERSIZE);
			if (!type)
				return MHD_NO;
			snprintf (type, MAXANSWERSIZE, data);
			con_info->type = type;
		}
		else
			con_info->type = NULL;
	}
	if (con_info->type == NULL)
	{
		return MHD_YES;
	}
	if ((0 == strcmp (con_info->type, "legalMoves"))&&\
		(con_info->occupied != uint32NOTSET)&&\
		(con_info->player != uint32NOTSET)&&\
		(con_info->king != uint32NOTSET)&&
		(con_info->turn < turnNOTSET))
	{
		con_info->ready = true;
		return MHD_NO;
	}
	if ((0 == strcmp (con_info->type, "compMove"))&&\
		(con_info->occupied != uint32NOTSET)&&\
		(con_info->player != uint32NOTSET)&&\
		(con_info->king != uint32NOTSET)&&\
		(con_info->k_val != uint32NOTSET)&&\
		(con_info->p_val != uint32NOTSET)&&\
		(con_info->prop_val != uint32NOTSET)&&\
		(con_info->k_val >= 0)&&\
		(con_info->p_val >= 0)&&\
		(con_info->prop_val >= 0)&&\
		(con_info->time != dNOTSET)&&
		(con_info->turn < turnNOTSET))
	{
		con_info->ready = true;
		return MHD_NO;
	}
	return MHD_YES;
}


static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
	const char *url, const char *method,
	const char *version, const char *upload_data,
	size_t *upload_data_size, void **con_cls)
{
	if (NULL == *con_cls)
	{
		struct connection_info_struct *con_info;
		con_info = malloc (sizeof (struct connection_info_struct));
		if (NULL == con_info)
		return MHD_NO;
		con_info->occupied = uint32NOTSET;
		con_info->player = uint32NOTSET;
		con_info->king = uint32NOTSET;
		con_info->k_val = uint32NOTSET;
		con_info->p_val = uint32NOTSET;
		con_info->prop_val = uint32NOTSET;
		con_info->time = dNOTSET;
		con_info->turn = turnNOTSET;
		con_info->ready = false;
		con_info->type = NULL;
		if (0 == strcmp (method, "POST"))
		{
			con_info->postprocessor =
				MHD_create_post_processor (connection, POSTBUFFERSIZE,
					iterate_post, (void *) con_info);
			if (NULL == con_info->postprocessor)
			{
				free (con_info);
				return MHD_NO;
			}
			con_info->connectiontype = POST;
		}
		else
			con_info->connectiontype = GET;
		*con_cls = (void *) con_info;
		return MHD_YES;
	}
	if (0 == strcmp (method, "GET"))
	{
		return send_file (connection, url);
	}
	if (0 == strcmp (method, "POST"))
	{
		struct connection_info_struct *con_info = *con_cls;
		if (*upload_data_size != 0)
		{
			MHD_post_process (con_info->postprocessor, upload_data,
				*upload_data_size);
			*upload_data_size = 0;
			return MHD_YES;
		}
		else if (con_info->ready)
		{
			if (0==strcmp(con_info->type, "legalMoves"))
			{
				con_info->answer_string= legal_moves_string(con_info->occupied,\
					con_info->player, con_info->king, con_info->turn);
				return send_data(connection, con_info->answer_string);
			}
			if (0==strcmp(con_info->type, "compMove"))
			{
				con_info->answer_string = aBsearch(con_info->occupied, \
					con_info->player, con_info->king, con_info->turn, \
					con_info->time, con_info->k_val, con_info->p_val, \
					con_info->prop_val);
				return send_data(connection, con_info->answer_string);
			}
			return send_data (connection, "Success");
		}
	}
	return send_data (connection, PAGEMISSING);
}

static void
request_completed (void *cls, struct MHD_Connection *connection,
	void **con_cls, enum MHD_RequestTerminationCode toe)
{
	struct connection_info_struct *con_info = *con_cls;
	if (NULL == con_info)
		return;
	if (con_info->connectiontype == POST)
	{
		MHD_destroy_post_processor (con_info->postprocessor);
		if (con_info->answer_string)
			free (con_info->answer_string);
	}
	free (con_info);
	*con_cls = NULL;
	return;
}

int
main ()
{
	struct MHD_Daemon *daemon;
	daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
		&answer_to_connection, NULL,
		MHD_OPTION_NOTIFY_COMPLETED, request_completed,
		NULL, MHD_OPTION_END);
	if (NULL == daemon)
		return 1;
	while(1)
	{
		sleep(1);
	}
	MHD_stop_daemon (daemon);
	return 0;
}



