/*
 * streaming_source.c
 *
 *  Created on: 06.01.2017
 *      Author: sefo
 */

#include <streaming_source.h>
#include <init.h>
#include <errors.h>
#include <stdlib.h>
#include <string.h>

struct video_stream_priv {
	int idx;
};

static struct video_streaming_device** video_devices;
static int num_of_video_devices;

uint32_t video_streaming_get_frame_size(struct video_streaming_device* dev)
{
	if (!dev)
		return 0;

	if (dev->ops->get_frame_bpp && \
		dev->ops->get_frame_height && \
		dev->ops->get_frame_width)
		return (dev->ops->get_frame_bpp(dev) >> 3) * dev->ops->get_frame_height(dev) * dev->ops->get_frame_width(dev);

	return 0;
}

struct video_streaming_device* video_streaming_get_device_by_index(int idx)
{
	struct video_streaming_device* vdev = NULL;

	if (idx < num_of_video_devices)
		vdev = video_devices[idx];

	return vdev;
}

struct video_streaming_device* video_streaming_get_device_by_name(char* name)
{
	int i;
	for (i = 0 ; i < num_of_video_devices; ++i) {
		struct video_streaming_device*  vdev = video_devices[i];

		if (strcmp(name, vdev->device_name) == 0)
			return vdev;
	}

	return NULL;
}

int video_streaming_get_device_name_by_index(char** name, int idx)
{
	if (idx < num_of_video_devices) {
		struct video_streaming_device* vdev = video_devices[idx];
		*name = vdev->device_name;
		return 0;
	} else
		return -ENOENT;
}

int register_video_streaming_source(struct video_streaming_device* device)
{
	struct video_stream_priv* priv;

	if (!device || !device->ops || !(device->ops->start_streaming && device->ops->stop_streaming))
		return -EINVALID;

	video_devices = (struct video_streaming_device**) calloc(++num_of_video_devices, sizeof(*device));

	if (!video_devices)
		return -ENOMEM;

	priv = (struct video_stream_priv*) malloc(sizeof(*priv));

	if (!priv)
		return -ENOMEM;

	device->priv = priv;

	priv->idx = num_of_video_devices - 1;
	video_devices[priv->idx] = device;

	return 0;
}

static int streaming_source_init(void)
{
	num_of_video_devices = 0;
	return 0;
}

early_initcall(streaming_source_init);
