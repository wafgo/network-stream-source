/*
 * main.c
 *
 *  Created on: 03.01.2017
 *      Author: sefo
 */

/* server.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <init.h>
#include <streaming_source.h>
#include <debug.h>

enum serverCommand{
	SRV_CMD_GET_FRAME_SIZE = 4, SRV_CMD_GET_IMG_WIDTH, SRV_CMD_GET_IMG_HEIGHT, SRV_CMD_GET_BPP, SVR_CMD_GET_FRAME
};

static int send_response(int socket, uint32_t len, void* data)
{
	int remaining = len;
	int size;
	char* cd = (char*) data;

	do {
		size = send(socket, cd, remaining, 0);
		remaining -= size;
		cd += size;
	} while (remaining);

	return 0;
}

static int read_header(int socket, uint32_t* cmd, uint32_t* len)
{
	int size;
	char dta[8];
	char* pd = (char*) dta;
	int remaining = sizeof(dta);

	do {
		size = recv(socket, pd, sizeof(dta), 0);
		remaining -= size;
		pd += size;
	} while (remaining);

	if (cmd)
		memcpy(cmd, &dta[0], 4);
	if (len)
		memcpy(len, &dta[4], 4);

	return 0;
}

static int write_response(struct video_streaming_device* sdev, int socket, uint32_t cmd, uint32_t len)
{
	switch (cmd) {
	case SRV_CMD_GET_FRAME_SIZE: {
		uint32_t size = video_streaming_get_frame_size(sdev);
		send_response(socket, len, &size);
	}
		break;
	case SRV_CMD_GET_IMG_WIDTH: {
		uint32_t width = video_streaming_get_frame_width(sdev);
		send_response(socket, len, &width);
	}
		break;
	case SRV_CMD_GET_IMG_HEIGHT: {
		uint32_t height = video_streaming_get_frame_height(sdev);
		send_response(socket, len, &height);
	}
		break;
	case SVR_CMD_GET_FRAME: {
		char* buff = (char *)sdev->ops->get_one_frame(sdev);
		send_response(socket, len, buff);
	}
		break;

	case SRV_CMD_GET_BPP: {
		uint32_t bpp = video_streaming_get_frame_bpp(sdev);
		send_response(socket, len, &bpp);
	}
		break;
	default:
		if (sdev->ops->decode)
			return sdev->ops->decode(sdev, cmd);
		else
			debug("Unknown command received (%d)\n", cmd);
		break;
	}

	return 0;
}

static int open_socket(struct sockaddr_in* address)
{
	int y = 1;
	int _socket;

	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
		debug("socket created\n");

	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(15000);
	if (bind(_socket, (struct sockaddr*) &*address, sizeof(*address)) != 0) {
		printf("port is currently busy!\n");
	}
	listen(_socket, 5);

	return _socket;
}

static int wait_for_client(int create_socket, struct sockaddr_in* address)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int new_socket = accept(create_socket, (struct sockaddr*) address, &addrlen);
	if (new_socket > 0)
		debug("client connected (%s)\n", inet_ntoa(address->sin_addr));

	return new_socket;
}

int main(void)
{
	int create_socket, new_socket;
	struct sockaddr_in address;
	struct video_streaming_device* sdev;

	do_initcalls();

	sdev = video_streaming_get_device_by_index(0);

	if (!sdev)
		printf("No streaming Device found");

	debug("Server started\n");
	debug("using streaming device: %s\n", sdev->device_name);

	create_socket = open_socket(&address);


	while (1) {
		new_socket = wait_for_client(create_socket, &address);

		video_streaming_start(sdev);

		do {
			static int frm_cnt = 0;
			uint32_t cmd, len;
			int err = read_header(new_socket, &cmd, &len);

			if (err == 0)
				debug("message_received: cmd = %d len = %d (%d)\n", cmd, len, frm_cnt++);

			write_response(sdev, new_socket, cmd, len);

		} while (1);

		close(new_socket);
	}

	close(create_socket);
	return EXIT_SUCCESS;
}
