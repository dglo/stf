/* stf.c, stf client program.  runs on
 * the pc and loads/saves xml files to
 * and from the altera board...
 */
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string.h>

#include <windows.h>

static HANDLE fdSer;

static void serialInit(const char *dev, int rate) {
   DCB dcb;
   COMMTIMEOUTS tout;
   fdSer = CreateFile(dev, 
		      GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 
		      FILE_FLAG_WRITE_THROUGH, 0);
   if (fdSer == INVALID_HANDLE_VALUE) {
     fprintf(stderr, "can't open serial port!\n");
     exit(1);
   }
   
   //set data protocol format
   GetCommState(fdSer,&dcb);
   FillMemory(&dcb, sizeof(dcb), 0);
   
   dcb.DCBlength = sizeof(dcb);
   
   if (rate==9600)        dcb.BaudRate = CBR_9600;
   else if (rate==19200)  dcb.BaudRate = CBR_19200;
   else if (rate==38400)  dcb.BaudRate = CBR_38400;
   else if (rate==57600)  dcb.BaudRate = CBR_57600;
   else if (rate==115200) dcb.BaudRate = CBR_115200;
   else {
     fprintf(stderr, "unsupported baud rate\n");
     exit(1);
   }
   
   dcb.fBinary=1;
   dcb.fParity=0;
   dcb.fDtrControl=DTR_CONTROL_DISABLE;
   dcb.fRtsControl=RTS_CONTROL_DISABLE;
   dcb.ByteSize=8;
   dcb.Parity=NOPARITY;
   dcb.StopBits=ONESTOPBIT;
   if (!SetCommState(fdSer, &dcb)) {
     fprintf(stderr, "can't set comm state!\n");
     exit(1);
   }
   
   GetCommTimeouts(fdSer,&tout);
   tout.ReadIntervalTimeout=100;
   tout.ReadTotalTimeoutConstant=100;
   tout.ReadTotalTimeoutMultiplier=0;
   tout.WriteTotalTimeoutConstant=0;
   tout.WriteTotalTimeoutMultiplier=0;
   SetCommTimeouts(fdSer,&tout);
   
   SetupComm(fdSer, 65536, 65536);
}

static void serialStop(void) {}

static int readall(char *buf, int nb) {
  int idx = 0;
  while (idx<nb) {
    unsigned long nr;

    if (!ReadFile(fdSer, buf+idx, nb-idx, &nr, NULL)) {
      fprintf(stderr, "can't read from serial port!\n");
      return -1;
    }

    if (nr==0) {
      fprintf(stderr, "unexpected eof!\n");
      return -1;
    }
    idx+=nr;
  }
  return 0;
}

static int getLine(char *buf, int max) {
  static char b[512];
  static int bidx = 0;

  if (bidx==0) memset(b, 0, sizeof(b));

  while (1) {
    char *t;
    unsigned long nr=0;

    /* check for newline in current buffer...
     */
    if (bidx>0) {
      if ((t=strstr(b, "\r\n"))!=NULL) {
	const int nleft = bidx - ((t+2) - b);

	/* copy buffer to line, increment bidx...
	 *
	 * FIXME: check max!
	 */
	*t = 0;
	strcpy(buf, b);
	
	if (nleft) {
	  memmove(b, t+2, nleft);
	  b[nleft] = 0;
	}
	bidx = nleft;
	return 0;
      }
    }

    if (bidx>=sizeof(b)) break;

    if (!ReadFile(fdSer, b+bidx, sizeof(b)-bidx, &nr, NULL)) {
      fprintf(stderr, "can't read from serial port!\n");
      return 1;
    }

    if (nr==0) {
      fprintf(stderr, "stf: eof on serial port!\n");
      return 1;
    }

    bidx+=nr;
  }
  fprintf(stderr, "buffer overflow!\n");
  return 1;
}

