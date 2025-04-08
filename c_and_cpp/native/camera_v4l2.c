/*
 * Wrapper functions for camera usage based on V4L2 framework.
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

#include "camera_v4l2.h"

#include <errno.h>
#include <string.h> /* For memset() and str*() */
#include <strings.h> /* For strcasecmp() */
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h> /* For open(). */
#include <unistd.h> /* For close(). */
#include <linux/videodev2.h> /* For V4L2_MEMORY_* and VIDIOC_*. */
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef NO_DEFAULT_FMT_LOG
/*
 * One example of fmt_log.h based on Qt console logging:
 *
 * #include "../qt/qt_print.hpp"
 *
 * #define qtD                                          qtDebug
 * #define qtDV                                         qtDebugV
 *
 * #define qtI                                          qtInfo
 * #define qtIV                                         qtInfoV
 *
 * #define qtN                                          qtNotice
 * #define qtNV                                         qtNoticeV
 *
 * #define qtW                                          qtWarn
 * #define qtWV                                         qtWarnV
 *
 * #define qtE                                          qtErr
 * #define qtEV                                         qtErrV
 *
 * #define FMT_LOG(_filter_, _tag_, _fmt_, ...)         qt##_tag_(_fmt_, ##__VA_ARGS__)
 * #define FMT_LOG_V(_filter_, _tag_, _fmt_, ...)       qt##_tag_##V(::, _fmt_, ##__VA_ARGS__)
 */
#include "fmt_log.h"
#else
#include "formatted_logging_adapter.h"
#endif

#define CAM_LOG_DEBUG(_fmt_, ...)               FMT_LOG(cam, D, _fmt_, __VA_ARGS__)
#define CAM_LOG_DEBUG_V(_fmt_, ...)             FMT_LOG_V(cam, D, _fmt_, __VA_ARGS__)

#define CAM_LOG_INFO(_fmt_, ...)                FMT_LOG(cam, I, _fmt_, __VA_ARGS__)
#define CAM_LOG_INFO_V(_fmt_, ...)              FMT_LOG_V(cam, I, _fmt_, __VA_ARGS__)

#define CAM_LOG_NOTICE(_fmt_, ...)              FMT_LOG(cam, N, _fmt_, __VA_ARGS__)
#define CAM_LOG_NOTICE_V(_fmt_, ...)            FMT_LOG_V(cam, N, _fmt_, __VA_ARGS__)

#define CAM_LOG_WARN(_fmt_, ...)                FMT_LOG(cam, W, _fmt_, __VA_ARGS__)
#define CAM_LOG_WARN_V(_fmt_, ...)              FMT_LOG_V(cam, W, _fmt_, __VA_ARGS__)

#define CAM_LOG_ERR(_fmt_, ...)                 FMT_LOG(cam, E, _fmt_, __VA_ARGS__)
#define CAM_LOG_ERR_V(_fmt_, ...)               FMT_LOG_V(cam, E, _fmt_, __VA_ARGS__)

#define RESET_OBJECT_STATUS_FIELDS(obj)         do { \
    (obj)->last_func = __func__; \
    (obj)->err = 0; \
} while (0)

static int cam_v4l2_open(struct camera_v4l2 *cam, const char *dev_path, bool is_nonblocking)
{
    RESET_OBJECT_STATUS_FIELDS(cam);

    if (NULL == dev_path)
        return (cam->err = -EINVAL);

    if ((cam->fd = open(dev_path, O_RDWR)) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** Failed to open(%s): %s", dev_path, strerror(-cam->err));
    }
    else
    {
        cam->dev_path = dev_path;
        CAM_LOG_NOTICE("Opened video device successfully: path = %s, fd = %d.", cam->dev_path, cam->fd);
    }

    return cam->err;
}

static int cam_v4l2_close(struct camera_v4l2 *cam)
{
    if (cam->fd < 0)
        return (cam->err = -EBADF);

    if (cam->stream_on)
        cam->stop_capture(cam);

    RESET_OBJECT_STATUS_FIELDS(cam);

    if (0 == close(cam->fd))
    {
        CAM_LOG_NOTICE("Closed video device: path = %s, fd = %d.", cam->dev_path, cam->fd);
        cam->fd = -1;
    }
    else
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to close: %s", cam->dev_path, strerror(-cam->err));
    }

    return cam->err;
}

