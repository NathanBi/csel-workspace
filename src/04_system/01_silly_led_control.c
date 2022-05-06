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
#define GPIO_K1       "/sys/class/gpio/gpio0"
#define K1            "0"
#define GPIO_K2       "/sys/class/gpio/gpio2"
#define K2            "2"
#define GPIO_K3       "/sys/class/gpio/gpio3"
#define K3            "3"

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

static int open_k1()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT,O_WRONLY);
    write(f,K1,strlen(K1));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, K1, strlen(K1));
    close(f);

    // config pin direction
    f = open(GPIO_K1 "/direction", O_WRONLY);
    write(f, "in", 3);
    close(f);

    // config pin edge
    f = open(GPIO_K1 "/edge", O_WRONLY);
    write(f, "falling", 8);
    close(f);

    // open gpio value attribute
    f = open(GPIO_K1 "/value", O_RDONLY);
    return f;

}

static int open_k2()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT,O_WRONLY);
    write(f,K2,strlen(K2));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, K2, strlen(K2));
    close(f);

    // config pin direction
    f = open(GPIO_K2 "/direction", O_WRONLY);
    write(f, "in", 3);
    close(f);

    // config pin edge
    f = open(GPIO_K2 "/edge", O_WRONLY);
    write(f, "falling", 8);
    close(f);

    // open gpio value attribute
    f = open(GPIO_K2 "/value", O_RDONLY);
    return f;

}

static int open_k3()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT,O_WRONLY);
    write(f,K3,strlen(K3));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, K3, strlen(K3));
    close(f);

    // config pin direction
    f = open(GPIO_K3 "/direction", O_WRONLY);
    write(f, "in", 3);
    close(f);

    // config pin edge
    f = open(GPIO_K3 "/edge", O_WRONLY);
    write(f, "falling", 8);
    close(f);

    // open gpio value attribute
    f = open(GPIO_K3 "/value", O_RDONLY);
    return f;

}

int main(int argc, char* argv[])
{
    // timer creation
    int timerfd, epfd, ret;
    uint64_t res;
    // struct epoll_event ev_tfd;
    struct itimerspec ts;
    int msec = 3000;

    // buttons
    char buffer_k1[2];
    char buffer_k2[2];
    char buffer_k3[2];

    int k1 = open_k1();
    int k2 = open_k2();
    int k3 = open_k3();

    int k = 0;
    int nr = 0;

    struct epoll_event ev[4];
    struct epoll_event events[4];

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

    ev[0].events = EPOLLET;
    ev[0].data.fd = k1;

    ev[1].events = EPOLLET;
    ev[1].data.fd = k2;

    ev[2].events = EPOLLET;
    ev[2].data.fd = k3;

    epoll_ctl(epfd,EPOLL_CTL_ADD,k1,&ev[0]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k2,&ev[1]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k3,&ev[2]);

    //syscall for context control
    ev[4].events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &ev[3]) == -1) {
		printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
		close(epfd);
		close(timerfd);
		return EXIT_FAILURE;
	}
	printf("added timerfd to epoll set\n");

    while(1)
    {
        // memset(&events[3], 0, sizeof(ev[3]));
	    ret = epoll_wait(epfd, &events, 4, 10000); // wait up to 10s for timer
	    if (ret < 0) {
            printf("epoll_wait() failed: errno=%d\n", errno);
            close(epfd);
            close(timerfd);
            return EXIT_FAILURE;
	    }
	    printf("waited on epoll, ret=%d\n", ret);
        printf("event=%ld on fd=%d\n", ev[3].events, ev[3].data.fd);

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