/* stfclient, communicate with the stfserv over
 * the serial port...
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

#include <poll.h>

#include "stf/stf.h"

static int fdSer = -1;
static int verbose = 0;

static void dummy(int i) {}

static int getLine(char *buf, int max) {
  int idx = 0;
  memset(buf, 0, max);
  
  while (idx<max) {
    char *t;
    const int nr = read(fdSer, buf+idx, 1);

    if (nr<0) {
      perror("can't read from serial port!\n");
      return 1;
    }
    else if (nr==0) {
      fprintf(stderr, "stftcp: eof on serial port!\n");
      return 1;
    }

    /* printf("nr: %d, %02x\n", nr, buf[idx]); */

    if ((t=strstr(buf, "\r\n"))!=NULL) {
      *t = 0;
      if (verbose) printf("getLine: '%s'\r\n", buf);
      return 0;
    }

    idx+=nr;
  }
  fprintf(stderr, "buffer overflow!\n");
  return 1;
}

static int waitAck(void) {
  char buf[2048];
  /* printf("waitAck!\n"); */
  while (1) {
    if (getLine(buf, sizeof(buf))) {
      fprintf(stderr, "can't get line!\n");
      return 1;
    }
    if (strcmp(buf, "OK")==0) return 0;
    else {
       if (verbose) printf("unexpected line: '%s'\n", buf);
    }
  }
  return 0;
}

static int readall(int fd, char *buf, int nb) {
  int idx = 0;
  while (idx<nb) {
    int nr = read(fd, buf+idx, nb-idx);

    if (nr<0) {
      perror("read");
      return -1;
    }
    else if (nr==0) {
      fprintf(stderr, "unexpected eof!\n");
      return -1;
    }
    idx+=nr;
  }
  return 0;
}

static int connectTCP(const char *hostname, short port) {
   struct sockaddr_in serv_addr;
   int sockfd;
   struct hostent *he;
   char **p;
   
   if ((he=gethostbyname(hostname))==NULL) {
      fprintf(stderr, "stftcp: can't get ip for host name '%s'!\n", hostname);
      return -1;
   }
   
   p = he->h_addr_list;
   
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family      = AF_INET;
   memcpy(&serv_addr.sin_addr.s_addr, *p, sizeof(serv_addr.sin_addr.s_addr));
   serv_addr.sin_port        = htons(port);
   
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      return -1;
   }
   
   if (connect(sockfd, 
	       (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      char str[100];
      sprintf(str, "connect %s %d", hostname, port);
      perror(str);
      close(sockfd);
      return -1;
   }
   
   return sockfd;
}

static void usage(void) {
   fprintf(stderr,
	   "usage: stftcp host port [options]\n"
	   "  options:\n"
	   "    -verbose\n"
	   "    -send file\n"
	   "    -go testname\n"
	   "    -rcv test file\n"
	   "    -ready\n");
}

/* serial input/output control...
 *
 * simple transfer protocol
 */
int main(int argc, char *argv[]) {
  char line[64];
  int ai;

  if (argc<3) {
     usage();
     return 1;
  }

  if ((fdSer = connectTCP(argv[1], atoi(argv[2]))) <0 ) {
     fprintf(stderr, "can't connect to '%s' port %d\n",
	     argv[1], atoi(argv[2]));
     return 1;
  }

  /* send OK, get OK back...
   */
  sprintf(line, "OK\r");
  if (write(fdSer, line, strlen(line))!=strlen(line)) {
    fprintf(stderr, "can't write initial OK\n");
    return 1;
  }

  if (waitAck()) {
    fprintf(stderr, "can't get wait ack!\n");
    return 1;
  }

  for (ai=3; ai<argc; ai++) {
     if (strcmp(argv[ai], "-ready")==0) {
	struct pollfd fds[1];
	fds[0].fd = fdSer;
	fds[0].events = POLLIN;
	
	/* make sure there is no extra crap after OK --
	 */
	return (poll(fds, 1, 500)==0) ? 0 : 1;
     }
     else if (strcmp(argv[ai], "-send")==0 && ai+1<argc) {
      struct stat st;
      char *buf;
      int rfd = open(argv[ai+1], O_RDONLY), ret;
      if (rfd<0) {
	perror("open send xml file!");
	return 1;
      }
      
      if (stat(argv[ai+1], &st)<0) {
	perror("stat send file");
	return 1;
      }

      sprintf(line, "SEND %lu\r", st.st_size);
      if (write(fdSer, line, strlen(line))!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }
      
      if ((buf = (char *) malloc(st.st_size))==0) {
	fprintf(stderr, "can't allocate %lu bytes\n", st.st_size);
	return 1;
      }

      if ((ret=read(rfd, buf, st.st_size))!=st.st_size) {
	fprintf(stderr, "can't read xml file: '%s' (%d -> %ld)\n", 
		argv[ai+1], ret, st.st_size);
	return 1;
      }
      close(rfd);

      waitAck();

      /* printf("msg: writing data...\n"); fflush(stdout);*/

      if (write(fdSer, buf, st.st_size)!=st.st_size) {
	fprintf(stderr, "can't write data to serial port!\n");
	return 1;
      }

      free(buf);

      /* wait for OK prompt again...
       */
      waitAck();

      ai++;
    }
    else if (strcmp(argv[ai], "-go")==0 && ai+1<argc) {
      sprintf(line, "GO %s\r", argv[ai+1]);
      if (write(fdSer, line, strlen(line))!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      waitAck();
      ai++;
    }
    else if (strcmp(argv[ai], "-rcv")==0 && ai+2<argc) {
      int nbytes, ret, wfd;
      char *bb;

      /* receive an xml file -- put it in filename...
       *
       * we send the rcv, we get back a SEND %d\r\n...
       * receive the data and write it to file...
       */
      sprintf(line, "RCV %s\r", argv[ai+1]);
      if (write(fdSer, line, strlen(line))!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      do {
	if (getLine(line, sizeof(line))) {
	  fprintf(stderr, "can't receive line!\n");
	  return 1;
	}
	
	ret = sscanf(line, "SEND %d", &nbytes);
      } while (ret!=1);

      if ((bb=(char *) malloc(nbytes))==NULL) {
	fprintf(stderr, "can't allocate %d bytes\n", nbytes);
	return 1;
      }

      sprintf(line, "OK\r");
      if (write(fdSer, line, strlen(line))!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      if (readall(fdSer, bb, nbytes)) {
	fprintf(stderr, "can't read from serial port!\n");
	return 1;
      }

      if ((wfd = open(argv[ai+2], O_CREAT|O_TRUNC|O_WRONLY, 0644))<0) {
	perror("open rcv xml file");
	return 1;
      }

      if (write(wfd, bb, nbytes)!=nbytes) {
	perror("write rcv xml file");
	return 1;
      }

      close(wfd);
      free(bb);

      sprintf(line, "OK\r");
      if (write(fdSer, line, strlen(line))!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      waitAck();

      ai+=2;
    }
    else if (strcmp(argv[ai], "-verbose")==0) {
       verbose = 1;
    }
    else {
       usage();
       return 1;
    }
  }

  signal(SIGALRM, dummy);
  alarm(10);
  shutdown(fdSer, 1);
  while (1) {
    char b[128];
    const int nr = read(fdSer, b, sizeof(b));
    if (nr<=0) break;
  }
  close(fdSer);
  return 0;
}
