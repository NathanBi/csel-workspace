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
#include <stdbool.h>

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
    uint64_t res;
    struct epoll_event ev;
    struct itimerspec ts;
    int msec = 3000;

    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    printf("created timerfd %d\n", timerfd);
    printf("Starting at (%d)...\n", (int)time(NULL));

    // interval for periodic timer
    ts.it_interval.tv_sec = msec/1000;
    ts.it_interval.tv_nsec = 0;
    // expiration
    ts.it_value.tv_sec = msec/1000;
	ts.it_value.tv_nsec = 0;

    int led = open_led();
    bool toggle = true;
    pwrite(led, "1", sizeof("1"), 0);

    if (timerfd_settime(timerfd, 0, &ts, NULL) < 0) {
		printf("timerfd_settime() failed: errno=%d\n", errno);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("set timerfd time=%s\n", itimerspec_dump(&ts));

    // create the context
	epfd = epoll_create1(0);
	if (epfd == -1) {
		printf("epoll_create() failed: errno=%d\n", errno);
		close(timerfd);
		return EXIT_FAILURE;
	}

    //syscall for context control
    ev.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &ev) == -1) {
		printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
		close(epfd);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("added timerfd to epoll set\n");

    while(1)
    {
        memset(&ev, 0, sizeof(ev));
	    ret = epoll_wait(epfd, &ev, 1000, 10000); // wait up to 500ms for timer
	    if (ret < 0) {
            printf("epoll_wait() failed: errno=%d\n", errno);
            close(epfd);
            close(timerfd);
            return EXIT_FAILURE;
	    }
	    printf("waited on epoll, ret=%d\n", ret);
        printf("event=%ld on fd=%d\n", ev.events, ev.data.fd);

        ret = read(timerfd, &res, sizeof(res));
        printf("Toggle led \n");
        
        if(toggle == true)
        {
            pwrite(led, "1", sizeof("1"), 0);
            toggle = false;
        }
        else
        {
            pwrite(led, "0", sizeof("0"), 0);
            toggle = true;
        }
    }
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