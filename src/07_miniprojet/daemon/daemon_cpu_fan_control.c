/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project:	HEIA-FR / Embedded Systems Laboratory
 *
 * Abstract: Process and daemon samples
 *
 * Purpose: This module implements a simple daemon to be launched
 *          from a /etc/init.d/S??_* script
 *          -> this application requires /opt/daemon as root directory
 *
 * Autĥor:  Daniel Gachet
 * Date:    17.11.2015
 */
#define _XOPEN_SOURCE 600
#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <string.h>
#include "ssd1306.h"


#define UNUSED(x) (void)(x)


/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_PWR_LED  "/sys/class/gpio/gpio362"
#define PWR_LED       "362"
#define GPIO_K1       "/sys/class/gpio/gpio0"
#define K1            "0"
#define GPIO_K2       "/sys/class/gpio/gpio2"
#define K2            "2"
#define GPIO_K3       "/sys/class/gpio/gpio3"
#define K3            "3"

static int update_oled(int mode, int temp, int freq)
{
    char line_mode[15];
    char line_temp[15];
    char line_freq[15];

    // Generic info
    ssd1306_set_position (0,0);
    ssd1306_puts("CSEL-Miniprojet");
    ssd1306_set_position (0,1);
    ssd1306_puts("Bischof Blin");
    ssd1306_set_position (0,2);
    ssd1306_puts("--------------");

    // Reinit strings
    memset(line_mode,0,strlen(line_mode));
    memset(line_temp,0,strlen(line_temp));
    memset(line_freq,0,strlen(line_freq));

    // Mode
    if(mode == 1)
    {
        memset(line_mode,0,strlen(line_mode));
        sprintf(line_mode, "Automatic mode");
    }
    else
    {
        memset(line_mode,0,strlen(line_mode));
        sprintf(line_mode, "Manual mode   ");
    }
    ssd1306_set_position (0,3);
    ssd1306_puts(line_mode);

    // Temp
    sprintf(line_temp, "Temp: %d °C   ", temp);
    ssd1306_set_position (0,4);
    ssd1306_puts(line_temp);

    // Freq
    sprintf(line_freq, "Freq: %d Hz   ", freq);
    ssd1306_set_position (0,5);
    ssd1306_puts(line_freq);

    return 0;
}

static void write_sysfs(int mode, int freq, int temp)
{
    char buf[20];
    int fd = open("/sys/class/my_sysfs_class/my_sysfs_device/Data", O_RDWR);

    snprintf(buf,20,"%d %d %d", mode, freq,temp);

    write(fd,buf,20);

    close(fd);
}

static void read_sysfs(int * mode, int * freq, int * temp)
{
    char buf[20];
    char tmp[10];
    int fd = open("/sys/class/my_sysfs_class/my_sysfs_device/Data", O_RDWR);

    read(fd,buf,20);

    *mode = atoi(buf); // "0 100 40" -> 0

    memmove(&buf[0], &buf[0 + 1], strlen(buf) - 0);
    memmove(&buf[0], &buf[0 + 1], strlen(buf) - 0);

    *freq = atoi(buf);

    snprintf(tmp,10,"%d",*freq);

    for(int i = 0;i < strlen(tmp); i++)
        memmove(&buf[0], &buf[0 + 1], strlen(buf) - 0);

    *temp = atoi(buf);

    close(fd);
}