static int cam_v4l2_query_capability(struct camera_v4l2 *cam, bool with_validation)
{
    struct v4l2_capability result;

    RESET_OBJECT_STATUS_FIELDS(cam);

    if (ioctl(cam->fd, VIDIOC_QUERYCAP, &result) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to query capability: %s", cam->dev_path, strerror(-cam->err));

        return cam->err;
    }

    CAM_LOG_INFO_V("Compatibility of %s:", cam->dev_path);
    CAM_LOG_INFO_V("    driver: %s", result.driver);
    CAM_LOG_INFO_V("    card: %s", result.card);
    CAM_LOG_INFO_V("    bus info: %s", result.bus_info);
    CAM_LOG_INFO_V("    version: 0x%X", result.version);
    CAM_LOG_INFO_V("    capabilities (whole): 0x%X", result.capabilities);
    CAM_LOG_INFO_V("    capabilities (this): 0x%X", result.device_caps);

    cam->capabilities = (0 == (result.capabilities & V4L2_CAP_DEVICE_CAPS))
        ? result.capabilities : result.device_caps;

    if (with_validation && (cam->err = camera_v4l2_validate_capabilities(cam->capabilities)) < 0)
        return cam->err;

    if (0 != (cam->capabilities & V4L2_CAP_VIDEO_CAPTURE))
        cam->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else if (0 != (cam->capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        cam->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        CAM_LOG_NOTICE("%s supports the multi-planar API", cam->dev_path);
    }
    else
    {
        cam->err = -ENOTSUP;
        CAM_LOG_ERR_V("*** %s: Video capture not supported!", cam->dev_path);
    }

    return cam->err;
}

#define PIXFMT_TO_FOURCC(pixfmt, fourcc_str)    do { \
    fourcc_str[0] = (pixfmt) & 0xff; \
    fourcc_str[1] = ((pixfmt) >> 8) & 0xff; \
    fourcc_str[2] = ((pixfmt) >> 16) & 0xff; \
    fourcc_str[3] = ((pixfmt) >> 24) & 0xff; \
} while (0)

static void print_frame_intervals(struct camera_v4l2 *cam, uint32_t pixelformat, int width, int height)
{
    struct v4l2_frmivalenum frame_interval;

    memset(&frame_interval, 0, sizeof(frame_interval));
    frame_interval.pixel_format = pixelformat;
    frame_interval.width = width;
    frame_interval.height = height;
    frame_interval.index = 0;

    while (ioctl(cam->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_interval) >= 0)
    {
        if (V4L2_FRMSIZE_TYPE_DISCRETE == frame_interval.type)
        {
            struct v4l2_fract *interval = &frame_interval.discrete;

            CAM_LOG_INFO_V("            Interval: Discrete %.03fs (%.03f fps)",
                interval->numerator / (float)interval->denominator,
                interval->denominator / (float)interval->numerator);

            ++frame_interval.index;
        }
        else
        {
            const struct
            {
                const char *name;
                const struct v4l2_fract *ptr;
            } intervals[] = {
                { "Min", &frame_interval.stepwise.min },
                { "Max", &frame_interval.stepwise.max },
                { "Step", &frame_interval.stepwise.step }
            };
            unsigned int i;

            CAM_LOG_INFO_V("            Interval: %s",
                ((V4L2_FRMSIZE_TYPE_STEPWISE == frame_interval.type) ? "Stepwise" : "Continuous"));
            for (i = 0; i < sizeof(intervals) / sizeof(intervals[0]); ++i)
            {
                if (intervals[i].ptr->numerator > 0 && intervals[i].ptr->denominator > 0)
                {
                    CAM_LOG_INFO_V("                %s: %.03fs (%03f fps)", intervals[i].name,
                        intervals[i].ptr->numerator / (float)intervals[i].ptr->denominator,
                        intervals[i].ptr->denominator / (float)intervals[i].ptr->numerator);
                }
            }

            break;
        }
    } /* while (ioctl(cam->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_interval) >= 0) */
}

