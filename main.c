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
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


#define BROKER_LISTEN_PORT 3001
#define TARGET_SERVER "192.168.1.106"
#define TARGET_PORT 3002

/* This is the same struct as used in the cube. Fully compatible, same processor core and standards */

struct payloadData {
  int8_t type;
  int8_t version;
  char IMEI[16];
  float temperature;
  float humidity;
  float luminance;
  float sound;
  float x_acc;
  float y_acc;
  float z_acc;
  float battery;
  int8_t button;
} __attribute__((packed));

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

/* Listen for UDP traffic and start a thread for each incoming packet */
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
	struct payloadData *pd;
        char *buf = malloc(65536);
        if ((recv_len = recvfrom(s, buf, 65535, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
	    printf("Error receiving! err = %s\n", strerror(errno));
            goto done;		/* Yes goto, to make clear this is an exception 
				   path with a predefined exitpoint instead of 
				   a break */
        }
        buf = realloc(buf, recv_len+1);
	buf[recv_len] = 0;
	pd = (struct payloadData *)buf;
	printf("received type %x;%x, len = %i == %i>n", pd->type, pd->version, recv_len, sizeof(struct payloadData));

	struct thread_params *tp = malloc(sizeof(*tp));
     	tp->len = recv_len;
	tp->buf = buf;	
   	my_pthread_create_small(udp_convert_and_transmit, tp);	
    }
done:
    close(s);
    pthread_exit(0);
}


/* Connect to a remote server
 * Returns socket or -1 in case of an error */
int connect_to(char *hostname, int port)
{
	int sockfd;
        struct hostent *he;
        struct sockaddr_in their_addr; /* connector's address information */

        if ((he=gethostbyname(hostname)) == NULL) {  /* get the host info */
            printf("gethostbyname error %s\n", strerror(errno));
	    return -1;
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("socket error: %s\n", strerror(errno));
	    return -1;
        }

        memset(&their_addr, 0, sizeof(their_addr));     /* zero the rest of the struct */
        their_addr.sin_family = AF_INET;      /* host byte order */
        their_addr.sin_port = htons(port);    /* short, network byte order */
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);

        if (connect(sockfd, (struct sockaddr *)&their_addr, \
                                              sizeof(struct sockaddr)) == -1) {
            printf("connect error: %s", strerror(errno));
	    return -1;
        }
	
	return sockfd;
}

/* High level socket send function (printf format) */

int va_my_write(int sock, const char *command_fmt, ...)
{
    va_list ap;
    char buffer[512];  /* Some hard limit */
    int len;

    va_start(ap, command_fmt);
    len = vsnprintf(buffer, sizeof(buffer), command_fmt, ap);
    va_end(ap);
    return write(sock, buffer, len);
}

/* Connect to the target server and send json data */
void *udp_convert_and_transmit(void *thread_params)
{
	struct thread_params *tp = thread_params;
	int fd = connect_to(TARGET_SERVER, TARGET_PORT);
	if (fd != -1)
	{
		struct payloadData *p = tp->buf;

		va_my_write(fd, "{\"type\":%i,\"version\":%i,\"Device_ID\":\"%s\",\"temp\":%.2f,\"lum\":%.2f,\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"bat\":%.2f,\"hum\":%.2f,\"db\":%.2f, \"button\":%d, \"timestamp\":%ld}\n",
        			p->type, p->version, p->IMEI, p->temperature, p->luminance, p->x_acc, p->y_acc, p->z_acc, p->battery, p->humidity, p->sound, p->button), time(NULL);
	close(fd);
	}
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
