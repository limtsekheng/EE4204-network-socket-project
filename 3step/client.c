/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

float str_cli4(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);

void tv_sub(struct timeval *out, struct timeval *in);

int main(int argc, char *argv[])
{
	int sockfd;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char **pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             //get host's information
		printf("error when gethostbyname");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd == -1)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);			//printf socket information
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

	if ((fp = fopen("myfile.txt", "r+t")) == NULL){
		printf("file does not exist\n");
		exit(0);
	}

	ti = str_cli4(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);   // receive and send

	rt = (len / (float)ti);
	printf("\ndata sent: %d bytes\ntime: %.3f ms\ntransmission rate: %f (Kbytes/s)\n", (int)len, ti, rt);

	close(sockfd);
	fclose(fp);
	exit(0);
}

float str_cli4(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len)
{
	char *buf;
	long lsize, ci = 0;
	struct ack_so ack;
	struct pack_so pack;
	int n, slen, flag = 1;
	float time_inv = 0.0;
	struct timeval sendt, recvt, timeout;

	//set time out-------
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	n = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	if (n == -1) {
		printf("Timeout value not set!\n");
	}

	fseek(fp, 0, SEEK_END);
	lsize = ftell(fp);
	rewind(fp);

	printf("the file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n", DATALEN);

	buf = (char *)malloc(lsize);
	if (buf == NULL){
		exit(2);
	}

	fread(buf, 1, lsize, fp);

	buf[lsize] = '\0';

	gettimeofday(&sendt, NULL);			//get the current time

	pack.num = 0;
	ack.num = -1;


	while (ci <= lsize) {

		if ((lsize + 1 - ci) <= DATALEN) {
			slen = lsize + 1 -ci;
		}
		else{
			slen = DATALEN;
		}

		pack.len = slen;

		if ((pack.num % 6 == 0 | pack.num % 6 == 1 | pack.num % 6 == 3) & (pack.num != 0)) {
			
			flag = 0;
			if ((n = recvfrom(sockfd, &ack, sizeof(ack), 0, addr, (socklen_t *)&addrlen)) == -1) {
			printf("error when receiving ack\n");
			exit(1);
			}
			if (ack.num == pack.num - 1) {
				flag = 1;
			}	
		}
		
		if (flag == 1) {

			memcpy(pack.data, (buf+ci), pack.len);
			if ((n = sendto(sockfd, &pack, sizeof(pack), 0, addr, addrlen)) == -1) {
				printf("send error!\n");
				printf("%s\n",strerror(errno));     //print error
				exit(1);
			}
			pack.num++;
			ci += slen;
		}

	}

	gettimeofday(&recvt, NULL);
	*len = ci;
	tv_sub(&recvt, &sendt);
	time_inv += (recvt.tv_sec) * 1000.0 + (recvt.tv_usec) / 1000.0;
	free(buf);
	printf("total number of packets transmitted is %d\n", pack.num);
	return (time_inv);
}


void tv_sub(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}