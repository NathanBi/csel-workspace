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

int main(int argc, char* argv[])
{
    long duty   = 2;     // %
    long period = 1000;  // ms
    if (argc >= 2) period = atoi(argv[1]);
    period *= 1000000;  // in ns

    // compute duty period...
    long p1 = period / 100 * duty;
    long p2 = period - p1;

    int led = open_led();
    pwrite(led, "0", sizeof("0"), 0);

    char buffer_k1[2];
    char buffer_k2[2];
    char buffer_k3[2];

    int k1 = open_k1();
    int k2 = open_k2();
    int k3 = open_k3();

    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    int epfd = epoll_create1(0);

    struct epoll_event ev[3];
    struct epoll_event events[3];
    
    ev[0].events = EPOLLET;
    ev[0].data.fd = k1;

    ev[1].events = EPOLLET;
    ev[1].data.fd = k2;

    ev[2].events = EPOLLET;
    ev[2].data.fd = k3;

    epoll_ctl(epfd,EPOLL_CTL_ADD,k1,&ev[0]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k2,&ev[1]);
    epoll_ctl(epfd,EPOLL_CTL_ADD,k3,&ev[2]);

    int k = 0;
    int nr = 0;
    while (1) {
        /*struct timespec t2;
        clock_gettime(CLOCK_MONOTONIC, &t2);

        long delta =
            (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

        int toggle = ((k == 0) && (delta >= p1)) | ((k == 1) && (delta >= p2));
        if (toggle) {
            t1 = t2;
            k  = (k + 1) % 2;
            if (k == 0)
                pwrite(led, "1", sizeof("1"), 0);
            else
                pwrite(led, "0", sizeof("0"), 0);

            pread(k1,buffer_k1,2,0);

            pread(k2,buffer_k2,2,0);

            pread(k3,buffer_k3,2,0);

            printf("%c %c %c\n",buffer_k1[0],buffer_k2[0],buffer_k3[0]);
        }*/

        nr = epoll_wait(epfd, &events, 3, -1);

        if(nr > 0)
        {
            for (int i=0; i<nr; i++) {
            printf ("event=%ld on fd=%d\n", events[i].events, events[i].data.fd);
            }
        }

    }

    return 0;
}