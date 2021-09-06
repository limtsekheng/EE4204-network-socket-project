#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define DATALEN 8000
#define BUFSIZE 80000
#define NEWFILE (O_WRONGLY|O_CREAT|O_TRINC)


struct pack_so
{
int32_t num;
uint32_t len;
char data[DATALEN];
};

struct ack_so
{
int32_t num;
uint32_t len;
};