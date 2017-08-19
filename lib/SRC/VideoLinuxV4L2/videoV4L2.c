/*
 *  videoV4L2.c
 *  ARToolKit5
 *
 *  This file is part of ARToolKit.
 *
 *  ARToolKit is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ARToolKit is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ARToolKit.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As a special exception, the copyright holders of this library give you
 *  permission to link this library with independent modules to produce an
 *  executable, regardless of the license terms of these independent modules, and to
 *  copy and distribute the resulting executable under terms of your choice,
 *  provided that you also meet, for each linked independent module, the terms and
 *  conditions of the license of that module. An independent module is a module
 *  which is neither derived from nor based on this library. If you modify this
 *  library, you may extend this exception to your version of the library, but you
 *  are not obligated to do so. If you do not wish to do so, delete this exception
 *  statement from your version.
 *
 *  Copyright 2015 Daqri, LLC.
 *  Copyright 2003-2015 ARToolworks, Inc.
 *
 *  Author(s): Atsushi Nakazawa, Hirokazu Kato, Philip Lamb, Simon Goodall
 *
 */
/*
 *   Video capture subrutine for Linux/Video4Linux2 devices
 *   Based upon the V4L 1 artoolkit code and v4l2 spec example
 *   at http://v4l2spec.bytesex.org/spec/a13010.htm
 *   Simon Goodall <sg@ecs.soton.ac.uk>
 */

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h> // gettimeofday(), struct timeval
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset()
#include <errno.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <AR/ar.h>
#include <AR/video.h>
#include <jpeglib.h>

// V4L2 code from https://gist.github.com/jayrambhia/5866483
static int xioctl(int const fd, int const request, void *const arg)
{
    int r;

    do {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);

    return r;
}

#define MAXCHANNEL   10

static void printPalette(int const pixelFormat) {
    char *name;

    switch (pixelFormat) {
        // YUV formats
        case V4L2_PIX_FMT_GREY: name = "Grey"; break;
        case V4L2_PIX_FMT_YUYV: name = "YUYV"; break;
        case V4L2_PIX_FMT_UYVY: name = "UYVY"; break;
        case V4L2_PIX_FMT_Y41P: name = "Y41P"; break;
        case V4L2_PIX_FMT_YVU420: name = "YVU420"; break;
        case V4L2_PIX_FMT_YVU410: name = "YVU410"; break;
        case V4L2_PIX_FMT_YUV422P: name = "YUV422P"; break;
        case V4L2_PIX_FMT_YUV411P: name = "YUV411P"; break;
        case V4L2_PIX_FMT_NV12: name = "NV12"; break;
        case V4L2_PIX_FMT_NV21: name = "NV21"; break;

        // RGB formats
        case V4L2_PIX_FMT_RGB332: name = "RGB332"; break;
        case V4L2_PIX_FMT_RGB555: name = "RGB555"; break;
        case V4L2_PIX_FMT_RGB565: name = "RGB565"; break;
        case V4L2_PIX_FMT_RGB555X: name = "RGB555X"; break;
        case V4L2_PIX_FMT_RGB565X: name = "RGB565X"; break;
        case V4L2_PIX_FMT_BGR24: name = "BGR24"; break;
        case V4L2_PIX_FMT_RGB24: name = "RGB24"; break;
        case V4L2_PIX_FMT_BGR32: name = "BGR32"; break;
        case V4L2_PIX_FMT_RGB32: name = "RGB32"; break;

        // Other formats
        case V4L2_PIX_FMT_MJPEG: name = "MJPEG"; break;
        default: name = "Unknown"; break;
    };

    ARLOGi("  Pixel Format: %s\n", name);
}

static int getControl(int fd, int type, int *value) {
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;

    memset (&queryctrl, 0, sizeof (queryctrl));
    // TODO: Manke sure this is a correct value
    queryctrl.id = type;

    if (-1 == xioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (errno != EINVAL) {
            ARLOGe("Error calling VIDIOC_QUERYCTRL\n");
            return 1;
        } else {
            ARLOGe("Control %d is not supported\n", type);
            return 1;
        }
    } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        ARLOGe("Control %s is not supported\n", queryctrl.name);
        return 1;
    } else {
        memset (&control, 0, sizeof (control));
        control.id = type;

        if (-1 == xioctl (fd, VIDIOC_G_CTRL, &control)) {
            ARLOGe("Error getting control %s value\n", queryctrl.name);
            return 1;
        }
        *value = control.value;
    }
    return 0;
}

