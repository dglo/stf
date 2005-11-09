/* stfclient, communicate with the stfserv over
 * the serial port...
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <termios.h>
#include <poll.h>

#include "stf/stf.h"

static int fdSer = -1;

static struct termios saveTermios;

static void serialInit(const char *dev, int rate) 
{
   struct termios buf;
   speed_t speed;

   if ((fdSer=open(dev, O_RDWR|O_NONBLOCK))<0) {
      perror("can't open serial device");
      exit(1);
   }

   /* setup raw serial input
    */
   if (tcgetattr(fdSer, &saveTermios)<0) {
      fprintf(stderr, "can't get termios: %s\n", dev);
      exit(1);
   }

   /* setup termio parameters
    */
   buf = saveTermios;
   buf.c_lflag = 0;
   buf.c_iflag = IGNBRK | IGNPAR;
   buf.c_cflag = B38400 | CS8 | CREAD | CLOCAL /* | CRTSXOFF | CRTSCTS */;
   buf.c_oflag = 0;
   buf.c_cc[VMIN] = 1;
   buf.c_cc[VTIME] = 0;

   if (rate==9600)        speed = B9600;
   else if (rate==19200)  speed = B19200;
   else if (rate==38400)  speed = B38400;
   else if (rate==57600)  speed = B57600;
   else if (rate==115200) speed = B115200;
   else {
      fprintf(stderr, "unsupported baud rate\n");
      exit(1);
   }
   
   cfsetispeed(&buf, speed);
   cfsetospeed(&buf, speed);
   
   if (tcsetattr(fdSer, TCSAFLUSH, &buf)<0) {
      fprintf(stderr, "can't set termios: %s\n", dev);
      exit(1);
   }

   /* blocking mode...
    */
   if (fcntl(fdSer, F_SETFL, O_RDWR)<0) {
      perror("fcntl");
      exit(1);
   }
}

/* stop serial input
 */
static void serialStop(void) {
   tcsetattr(fdSer, TCSAFLUSH, &saveTermios);
   close(fdSer);
}

/* signal handler
static void serialSig(int sig) {}
 */

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
      fprintf(stderr, "stfclient: eof on serial port!\n");
      return 1;
    }

    if ((t=strstr(buf, "\r\n"))!=NULL) {
      *t = 0;
      /* printf("getLine: '%s'\r\n", buf); */
      return 0;
    }

    idx+=nr;
  }
  fprintf(stderr, "buffer overflow!\n");
  return 1;
}

static int waitAck(void) {
  char buf[512];
  /* printf("waitAck!\n");*/
  while (1) {
    if (getLine(buf, sizeof(buf))) {
      fprintf(stderr, "can't get line!\n");
      return 1;
    }
    if (strcmp(buf, "OK")==0) return 0;
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

/* serial input/output control...
 *
 * simple transfer protocol
 */
int main(int argc, char *argv[]) {
  char line[64];
  int baud = 57600;
  int ai;

  serialInit("COM1", baud);

  /* send OK, get OK back...
   */
  sprintf(line, "OK\r");
  if (write(fdSer, line, strlen(line))!=strlen(line)) {
    fprintf(stderr, "can't write initial OK\n");
    return 1;
  }
  tcdrain(fdSer);

  if (waitAck()) {
    fprintf(stderr, "can't get wait ack!\n");
    return 1;
  }

  for (ai=1; ai<argc; ai++) {
    if (strcmp(argv[ai], "-send")==0 && ai+1<argc) {
      struct stat st;
      char *buf;
      int rfd = open(argv[ai+1], O_RDONLY|O_BINARY), ret;
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
      tcdrain(fdSer);
      
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
      tcdrain(fdSer);

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
      tcdrain(fdSer);

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
      tcdrain(fdSer);

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
      tcdrain(fdSer);

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
      tcdrain(fdSer);

      waitAck();

      ai+=2;
    }
    else {
      fprintf(stderr,
	      "usage: stfclient \n"
	      "  [-send file | -go testname | -rcv test file]\n");
      return 1;
    }
  }

  serialStop();
  return 0;
}