static int cam_v4l2_match_format(struct camera_v4l2 *cam, const char *expected_format)
{
    bool is_auto_selecting = (0 == strcasecmp(expected_format, "auto"));
    bool found_bgr = false;
    bool found_rgb = false;
    bool found_nv12 = false;
    char fourcc_str[5] = { 0 };
    struct v4l2_fmtdesc desc;
    struct v4l2_frmsizeenum frame_size;

    RESET_OBJECT_STATUS_FIELDS(cam);

    CAM_LOG_NOTICE("%s: Capture format in configuration is %s, doing matching for it ...",
        cam->dev_path, expected_format);

    desc.index = 0;
    desc.type = cam->buf_type;

    while (ioctl(cam->fd, VIDIOC_ENUM_FMT, &desc) >= 0)
    {
        PIXFMT_TO_FOURCC(desc.pixelformat, fourcc_str);

        bool matched = is_auto_selecting ? false : (0 == strcasecmp(expected_format, fourcc_str));

        if (matched)
        {
            CAM_LOG_NOTICE_V("    [%d]: %s (%s%s%s) <-- [Perfect Matched]", desc.index, fourcc_str, desc.description,
                ((0 != (V4L2_FMT_FLAG_COMPRESSED & desc.flags)) ? ", compressed" : ""),
                ((0 != (V4L2_FMT_FLAG_EMULATED & desc.flags)) ? ", emulated" : ""));
        }
        else
        {
            CAM_LOG_INFO_V("    [%d]: %s (%s%s%s)", desc.index, fourcc_str, desc.description,
                ((0 != (V4L2_FMT_FLAG_COMPRESSED & desc.flags)) ? ", compressed" : ""),
                ((0 != (V4L2_FMT_FLAG_EMULATED & desc.flags)) ? ", emulated" : ""));
        }

        memset(&frame_size, 0, sizeof(frame_size));
        frame_size.pixel_format = desc.pixelformat;
        frame_size.index = 0;

        while (ioctl(cam->fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) >= 0)
        {
            if (V4L2_FRMSIZE_TYPE_DISCRETE == frame_size.type)
            {
                struct v4l2_frmsize_discrete *size = &frame_size.discrete;

                CAM_LOG_INFO_V("        Size: Discrete %ux%u", size->width, size->height);
                print_frame_intervals(cam, desc.pixelformat, size->width, size->height);

                ++frame_size.index;
            }
            else
            {
                struct v4l2_frmsize_stepwise *size = &frame_size.stepwise;

                CAM_LOG_INFO_V("        Size: %s %ux%u - %ux%u with step %u/%u",
                    ((V4L2_FRMSIZE_TYPE_STEPWISE == frame_size.type) ? "Stepwise" : "Continuous"),
                    size->min_width, size->min_height, size->max_width, size->max_height,
                    size->step_width, size->step_height);
                print_frame_intervals(cam, desc.pixelformat, size->min_width, size->min_height);
                print_frame_intervals(cam, desc.pixelformat, size->max_width, size->max_height);
                print_frame_intervals(cam, desc.pixelformat, size->step_width, size->step_height);

                break;
            }
        } /* while (ioctl(cam->fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) >= 0) */

        if (matched)
        {
            cam->fmt_fourcc = desc.pixelformat;

            return cam->err;
        }

        switch (desc.pixelformat)
        {
        case V4L2_PIX_FMT_BGR24:
            found_bgr = true;
            break;

        case V4L2_PIX_FMT_RGB24:
            found_rgb = true;
            break;

        case V4L2_PIX_FMT_NV12:
            found_nv12 = true;
            break;

        default:
            break;
        }

        ++desc.index;
    } /* while (ioctl(cam->fd, VIDIOC_ENUM_FMT, &desc) >= 0) */

    if (!is_auto_selecting)
        cam->err = -ENOTSUP;
    else
    {
        if (found_bgr)
            cam->fmt_fourcc = V4L2_PIX_FMT_BGR24;
        else if (found_rgb)
            cam->fmt_fourcc = V4L2_PIX_FMT_RGB24;
        else if (found_nv12)
            cam->fmt_fourcc = V4L2_PIX_FMT_NV12;
        else
            cam->err = -ENOTSUP;
    }

    if (cam->err < 0)
        CAM_LOG_ERR_V("*** %s: Can not find suitable format!", cam->dev_path);
    else
    {
        PIXFMT_TO_FOURCC(cam->fmt_fourcc, fourcc_str);
        CAM_LOG_NOTICE_V("%s: Auto chosen format: %s", cam->dev_path, fourcc_str);
    }

    return cam->err;
}

