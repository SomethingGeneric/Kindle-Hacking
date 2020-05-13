/*
 * fbgrab - takes screenshots using the framebuffer.
 *
 * (C) Gunnar Monell <gmo@linux.nu> 2002
 *
 * This program is free Software, see the COPYING file
 * and is based on Stephan Beyer's <fbshot@s-beyer.de> FBShot
 * (C) 2000.
 *
 * For features and differences, read the manual page.
 *
 * This program has been checked with "splint +posixlib" without
 * warnings. Splint is available from http://www.splint.org/ .
 * Patches and enhancements of fbgrab have to fulfill this too.
 *
 * $Id: fbgrab.c 16997 2020-03-28 02:15:30Z NiLuJe $
 * $Revision: 16997 $
 *
 */

// Make KDevelop happy.
#ifndef _DEFAULT_SOURCE
#	define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <getopt.h>
#include <linux/fb.h> /* to handle framebuffer ioctls */
#include <png.h>      /* PNG lib */
#include <string.h>
#include <sys/vt.h> /* to handle vt changing */
#include <zlib.h>

// Make sure we have a fallback version if the Makefile wasn't used.
#ifndef VERSION
#	define VERSION "1.3.N"
#endif
#define DEFAULT_FB "/dev/fb0"
#define MAX_LEN    512
#define UNDEFINED  -1

// CLOEXEC is probably a no-go on FW 2.x...
#ifdef KINDLE_LEGACY
#	ifdef O_CLOEXEC
#		undef O_CLOEXEC
#	endif
#	define O_CLOEXEC 0
#	define STDIO_CLOEXEC
#else
#	define STDIO_CLOEXEC "e"
#endif

static uint32_t srcBlue        = 0U;
static uint32_t srcGreen       = 1U;
static uint32_t srcRed         = 2U;
static uint32_t srcAlpha       = 3U;
bool            srcAlphaIsUsed = false;

#define B 0
#define G 1
#define R 2
#define A 3

static void
    fatal_error(const char* restrict message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

static void
    usage(char* restrict binary)
{
	fprintf(stderr,
		"Usage:   %s\t[-hikv] [-{C|c} vt] [-d dev] [-s n] [-z n]\n"
		"\t\t[-f fromfile -w n -h n -b n] filename.png\n",
		binary);
}

static void
    help(char* restrict binary)
{
	fprintf(stderr, "fbgrab - takes screenshots using the framebuffer, v%s\n", VERSION);

	usage(binary);

	fprintf(stderr, "\nPossible options:\n");
	/* please keep this list alphabetical */
	fprintf(stderr, "\t-b n  \tforce use of n bits/pixel, required when reading from file\n");
	fprintf(stderr, "\t-C n  \tgrab from console n, for slower framebuffers\n");
	fprintf(stderr, "\t-c n  \tgrab from console n\n");
	fprintf(stderr, "\t-d dev\tuse framebuffer device dev instead of default\n");
	fprintf(stderr, "\t-f file\t read from file instead of framebuffer\n");
	fprintf(stderr,
		"\t-h n  \tset height to n pixels, required when reading from file\n"
		"\t\tcan be used to force height when reading from framebuffer\n");
	fprintf(stderr, "\t-i    \tturns on interlacing in PNG\n");
	fprintf(stderr, "\t-k    \tdisables automagic cropping (Kindle/Kobo)\n");
	fprintf(stderr, "\t-s n  \tsleep n seconds before making screenshot\n");
	fprintf(stderr, "\t-v    \tverbose, print debug information.\n");
	fprintf(stderr,
		"\t-w n  \tset width to n pixels, required when reading from file\n"
		"\t\tcan be used to force width when reading from framebuffer\n");
	fprintf(stderr, "\t-z n  \tPNG compression level: 0 (fast) .. 9 (best)\n");
	fprintf(stderr, "\t-?    \tprint this usage information\n");
}

static void
    chvt(int num)
{
	int fd;

	if ((fd = open("/dev/console", O_RDWR | O_CLOEXEC)) == -1) {
		perror("open");
		fatal_error("Cannot open /dev/console");
	}

	if (ioctl(fd, VT_ACTIVATE, num) != 0) {
		perror("ioctl VT_ACTIVATE");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, VT_WAITACTIVE, num) != 0) {
		perror("ioctl VT_WAITACTIVE");
		exit(EXIT_FAILURE);
	}

	(void) close(fd);
}

static unsigned short int
    change_to_vt(unsigned short int vt_num)
{
	int                fd;
	unsigned short int old_vt;
	struct vt_stat     vt_info = { 0 };

	if ((fd = open("/dev/console", O_RDONLY | O_CLOEXEC)) == -1) {
		perror("open");
		fatal_error("Couldn't open /dev/console");
	}

	if (ioctl(fd, VT_GETSTATE, &vt_info) != 0) {
		perror("ioctl VT_GETSTATE");
		exit(EXIT_FAILURE);
	}

	(void) close(fd);

	old_vt = vt_info.v_active;

	chvt((int) vt_num); /* go there for information */

	return old_vt;
}