static int waitAck(void) {
  char buf[512];
  /*printf("waitAck!\n");*/
  while (1) {
    if (getLine(buf, sizeof(buf))) {
      fprintf(stderr, "can't get line!\n");
      return 1;
    }

    /*printf("line: '%s'\n", buf);*/

    if (strcmp(buf, "OK")==0) return 0;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  char line[64];
  int baud = 57600;
  int ai;
  unsigned long nw;

  serialInit("COM1", baud);

  /* send OK, get OK back...
   */
  sprintf(line, "OK\r");
  if (!WriteFile(fdSer, line, strlen(line), &nw, NULL) || nw!=strlen(line)) {
    fprintf(stderr, "can't write initial OK\n");
    return 1;
  }

  if (waitAck()) {
    fprintf(stderr, "can't get wait ack!\n");
    return 1;
  }

  for (ai=1; ai<argc; ai++) {
    if (strcmp(argv[ai], "-send")==0 && ai+1<argc) {
      DWORD sz, nw, nr;
      char *buf;

      HANDLE rfd = CreateFile(argv[ai+1], 
			      FILE_READ_DATA,
			      FILE_SHARE_READ, 
			      NULL,
			      OPEN_EXISTING, 
			      FILE_ATTRIBUTE_NORMAL,
			      0);
      if (rfd==INVALID_HANDLE_VALUE) {
	fprintf(stderr, "can't open send xml file!\n");
	return 1;
      }
      
      sz = GetFileSize(rfd, NULL);
      
      sprintf(line, "SEND %lu\r", sz);
      if (!WriteFile(fdSer, line, strlen(line), &nw, 0) ||
	  nw!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }
      
      if ((buf = (char *) malloc(sz))==0) {
	fprintf(stderr, "can't allocate %lu bytes\n", sz);
	return 1;
      }
      
      /* printf("reading data from file...\n"); fflush(stdout);*/

      if (!ReadFile(rfd, buf, sz, &nr, NULL) || nr!=sz) {
	fprintf(stderr, "can't read xml file: '%s' (%lu -> %lu)\n", 
		argv[ai+1], nr, sz);
	return 1;
      }
      CloseHandle(rfd);

      if (waitAck()) {
	fprintf(stderr, "can't get ACK in send XML!\n");
	return 1;
      }

      /* printf("msg: writing data...\n"); fflush(stdout);*/

      if (!WriteFile(fdSer, buf, sz, &nw, NULL) || nw!=sz) {
	fprintf(stderr, "can't write data to serial port!\n");
	return 1;
      }
      free(buf);

      /* wait for OK prompt again...
       */
      if (waitAck()) {
	fprintf(stderr, "can't get ACK after send XML!\n");
	return 1;
      }

      ai++;
    }
    else if (strcmp(argv[ai], "-go")==0 && ai+1<argc) {
      DWORD nw;

      sprintf(line, "GO %s\r", argv[ai+1]);
      if (!WriteFile(fdSer, line, strlen(line), &nw, NULL) || 
	  nw!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      if (waitAck()) {
	fprintf(stderr, "can't get ACK in -go\n");
	return 1;
      }
      ai++;
    }
    else if (strcmp(argv[ai], "-rcv")==0 && ai+2<argc) {
      int nbytes, ret;
      char *bb;
      DWORD nw;
      HANDLE wfd;

      /* receive an xml file -- put it in filename...
       *
       * we send the rcv, we get back a SEND %d\r\n...
       * receive the data and write it to file...
       */
      sprintf(line, "RCV %s\r", argv[ai+1]);
      if (!WriteFile(fdSer, line, strlen(line), &nw, NULL) || 
	  nw!=strlen(line)) {
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
      if (!WriteFile(fdSer, line, strlen(line), &nw, NULL) ||
	  nw!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      if (readall(bb, nbytes)) {
	fprintf(stderr, "can't read from serial port!\n");
	return 1;
      }

      wfd = CreateFile(argv[ai+2], 
		       FILE_WRITE_DATA,
		       0,
		       NULL,
		       CREATE_ALWAYS,
		       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
		       0);

      if (wfd==INVALID_HANDLE_VALUE) {
	fprintf(stderr, "can't open xml file for writing!\n");
	return 1;
      }

      if (!WriteFile(wfd, bb, nbytes, &nw, NULL) ||
	  nw!=nbytes) {
	fprintf(stderr, "can't write rcv xml file\n");
	return 1;
      }

      CloseHandle(wfd);
      free(bb);

      sprintf(line, "OK\r");
      if (!WriteFile(fdSer, line, strlen(line), &nw, 0) ||
	  nw!=strlen(line)) {
	fprintf(stderr, "can't write send command to serial port\n");
	return 1;
      }

      if (waitAck()) {
	fprintf(stderr, "can't get ACK in receive file!\n");
	return 1;
      }

      ai+=2;
    }
    else {
      fprintf(stderr,
	      "usage: stf \n"
	      "  [-send file | -go testname | -rcv test file]\n");
      return 1;
    }
  }

  serialStop();
  return 0;
}