static int cam_v4l2_set_size_and_format(struct camera_v4l2 *cam, uint16_t width, uint16_t height)
{
    struct v4l2_format format;
    char fourcc_str[5] = { 0 };

    RESET_OBJECT_STATUS_FIELDS(cam);

    memset(&format, 0, sizeof(format));
    format.type = cam->buf_type;
    if (V4L2_TYPE_IS_MULTIPLANAR(format.type))
    {
        format.fmt.pix_mp.width = cam->width = width;
        format.fmt.pix_mp.height = cam->height = height;
        format.fmt.pix_mp.pixelformat = cam->fmt_fourcc; /* auto detected in *_match_format() */
    }
    else
    {
        format.fmt.pix.width = cam->width = width;
        format.fmt.pix.height = cam->height = height;
        format.fmt.pix.pixelformat = cam->fmt_fourcc; /* auto detected in *_match_format() */
    }

    if (ioctl(cam->fd, VIDIOC_S_FMT, &format) < 0)
    {
        cam->err = -errno;
        PIXFMT_TO_FOURCC(cam->fmt_fourcc, fourcc_str);
        CAM_LOG_ERR_V("*** %s: Failed to set V4L2 format: width = %d, height = %d, fmt_fourcc = %s, err = %s",
            cam->dev_path, cam->width, cam->height, fourcc_str, strerror(-cam->err));

        return cam->err;
    }

    memset(&format, 0, sizeof(format));
    format.type = cam->buf_type;

    if (ioctl(cam->fd, VIDIOC_G_FMT, &format) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to read V4L2 format: %s", cam->dev_path, strerror(-cam->err));

        return cam->err;
    }

    CAM_LOG_INFO_V("%s: ioctl(VIDIOC_G_FMT):", cam->dev_path);
    if (V4L2_TYPE_IS_MULTIPLANAR(cam->buf_type))
    {
        cam->plane_count = format.fmt.pix_mp.num_planes;
        CAM_LOG_INFO_V("    width: %u", format.fmt.pix_mp.width);
        CAM_LOG_INFO_V("    height: %u", format.fmt.pix_mp.height);
        PIXFMT_TO_FOURCC(format.fmt.pix_mp.pixelformat, fourcc_str);
        CAM_LOG_INFO_V("    pixelformat: 0x%X -> %s", format.fmt.pix_mp.pixelformat, fourcc_str);
        CAM_LOG_INFO_V("    num_planes: %u", format.fmt.pix_mp.num_planes);
        for (uint8_t i = 0; i < format.fmt.pix_mp.num_planes; ++i)
        {
            CAM_LOG_INFO_V("    plane_fmt[%u]:", i);
            CAM_LOG_INFO_V("        bytesperline: %u", format.fmt.pix_mp.plane_fmt[i].bytesperline);
            CAM_LOG_INFO_V("        sizeimage: %u", format.fmt.pix_mp.plane_fmt[i].sizeimage);
        }
        CAM_LOG_INFO_V("    flags: 0x%X", format.fmt.pix_mp.flags);
    }
    else
    {
        cam->plane_count = 1;
        CAM_LOG_INFO_V("    width: %u", format.fmt.pix.width);
        CAM_LOG_INFO_V("    height: %u", format.fmt.pix.height);
        PIXFMT_TO_FOURCC(format.fmt.pix.pixelformat, fourcc_str);
        CAM_LOG_INFO_V("    pixelformat: 0x%X -> %s", format.fmt.pix.pixelformat, fourcc_str);
        CAM_LOG_INFO_V("    bytesperline: %u", format.fmt.pix.bytesperline);
        CAM_LOG_INFO_V("    sizeimage: %u", format.fmt.pix.sizeimage);
        CAM_LOG_INFO_V("    flags: 0x%X", format.fmt.pix.flags);
    }

    if (cam->plane_count > 2)
    {
        cam->err = -ENOTSUP;
        CAM_LOG_ERR_V("*** %s: Too many planes: %u", cam->dev_path, cam->plane_count);

        return cam->err;
    }

    if (V4L2_TYPE_IS_MULTIPLANAR(cam->buf_type))
    {
        if (format.fmt.pix_mp.width != cam->width
            || format.fmt.pix_mp.height != cam->height
            || format.fmt.pix_mp.pixelformat != cam->fmt_fourcc)
        {
            cam->err = -ENOTSUP;
        }
    }
    else
    {
        if (format.fmt.pix.width != cam->width
            || format.fmt.pix.height != cam->height
            || format.fmt.pix.pixelformat != cam->fmt_fourcc)
        {
            cam->err = -ENOTSUP;
        }
    }

    if (cam->err)
    {
        PIXFMT_TO_FOURCC(cam->fmt_fourcc, fourcc_str);
        CAM_LOG_ERR_V("*** %s: Format combination not supported: width = %d, height = %d, fmt_fourcc = %s",
            cam->dev_path, cam->width, cam->height, fourcc_str);
    }

    return cam->err;
}

static float calculate_frame_rate(const struct v4l2_fract *frame_time)
{
    float denominator = frame_time->denominator;
    float numerator = frame_time->numerator ? frame_time->numerator : 1;

    return denominator / numerator;
}

static int cam_v4l2_set_frame_rate(struct camera_v4l2 *cam, float frames_per_second, float fallback_fps)
{
    struct v4l2_streamparm fps;
    struct v4l2_captureparm *capture_param = &fps.parm.capture;
    struct v4l2_fract *frame_time = &capture_param->timeperframe;

    RESET_OBJECT_STATUS_FIELDS(cam);
    cam->fps = frames_per_second;

    memset(&fps, 0, sizeof(fps));
    fps.type = cam->buf_type;

    if (ioctl(cam->fd, VIDIOC_G_PARM, &fps) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to read V4L2 stream param: %s", cam->dev_path, strerror(-cam->err));

        /* return cam->err; */
    }

    if (!cam->err && ((uint32_t)calculate_frame_rate(frame_time)) == (uint32_t)cam->fps)
    {
        CAM_LOG_NOTICE_V("%s: No need to set FPS since it's already %.1f", cam->dev_path, cam->fps);

        return cam->err;
    }

    if (cam->fps <= 0.0)
    {
        if (cam->err)
        {
            cam->fps = fallback_fps;
            cam->err = 0;
            CAM_LOG_WARN_V("%s: Unable to detect FPS! Use the fallback value: %.1f fps", cam->dev_path, cam->fps);
        }
        else
        {
            cam->fps = calculate_frame_rate(frame_time);
            CAM_LOG_WARN_V("%s: Skipped FPS setting! Use the result just read: %.1f fps", cam->dev_path, cam->fps);
        }

        return cam->err;
    }

    if (!cam->err && 0 == (capture_param->capability & V4L2_CAP_TIMEPERFRAME))
        CAM_LOG_WARN_V("%s does not support FPS setting!", cam->dev_path);
    else
    {
        frame_time->numerator = 1;
        frame_time->denominator = cam->fps;

        if (ioctl(cam->fd, VIDIOC_S_PARM, &fps) < 0) /* try it even if the previous read failed */
        {
            cam->err = -errno;
            CAM_LOG_ERR_V("*** %s: Failed to set V4L2 stream param: %s", cam->dev_path, strerror(-cam->err));

            return cam->err;
        }

        memset(&fps, 0, sizeof(fps));
        fps.type = cam->buf_type;
        ioctl(cam->fd, VIDIOC_G_PARM, &fps);
    }

    CAM_LOG_INFO_V("%s: Stream parameters:", cam->dev_path);
    CAM_LOG_INFO_V("    capability: 0x%X", capture_param->capability);
    CAM_LOG_INFO_V("    capturemode: 0x%X", capture_param->capturemode);
    CAM_LOG_INFO_V("    timeperframe: %u/%u", frame_time->numerator, frame_time->denominator);

    if (((uint32_t)calculate_frame_rate(frame_time)) != (uint32_t)cam->fps)
    {
        cam->err = -ENOTSUP;
        CAM_LOG_ERR_V("*** %s: Could not set FPS as expected: %.1f", cam->dev_path, cam->fps);
    }

    return cam->err;
}

