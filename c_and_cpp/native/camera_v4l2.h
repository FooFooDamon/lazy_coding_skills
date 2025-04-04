/*
 * Wrapper struct and functions for camera usage based on V4L2 framework.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CAMERA_V4L2_H__
#define __CAMERA_V4L2_H__

#include <stdbool.h>
#include <stdint.h>

#define CAMERA_V4L2_MAX_BUF_COUNT                       8
#define CAMERA_V4L2_MAX_PLANE_COUNT                     4

struct camera_v4l2;
struct v4l2_buffer;

typedef struct camera_v4l2
{
    /*
     * Methods:
     */
    int (*open)(struct camera_v4l2 *cam, const char *dev_path, bool is_nonblocking/* not used yet */);
    int (*close)(struct camera_v4l2 *cam);
    int (*query_capabilities)(struct camera_v4l2 *cam, bool with_validation);
    /* The value of expected_format should be "auto", or a Four-Character Code (for example: NV12). */
    int (*match_format)(struct camera_v4l2 *cam, const char *expected_format);
    int (*set_size_and_format)(struct camera_v4l2 *cam, uint16_t width, uint16_t height/*, use format matched before*/);
    int (*set_frame_rate)(struct camera_v4l2 *cam, float frames_per_second, float fallback_fps/* suggest: 15.0 */);
    /* An example of io_mode_candidates could be: */
    /*      { V4L2_MEMORY_DMABUF, V4L2_MEMORY_MMAP, V4L2_MEMORY_USERPTR, 0 }, // The last 0 is a sentinel. */
    /* you can reorder or reduce its items depending on the ability of your hareware. */
    /* When it's null, an inner candidates array will be used. */
    int (*alloc_buffers)(struct camera_v4l2 *cam, uint8_t buf_count, const uint32_t io_mode_candidates[]);
    int (*free_buffers_if_any)(struct camera_v4l2 *cam);
    int (*start_capture)(struct camera_v4l2 *cam);
    int (*stop_capture)(struct camera_v4l2 *cam);
    int (*wait_and_fetch)(struct camera_v4l2 *cam, int timeout_msecs/* not used yet */, struct v4l2_buffer *out_frame);
    int (*enqueue_buffer)(struct camera_v4l2 *cam, uint8_t buf_index);

    /*
     * Fields:
     */
    const char *dev_path; /* Path to camera device, for example: /dev/video0 */
    int fd;
    uint32_t fmt_fourcc; /* Four-Character Code of capture format */
    float fps;
    uint16_t width;
    uint16_t height;
    uint32_t capabilities;
    uint32_t buf_type;
    uint32_t buf_sizes[CAMERA_V4L2_MAX_BUF_COUNT][CAMERA_V4L2_MAX_PLANE_COUNT];
    int buf_file_descriptors[CAMERA_V4L2_MAX_BUF_COUNT][CAMERA_V4L2_MAX_PLANE_COUNT];
    unsigned char *buf_pointers[CAMERA_V4L2_MAX_BUF_COUNT][CAMERA_V4L2_MAX_PLANE_COUNT];
    const char *last_func;
    int err; /* 0 or -errno */
    uint32_t io_mode:8; /* V4L2_MEMORY_* */
    uint32_t plane_count:8; /* range: [1, CAMERA_V4L2_MAX_PLANE_COUNT] */
    uint32_t buf_count:8; /* range: [1, CAMERA_V4L2_MAX_BUF_COUNT] */
    uint32_t stream_on:1; /* 0 or 1 */
    uint32_t log_level:7; /* CAMERA_V4L2_LOG_* */
} camera_v4l2_t;

camera_v4l2_t camera_v4l2(const char *log_level/* = "debug", "info", "notice", "warning", "error"*/);

/*
 * Functions below are allowed to be reimplemented for better customization.
 */

int camera_v4l2_validate_capabilities(uint32_t capabilities);

int camera_v4l2_alloc_user_buffers(camera_v4l2_t *cam);
int camera_v4l2_free_user_buffers_if_any(camera_v4l2_t *cam);

int camera_v4l2_acquire_dma_buffers(camera_v4l2_t *cam);
int camera_v4l2_release_dma_buffers_if_any(camera_v4l2_t *cam);

#endif /* #ifndef __CAMERA_V4L2_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-04-04, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

