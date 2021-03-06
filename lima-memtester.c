/*
 * Copyright (c) 2014 Siarhei Siamashka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "load_mali_kernel_module.h"

int textured_cube_main(void);
int memtester_main(int argc, char *argv[]);

void *fb_unblank_thread(void *data)
{
	int fd, ret;

	fd = open("/dev/fb0", O_RDWR);
	assert(fd != -1);

	while (1) {
		ret = ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
		assert(!ret);
		sleep(1);
	}

	close(fd);
	return 0;
}

static void *lima_thread(void *threadid)
{
	textured_cube_main();
	/* If we reach here, something bad has happened */
	abort();
	return NULL;
}

static void start_lima_thread(void)
{
	pthread_t th1, th2;

	load_mali_kernel_module();
	pthread_create(&th1, NULL, lima_thread, NULL);
	pthread_create(&th2, NULL, fb_unblank_thread, NULL);

	/* Wait a bit and let lima stop spamming to the console */
	usleep(300000);

	printf("\n");
}

int main (int argc, char *argv[])
{
	printf("This is a simple textured cube demo from the lima driver and\n");
	printf("a memtester. Both combined in a single program. The mali400\n");
	printf("hardware is only used to stress RAM in the background. But\n");
	printf("this happens to significantly increase chances of exposing\n");
	printf("memory stability related problems.\n\n");

	if (argc > 1)
		start_lima_thread();

	memtester_main(argc, argv);

	return 0;
}