static const char* cam_v4l2_io_mode_name(uint32_t mode_value)
{
    switch (mode_value)
    {
    case V4L2_MEMORY_MMAP:
        return "mmap";

    case V4L2_MEMORY_USERPTR:
        return "userptr";

    case V4L2_MEMORY_OVERLAY:
        return "overlay";

    case V4L2_MEMORY_DMABUF:
        return "dmabuf";

    default:
        return "<unknown-io-mode>";
    }
}

static int munmap_buffers_if_any(camera_v4l2_t *);

static int mmap_buffers(camera_v4l2_t *cam)
{
    bool is_mplane = V4L2_TYPE_IS_MULTIPLANAR(cam->buf_type);

    for (uint8_t i = 0; i < cam->buf_count; ++i)
    {
        struct v4l2_buffer buffer;
        struct v4l2_plane planes[CAMERA_V4L2_MAX_PLANE_COUNT];

        memset(&buffer, 0, sizeof(buffer));
        buffer.type = cam->buf_type;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        if (is_mplane)
        {
            buffer.length = cam->plane_count;
            memset(&planes, 0, sizeof(planes));
            buffer.m.planes = planes;
        }

        if (ioctl(cam->fd, VIDIOC_QUERYBUF, &buffer) < 0)
        {
            cam->err = -errno;
            CAM_LOG_ERR_V("*** %s: ioctl(VIDIOC_QUERYBUF) failed: %s", cam->dev_path, strerror(-cam->err));

            break;
        }

        for (uint8_t j = 0; j < cam->plane_count; ++j)
        {
            cam->buf_sizes[i][j] = is_mplane ? buffer.m.planes[j].length : buffer.length;

            cam->buf_pointers[i][j] = (unsigned char *)mmap(NULL, cam->buf_sizes[i][j], PROT_READ | PROT_WRITE,
                MAP_SHARED, cam->fd, (is_mplane ? buffer.m.planes[j].m.mem_offset : buffer.m.offset));

            if (MAP_FAILED == cam->buf_pointers[i][j])
            {
                cam->err = -errno;
                cam->buf_pointers[i][j] = NULL;
                CAM_LOG_ERR_V("*** %s: mmap() for buf_pointers[%d][%d] failed: %s",
                    cam->dev_path, i, j, strerror(-cam->err));

                break;
            }
            else
            {
                CAM_LOG_DEBUG("%s: Mapped buf_pointers[%d][%d] to %p with total %u bytes.",
                    cam->dev_path, i, j, cam->buf_pointers[i][j], cam->buf_sizes[i][j]);
            }
        } /* for (j : cam->plane_count) */

        if (cam->err)
            break;
    } /* for (i : cam->buf_count) */

    if (cam->err)
    {
        int mmap_err = cam->err;

        munmap_buffers_if_any(cam);
        cam->err = mmap_err;
    }

    return cam->err;
}

__attribute__((weak))
int camera_v4l2_alloc_user_buffers(camera_v4l2_t *cam)
{
    cam->err = -ENOTSUP;
    CAM_LOG_ERR_V("*** %s: I/O mode not supported yet: 0x%X -> %s",
        cam->dev_path, cam->io_mode, cam_v4l2_io_mode_name(cam->io_mode));

    return cam->err;
}

__attribute__((weak))
int camera_v4l2_acquire_dma_buffers(camera_v4l2_t *cam)
{
    cam->err = -ENOTSUP;
    CAM_LOG_ERR_V("*** %s: I/O mode not supported yet: 0x%X -> %s",
        cam->dev_path, cam->io_mode, cam_v4l2_io_mode_name(cam->io_mode));

    return cam->err;
}

static const uint32_t S_DEFAULT_IO_MODES[] = {
    V4L2_MEMORY_DMABUF,
    V4L2_MEMORY_USERPTR,
    V4L2_MEMORY_MMAP,
    0/* sentinel */
};

