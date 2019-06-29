/*
 * (C) Erik Oomen
 * 29-6-2019
 * This function received UDP binary messages at port 3001 and retransmits them 
 * over TCP in json format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define BROKER_LISTEN_PORT 3001
#define TARGET_SERVER 192.168.1.106
#define TARGET_PORT 3002

struct thread_params
{
	int len;
	void *buf;
};

void *udp_convert_and_transmit(void *thread_params);

/* This function tries to start a lightweight thread, that means the HEAP 
 * space is limited to 64/128KByte instead of the standarde 8MByte. */

int my_pthread_create_small(void *(*start_routine) (void *), void *arg)
{
    pthread_attr_t attr;
    pthread_t s_thread;

    pthread_attr_init(&attr);
#ifdef ENV64BIT
    pthread_attr_setstacksize(&attr, 128 * 1024);
#else
    pthread_attr_setstacksize(&attr, 64 * 1024);
#endif
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&s_thread, &attr, start_routine, arg);
    return s_thread;
}

void start_listener(void)
{
    int recv_len;
    int s;
    struct sockaddr_in si_me,  si_other;
    socklen_t slen = sizeof(si_other);
    
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(BROKER_LISTEN_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(s , (struct sockaddr*)&si_me, sizeof(si_me));
    while(1)
    {
        char *buf = malloc(65536);
        if ((recv_len = recvfrom(s, buf, 65535, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
	    printf("Error receiving! err = %s\n", strerror(errno));
            goto done;		/* Yes goto, to make clear this is an exception 
				   path with a predefined exitpoint instead of 
				   a break */
        }
        buf = realloc(buf, recv_len);
	struct thread_params *tp = malloc(sizeof(*tp));
     	tp->len = recv_len;
	tp->buf = buf;	
   	my_pthread_create_small(udp_convert_and_transmit, tp);	
    }
done:
    close(s);
    pthread_exit(0);
}

void *udp_convert_and_transmit(void *thread_params)
{
	struct thread_params *tp = thread_params;
	free(tp->buf);
	free(tp);
	pthread_exit(0);
	return NULL;
}

int main(int argc, char *argv[])
{
	/* Supress warnings */
	(void)argc;
	(void)argv;

	/* Start receiver loop */
	start_listener();
	exit(0);
	return 0; 
}