static void
    get_framebufferdata(const char* restrict      device,
			struct fb_fix_screeninfo* fb_fixedinfo_p,
			struct fb_var_screeninfo* fb_varinfo_p,
			bool                      verbose)
{
	int fd;

	/* now open framebuffer device */
	if ((fd = open(device, O_RDONLY | O_CLOEXEC)) == -1) {
		perror("open");
		fprintf(stderr, "Error: Couldn't open %s.\n", device);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, fb_varinfo_p) != 0) {
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, fb_fixedinfo_p) != 0) {
		perror("ioctl FBIOGET_FSCREENINFO");
		exit(EXIT_FAILURE);
	}

	if (verbose) {
		fprintf(stderr, "Fixed framebuffer info:\n");
		fprintf(stderr, "id: \"%s\"\n", fb_fixedinfo_p->id);
		fprintf(stderr, "start of fb mem: %#lx\n", fb_fixedinfo_p->smem_start);
		fprintf(stderr, "length of fb mem: %u bytes\n", fb_fixedinfo_p->smem_len);
		switch (fb_fixedinfo_p->type) {
			case FB_TYPE_PACKED_PIXELS:
				fprintf(stderr, "type: packed pixels\n");
				break;
			case FB_TYPE_PLANES:
				fprintf(stderr, "type: non interleaved planes\n");
				break;
			case FB_TYPE_INTERLEAVED_PLANES:
				fprintf(stderr, "type: interleaved planes\n");
				break;
			case FB_TYPE_TEXT:
				fprintf(stderr, "type: text/attributes\n");
				break;
			case FB_TYPE_VGA_PLANES:
				fprintf(stderr, "type: EGA/VGA planes\n");
				break;
#ifdef FB_TYPE_FOURCC
			case FB_TYPE_FOURCC:
				fprintf(stderr, "type: identified by a V4L2 FOURCC\n");
				break;
#endif
			default:
				fprintf(stderr, "type: undefined! (%u)\n", fb_fixedinfo_p->type);
				break;
		}
		fprintf(stderr, "interleave for interleaved planes: %u\n", fb_fixedinfo_p->type_aux);
		switch (fb_fixedinfo_p->visual) {
			case FB_VISUAL_MONO01:
				fprintf(stderr, "visual: monochrome, B=1 W=0\n");
				break;
			case FB_VISUAL_MONO10:
				fprintf(stderr, "visual: monochrome, W=1 B=0\n");
				break;
			case FB_VISUAL_TRUECOLOR:
				fprintf(stderr, "visual: true color\n");
				break;
			case FB_VISUAL_PSEUDOCOLOR:
				fprintf(stderr, "visual: pseudo color\n");
				break;
			case FB_VISUAL_DIRECTCOLOR:
				fprintf(stderr, "visual: direct color\n");
				break;
			case FB_VISUAL_STATIC_PSEUDOCOLOR:
				fprintf(stderr, "visual: pseudo color readonly\n");
				break;
#ifdef FB_VISUAL_FOURCC
			case FB_VISUAL_FOURCC:
				fprintf(stderr, "visual: identified by a V4L2 FOURCC\n");
				break;
#endif
			default:
				fprintf(stderr, "visual: undefined! (%u)\n", fb_fixedinfo_p->visual);
				break;
		}
		fprintf(stderr, "hw panning x step: %hu\n", fb_fixedinfo_p->xpanstep);
		fprintf(stderr, "hw panning y step: %hu\n", fb_fixedinfo_p->ypanstep);
		fprintf(stderr, "hw y wrap: %hu\n", fb_fixedinfo_p->ywrapstep);
		// Avoid a division by zero on < 8bpp fbs, do the maths w/ a FP dividend...
		fprintf(stderr,
			"line length: %u bytes (%.0f pixels)\n",
			fb_fixedinfo_p->line_length,
			fb_fixedinfo_p->line_length / ((double) fb_varinfo_p->bits_per_pixel / 8U));
		fprintf(stderr, "start of mmio: %#lx\n", fb_fixedinfo_p->mmio_start);
		fprintf(stderr, "length of mmio: %u\n", fb_fixedinfo_p->mmio_len);
		fprintf(stderr, "accel chip/card: %u\n", fb_fixedinfo_p->accel);
#ifdef FB_CAP_FOURCC
		switch (fb_fixedinfo_p->capabilities) {
			case FB_CAP_FOURCC:
				fprintf(stderr, "capabilities: supports FOURCC-based formats\n");
				break;
			default:
				fprintf(stderr, "capabilities: undefined! (%u)\n", fb_fixedinfo_p->capabilities);
				break;
		}
#endif

		fprintf(stderr, "\nVariable framebuffer info:\n");
		fprintf(stderr, "visible resolution: %ux%u\n", fb_varinfo_p->xres, fb_varinfo_p->yres);
		fprintf(stderr, "virtual resolution: %ux%u\n", fb_varinfo_p->xres_virtual, fb_varinfo_p->yres_virtual);
		fprintf(stderr,
			"offset from virtual to visible resolution: %ux%u\n",
			fb_varinfo_p->xoffset,
			fb_varinfo_p->yoffset);
		fprintf(stderr, "bits per pixel: %u\n", fb_varinfo_p->bits_per_pixel);
		switch (fb_varinfo_p->bits_per_pixel) {
			case 4U:
				switch (fb_varinfo_p->grayscale) {
					case 0:
						fprintf(stderr, "grayscale: false\n");
						break;
					case 1:
						// NOTE: einkfb happily uses 1 here...
						fprintf(stderr, "grayscale: true\n");
						break;
					case 0x3:
						// GRAYSCALE_4BIT
						fprintf(stderr, "grayscale: true (GRAYSCALE_4BIT)\n");
						break;
					case 0x4:
						// GRAYSCALE_4BIT_INVERTED
						fprintf(stderr, "grayscale: true (GRAYSCALE_4BIT_INVERTED)\n");
						break;
					default:
						fprintf(stderr, "grayscale: undefined! (%u)\n", fb_varinfo_p->grayscale);
						break;
				}
				break;
			case 8U:
				switch (fb_varinfo_p->grayscale) {
					case 0:
						fprintf(stderr, "grayscale: false\n");
						break;
					case 0x1:
						// GRAYSCALE_8BIT
						fprintf(stderr, "grayscale: true\n");
						break;
					case 0x2:
						// GRAYSCALE_8BIT_INVERTED
						fprintf(stderr, "grayscale: true (inverted)\n");
						break;
					default:
						fprintf(stderr, "grayscale: undefined! (%u)\n", fb_varinfo_p->grayscale);
						break;
				}
				break;
			default:
				switch (fb_varinfo_p->grayscale) {
					case 0:
						fprintf(stderr, "grayscale: false\n");
						break;
					case 1:
						fprintf(stderr, "grayscale: true\n");
						break;
					default:
						// NOTE: > 1 = FOURCC
						fprintf(
						    stderr, "grayscale: false (FOURCC: %u)\n", fb_varinfo_p->grayscale);
						break;
				}
				break;
		}
		fprintf(stderr,
			"red:   bitfield offset: %u, bitfield length: %u, MSB is right: %s\n",
			fb_varinfo_p->red.offset,
			fb_varinfo_p->red.length,
			fb_varinfo_p->red.msb_right == 0U ? "no" : "yes");
		fprintf(stderr,
			"green: bitfield offset: %u, bitfield length: %u, MSB is right: %s\n",
			fb_varinfo_p->green.offset,
			fb_varinfo_p->green.length,
			fb_varinfo_p->green.msb_right == 0U ? "no" : "yes");
		fprintf(stderr,
			"blue:  bitfield offset: %u, bitfield length: %u, MSB is right: %s\n",
			fb_varinfo_p->blue.offset,
			fb_varinfo_p->blue.length,
			fb_varinfo_p->blue.msb_right == 0U ? "no" : "yes");
		fprintf(stderr,
			"alpha: bitfield offset: %u, bitfield length: %u, MSB is right: %s\n",
			fb_varinfo_p->transp.offset,
			fb_varinfo_p->transp.length,
			fb_varinfo_p->transp.msb_right == 0U ? "no" : "yes");
		fprintf(stderr, "pixel format: %s\n", fb_varinfo_p->nonstd == 0U ? "standard" : "non-standard");
		fprintf(stderr, "activate: %u\n", fb_varinfo_p->activate);
		fprintf(stderr, "height: %u mm\n", fb_varinfo_p->height);
		fprintf(stderr, "width: %u mm\n", fb_varinfo_p->width);
		fprintf(stderr, "obsolete accel_flags: %u\n", fb_varinfo_p->accel_flags);
		fprintf(stderr, "pixel clock: %u ps\n", fb_varinfo_p->pixclock);
		fprintf(stderr, "time from sync to picture (left margin): %u pixclocks\n", fb_varinfo_p->left_margin);
		fprintf(stderr, "time from picture to sync (right margin): %u pixclocks\n", fb_varinfo_p->right_margin);
		fprintf(stderr, "time from sync to picture (upper margin): %u pixclocks\n", fb_varinfo_p->upper_margin);
		fprintf(stderr, "time from picture to sync (lower margin): %u pixclocks\n", fb_varinfo_p->lower_margin);
		fprintf(stderr, "length of horizontal sync: %u pixclocks\n", fb_varinfo_p->hsync_len);
		fprintf(stderr, "length of vertical sync: %u pixclocks\n", fb_varinfo_p->vsync_len);
		switch (fb_varinfo_p->sync) {
			case FB_SYNC_HOR_HIGH_ACT:
				fprintf(stderr, "sync: horizontal sync high active\n");
				break;
			case FB_SYNC_VERT_HIGH_ACT:
				fprintf(stderr, "sync: vertical sync high active\n");
				break;
			case FB_SYNC_EXT:
				fprintf(stderr, "sync: external sync\n");
				break;
			case FB_SYNC_COMP_HIGH_ACT:
				fprintf(stderr, "sync: composite sync high active\n");
				break;
			case FB_SYNC_BROADCAST:
				fprintf(stderr, "sync: broadcast video timings\n");
				break;
			case FB_SYNC_ON_GREEN:
				fprintf(stderr, "sync: sync on green\n");
				break;
			default:
				fprintf(stderr, "sync: undefined! (%u)\n", fb_varinfo_p->sync);
				break;
		}
		switch (fb_varinfo_p->vmode) {
			case FB_VMODE_NONINTERLACED:
				fprintf(stderr, "vmode: non interlaced\n");
				break;
			case FB_VMODE_INTERLACED:
				fprintf(stderr, "vmode: interlaced\n");
				break;
			case FB_VMODE_DOUBLE:
				fprintf(stderr, "vmode: double scan\n");
				break;
			case FB_VMODE_ODD_FLD_FIRST:
				fprintf(stderr, "vmode: interlaced: top line first\n");
				break;
			case FB_VMODE_MASK:
				fprintf(stderr, "vmode: mask\n");
				break;
			default:
				fprintf(stderr, "vmode: undefined! (%u)\n", fb_varinfo_p->vmode);
				break;
		}
		fprintf(stderr, "rotate: %u\n", fb_varinfo_p->rotate);
#ifdef FB_TYPE_FOURCC
		fprintf(stderr, "colorspace for FOURCC-based modes: %u\n", fb_varinfo_p->colorspace);
#endif

		// Attempt to print color map info, where applicable...
		// NOTE: All Kindles are using FB_VISUAL_STATIC_PSEUDOCOLOR, the DIRECTCOLOR bit comes from fbida
		if ((fb_varinfo_p->bits_per_pixel == 8 || fb_varinfo_p->bits_per_pixel == 4) ||
		    fb_fixedinfo_p->visual == FB_VISUAL_DIRECTCOLOR) {
			// NOTE: Always leave enough space for 8bpp, to keep things simple.
			uint16_t cmr[256];
			uint16_t cmg[256];
			uint16_t cmb[256];
			// NOTE: Apparently, we have to supply len ourselves... yay?
			//       (Which makes sense, since we're the ones providing the storage space).
			//       In our case, it should be equal to (bpp/2) ** 4 (i.e., 256 @ 8bpp, 16 @ 4bpp).
			struct fb_cmap cmap = { 0, fb_varinfo_p->bits_per_pixel == 4U ? 16 : 256, cmr, cmg, cmb, NULL };

			if (ioctl(fd, FBIOGETCMAP, &cmap) != 0) {
				// Make that non-fatal, because apparently hardware support is spotty at best...
				perror("ioctl FBIOGETCMAP");
			} else {
				fprintf(stderr, "\nColor map info:\n");
				fprintf(stderr, "start: %u, len: %u\n", cmap.start, cmap.len);
				fprintf(stderr, "  N\t #R\t #G\t #B\n");
				// NOTE: We'll try to output that as an actual palette, i.e., unique colors,
				//       because that plays well with eInk palettes, where that allows us to only
				//       print 16 lines instead of 256 ;).
				// Init to the first color so as not to break our range detection...
				unsigned int n     = cmap.start;
				uint16_t     lastr = cmap.red[n];
				uint16_t     lastg = cmap.green[n];
				uint16_t     lastb = cmap.blue[n];
				for (unsigned int i = cmap.start; i < cmap.len; i++) {
					// Is it a new color?
					if (cmap.red[i] != lastr && cmap.green[i] != lastg && cmap.blue[i] != lastb) {
						// Then recap the previous range!
						fprintf(stderr,
							"%02X-%02X\t%04X\t%04X\t%04X\n",
							n,
							(i - 1),
							lastr,
							lastg,
							lastb);
						n = i;
					}
					lastr = cmap.red[i];
					lastg = cmap.green[i];
					lastb = cmap.blue[i];
				}
				// And that leaves the final entry...
				fprintf(stderr, "%02X-%02X\t%04X\t%04X\t%04X\n", n, (cmap.len - 1U), lastr, lastg, lastb);
			}
		}
	}
	srcBlue  = fb_varinfo_p->blue.offset >> 3U;
	srcGreen = fb_varinfo_p->green.offset >> 3U;
	srcRed   = fb_varinfo_p->red.offset >> 3U;

	if (fb_varinfo_p->transp.length > 0U) {
		srcAlpha       = fb_varinfo_p->transp.offset >> 3U;
		srcAlphaIsUsed = true;
	} else {
		// NOTE: We'll need an accurate idea of *where* the alpha/padding component is in convertRGB32to32...
		//       We can't compute it, but we can attempt to discriminate ARGB/ABGR from BGRA/RGBA via simple heuristics.
		if (srcBlue == 0U || srcRed == 0U) {
			// BGRA/RGBA
			srcAlpha = 3U;
		} else {
			// ARGB/ABGR
			srcAlpha = 0U;
		}
		srcAlphaIsUsed = false;    // not used
	}

	if (verbose) {
		fprintf(stderr, "\nPixel format details:\n");
		if (fb_varinfo_p->bits_per_pixel == 32U) {
			if (fb_varinfo_p->transp.length > 0U) {
				fprintf(
				    stderr, "B: %u / G: %u / R: %u / A: %u (i.e., ", srcBlue, srcGreen, srcRed, srcAlpha);
			} else {
				fprintf(stderr, "B: %u / G: %u / R: %u / A: ∅ (i.e., ", srcBlue, srcGreen, srcRed);
			}
			for (uint8_t i = B; i <= A; i++) {
				if (i == srcBlue) {
					fprintf(stderr, "B");
				} else if (i == srcGreen) {
					fprintf(stderr, "G");
				} else if (i == srcRed) {
					fprintf(stderr, "R");
				} else if (i == srcAlpha) {
					fprintf(stderr, "A");
				} else {
					fprintf(stderr, "∅");
				}
			}
			fprintf(stderr, ")\n");
		} else if (fb_varinfo_p->bits_per_pixel == 24U) {
			fprintf(stderr, "B: %u / G: %u / R: %u (i.e., ", srcBlue, srcGreen, srcRed);
			for (uint8_t i = B; i <= R; i++) {
				if (i == srcBlue) {
					fprintf(stderr, "B");
				} else if (i == srcGreen) {
					fprintf(stderr, "G");
				} else if (i == srcRed) {
					fprintf(stderr, "R");
				} else {
					fprintf(stderr, "∅");
				}
			}
			fprintf(stderr, ")\n");
		} else {
			fprintf(stderr,
				"B: %u >> %u\nG: %u >> %u\nR: %u >> %u\nA: %u >> %u\n",
				fb_varinfo_p->blue.length,
				fb_varinfo_p->blue.offset,
				fb_varinfo_p->green.length,
				fb_varinfo_p->green.offset,
				fb_varinfo_p->red.length,
				fb_varinfo_p->red.offset,
				fb_varinfo_p->transp.length,
				fb_varinfo_p->transp.offset);
		}
		fprintf(stderr, "Should alpha be honored? %s\n", srcAlphaIsUsed ? "Yes." : "No.");
		fprintf(stderr, "\n");
	}

	(void) close(fd);
}