static int cam_v4l2_alloc_buffers(struct camera_v4l2 *cam, uint8_t buf_count, const uint32_t io_mode_candidates[])
{
    struct v4l2_requestbuffers req;

    RESET_OBJECT_STATUS_FIELDS(cam);
    cam->buf_count = (buf_count > CAMERA_V4L2_MAX_BUF_COUNT) ? CAMERA_V4L2_MAX_BUF_COUNT : (buf_count ? buf_count : 2);

    memset(&req, 0, sizeof(req));
    req.type = cam->buf_type;
    req.memory = V4L2_MEMORY_MMAP; /* for querying capabilities */
    req.count = 0; /* for querying capabilities */

    if (ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to query supported I/O capabilities: %s", cam->dev_path, strerror(-cam->err));

        return cam->err;
    }

    uint32_t capabilities = req.capabilities;
    const uint32_t *io_mode_values = io_mode_candidates ? io_mode_candidates : S_DEFAULT_IO_MODES;

    for (int i = 0; 0 != io_mode_values[i]; ++i)
    {
        bool not_supported = (0 == (io_mode_values[i] & capabilities));
        bool not_implemented = (V4L2_MEMORY_USERPTR == io_mode_values[i] || V4L2_MEMORY_DMABUF == io_mode_values[i]);
        const char *io_mode_name = cam_v4l2_io_mode_name(io_mode_values[i]);

        if (not_supported || not_implemented)
        {
            cam->err = -ENOTSUP;
            CAM_LOG_WARN_V("%s: Streaming I/O mode not %s: %s",
                cam->dev_path, (not_supported ? "supported" : "implemented yet"), io_mode_name);

            continue;
        }
        cam->io_mode = io_mode_values[i];

        memset(&req, 0, sizeof(req));
        req.type = cam->buf_type;
        req.memory = cam->io_mode;
        req.count = (V4L2_MEMORY_USERPTR == req.type) ? 0 : cam->buf_count;

        if (ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0)
        {
            cam->err = -errno;
            CAM_LOG_WARN_V("%s: Failed to request %s I/O mode: %s", cam->dev_path, io_mode_name, strerror(-cam->err));

            continue;
        }
        else
        {
            cam->err = 0;
            CAM_LOG_NOTICE_V("%s: Requested %s I/O mode successfully.", cam->dev_path, io_mode_name);

            break;
        }
    } /* for (int i = 0; 0 != io_modes[i]; ++i) */

    if (!cam->err && req.count < 1)
    {
        cam->err = -ENOMEM;
        CAM_LOG_ERR_V("*** %s: Cannot reqeust I/O buffers", cam->dev_path);
    }

    if (cam->err)
        return cam->err;

    if (cam->buf_count != req.count)
    {
        cam->buf_count = req.count;
        CAM_LOG_WARN_V("%s: Adjusted I/O buffer count to %u", cam->dev_path, cam->buf_count);
    }

    switch (cam->io_mode)
    {
    case V4L2_MEMORY_MMAP:
        return mmap_buffers(cam);

    case V4L2_MEMORY_USERPTR:
        return camera_v4l2_alloc_user_buffers(cam);

    case V4L2_MEMORY_DMABUF:
        return camera_v4l2_acquire_dma_buffers(cam);

    default:
        CAM_LOG_ERR_V("*** %s: Unsupported I/O mode: 0x%X -> %s",
            cam->dev_path, cam->io_mode, cam_v4l2_io_mode_name(cam->io_mode));
        cam->err = -ENOTSUP;
        break;
    }

    return cam->err;
}

static int munmap_buffers_if_any(camera_v4l2_t *cam)
{
    for (uint8_t i = 0; i < cam->buf_count; ++i)
    {
        for (uint8_t j = 0; j < cam->plane_count; ++j)
        {
            if (NULL == cam->buf_pointers[i][j])
                continue;

            if (munmap(cam->buf_pointers[i][j], cam->buf_sizes[i][j]) < 0)
            {
                cam->err = -errno;
                CAM_LOG_ERR_V("*** %s: Failed to unmap buf_pointers[%d][%d](%p): %s",
                    cam->dev_path, i, j, cam->buf_pointers[i][j], strerror(-cam->err));
            }
            else
            {
                CAM_LOG_DEBUG("%s: Unmapped buf_pointers[%d][%d] successfully.", cam->dev_path, i, j);
                cam->buf_pointers[i][j] = NULL;
            }
        }
    }

    return cam->err;
}

__attribute__((weak))
int camera_v4l2_free_user_buffers_if_any(camera_v4l2_t *cam)
{
    return 0; /* TODO */
}

__attribute__((weak))
int camera_v4l2_release_dma_buffers_if_any(camera_v4l2_t *cam)
{
    return 0; /* TODO */
}

static int cam_v4l2_free_buffers_if_any(struct camera_v4l2 *cam)
{
    RESET_OBJECT_STATUS_FIELDS(cam);

    switch (cam->io_mode)
    {
    case V4L2_MEMORY_MMAP:
        return munmap_buffers_if_any(cam);

    case V4L2_MEMORY_USERPTR:
        return camera_v4l2_free_user_buffers_if_any(cam);

    case V4L2_MEMORY_DMABUF:
        return camera_v4l2_release_dma_buffers_if_any(cam);

    default:
        CAM_LOG_ERR_V("*** %s: Unsupported I/O mode: 0x%X -> %s",
            cam->dev_path, cam->io_mode, cam_v4l2_io_mode_name(cam->io_mode));
        cam->err = -ENOTSUP;
        break;
    }

    return cam->err;
}

