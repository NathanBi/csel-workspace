/**
 * Copyright 2018 University of Applied Sciences Western Switzerland / Fribourg
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
 *
 * Project:	HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose:	NanoPi silly status led control system
 *
 * Autĥor:	Daniel Gachet
 * Date:	07.11.2018
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED      "/sys/class/gpio/gpio10"
#define LED           "10"

static char *itimerspec_dump(struct itimerspec *ts);

static int open_led()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // config pin
    f = open(GPIO_LED "/direction", O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    f = open(GPIO_LED "/value", O_RDWR);
    return f;
}

int main(int argc, char* argv[])
{
    // timer creation
    int timerfd, epfd, ret;
    struct epoll_event ev;
    struct itimerspec ts;
    int msec = 5000;
    uint64_t res;
    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    printf("created timerfd %d\n", timerfd);
    printf("Starting at (%d)...\n", (int)time(NULL));

    // interval for periodic timer
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    // expiration
    ts.it_value.tv_sec = msec / 1000;
	ts.it_value.tv_nsec = (msec % 1000) * 1000000;

    if (timerfd_settime(timerfd, 0, &ts, NULL) < 0) {
		printf("timerfd_settime() failed: errno=%d\n", errno);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("set timerfd time=%s\n", itimerspec_dump(&ts));

	epfd = epoll_create(1);
	if (epfd == -1) {
		printf("epoll_create() failed: errno=%d\n", errno);
		close(timerfd);
		return EXIT_FAILURE;
	}

    ev.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &ev) == -1) {
		printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
		close(epfd);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("added timerfd to epoll set\n");

    sleep(1);

    memset(&ev, 0, sizeof(ev));
	ret = epoll_wait(epfd, &ev, 1, 500); // wait up to 500ms for timer
	if (ret < 0) {
		printf("epoll_wait() failed: errno=%d\n", errno);
		close(epfd);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("waited on epoll, ret=%d\n", ret);

	ret = read(timerfd, &res, sizeof(res));
	printf("read() returned %d\n", ret);

	if (close(epfd) == -1) {
		printf("failed to close epollfd: errno=%d\n", errno);
		return EXIT_FAILURE;
	}

	if (close(timerfd) == -1) {
		printf("failed to close timerfd: errno=%d\n", errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

    // long duty   = 2;     // %
    // long period = 1000;  // ms
    // if (argc >= 2) period = atoi(argv[1]);
    // period *= 1000000;  // in ns

    // // compute duty period...
    // long p1 = period / 100 * duty;
    // long p2 = period - p1;

    // int led = open_led();
    // pwrite(led, "1", sizeof("1"), 0);

    // struct timespec t1;
    // clock_gettime(CLOCK_MONOTONIC, &t1);

    // int k = 0;
    // while (1) {
    //     struct timespec t2;
    //     clock_gettime(CLOCK_MONOTONIC, &t2);

    //     long delta =
    //         (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

    //     int toggle = ((k == 0) && (delta >= p1)) | ((k == 1) && (delta >= p2));
    //     if (toggle) {
    //         t1 = t2;
    //         k  = (k + 1) % 2;
    //         if (k == 0)
    //             pwrite(led, "1", sizeof("1"), 0);
    //         else
    //             pwrite(led, "0", sizeof("0"), 0);
    //     }
    // }

    // return 0;
}


static char *itimerspec_dump(struct itimerspec *ts)
{
    static char buf[1024];

    snprintf(buf, sizeof(buf),
            "itimer: [ interval=%lu s %lu ns, next expire=%lu s %lu ns ]",
            ts->it_interval.tv_sec,
            ts->it_interval.tv_nsec,
            ts->it_value.tv_sec,
            ts->it_value.tv_nsec
           );

    return (buf);
}