static void
    convert1555to32(unsigned int                  width,
		    unsigned int                  height,
		    const unsigned char* restrict inbuffer,
		    unsigned char* restrict       outbuffer)
{
	size_t outoff;
	for (unsigned int i = 0U; i < (height * width) * 2U; i += 2U) {
		outoff                = i << 1U;                                             // * 2
		outbuffer[outoff + B] = (unsigned char) ((inbuffer[i + 1] & 0x7C) << 1U);    // B
		outbuffer[outoff + G] =
		    (unsigned char) ((((inbuffer[i + 1] & 0x3) << 3U) | ((inbuffer[i] & 0xE0) >> 5U)) << 3U);    // G
		outbuffer[outoff + R] = (unsigned char) ((inbuffer[i] & 0x1F) << 3U);                            // R
		outbuffer[outoff + A] = (inbuffer[i] & 0x80) ? 0xFF : 0;                                         // A
	}
}

/*
static void
    convert565to32(unsigned int         width,
		   unsigned int         height,
		   const unsigned char* restrict inbuffer,
		   unsigned char* restrict outbuffer)
{
	size_t outoff;
	for (unsigned int i = 0U; i < (height * width) * 2U; i += 2U) {
#ifdef __BIG_ENDIAN__
		const uint16_t v = (uint16_t)((inbuffer[i] << 8U) + inbuffer[i + 1U]);
#else
		const uint16_t v = (uint16_t)((inbuffer[i + 1] << 8U) + inbuffer[i]);
#endif

		outoff = i << 1U;    // * 2

		// NOTE: c.f., https://stackoverflow.com/q/2442576
		//       Extremely similar to what we do in KOReader
		const uint8_t r = (uint8_t)((v & 0xf800) >> 11U);
		const uint8_t g = (v & 0x07e0) >> 5U;
		const uint8_t b = (v & 0x001f);

		outbuffer[outoff + B] = (uint8_t)((b << 3U) | (b >> 2U));    // B
		outbuffer[outoff + G] = (uint8_t)((g << 2U) | (g >> 4U));    // G
		outbuffer[outoff + R] = (uint8_t)((r << 3U) | (r >> 2U));    // R

		// NOTE: Once again, ported from KOReader ;)
		//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/blitbuffer.lua#L355
		//        & https://github.com/koreader/koreader-base/blob/master/blitbuffer.c#L50
		//b                     = v & 0x001F;
		//outbuffer[outoff + B] = (unsigned char) ((b << 3) + (b >> 2));    // B
		//g                     = (v >> 5) & 0x3F;
		//outbuffer[outoff + G] = (unsigned char) ((g << 2) + (g >> 4));    // G
		//r                     = (v >> 11);
		//outbuffer[outoff + R] = (unsigned char) ((r << 3) + (r >> 2));    // R

		// NOTE: Here's FBGrab's original implementation,
		//       which appears to be ever-so-slightly off (white @ 248,252,248), at least for Kobo fbs...
		//outbuffer[outoff+B] = (unsigned char) ((v << 3) & 0xF8);		// B
		//outbuffer[outoff+G] = (unsigned char) ((v >> 3) & 0xFC);		// G
		//outbuffer[outoff+R] = (unsigned char) ((v >> 8) & 0xF8);		// R

		// NOTE: And the perl script implementation, which is as off as FBGrab's original implementation...
		//       c.f., http://www.mobileread.com/forums/showpost.php?p=2366353&postcount=1
		// NOTE: This roughly matches SDL's naive implementation, too
		//       https://hg.libsdl.org/SDL/file/3e05d58dc84f/src/video/SDL_blit.h
		//outbuffer[outoff+B] = (unsigned char) ((v & 0x001F) << 3);		// B
		//outbuffer[outoff+G] = (unsigned char) ((v & 0x07E0) >> 3);		// G
		//outbuffer[outoff+R] = (unsigned char) ((v & 0xF800) >> 8);		// R

		outbuffer[outoff + A] = 0xFF;    // A
	}
}
*/