static int open_pwr_led()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, PWR_LED, strlen(PWR_LED));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, PWR_LED, strlen(PWR_LED));
    close(f);

    // config pin
    f = open(GPIO_PWR_LED "/direction", O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    f = open(GPIO_PWR_LED "/value", O_RDWR);
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

static int signal_catched = 0;

static void catch_signal(int signal)
{
    syslog(LOG_INFO, "signal=%d catched\n", signal);
    signal_catched++;
}

static void fork_process()
{
    pid_t pid = fork();
    switch (pid) {
        case 0:
            break;  // child process has been created
        case -1:
            syslog(LOG_ERR, "ERROR while forking");
            exit(1);
            break;
        default:
            exit(0);  // exit parent process with success
    }
}

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    // 1. fork off the parent process
    fork_process();

    // 2. create new session
    if (setsid() == -1) {
        syslog(LOG_ERR, "ERROR while creating new session");
        exit(1);
    }

    // 3. fork again to get rid of session leading process
    fork_process();

    // 4. capture all required signals
    struct sigaction act = {
        .sa_handler = catch_signal,
    };
    sigaction(SIGHUP, &act, NULL);   //  1 - hangup
    sigaction(SIGINT, &act, NULL);   //  2 - terminal interrupt
    sigaction(SIGQUIT, &act, NULL);  //  3 - terminal quit
    sigaction(SIGABRT, &act, NULL);  //  6 - abort
    sigaction(SIGTERM, &act, NULL);  // 15 - termination
    sigaction(SIGTSTP, &act, NULL);  // 19 - terminal stop signal

    // 5. update file mode creation mask
    umask(0027);

    // 6. change working directory to appropriate place
    if (chdir("/opt") == -1) {
        syslog(LOG_ERR, "ERROR while changing to working directory");
        exit(1);
    }

    // 7. close all open file descriptors
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }

    // 8. redirect stdin, stdout and stderr to /dev/null
    if (open("/dev/null", O_RDWR) != STDIN_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stdin");
        exit(1);
    }
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stdout");
        exit(1);
    }
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stderr");
        exit(1);
    }

    // 9. option: open syslog for message logging
    openlog(NULL, LOG_NDELAY | LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon has started...");

     // LED init
    int led = open_pwr_led();
    pwrite(led, "0", sizeof("0"), 0);

    // OLED init
    ssd1306_init();
    syslog(LOG_INFO, "OLED init done");

        // Buttons
    int k1 = open_k1();
    int k2 = open_k2();
    int k3 = open_k3();

    // Context creation
    int epfd = epoll_create1(0);

    // Events
    struct epoll_event ev[5];
    struct epoll_event events[5];

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

    mkfifo("/tmp/myfifo",0666);
    int fifo = open("/tmp/myfifo",O_RDWR);

    ev[3].events = EPOLLIN;
    ev[3].data.fd = fifo;

    epoll_ctl(epfd,EPOLL_CTL_ADD,fifo,&ev[3]);

    // Timerfd settings
    struct itimerspec timer_freq;
    timer_freq.it_value.tv_sec = 1;
    timer_freq.it_value.tv_nsec = 0;

    // Timerfd creation
    int timer = timerfd_create(CLOCK_MONOTONIC,0);

    // Set time
    timerfd_settime(timer,0,&timer_freq,NULL);

    // Event triggered / read operation
    ev[4].events = EPOLLET | EPOLLIN;
    ev[4].data.fd = timer;

    // Control interface
    epoll_ctl(epfd,EPOLL_CTL_ADD,timer,&ev[4]);

    int nr = 0;
    char buf[20];

    int bFirstBtn = 1;

    int mode = 0, freq = 0, temp = 0;

    read_sysfs(&mode,&freq,&temp);

    while (1)
    {
        // Wait for event
        nr = epoll_wait(epfd, &events, 4, -1);

        update_oled(mode, temp, freq);

        syslog(LOG_INFO, "event");

        // Determine which event
        if(nr > 0 && !bFirstBtn)
        {
            for (int i=0; i<nr; i++) {
                // Button 1 pressed: increase frequency
                if(events[i].data.fd == k1)
                {
                    if(mode == 0)
                    {
                        // led button pushed signaling
                        pwrite(led, "1", sizeof("1"), 0);
                        syslog(LOG_INFO, "increase frequency button pushed");
                        // increment if in range
                        if(freq < 20 && freq >= 2)
                        {
                            freq += 1;
                            syslog(LOG_INFO, "frequency increased to %d Hz", freq);
                        }
                        else
                            syslog(LOG_INFO, "Maximum frequency reached %d Hz", freq);

                        update_oled(mode, temp, freq);
                        // write values in sysfs
                        write_sysfs(mode,freq,temp);
                        sleep(.5);
                        pwrite(led, "0", sizeof("0"), 0);
                    }
                }
                // Button 2 pressed: decrease frequency
                else if (events[i].data.fd == k2)
                {
                    if(mode == 0)
                    {
                        // led button pushed signaling
                        pwrite(led, "1", sizeof("1"), 0);
                        syslog(LOG_INFO, "decrease frequency button pushed");
                        // decrement if in range
                        if(freq <= 20 && freq > 2)
                        {
                            freq -= 1;
                            syslog(LOG_INFO, "frequency decreased to %d Hz", freq);
                        }
                        else
                            syslog(LOG_INFO, "Minimum frequency reached %d Hz", freq);
                        update_oled(mode, temp, freq);
                        // write values in sysfs
                        write_sysfs(mode,freq,temp);
                        // turn off signaling led
                        sleep(.5);
                        pwrite(led, "0", sizeof("0"), 0);
                    }
                }
                // Button 3 pressed: Switch modes
                else if(events[i].data.fd == k3)
                {
                    // led button pushed signaling
                    pwrite(led, "1", sizeof("1"), 0);
                    if(mode == 1)
                        syslog(LOG_INFO, "switches to manual mode");
                    else
                        syslog(LOG_INFO, "switches to automatic mode");
                    // toggle mode
                    mode = !mode;
                    update_oled(mode, temp, freq);
                    // write values in sysfs
                    write_sysfs(mode,freq,temp);
                    // turn off signaling led
                    sleep(.5);
                    pwrite(led, "0", sizeof("0"), 0);
                }
                // timer expired
                else if(events[i].data.fd == timer)
                {
                    // read values in sysfs
                    read_sysfs(&mode,&freq,&temp);
                    update_oled(mode, temp, freq);

                    // reset timer
                    timerfd_settime(timer,0,&timer_freq,NULL);
                }
                // event in fifo
                else if(events[i].data.fd == fifo)
                {
                    read(fifo,buf,20);
                    mode = atoi(buf);
                    memmove(&buf[0], &buf[0 + 1], strlen(buf) - 0);
                    memmove(&buf[0], &buf[0 + 1], strlen(buf) - 0);
                    freq = atoi(buf);

                    syslog(LOG_INFO,"%s",buf);

                    write_sysfs(mode,freq,temp);
                }
            }

        }
        bFirstBtn = 0; // first button reinit
    }

    close(fifo);

    syslog(LOG_INFO,
           "daemon stopped. Number of signals catched=%d\n",
           signal_catched);
    closelog();

    return 0;
}