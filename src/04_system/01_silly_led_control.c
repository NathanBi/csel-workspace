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
#include <time.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <stdio.h>

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

int main()
{
    // LED init
    int led = open_led();
    pwrite(led, "0", sizeof("0"), 0);

    // Buttons
    int k1 = open_k1();
    int k2 = open_k2();
    int k3 = open_k3();

    // Context creation
    int epfd = epoll_create1(0);

    // Events
    struct epoll_event ev[4];
    struct epoll_event events[4];

    // Edge triggered
    ev[0].events = EPOLLET;
    ev[0].data.fd = k1;

    ev[1].events = EPOLLET;
    ev[1].data.fd = k2;

    ev[2].events = EPOLLET;
    ev[2].data.fd = k3;

    // Context control
    epoll_ctl(epfd,EPOLL_CTL_ADD,k1,&ev[0]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k2,&ev[1]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k3,&ev[2]);

    // Timerfd settings
    struct itimerspec timer_freq;
    timer_freq.it_value.tv_sec = 1;
    timer_freq.it_value.tv_nsec = 0;

    // Timerfd creation
    int timer = timerfd_create(CLOCK_MONOTONIC,0);
    
    // Set time
    timerfd_settime(timer,0,&timer_freq,NULL);

    // Event triggered or read operation
    ev[3].events = EPOLLET | EPOLLIN;
    ev[3].data.fd = timer;
    
    // Control interface
    epoll_ctl(epfd,EPOLL_CTL_ADD,timer,&ev[3]);

    int toggle = 1;
    int First = 1;
    int nr = 0;

    while (1) {
        // Wait for event
        nr = epoll_wait(epfd, &events, 4, -1);

        // Determine which event
        if(nr > 0 && !First)
        {
            for (int i=0; i<nr; i++) {
                printf("event=%d on fd=%d\n", events[i].events, events[i].data.fd);
                // Button 1 pressed: increase blinking rate
                if(events[i].data.fd == k1)
                {
                    if(timer_freq.it_value.tv_sec >= 1)
                        timer_freq.it_value.tv_sec -= 1;
                    timerfd_settime(timer,0,&timer_freq,NULL);
                }
                // Button 2 pressed: default blinking rate
                else if (events[i].data.fd == k2)
                {
                    timer_freq.it_value.tv_sec = 1;
                    timerfd_settime(timer,0,&timer_freq,NULL);
                }
                // Button 3 pressed: decrease blinking rate
                else if(events[i].data.fd == k3)
                {
                    timer_freq.it_value.tv_sec += 1;
                    timerfd_settime(timer,0,&timer_freq,NULL);
                }
                // Timer event --> toggle LED
                else if(events[i].data.fd == timer)
                {
                    if (toggle)
                    {
                        pwrite(led, "1", sizeof("1"), 0);
                    }
                    else
                    {
                        pwrite(led, "0", sizeof("0"), 0);
                    }
                    // Re-arm the timer
                    timerfd_settime(timer,0,&timer_freq,NULL);

                    toggle = !toggle;
                    
                }
            }
        }

        First = 0;

    }

    return 0;
}