static int cam_v4l2_enqueue_buffer(struct camera_v4l2 *cam, uint8_t buf_index);

static int cam_v4l2_start_capture(struct camera_v4l2 *cam)
{
    RESET_OBJECT_STATUS_FIELDS(cam);

    if ((cam->err = cam->stream_on ? -EBUSY : 0) < 0)
    {
        CAM_LOG_ERR_V("*** %s: Video stream is already on!", cam->dev_path);

        return cam->err;
    }

    for (uint8_t i = 0; i < cam->buf_count; ++i)
    {
        if (cam_v4l2_enqueue_buffer(cam, i) < 0)
            return cam->err;
    }

    if (ioctl(cam->fd, VIDIOC_STREAMON, &cam->buf_type) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to turn on video stream: %s", cam->dev_path, strerror(-cam->err));
    }
    else
    {
        CAM_LOG_NOTICE("%s: Turned on video streaming.", cam->dev_path);
        cam->stream_on = true;
    }

    return cam->err;
}

static int cam_v4l2_stop_capture(struct camera_v4l2 *cam)
{
    RESET_OBJECT_STATUS_FIELDS(cam);

    if (!cam->stream_on)
        return cam->err;

    if (ioctl(cam->fd, VIDIOC_STREAMOFF, &cam->buf_type) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to turn off video stream: %s", cam->dev_path, strerror(-cam->err));
    }
    else
    {
        cam->stream_on = 0;
        CAM_LOG_NOTICE("%s: Turned off video streaming.", cam->dev_path);
    }

    return cam->err;
}

static int cam_v4l2_wait_and_fetch(struct camera_v4l2 *cam, int timeout_msecs, struct v4l2_buffer *out_frame)
{
    bool is_stream_on = cam->stream_on;
    bool is_mplane = V4L2_TYPE_IS_MULTIPLANAR(cam->buf_type);
    struct v4l2_plane planes[CAMERA_V4L2_MAX_PLANE_COUNT];

    RESET_OBJECT_STATUS_FIELDS(cam);

    if ((cam->err = (NULL == out_frame) ? -EINVAL : (cam->stream_on ? 0 : -EIO)) < 0)
    {
        CAM_LOG_ERR_V("*** %s: %s", cam->dev_path,
            is_stream_on ? "Null frame pointer" : "Video stream not turned on yet");

        return cam->err;
    }

    memset(out_frame, 0, sizeof(*out_frame));
    out_frame->type = cam->buf_type;
    out_frame->memory = cam->io_mode;
    if (is_mplane)
    {
        out_frame->length = cam->plane_count;
        memset(&planes, 0, sizeof(planes));
        out_frame->m.planes = planes;
    }

    if (ioctl(cam->fd, VIDIOC_DQBUF, out_frame) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to dequeue buffer item: %s", cam->dev_path, strerror(-cam->err));
    }

    return cam->err;
}

static int cam_v4l2_enqueue_buffer(struct camera_v4l2 *cam, uint8_t buf_index)
{
    struct v4l2_buffer buffer;
    struct v4l2_plane planes[CAMERA_V4L2_MAX_PLANE_COUNT];

    RESET_OBJECT_STATUS_FIELDS(cam);

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = cam->buf_type;
    buffer.memory = cam->io_mode;
    buffer.index = buf_index;
    if (V4L2_TYPE_IS_MULTIPLANAR(cam->buf_type))
    {
        buffer.length = cam->plane_count;
        memset(&planes, 0, sizeof(planes));
        buffer.m.planes = planes;
    }
    /* TODO: fields for V4L2_MEMORY_USERPTR and V4L2_MEMORY_DMABUF */

    if (ioctl(cam->fd, VIDIOC_QBUF, &buffer) < 0)
    {
        cam->err = -errno;
        CAM_LOG_ERR_V("*** %s: Failed to enqueue buffer item [%d]: %s",
            cam->dev_path, buf_index, strerror(-cam->err));
    }

    return cam->err;
}

static struct
{
    int log_level;
} s_global_filter, *cam = &s_global_filter;