static int setControl(int fd, int type, int value) {
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;

    memset (&queryctrl, 0, sizeof (queryctrl));
    // TODO: Manke sure this is a correct value
    queryctrl.id = type;

    if (-1 == xioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (errno != EINVAL) {
            ARLOGe("Error calling VIDIOC_QUERYCTRL\n");
            return 1;
        } else {
            ARLOGe("Control %d is not supported\n", type);
            return 1;
        }
    } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        ARLOGe("Control %s is not supported\n", queryctrl.name);
        return 1;
    } else {
        memset (&control, 0, sizeof (control));
        control.id = type;
        // TODO check min/max range
        // If value is -1, then we use the default value
        control.value = (value == -1) ? (queryctrl.default_value) : (value);

        if (-1 == xioctl (fd, VIDIOC_S_CTRL, &control)) {
            ARLOGe("Error setting control %s to %d\n", queryctrl.name, value);
            return 1;
        }
    }
    return 0;
}

// YUYV, aka YUV422, to RGB
// from http://pastebin.com/mDcwqJV3
static inline
void saturate(int* value, int min_val, int max_val)
{
    if (*value < min_val) *value = min_val;
    if (*value > max_val) *value = max_val;
}

static int bgr24ToBgr24(const int width, const int height,
                        const void *const src, void *const dst)
{
    memcpy(dst, src, width * height * 3);
    return 0;
}

// destination format AR_PIX_FORMAT_BGR
static int yuyvToBgr24(int width, int height, const void *src, void *dst)
{
    unsigned char *yuyv_image = (unsigned char*) src;
    unsigned char *rgb_image = (unsigned char*) dst;
    const int K1 = (int)(1.402f * (1 << 16));
    const int K2 = (int)(0.714f * (1 << 16));
    const int K3 = (int)(0.334f * (1 << 16));
    const int K4 = (int)(1.772f * (1 << 16));

    typedef unsigned char T;
    T* out_ptr = &rgb_image[0];
    const int pitch = width * 2; // 2 bytes per one YU-YV pixel
    int x, y;
    for (y=0; y<height; y++) {
        const T* src = yuyv_image + pitch * y;
        for (x=0; x<width*2; x+=4) { // Y1 U Y2 V
            T Y1 = src[x + 0];
            T U  = src[x + 1];
            T Y2 = src[x + 2];
            T V  = src[x + 3];

            char uf = U - 128;
            char vf = V - 128;

            int R = Y1 + (K1*vf >> 16);
            int G = Y1 - (K2*vf >> 16) - (K3*uf >> 16);
            int B = Y1 + (K4*uf >> 16);

            saturate(&R, 0, 255);
            saturate(&G, 0, 255);
            saturate(&B, 0, 255);

            *out_ptr++ = (T)(B);
            *out_ptr++ = (T)(G);
            *out_ptr++ = (T)(R);

            R = Y2 + (K1*vf >> 16);
            G = Y2 - (K2*vf >> 16) - (K3*uf >> 16);
            B = Y2 + (K4*uf >> 16);

            saturate(&R, 0, 255);
            saturate(&G, 0, 255);
            saturate(&B, 0, 255);

            *out_ptr++ = (T)(B);
            *out_ptr++ = (T)(G);
            *out_ptr++ = (T)(R);
        }

    }

    return 0;
}