static void
    convert565toRGB(unsigned int                  width,
		    unsigned int                  height,
		    const unsigned char* restrict inbuffer,
		    unsigned char* restrict       outbuffer)
{
	size_t outoff;
	for (unsigned int i = 0U; i < ((height * width) << 1U); i += 2U) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
		const uint16_t v = *((const uint16_t*) (inbuffer + i));
#pragma GCC diagnostic pop

		outoff = (i >> 1U) * 3U;

		// NOTE: c.f., https://stackoverflow.com/q/2442576
		//       Extremely similar to what we do in KOReader
		const uint8_t r = (uint8_t)((v & 0xf800) >> 11U);
		const uint8_t g = (v & 0x07e0) >> 5U;
		const uint8_t b = (v & 0x001f);

		outbuffer[outoff]      = (uint8_t)((r << 3U) | (r >> 2U));    // R
		outbuffer[outoff + 1U] = (uint8_t)((g << 2U) | (g >> 4U));    // G
		outbuffer[outoff + 2U] = (uint8_t)((b << 3U) | (b >> 2U));    // B
	}
}

static void
    convertRGB32to32(unsigned int                  width,
		     unsigned int                  height,
		     const unsigned char* restrict inbuffer,
		     unsigned char* restrict       outbuffer)
{
	// NOTE: We basically just need to make sure alpha is consistently set to fully opaque.
	//       The Linux console will properly ignore it and set it to 0,
	//       but other stuff (i.e., FBInk) will only see a 32bpp fb, and will set the alpha byte,
	//       not knowing to ignore it because fb_varinfo_p->transp.length is 0.
	memcpy(outbuffer, inbuffer, (width * height) << 2U);
	const unsigned char* end = outbuffer + ((width * height) << 2U);
	// Start at the first alpha byte, then loop over each subsequent alpha byte, pixel-by-pixel
	for (unsigned char* p = outbuffer + srcAlpha; p <= end; p += 4U) {
		*p = 0xFF;
	}
}