camera_v4l2_t camera_v4l2(const char *log_level)
{
    camera_v4l2_t obj;

    memset(&obj, 0, sizeof(obj));

    obj.fd = -1;
    obj.last_func = "<none>";
#ifdef NO_DEFAULT_FMT_LOG
    cam->log_level = 0; /* eliminate -Wunused-variable warning */
#else
    obj.log_level = to_log_level(log_level);
    s_global_filter.log_level = obj.log_level;
#endif

    obj.open = cam_v4l2_open;
    obj.close = cam_v4l2_close;
    obj.query_capabilities = cam_v4l2_query_capability;
    obj.match_format = cam_v4l2_match_format;
    obj.set_size_and_format = cam_v4l2_set_size_and_format;
    obj.set_frame_rate = cam_v4l2_set_frame_rate;
    obj.alloc_buffers = cam_v4l2_alloc_buffers;
    obj.free_buffers_if_any = cam_v4l2_free_buffers_if_any;
    obj.start_capture = cam_v4l2_start_capture;
    obj.stop_capture = cam_v4l2_stop_capture;
    obj.wait_and_fetch = cam_v4l2_wait_and_fetch;
    obj.enqueue_buffer = cam_v4l2_enqueue_buffer;

    return obj;
}

__attribute__((weak))
int camera_v4l2_validate_capabilities(uint32_t capabilities)
{
#define V4L2_CAPABILITY_ITEM(val)                       { #val, val }
    struct
    {
        const char *name;
        uint32_t value;
    } items[] = {
        V4L2_CAPABILITY_ITEM(V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE),
        V4L2_CAPABILITY_ITEM(V4L2_CAP_STREAMING),
    };
    unsigned int i;

    for (i = 0; i < sizeof(items) / sizeof(items[0]); ++i)
    {
        if (0 == (capabilities & items[i].value))
        {
            CAM_LOG_ERR_V("*** Missing capability: 0x%X -> %s", items[i].value, items[i].name);

            return -EPERM;
        }
    }

    return 0;
}

#ifdef TEST

#include <stdlib.h>

int main(int argc, char **argv)
{
    camera_v4l2_t cam_obj = camera_v4l2("info");
    const char *dev_path = (argc > 1) ? argv[1] : "/dev/video0";
    const char *format = (argc > 2) ? argv[2] : "AUTO";
    uint16_t width = (argc > 3) ? atoi(argv[3]) : 640;
    uint16_t height = (argc > 4) ? atoi(argv[4]) : 480;
    float fps = (argc > 5) ? atof(argv[5]) : 30.0;
    float fallback_fps = 15.0;
    uint32_t io_mode_candidates[] = { V4L2_MEMORY_DMABUF, V4L2_MEMORY_MMAP, V4L2_MEMORY_USERPTR, 0 };
    struct v4l2_buffer frame;
    int max_frame_count;
    int i;

    if (cam_obj.open(&cam_obj, dev_path, /* is_nonblocking = */false) < 0
        || cam_obj.query_capabilities(&cam_obj, /* with_validation = */true) < 0
        || cam_obj.match_format(&cam_obj, format) < 0
        || cam_obj.set_size_and_format(&cam_obj, width, height) < 0
        || cam_obj.set_frame_rate(&cam_obj, fps, fallback_fps) < 0
        || cam_obj.alloc_buffers(&cam_obj, /* buf_count = */4, io_mode_candidates) < 0
        || cam_obj.start_capture(&cam_obj) < 0)
    {
        FMT_LOG(&cam_obj, E, "*** %s() failed: %s", cam_obj.last_func, strerror(-cam_obj.err));
        FMT_LOG(&cam_obj, E, "%s", "Make sure you pass correct arguments!");
        FMT_LOG(&cam_obj, E, "Usage example: %s /dev/video0 auto 640 480 30.0", argv[0]);
        cam_obj.free_buffers_if_any(&cam_obj);
        cam_obj.close(&cam_obj);

        return EXIT_FAILURE;
    }

    max_frame_count = cam_obj.fps * 3;
    FMT_LOG(&cam_obj, W, "%s", "Test started, please be patient ...");

    for (i = 0; i < max_frame_count; ++i)
    {
        if (cam_obj.wait_and_fetch(&cam_obj, /* timeout_msecs = */0, &frame) < 0 && -EAGAIN != cam_obj.err)
            break;

        if (i == max_frame_count - 1)
        {
            char fourcc_str[5] = { 0 };
            char path[1024] = { 0 };
            FILE *stream = NULL;

            PIXFMT_TO_FOURCC(cam_obj.fmt_fourcc, fourcc_str);
            snprintf(path, sizeof(path) - 1, "%s.%s", argv[0], fourcc_str);
            if (NULL != (stream = fopen(path, "w")))
            {
                fwrite(cam_obj.buf_pointers[frame.index][0], sizeof(char), cam_obj.buf_sizes[frame.index][0], stream);
                FMT_LOG(&cam_obj, N, "Captured image: %s", path);
                fclose(stream);
            }

            break;
        }

        if (cam_obj.enqueue_buffer(&cam_obj, frame.index) < 0)
            break;
    } /* for (i : max_frame_count) */

    cam_obj.stop_capture(&cam_obj);
    cam_obj.free_buffers_if_any(&cam_obj);
    cam_obj.close(&cam_obj);

    return EXIT_SUCCESS;
}

#endif /* #ifdef TEST */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-04-04, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2025-04-08, Man Hung-Coeng <udc577@126.com>:
 *  01. Change the type of global log filter and thus greatly reduce its size.
 *  02. Add fmt_log.h example comment.
 */