// destination format AR_PIX_FORMAT_BGRA
static int yuyvToBgr32(int width, int height, const void *src, void *dst)
{
    unsigned char *yuyv_image = (unsigned char*) src;
    unsigned char *rgb_image = (unsigned char*) dst;
    const int K1 = (int)(1.402f * (1 << 16));
    const int K2 = (int)(0.714f * (1 << 16));
    const int K3 = (int)(0.334f * (1 << 16));
    const int K4 = (int)(1.772f * (1 << 16));

    typedef unsigned char T;
    T* out_ptr = &rgb_image[0];
    const T a = 0xff;
    const int pitch = width * 2; // 2 bytes per one YU-YV pixel
    int x, y;
    for (y=0; y<height; y++) {
        const T* src = yuyv_image + pitch * y;
        for (x=0; x<width*2; x+=4) { // Y1 U Y2 V
            T Y1 = src[x + 0];
            T U  = src[x + 1];
            T Y2 = src[x + 2];
            T V  = src[x + 3];

            char uf = U - 128;
            char vf = V - 128;

            int R = Y1 + (K1*vf >> 16);
            int G = Y1 - (K2*vf >> 16) - (K3*uf >> 16);
            int B = Y1 + (K4*uf >> 16);

            saturate(&R, 0, 255);
            saturate(&G, 0, 255);
            saturate(&B, 0, 255);

            *out_ptr++ = (T)(B);
            *out_ptr++ = (T)(G);
            *out_ptr++ = (T)(R);
            *out_ptr++ = a;

            R = Y2 + (K1*vf >> 16);
            G = Y2 - (K2*vf >> 16) - (K3*uf >> 16);
            B = Y2 + (K4*uf >> 16);

            saturate(&R, 0, 255);
            saturate(&G, 0, 255);
            saturate(&B, 0, 255);

            *out_ptr++ = (T)(B);
            *out_ptr++ = (T)(G);
            *out_ptr++ = (T)(R);
            *out_ptr++ = a;
        }

    }

    return 0;
}

static int mjpegToBgr24(__attribute__((unused)) int width,
                        __attribute__((unused)) int height,
                        __attribute__((unused)) const void *src,
                        __attribute__((unused)) void *dst) {
    return 0;
}

/*-------------------------------------------*/

int ar2VideoDispOptionV4L2( void )
{
    ARLOG(" -device=LinuxV4L2\n");
    ARLOG("\n");
    ARLOG("DEVICE CONTROLS:\n");
    ARLOG(" -dev=filepath\n");
    ARLOG("    specifies device file.\n");
    ARLOG(" -channel=N\n");
    ARLOG("    specifies source channel.\n");
    ARLOG(" -width=N\n");
    ARLOG("    specifies expected width of image.\n");
    ARLOG(" -height=N\n");
    ARLOG("    specifies expected height of image.\n");
    ARLOG(" -palette=[GREY|HI240|RGB565|RGB555|BGR24|BGR32|YUYV|UYVY|\n");
    ARLOG("    Y41P|YUV422P|YUV411P|YVU420|YVU410|MJPEG]\n");
    ARLOG("    specifies the camera palette (WARNING: not all options are supported by\n");
    ARLOG("    every camera).\n");
    ARLOG("IMAGE CONTROLS (WARNING: not all options are not supported by every camera):\n");
    ARLOG(" -brightness=N\n");
    ARLOG("    specifies brightness. (0.0 <-> 1.0)\n");
    ARLOG(" -contrast=N\n");
    ARLOG("    specifies contrast. (0.0 <-> 1.0)\n");
    ARLOG(" -saturation=N\n");
    ARLOG("    specifies saturation (color). (0.0 <-> 1.0) (for color camera only)\n");
    ARLOG(" -hue=N\n");
    ARLOG("    specifies hue. (0.0 <-> 1.0) (for color camera only)\n");
    ARLOG("OPTION CONTROLS:\n");
    ARLOG(" -mode=[PAL|NTSC|SECAM]\n");
    ARLOG("    specifies TV signal mode (for tv/capture card).\n");
    ARLOG("\n");

    return 0;
}