/*
static void
    convert888to32(unsigned int         width,
		   unsigned int         height,
		   const unsigned char* restrict inbuffer,
		   unsigned char* restrict outbuffer)
{
	size_t outoff;
	size_t inoff;
	for (unsigned int i = 0U; i < height * width; i++) {
		outoff                = i << 2U;    // * 4
		inoff                 = i * 3U;
		outbuffer[outoff + B] = inbuffer[inoff + srcBlue];     // B
		outbuffer[outoff + G] = inbuffer[inoff + srcGreen];    // G
		outbuffer[outoff + R] = inbuffer[inoff + srcRed];      // R
		outbuffer[outoff + A] = 0xFF;                          // A
	}
}
*/

/*
static void
    convert8888to32(unsigned int         width,
		    unsigned int         height,
		    const unsigned char* restrict inbuffer,
		    unsigned char* restrict outbuffer)
{
	size_t outoff;
	size_t inoff;
	for (unsigned int i = 0U; i < height * width; i++) {
		outoff                = i << 2U;    // * 4
		inoff                 = outoff;
		outbuffer[outoff + B] = inbuffer[inoff + srcBlue];                             // B
		outbuffer[outoff + G] = inbuffer[inoff + srcGreen];                            // G
		outbuffer[outoff + R] = inbuffer[inoff + srcRed];                              // R
		outbuffer[outoff + A] = srcAlphaIsUsed ? inbuffer[inoff + srcAlpha] : 0xFF;    // A
	}
}
*/

/*
static void
    convertgray8to32(unsigned int   width,
		     unsigned int   height,
		     const unsigned char* restrict inbuffer,
		     unsigned char* restrict outbuffer,
		     bool           invert)
{
	size_t outoff;
	for (unsigned int i = 0U; i < height * width; i++) {
		outoff = i << 2U;    // * 4
		// NOTE: Gray means B = G = R == i, and RGBA offsets == 0.
		// Needs to be inverted on the K4 for some reason (weird driver shims mixing eink_fb & mxc_epdc_fb)...
		outbuffer[outoff + B] = outbuffer[outoff + G] = outbuffer[outoff + R] =
		    invert ? (inbuffer[i] ^ 0xFF) : inbuffer[i];    // BGR
		outbuffer[outoff + A] = 0xFF;                       // A
	}
}
*/

static void
    invertgray8(unsigned int                  width,
		unsigned int                  height,
		const unsigned char* restrict inbuffer,
		unsigned char* restrict       outbuffer)
{
	for (unsigned int i = 0U; i < height * width; i++) {
		// Needs to be inverted on the K4 for some reason (weird driver shims mixing eink_fb & mxc_epdc_fb)...
		outbuffer[i] = inbuffer[i] ^ 0xFF;
	}
}

/*
static void
    convertgray4to32(unsigned int width, unsigned int height, const unsigned char* restrict inbuffer, unsigned char* restrict outbuffer)
{
	size_t outoff;
	size_t inoff;
	for (unsigned int i = 0U; i < height * width; i += 2U) {
		// NOTE: Still grayscale, so B = G = R, and the RGBA offsets are useless.
		// AFAIK, always needs to be inverted on the Kindle devices sporting that kind of fb.
		// Ported from KOReader's https://github.com/koreader/koreader-base/blob/master/ffi/blitbuffer.lua#L270
		// See also https://github.com/dpavlin/k3libre/blob/master/fb2pgm.pl for an alternative
		// (w/ a slight contrast issue [too light]), and https://github.com/koreader/koreader/wiki/porting &
		// https://github.com/koreader/koreader-base/blob/c109f02680ca55ba2515bf11d28dd928b19df416/einkfb.c for
		// more background info.
		inoff = i >> 1U;    // / 2
		// First pixel
		outoff                = i << 2U;    // * 4
		outbuffer[outoff + B] = outbuffer[outoff + G] = outbuffer[outoff + R] =
		    (unsigned char) (((inbuffer[inoff] & 0xF0) | ((inbuffer[inoff] & 0xF0) >> 4)) ^ 0xFF);    // BGR
		outbuffer[outoff + A] = 0xFF;                                                                 // A

		// Second pixel
		outoff                = ((i + 1U) << 2U);    // * 4
		outbuffer[outoff + B] = outbuffer[outoff + G] = outbuffer[outoff + R] =
		    (unsigned char) (((inbuffer[inoff] & 0x0F) * 0x11) ^ 0xFF);    // BGR
		outbuffer[outoff + A] = 0xFF;                                      // A
	}
}
*/

static void
    convertgray4togray8(unsigned int                  width,
			unsigned int                  height,
			const unsigned char* restrict inbuffer,
			unsigned char* restrict       outbuffer)
{
	size_t outoff;
	size_t inoff;
	for (unsigned int i = 0U; i < height * width; i += 2U) {
		// AFAIK, always needs to be inverted on the Kindle devices sporting that kind of fb.
		// Ported from KOReader's https://github.com/koreader/koreader-base/blob/master/ffi/blitbuffer.lua#L270
		// See also https://github.com/dpavlin/k3libre/blob/master/fb2pgm.pl for an alternative
		// (w/ a slight contrast issue [too light]), and https://github.com/koreader/koreader/wiki/porting &
		// https://github.com/koreader/koreader-base/blob/c109f02680ca55ba2515bf11d28dd928b19df416/einkfb.c for
		// more background info.
		inoff           = i >> 1U;    // / 2
		const uint8_t b = inbuffer[inoff];
		// First pixel (high nibble)
		outoff            = i;
		outbuffer[outoff] = (unsigned char) (((b & 0xF0) | ((b & 0xF0) >> 4)) ^ 0xFF);

		// Second pixel (low nibble)
		outoff            = i + 1U;
		outbuffer[outoff] = (unsigned char) (((b & 0x0F) * 0x11) ^ 0xFF);
	}
}

