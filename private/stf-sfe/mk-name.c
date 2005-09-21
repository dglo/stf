/* mk-name.c, make a new file name (just the base)
 *
 * 1) grab a lock file
 * 2) get value from number file, increment and put it back in
 * 3) delete lock file
 */
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/file.h>

#include <unistd.h>
#include <fcntl.h>

int main() {
  const char *numFile = "/var/www/stf/xml/uniq.txt";
  unsigned v;
  char line[80];
  int nw;
  int fd = open(numFile, O_RDWR);
  
  if (fd<0) {
     perror("open");
     return 1;
  }

  /* lock the file...
   */
  if (flock(fd, LOCK_EX)<0) {
     perror("flock");
     return 1;
  }

  memset(line, 0, sizeof(line));
  if (read(fd, line, sizeof(line)-1)<0) {
     perror("read");
     return 1;
  }

  if (sscanf(line, "%u", &v)!=1) {
    fprintf(stderr, "can't parse num file!\n");
    return 1;
  }
  
  printf("%08u\n", v);

  lseek(fd, 0, SEEK_SET);

  sprintf(line, "%u", v+1);
  if ((nw=write(fd, line, strlen(line)))!=strlen(line)) {
    fprintf(stderr, "can't write new value to num file...\n");
    return 1;
  }

  /* lock the file...
   */
  if (flock(fd, LOCK_UN)<0) {
     perror("flock");
     return 1;
  }

  close(fd);
  
  return 0;
}