// XXX: This function leaks badly when an error occurs.
AR2VideoParamV4L2T *ar2VideoOpenV4L2(char const *const config) {
    AR2VideoParamV4L2T *vid;
    struct v4l2_capability vd;
    struct v4l2_requestbuffers req;

    const char *a;
    char line[256];

    arMalloc(vid, AR2VideoParamV4L2T, 1);
    strcpy(vid->dev, AR_VIDEO_V4L2_DEFAULT_DEVICE);

    vid->width       = AR_VIDEO_V4L2_DEFAULT_WIDTH;
    vid->height      = AR_VIDEO_V4L2_DEFAULT_HEIGHT;
    vid->channel     = AR_VIDEO_V4L2_DEFAULT_CHANNEL;
    vid->mode        = AR_VIDEO_V4L2_DEFAULT_MODE;
    vid->format      = AR_INPUT_V4L2_DEFAULT_PIXEL_FORMAT;
    vid->palette     = V4L2_PIX_FMT_YUYV;
    vid->contrast    = -1;
    vid->brightness  = -1;
    vid->saturation  = -1;
    vid->hue         = -1;
    vid->gamma       = -1;
    vid->exposure    = -1;
    vid->gain        = 1;
    vid->debug       = 0;
    vid->videoBuffer = NULL;

    a = config;
    if( a != NULL) {
        for(;;) {
            while( *a == ' ' || *a == '\t' ) a++;
            if( *a == '\0' ) break;

            if (sscanf(a, "%255s", line) != 1) {
                break;
            }

            if( strncmp( a, "-dev=", 5 ) == 0 ) {
                if (sscanf(&line[5], "%255s", vid->dev) != 1 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-channel=", 9 ) == 0 ) {
                if( sscanf( &line[9], "%d", &vid->channel ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-width=", 7 ) == 0 ) {
                if( sscanf( &line[7], "%d", &vid->width ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-height=", 8 ) == 0 ) {
                if( sscanf( &line[8], "%d", &vid->height ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-palette=", 9 ) == 0 ) {
                if( strncmp( &a[9], "GREY", 4) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_GREY;
                } else if( strncmp( &a[9], "HI240", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_HI240;
                } else if( strncmp( &a[9], "RGB565", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_RGB565;
                } else if( strncmp( &a[9], "RGB555", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_RGB555;
                } else if( strncmp( &a[9], "BGR24", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_BGR24;
                } else if( strncmp( &a[9], "BGR32", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_BGR32;
                } else if( strncmp( &a[9], "YUYV", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_YUYV;
                } else if( strncmp( &a[9], "UYVY", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_UYVY;
                } else if( strncmp( &a[9], "Y41P", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_Y41P;
                } else if( strncmp( &a[9], "YUV422P", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_YUV422P;
                } else if( strncmp( &a[9], "YUV411P", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_YUV411P;
                } else if( strncmp( &a[9], "YVU420", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_YVU420;
                } else if( strncmp( &a[9], "YVU410", 3) == 0 ) {
                    vid->palette = V4L2_PIX_FMT_YVU410;
                } else if (strncmp(&a[9], "MJPEG", 4) == 0) {
                    vid->palette = V4L2_PIX_FMT_MJPEG;
                }
            }
            else if( strncmp( a, "-contrast=", 10 ) == 0 ) {
                if( sscanf( &line[10], "%d", &vid->contrast ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-brightness=", 12 ) == 0 ) {
                if( sscanf( &line[12], "%d", &vid->brightness ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-saturation=", 12 ) == 0 ) {
                if( sscanf( &line[12], "%d", &vid->saturation ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-hue=", 5 ) == 0 ) {
                if( sscanf( &line[5], "%d", &vid->hue ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-gamma=", 7 ) == 0 ) {
                if( sscanf( &line[7], "%d", &vid->gamma ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-exposure=", 10 ) == 0 ) {
                if( sscanf( &line[10], "%d", &vid->exposure ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-gain=", 6 ) == 0 ) {
                if( sscanf( &line[6], "%d", &vid->gain ) == 0 ) {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-mode=", 6 ) == 0 ) {
                if( strncmp( &a[6], "PAL", 3 ) == 0 )        vid->mode = V4L2_STD_PAL;
                else if( strncmp( &a[6], "NTSC", 4 ) == 0 )  vid->mode = V4L2_STD_NTSC;
                else if( strncmp( &a[6], "SECAM", 5 ) == 0 ) vid->mode = V4L2_STD_SECAM;
                else {
                    ar2VideoDispOptionV4L2();
                    free( vid );
                    return 0;
                }
            }
            else if( strcmp( line, "-debug" ) == 0 ) {
                vid->debug = 1;
            }
            else if( strcmp( line, "-device=LinuxV4L2" ) == 0 )    {
            }
            else {
                ARLOGe("Error: unrecognised configuration option '%s'.\n", a);
                ar2VideoDispOptionV4L2();
                free( vid );
                return 0;
            }

            while( *a != ' ' && *a != '\t' && *a != '\0') a++;
        }
    }

    // It appears that read-write is required for memory-mapping to works.
    vid->fd = open(vid->dev, O_RDWR);

    if (vid->fd < 0) {
        ARLOGe("Could not open video device. (%s)\n", vid->dev);
        free(vid);
        return NULL;
    }

    if (xioctl(vid->fd, VIDIOC_QUERYCAP, &vd) < 0) {
        ARLOGe("Coult not query video device capabilities.\n");
        free(vid);
        return NULL;
    }

    if (!(vd.capabilities & V4L2_CAP_STREAMING)) {
        ARLOGw("Device does not support streaming IO\n");
    }

    if (vid->debug) {
        ARLOGi("Video Device:\n");
        ARLOGi("  Driver:   %s\n", vd.driver);
        ARLOGi("  Card:     %s\n", vd.card);
        ARLOGi("  Bus Info: %s\n", vd.bus_info);
        ARLOGi("  Version:  %d\n", vd.version);
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = vid->width;
    fmt.fmt.pix.height      = vid->height;
    fmt.fmt.pix.pixelformat = vid->palette;
    fmt.fmt.pix.field       = V4L2_FIELD_NONE;

    if (xioctl(vid->fd, VIDIOC_S_FMT, &fmt) < 0) {
        close(vid->fd);
        free(vid);
        ARLOGe("ar2VideoOpen: Error setting video format (%d)\n", errno);
        return NULL;
    }

    // Get actual camera settings.
    vid->width = fmt.fmt.pix.width;
    vid->height = fmt.fmt.pix.height;
    vid->palette = fmt.fmt.pix.pixelformat;

    switch (vid->palette) {
        case V4L2_PIX_FMT_MJPEG:
            vid->bytes_per_pixel = 3;
            break;
        default:
            close(vid->fd);
            free(vid);
            ARLOGe("Bytes per pixel unknown for format %d.\n", vid->palette);
            return NULL;
    }

    if (vid->debug) {
        ARLOGi("  Width:    %d\n", vid->width);
        ARLOGi("  Height:   %d\n", vid->height);
        ARLOGi("  Bytes Per Pixel: %d\n", vid->bytes_per_pixel);
        printPalette(vid->palette);
    }

    switch (vid->palette) {
        case V4L2_PIX_FMT_BGR24:
            if (vid->format == AR_PIXEL_FORMAT_RGB) {
                vid->toArPixelFormat = bgr24ToBgr24;
            }
            break;

        case V4L2_PIX_FMT_YUYV:
            if (vid->format == AR_PIXEL_FORMAT_RGB) {
                vid->toArPixelFormat = yuyvToBgr24;
            } else if (vid->format == AR_PIXEL_FORMAT_RGBA) {
                vid->toArPixelFormat = yuyvToBgr32;
            }
            break;

        case V4L2_PIX_FMT_MJPEG:
            vid->toArPixelFormat = mjpegToBgr24;
            break;

        default:
            close(vid->fd);
            free(vid);
            ARLOGe("ar2VideoOpen: Cannot convert video pixel format to "
                   "internal pixel format.\n");
            return NULL;
    }

    struct v4l2_input ipt;
    memset(&ipt, 0, sizeof(ipt));
    ipt.index = vid->channel;
    ipt.std = vid->mode;

    if (xioctl(vid->fd,VIDIOC_ENUMINPUT,&ipt) < 0) {
        ARLOGe("arVideoOpen: Error querying input device type\n");
        close(vid->fd);
        free( vid );
        return 0;
    }

    if (vid->debug) {
        if (ipt.type == V4L2_INPUT_TYPE_TUNER) {
            ARLOGe("  Type: Tuner\n");
        }
        if (ipt.type == V4L2_INPUT_TYPE_CAMERA) {
            ARLOGe("  Type: Camera\n");
        }
    }

    // Set channel
    if (xioctl(vid->fd, VIDIOC_S_INPUT, &ipt)) {
        ARLOGe("arVideoOpen: Error setting video input\n");
        close(vid->fd);
        free( vid );
        return 0;
    }


    // Attempt to set some camera controls
    setControl(vid->fd, V4L2_CID_BRIGHTNESS, vid->brightness);
    setControl(vid->fd, V4L2_CID_CONTRAST, vid->contrast);
    setControl(vid->fd, V4L2_CID_SATURATION, vid->saturation);
    setControl(vid->fd, V4L2_CID_HUE, vid->hue);
    setControl(vid->fd, V4L2_CID_GAMMA, vid->gamma);
    setControl(vid->fd, V4L2_CID_EXPOSURE, vid->exposure);
    setControl(vid->fd, V4L2_CID_GAIN, vid->gain);

    // Print out current control values
    if (vid->debug) {
        int value;

        if (!getControl(vid->fd, V4L2_CID_BRIGHTNESS, &value)) {
            ARLOGe("Brightness: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_CONTRAST, &value)) {
            ARLOGe("Contrast: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_SATURATION, &value)) {
            ARLOGe("Saturation: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_HUE, &value)) {
            ARLOGe("Hue: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_GAMMA, &value)) {
            ARLOGe("Gamma: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_EXPOSURE, &value)) {
            ARLOGe("Exposure: %d\n", value);
        }
        if (!getControl(vid->fd, V4L2_CID_GAIN, &value)) {
            ARLOGe("Gain: %d\n", value);
        }
    }

    arMalloc(vid->videoBuffer,
             ARUint8,
             vid->width * vid->height * vid->bytes_per_pixel);

    // Setup memory mapping
    memset(&req, 0, sizeof(req));
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(vid->fd, VIDIOC_REQBUFS, &req)) {
        ARLOGe("Error calling VIDIOC_REQBUFS\n");
        close(vid->fd);
        if(vid->videoBuffer!=NULL) free(vid->videoBuffer);
        free( vid );
        return 0;
    }

    if (req.count < 2) {
        ARLOGe("this device can not be supported by libARvideo.\n");
        ARLOGe("(req.count < 2)\n");
        close(vid->fd);
        if(vid->videoBuffer!=NULL) free(vid->videoBuffer);
        free( vid );

        return 0;
    }

    vid->buffers = (struct buffer_ar_v4l2 *)calloc(req.count , sizeof(*vid->buffers));

    if (vid->buffers == NULL ) {
        ARLOGe("ar2VideoOpen: Error allocating buffer memory\n");
        close(vid->fd);
        if(vid->videoBuffer!=NULL) free(vid->videoBuffer);
        free( vid );
        return 0;
    }

    for (vid->n_buffers = 0; (size_t)vid->n_buffers < req.count; ++vid->n_buffers) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = vid->n_buffers;

        if (xioctl (vid->fd, VIDIOC_QUERYBUF, &buf)) {
            ARLOGe("error VIDIOC_QUERYBUF\n");
            close(vid->fd);
            if(vid->videoBuffer!=NULL) free(vid->videoBuffer);
            free( vid );
            return 0;
        }

        vid->buffers[vid->n_buffers].length = buf.length;
        vid->buffers[vid->n_buffers].start =
        mmap (NULL /* start anywhere */,
              buf.length,
              PROT_READ | PROT_WRITE /* required */,
              MAP_SHARED /* recommended */,
              vid->fd, buf.m.offset);

        if (MAP_FAILED == vid->buffers[vid->n_buffers].start) {
            ARLOGe("Error mmap\n");
            close(vid->fd);
            if(vid->videoBuffer!=NULL) free(vid->videoBuffer);
            free( vid );
            return 0;
        }
    }

    vid->video_cont_num = -1;

    return vid;
}

int ar2VideoCloseV4L2( AR2VideoParamV4L2T *vid )
{
    if (vid->video_cont_num >= 0){
        ar2VideoCapStopV4L2( vid );
    }
    close(vid->fd);
    free(vid->videoBuffer);
    free(vid);

    return 0;
}

int ar2VideoGetIdV4L2( AR2VideoParamV4L2T *vid, ARUint32 *id0, ARUint32 *id1 )
{
    if (!vid) return -1;

    if (id0) *id0 = 0;
    if (id1) *id1 = 0;

    return -1;
}

int ar2VideoGetSizeV4L2(AR2VideoParamV4L2T *vid, int *x,int *y)
{
    if (!vid) return -1;

    if (x) *x = vid->width;
    if (y) *y = vid->height;

    return 0;
}

AR_PIXEL_FORMAT ar2VideoGetPixelFormatV4L2( AR2VideoParamV4L2T *vid )
{
    if (!vid) return AR_PIXEL_FORMAT_INVALID;

    return vid->format;
}

int ar2VideoCapStartV4L2( AR2VideoParamV4L2T *vid )
{
    enum v4l2_buf_type type;
    struct v4l2_buffer buf;
    int i;

    if (vid->video_cont_num >= 0){
        ARLOGe("arVideoCapStart has already been called.\n");
        return -1;
    }

    vid->video_cont_num = 0;

    for (i = 0; i < vid->n_buffers; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (xioctl(vid->fd, VIDIOC_QBUF, &buf)) {
            ARLOGe("ar2VideoCapStart: Error calling VIDIOC_QBUF: %d\n", errno);
            return -1;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(vid->fd, VIDIOC_STREAMON, &type)) {
        ARLOGe("ar2VideoCapStart: Error calling VIDIOC_STREAMON\n");
        return -1;
    }

    return 0;
}

int ar2VideoCapStopV4L2( AR2VideoParamV4L2T *vid )
{
    if (!vid) return -1;

    if (vid->video_cont_num < 0){
        ARLOGe("arVideoCapStart has never been called.\n");
        return -1;
    }

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(vid->fd, VIDIOC_STREAMOFF, &type)) {
        ARLOGe("Error calling VIDIOC_STREAMOFF\n");
        return -1;
    }

    vid->video_cont_num = -1;

    return 0;
}


AR2VideoBufferT *ar2VideoGetImageV4L2( AR2VideoParamV4L2T *vid )
{
    if (!vid) return NULL;

    if (vid->video_cont_num < 0){
        ARLOGe("arVideoCapStart has never been called.\n");
        return NULL;
    }

    AR2VideoBufferT *out = &(vid->buffer.out);
    memset(out, 0, sizeof(*out));

    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (xioctl(vid->fd, VIDIOC_DQBUF, &buf) < 0) {
        ARLOGe("Error calling VIDIOC_DQBUF: %d\n", errno);
        return out;
    }

    vid->video_cont_num = buf.index;

    ARUint8 *buffer = (ARUint8*)vid->buffers[buf.index].start;


    if (vid->palette == V4L2_PIX_FMT_MJPEG) {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, buffer, buf.length);

        int rc = jpeg_read_header(&cinfo, TRUE);
        if (rc != 1) {
            ARLOGe("Bad JPEG\n");
            exit(-1);
        }

        cinfo.out_color_space = JCS_EXT_BGR;
        jpeg_start_decompress(&cinfo);

        int width = cinfo.output_width;
        int pixelSize = cinfo.output_components;

        while (cinfo.output_scanline < cinfo.output_height) {
            unsigned char *buffer_array[1];
            buffer_array[0] = vid->videoBuffer + cinfo.output_scanline * \
                              (width * pixelSize);
            jpeg_read_scanlines(&cinfo, buffer_array, 1);
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
    } else if (vid->toArPixelFormat(vid->width, vid->height, buffer,
                                   vid->videoBuffer) != 0) {
        ARLOGe("Could not convert image to ARToolkit pixel format.");
        return out;
    }

    out->buff = vid->videoBuffer;
    out->time_sec = buf.timestamp.tv_sec;
    out->time_usec = buf.timestamp.tv_usec;
    out->fillFlag = 1;
    out->buffLuma = NULL;

    struct v4l2_buffer buf_next;
    memset(&buf_next, 0, sizeof(buf_next));
    buf_next.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf_next.memory = V4L2_MEMORY_MMAP;
    buf_next.index = vid->video_cont_num;

    if (xioctl(vid->fd, VIDIOC_QBUF, &buf_next)) {
        ARLOGe("ar2VideoCapNext: Error calling VIDIOC_QBUF: %d\n", errno);
    }

    return out;
}

int ar2VideoGetParamiV4L2(__attribute__((unused)) AR2VideoParamV4L2T *vid,
                          __attribute__((unused)) int paramName,
                          __attribute__((unused)) int *value) {
    return -1;
}

int ar2VideoSetParamiV4L2(__attribute__((unused)) AR2VideoParamV4L2T *vid,
                          __attribute__((unused)) int paramName,
                          __attribute__((unused)) int value) {
    return -1;
}

int ar2VideoGetParamdV4L2(__attribute__((unused)) AR2VideoParamV4L2T *vid,
                          __attribute__((unused)) int paramName,
                          __attribute__((unused)) double *value) {
    return -1;
}

int ar2VideoSetParamdV4L2(__attribute__((unused)) AR2VideoParamV4L2T *vid,
                          __attribute__((unused)) int paramName,
                          __attribute__((unused)) double value) {
    return -1;
}