static void
    write_PNG(unsigned char* restrict outbuffer,
	      const char* restrict    filename,
	      unsigned int            width,
	      unsigned int            height,
	      int                     interlace,
	      int                     compression,
	      short int               x_offset,
	      short int               y_offset,
	      int                     color_type,
	      bool                    is_bgr)
{
	png_bytep   row_pointers[height];
	png_structp png_ptr;
	png_infop   info_ptr;
	FILE*       outfile;

	if (strcmp(filename, "-") == 0) {
		outfile = stdout;
	} else {
		outfile = fopen(filename, "wb" STDIO_CLOEXEC);
		if (!outfile) {
			perror("fopen");
			fprintf(stderr, "Error: Couldn't open %s.\n", filename);
			exit(EXIT_FAILURE);
		}
	}

	// NOTE: Tweak the pitch if we have a positive x offset...
	const size_t effective_width = x_offset > 0 ? (width + (unsigned int) x_offset) : width;
	size_t       pitch;
	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			pitch = effective_width;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			pitch = effective_width << 1U;
			break;
		case PNG_COLOR_TYPE_RGB:
			pitch = effective_width * 3U;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
		default:
			pitch = effective_width << 2U;
			break;
	}
	// NOTE: Kobo hack: crop what's hidden by the bezel (on the top).
	//       The height we're passed has already been cropped ;).
	if (y_offset > 0) {
		for (unsigned int i = 0U; i < height; i++) {
			row_pointers[i] = outbuffer + ((i + (unsigned int) y_offset) * pitch);
		}
	} else {
		// Otherwise, go as planned.
		for (unsigned int i = 0U; i < height; i++) {
			row_pointers[i] = outbuffer + (i * pitch);
		}
	}

	// NOTE: Kindle/Kobo hack: crop the garbage on the sides.
	// Note that we need to have read outbuffer with its original width before tweaking width itself.
	width -= (unsigned int) abs(x_offset);

	png_ptr =
	    png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL);

	if (!png_ptr) {
		fatal_error("Error: Couldn't create PNG write struct.");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		fatal_error("Error: Couldn't create PNG info struct.");
	}

	png_init_io(png_ptr, outfile);

	png_set_IHDR(png_ptr,
		     info_ptr,
		     width,
		     height,
		     8,
		     color_type,
		     interlace,
		     PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT);

	if (is_bgr) {
		png_set_bgr(png_ptr);
	}

	png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FAST_FILTERS);
	png_set_compression_level(png_ptr, compression);

	png_write_info(png_ptr, info_ptr);

	fprintf(stderr, "Now writing PNG file (compression %d)\n", compression);

	png_write_image(png_ptr, row_pointers);

	png_write_end(png_ptr, info_ptr);
	/* puh, done, now freeing memory... */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	if (outfile != NULL) {
		(void) fclose(outfile);
	}
}

static void
    convert_and_write(const unsigned char* restrict inbuffer,
		      const char* restrict          filename,
		      unsigned int                  width,
		      unsigned int                  height,
		      unsigned int                  bits,
		      int                           interlace,
		      int                           compression,
		      bool                          invert,
		      short int                     x_offset,
		      short int                     y_offset)
{
	unsigned char* outbuffer = NULL;

	// We don't need an intermediary buffer for uninverted 8bpp fbs, or for 24bpp, or 32bpp fbs w/ true alpha ;).
	if ((bits == 8U && !invert) || bits == 24U || (bits == 32U && srcAlphaIsUsed)) {
		fprintf(stderr, "No conversion needed\n");
		// NOP :)
	} else {
		// We won't expand Grayscale to RGB32
		size_t bufsize = (size_t)(width * height);
		// Assume 4bpp & 8bpp are grayscale
		if (bits > 8U) {
			if (bits == 16U) {
				// We convert RGB565 to RGB, not BGRA
				bufsize *= 3U;
				fprintf(stderr, "Converting to RGB\n");
			} else if (bits == 32U && !srcAlphaIsUsed) {
				// We convert RGB32 to true 32bpp, in the same component order as the source.
				fprintf(stderr, "Converting to true 32bpp\n");
				bufsize <<= 2U;
			} else {
				fprintf(stderr, "Converting to BGRA\n");
				bufsize <<= 2U;
			}
		} else {
			fprintf(stderr, "Converting to Gray8\n");
		}

		fprintf(stderr, "Allocating %zu bytes for conversion buffer\n", bufsize);
		outbuffer = calloc(bufsize, sizeof(*outbuffer));
		if (outbuffer == NULL) {
			perror("calloc");
			fatal_error("Not enough memory");
		}
	}

	// NOTE: Tweak height according to the offsets between the two steps to handle the Kobo cropping.
	unsigned int cropped_height = height - (unsigned int) abs(y_offset);

	switch (bits) {
		case 4U:
			convertgray4togray8(width, height, inbuffer, outbuffer);
			write_PNG(outbuffer,
				  filename,
				  width,
				  cropped_height,
				  interlace,
				  compression,
				  x_offset,
				  y_offset,
				  PNG_COLOR_TYPE_GRAY,
				  false);
			break;
		case 8U:
			if (invert) {
				invertgray8(width, height, inbuffer, outbuffer);
				write_PNG(outbuffer,
					  filename,
					  width,
					  cropped_height,
					  interlace,
					  compression,
					  x_offset,
					  y_offset,
					  PNG_COLOR_TYPE_GRAY,
					  false);
			} else {
				// Passthrough :)
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma GCC diagnostic   ignored "-Wdiscarded-qualifiers"
                                write_PNG(inbuffer,
                                          filename,
                                          width,
                                          cropped_height,
                                          interlace,
                                          compression,
                                          x_offset,
                                          y_offset,
                                          PNG_COLOR_TYPE_GRAY,
                                          false);
#pragma GCC diagnostic pop
			}
			break;
		case 15U:
			convert1555to32(width, height, inbuffer, outbuffer);
			write_PNG(outbuffer,
				  filename,
				  width,
				  cropped_height,
				  interlace,
				  compression,
				  x_offset,
				  y_offset,
				  PNG_COLOR_TYPE_RGB_ALPHA,
				  true);
			break;
		case 16U:
			convert565toRGB(width, height, inbuffer, outbuffer);
			write_PNG(outbuffer,
				  filename,
				  width,
				  cropped_height,
				  interlace,
				  compression,
				  x_offset,
				  y_offset,
				  PNG_COLOR_TYPE_RGB,
				  false);
			break;
		case 24U:
			// Passthrough :)
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma GCC diagnostic   ignored "-Wdiscarded-qualifiers"
                        write_PNG(inbuffer,
                                  filename,
                                  width,
                                  cropped_height,
                                  interlace,
                                  compression,
                                  x_offset,
                                  y_offset,
                                  PNG_COLOR_TYPE_RGB,
                                  srcBlue < srcRed);
#pragma GCC diagnostic pop
			break;
		case 32U:
			if (srcAlphaIsUsed) {
				// Passthrough :)
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma GCC diagnostic   ignored "-Wdiscarded-qualifiers"
                                write_PNG(inbuffer,
                                          filename,
                                          width,
                                          cropped_height,
                                          interlace,
                                          compression,
                                          x_offset,
                                          y_offset,
                                          PNG_COLOR_TYPE_RGB_ALPHA,
                                          srcBlue < srcRed);
#pragma GCC diagnostic pop
			} else {
				// Enforce a real opaque alpha channel in RGB32
				convertRGB32to32(width, height, inbuffer, outbuffer);
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma GCC diagnostic   ignored "-Wdiscarded-qualifiers"
                                write_PNG(outbuffer,
                                          filename,
                                          width,
                                          cropped_height,
                                          interlace,
                                          compression,
                                          x_offset,
                                          y_offset,
                                          PNG_COLOR_TYPE_RGB_ALPHA,
                                          srcBlue < srcRed);
#pragma GCC diagnostic pop
			}
			break;
		default:
			fprintf(stderr, "%u bits per pixel are not supported!\n", bits);
			exit(EXIT_FAILURE);
	}

	(void) free(outbuffer);
}

