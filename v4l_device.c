/*
 * v4l_device.c
 *
 *  Created on: 06.01.2017
 *      Author: sefo
 */

#include <linux/videodev2.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <streaming_source.h>
#include <init.h>
#include <helper.h>
#include <debug.h>


static uint32_t v4l_dev_get_frame_width(struct video_streaming_device* sdev);
static uint32_t v4l_dev_get_frame_height(struct video_streaming_device* sdev);
static uint32_t v4l_dev_get_frame_bpp(struct video_streaming_device* sdev);
static void* v4l_dev_get_one_frame(struct video_streaming_device* sdev);
static int v4l_dev_start(struct video_streaming_device* sdev);
static int v4l_dev_stop(struct video_streaming_device* sdev);

#define to_v4l_device(__sdev) \
		container_of(__sdev, struct v4l_video_device, video_stream_dev)

struct v4l_video_device
{
	struct v4l2_buffer bufferinfo;
	struct v4l2_capability cap;
	int fd;
	struct v4l2_format format;
	void* buffer;
	int buf_type;
	int mem_type;
	struct video_streaming_device video_stream_dev;
};

static struct video_streaming_ops v4l_ops = { .get_frame_width = v4l_dev_get_frame_width, .get_frame_height =
		v4l_dev_get_frame_height, .get_frame_bpp = v4l_dev_get_frame_bpp, .get_one_frame = v4l_dev_get_one_frame,
		.start_streaming = v4l_dev_start, .stop_streaming = v4l_dev_stop };

static int v4l_dev_stop(struct video_streaming_device* sdev)
{
	struct v4l_video_device* vdev = to_v4l_device(sdev);
	int type = vdev->buf_type;

	if (ioctl(vdev->fd, VIDIOC_STREAMOFF, &type) < 0) {
		perror("VIDIOC_STREAMOFF");
		return -EINVAL;
	}

	return 0;
}

static int v4l_dev_start(struct video_streaming_device* sdev)
{
	struct v4l_video_device* vdev = to_v4l_device(sdev);
	int type;

	memset(&vdev->bufferinfo, 0, sizeof(vdev->bufferinfo));

	vdev->bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdev->bufferinfo.memory = V4L2_MEMORY_MMAP;
	vdev->bufferinfo.index = 0;

	type = vdev->bufferinfo.type;
	if (ioctl(vdev->fd, VIDIOC_STREAMON, &type) < 0) {
		perror("VIDIOC_QUERYCAP");
		return -EINVAL;
	}
	return 0;
}

static void* v4l_dev_get_one_frame(struct video_streaming_device* sdev)
{
	struct v4l_video_device* vdev = to_v4l_device(sdev);

	if (ioctl(vdev->fd, VIDIOC_QBUF, &vdev->bufferinfo) < 0) {
		perror("VIDIOC_QBUF");
		goto err_out;
	}

	if (ioctl(vdev->fd, VIDIOC_DQBUF, &vdev->bufferinfo) < 0) {
		perror("VIDIOC_DQBUF");
		goto err_out;
	}

	/* only for a full length frame -> give the buffer*/
	if (vdev->bufferinfo.length == vdev->bufferinfo.bytesused)
		return vdev->buffer;

	err_out: return NULL;
}

static uint32_t v4l_dev_get_frame_bpp(struct video_streaming_device* sdev)
{
	/* FIXME: */
	return 16;
}

static uint32_t v4l_dev_get_frame_height(struct video_streaming_device* sdev)
{
	struct v4l_video_device* vdev = to_v4l_device(sdev);
	return vdev->format.fmt.pix.height;
}

static uint32_t v4l_dev_get_frame_width(struct video_streaming_device* sdev)
{
	struct v4l_video_device* vdev = to_v4l_device(sdev);
	return vdev->format.fmt.pix.width;
}

static int open_video_device(struct v4l_video_device *vdev, char* dev_string)
{
	struct v4l2_requestbuffers bufrequest;

	if ((vdev->fd = open(dev_string, O_RDWR)) < 0) {
		perror("open");
		return -EINVAL;
	}

	if (ioctl(vdev->fd, VIDIOC_QUERYCAP, &vdev->cap) < 0) {
		perror("VIDIOC_QUERYCAP");
		exit(1);
	}

	if ((vdev->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		debug("The device can handle single-planar video capture.\n");
	}

	if ((vdev->cap.capabilities & V4L2_CAP_STREAMING)) {
		debug("The device can stream.\n");
	}

	if ((vdev->cap.capabilities & V4L2_CAP_READWRITE)) {
		debug("The device can handle read/write syscalls.\n");
	}
	vdev->buf_type = vdev->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdev->mem_type = V4L2_MEMORY_MMAP;
	/* FIXME: */
	vdev->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	/* FIXME */
	vdev->format.fmt.pix.width = 640;
	vdev->format.fmt.pix.height = 480;
	if (ioctl(vdev->fd, VIDIOC_S_FMT, &vdev->format) < 0) {
		perror("VIDIOC_S_FMT");
		return -EINVAL;
	}

	bufrequest.type = vdev->buf_type;
	bufrequest.memory = vdev->mem_type;
	bufrequest.count = 1;
	if (ioctl(vdev->fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
		perror("VIDIOC_REQBUFS");
		return -EINVAL;
	}

	memset(&vdev->bufferinfo, 0, sizeof(vdev->bufferinfo));
	vdev->bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdev->bufferinfo.memory = V4L2_MEMORY_MMAP;
	vdev->bufferinfo.index = 0;
	if (ioctl(vdev->fd, VIDIOC_QUERYBUF, &vdev->bufferinfo) < 0) {
		perror("VIDIOC_QUERYBUF");
		return -EINVAL;
	}

	vdev->buffer = mmap( NULL, vdev->bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, vdev->fd,
			vdev->bufferinfo.m.offset);
	if (vdev->buffer == MAP_FAILED) {
		perror("mmap");
		return EFAULT;
	}

	memset(vdev->buffer, 0, vdev->bufferinfo.length);
	return 0;
}

static int v4l_init(void)
{
	struct v4l_video_device *vdev = malloc(sizeof(*vdev));
	struct video_streaming_device* sdev;

	if (!vdev)
		return -ENOMEM;

	sdev = &vdev->video_stream_dev;

	snprintf(sdev->device_name, sizeof(sdev->device_name), "v2l_streaming_device");
	sdev->ops = &v4l_ops;

	/* FIXME: search all*/
	open_video_device(vdev, "/dev/video0");
	return register_video_streaming_source(sdev);
}

module_init(v4l_init);