/********
 * MAIN *
 ********/

int
    main(int argc, char** argv)
{
	const char*  device  = NULL;
	char*        outfile = argv[argc - 1];
	int          optc;
	int          vt_num   = UNDEFINED;
	unsigned int bitdepth = 0U;
	unsigned int height   = 0U;
	unsigned int width    = 0U;
	int          old_vt   = UNDEFINED;
	const char*  infile   = NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	struct fb_fix_screeninfo fb_fixedinfo = { 0 };
#pragma GCC diagnostic pop
	struct fb_var_screeninfo fb_varinfo = { 0 };
	bool                     waitbfg    = false; /* wait before grabbing (for -C )... */
	int                      interlace  = PNG_INTERLACE_NONE;
	bool                     noautocrop = false;
	bool                     verbose    = false;
	int       png_compression           = 4;    // Was Z_DEFAULT_COMPRESSION (6), which is massively more expensive
	off_t     skip_bytes                = 0;
	bool      invert                    = false;
	short int x_offset                  = 0;
	short int y_offset                  = 0;

	for (;;) {
		optc = getopt(argc, argv, "f:z:w:b:h:iC:c:d:s:?vk");
		if (optc == -1) {
			break;
		}
		switch (optc) {
			/* please keep this list alphabetical */
			case 'b':
				bitdepth = (unsigned int) strtoul(optarg, NULL, 10);
				break;
			case 'C':
				waitbfg = true;
				/*@fallthrough@*/
			case 'c':
				vt_num = atoi(optarg);
				break;
			case 'd':
				device = optarg;
				break;
			case 'f':
				infile = optarg;
				break;
			case 'h':
				height = (unsigned int) strtoul(optarg, NULL, 10);
				break;
			case '?':
				help(argv[0]);
				return 1;
			case 'i':
				interlace = PNG_INTERLACE_ADAM7;
				break;
			case 'k':
				noautocrop = true;
				break;
			case 'v':
				verbose = true;
				break;
			case 's':
				(void) sleep((unsigned int) strtoul(optarg, NULL, 10));
				break;
			case 'w':
				width = (unsigned int) strtoul(optarg, NULL, 10);
				break;
			case 'z':
				png_compression = atoi(optarg);
				break;
			default:
				usage(argv[0]);
		}
	}

	if ((optind == argc) || ((argc - optind) != 1)) {
		usage(argv[0]);
		return 1;
	}

	if (vt_num != UNDEFINED) {
		old_vt = (int) change_to_vt((unsigned short int) vt_num);
		if (waitbfg) {
			(void) sleep(3);
		}
	}

	if (infile && strlen(infile) > 0) {
		if (!bitdepth || !width || !height) {
			fprintf(stderr, "Width, height and bitdepth are mandatory when reading from file\n");
			exit(EXIT_FAILURE);
		}
	} else {
		if (device == NULL) {
			device = getenv("FRAMEBUFFER");
			if (device == NULL) {
				device = DEFAULT_FB;
			}
		}

		get_framebufferdata(device, &fb_fixedinfo, &fb_varinfo, verbose);

		if (!bitdepth) {
			bitdepth = fb_varinfo.bits_per_pixel;
		}

		if (!width) {
			// NOTE: Kindle/Kobo hack (Freescale iMX5 & iMX6).
			// Use the virtual xres if driver is mxc_epdc_fb @ 8bpp (Kindle)
			// or @ 16bpp (Kobo) or @ 32bpp (Kobo on FW >= 4.x) to get a correct dump on those devices...
			// NOTE: Assume the new driver on the Oasis 2 (mxc_epdc_v2_fb on an iMX7) behaves the same way.
			if (strcmp(fb_fixedinfo.id, "mxc_epdc_fb") == 0 &&
			    (bitdepth == 8 || bitdepth == 16 || bitdepth == 32)) {
				width = fb_varinfo.xres_virtual;
				// ...and store the x offset in order to crop the 8~10px of garbage when writing the PNG
				if (fb_varinfo.xres_virtual != fb_varinfo.xres) {
					x_offset = (short int) -(fb_varinfo.xres_virtual - fb_varinfo.xres);
				}

				// NOTE: Kobo hack. Crop the pixels hidden behind the bezel on some models...
				// We detect the model by parsing the S/N ourselves,
				// to avoid a call to /bin/kobo_config.sh
				// Unfortunately, after that,
				// we pretty much have to maintain a list of the amount/location of the hidden pixels
				// ourselves, since it's of course model-specific...
				// FWIW, said pixels are fully transparent on FW 4.x, and all-black on earlier FWs...
				if (access("/proc/usid", F_OK) == -1) {
					// We're not on a Kindle, now check if we're on a Kobo...
					// Get the model from Nickel's version tag file...
					FILE* fp = fopen("/mnt/onboard/.kobo/version", "r" STDIO_CLOEXEC);
					if (!fp) {
						fprintf(
						    stderr,
						    "** Couldn't find a Kobo version tag (not running on a Kobo?)! **\n");
					} else {
						// NOTE: I'm not entirely sure this will always have a fixed length, so,
						// rely on getline()'s dynamic allocation to be safe...
						char*              line = NULL;
						size_t             len  = 0U;
						ssize_t            nread;
						unsigned short int kobo_id = 0U;
						while ((nread = getline(&line, &len, fp)) != -1) {
							// Thankfully, the device code is always located in the three
							// final characters, so that's easy enough to extract without
							// having to worry about the formatting...
							kobo_id =
							    (unsigned short int) strtoul(line + (nread - 3), NULL, 10);
							// NOTE: Device code list pilfered from
							// https://github.com/geek1011/KoboStuff/blob/gh-pages/kobofirmware.js#L11
							switch (kobo_id) {
								case 310U:
									// Touch A/B (trilogy)
									break;
								case 320U:
									// Touch C (trilogy)
									break;
								case 340U:
									// Mini (pixie)
									break;
								case 330U:
									// Glo (kraken)
									break;
								case 371U:
									// Glo HD (alyssum)
									break;
								case 372U:
									// Touch 2.0 (pika)
									break;
								case 360U:
									// Aura (phoenix)
									y_offset = -10;    // bottom 10 pixels
									break;
								case 350U:
									// Aura HD (dragon)
									break;
								case 370U:
									// Aura H2O (dahlia)
									y_offset = 11;    // top 11 pixels.
									break;
								case 374U:
									// Aura H2O² (snow)
									break;
								case 378U:
									// Aura H2O² r2 (snow)
									break;
								case 373U:
									// Aura ONE (daylight)
									break;
								case 381U:
									// Aura ONE LE (daylight)
									break;
								case 375U:
									// Aura SE (star)
									break;
								case 379U:
									// Aura SE r2 (star)
									break;
								case 376U:
									// Clara HD (nova)
									break;
								case 377U:
								case 380U:
									// Forma (frost)
									break;
								case 384U:
									// Libra (storm)
									break;
								case 0U:
								default:
									fprintf(
									    stderr,
									    "** Unidentified Kobo device code (%hu)! **\n",
									    kobo_id);
									break;
							}
						}
						free(line);
						fclose(fp);
					}
				}
			} else {
				// NOTE: Actually, on my Linux simplefb (EFI), the same kind of quirks applies,
				//       we have to trust line_length to get a sane dump...
				width = (unsigned int) (fb_fixedinfo.line_length / ((double) bitdepth / 8U));
				if (width != fb_varinfo.xres) {
					x_offset = (short int) -(width - fb_varinfo.xres);
				}
				// width = fb_varinfo.xres;
			}
		}

		if (!height) {
			height = fb_varinfo.yres;
		}

		skip_bytes = (off_t)((fb_varinfo.yoffset * fb_varinfo.xres) * (fb_varinfo.bits_per_pixel >> 3U));
		fprintf(stderr, "Skipping %jd bytes\n", (intmax_t) skip_bytes);

		// Disable automagic cropping on request
		if (noautocrop) {
			x_offset = 0;
			y_offset = 0;
		}

		// NOTE: Kindle hack.
		// On the K4, we need to invert the fb like on legacy devices.
		// Crappy detection based on bpp & xres/yres vs. virtual inconsistencies.
		// We might also be able to get away with only the id (eink_fb),
		// but I vaguely remember that being unreliable on some devices between portrait and landscape mode...
		// c.f., http://www.mobileread.com/forums/showpost.php?p=2045889&postcount=1 &
		// http://www.mobileread.com/forums/showpost.php?p=2832686&postcount=10 for more background...
		// The id inconsistencies appear to have been fixed on FW 4.1.1, so, let's handle the rest... :).
		if (strcmp(fb_fixedinfo.id, "eink_fb") == 0 && bitdepth == 8U &&
		    fb_varinfo.xres_virtual == fb_varinfo.xres && fb_varinfo.yres == fb_varinfo.yres_virtual) {
			invert = true;
		}

		fprintf(stderr, "Resolution: %ux%u", width, height);
		// Only print the cropping status when one is applied
		if (x_offset != 0 || y_offset != 0) {
			fprintf(stderr,
				" -> %ux%u (cropping:",
				width - (unsigned int) abs(x_offset),
				height - (unsigned int) abs(y_offset));
			if (x_offset != 0) {
				fprintf(stderr,
					" %hi %s pixels",
					(short int) abs(x_offset),
					(x_offset > 0) ? "leftmost" : "rightmost");
			}
			if (x_offset != 0 && y_offset != 0) {
				fprintf(stderr, " &");
			}
			if (y_offset != 0) {
				fprintf(stderr,
					" %s %hi pixels",
					(y_offset > 0) ? "top" : "bottom",
					(short int) abs(y_offset));
			}
			fprintf(stderr, ")");
		}
		fprintf(stderr, " @ depth %u", bitdepth);
		// Only print the invert status when it's actually needed
		// (8bpp, to differentiate between the K4 and K5 devices)
		if (bitdepth == 8U) {
			fprintf(stderr, " (invert: %s)\n", invert ? "yes" : "no");
		} else {
			fprintf(stderr, "\n");
		}

		infile = device;
	}

	size_t map_size = width * height * ((bitdepth + 7U) >> 3U);
	// NOTE: 4bpp hack!
	if (bitdepth == 4U) {
		map_size /= 2U;
	}
	// NOTE: Usually, when we're dealing with an actual fb device, we can simply trust smem_len or (line_length * yres_virtual)...
	// map_size = fb_fixedinfo.smem_len;
	// map_size = fb_fixedinfo.line_length * fb_varinfo.yres_virtual;

	// Open the fb (or file)
	int fbfd = -1;
	if ((fbfd = open(device, O_RDONLY | O_CLOEXEC)) == -1) {
		perror("open");
		fprintf(stderr, "Error: Couldn't open %s.\n", device);
		exit(EXIT_FAILURE);
	}

	// mmap the framebuffer (or file)
	unsigned char* fb_p = (unsigned char*) mmap(NULL, map_size, PROT_READ, MAP_SHARED, fbfd, 0);
	if (fb_p == MAP_FAILED) {
		perror("mmap");
		fprintf(stderr, "Error: Failed to mmap %s.\n", device);
		exit(EXIT_FAILURE);
	}
	// Honor skip_bytes
	fb_p += skip_bytes;

	if (old_vt != UNDEFINED) {
		(void) change_to_vt((unsigned short int) old_vt);
	}

	convert_and_write(fb_p, outfile, width, height, bitdepth, interlace, png_compression, invert, x_offset, y_offset);

	// Unmap & close the fb/file
	if (munmap(fb_p, map_size) < 0) {
		perror("munmap");
		fatal_error("Failed to unmap!");
	}

	if (close(fbfd) < 0) {
		perror("close");
		fatal_error("Failed to close fd!");
	}

	return 0;
}
