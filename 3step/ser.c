/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

void str_ser4(int sockfd, struct sockaddr *addr, int addrlen);                                                           // transmitting and receiving function

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in my_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {			//create socket
		printf("error in socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {           //bind socket
		printf("error in binding");
		exit(1);
	}
	printf("waiting for data\n");
	
	str_ser4(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));                        // send and receive

	close(sockfd);
	exit(0);
}

void str_ser4(int sockfd, struct sockaddr *addr, int addrlen)
{
	FILE *fp;
	char buf[BUFSIZE];
	struct ack_so ack;
	struct pack_so pack;
	long lseek = 0;
	int n = 0, end = 0;

	ack.len = 0;
	ack.num = -1;
	pack.num = -1;

	while(!end) {

		if ((n = recvfrom(sockfd, &pack, sizeof(pack), 0, addr, (socklen_t *)&addrlen)) == -1) {      //receive the packet
			printf("error receiving\n");
			exit(1);
		}

		if (pack.data[pack.len - 1] == '\0') {			//check if it is the end of the file
			end = 1;
			pack.len--;
		}

		if (pack.num > ack.num) {			//copy packet data to buffer
			memcpy((buf + lseek), pack.data, pack.len);
			lseek += pack.len;
			ack.num = pack.num;

			if (ack.num % 6 == 0 | ack.num % 6 == 2 | ack.num % 6 == 5) {			//check if need to send ack

				if ((n = sendto(sockfd, &ack, sizeof(ack), 0, addr, addrlen)) == -1){
					printf("send error!\n");
					exit(1);
				}
			}
		}

	}

	if ((fp = fopen("my_receive.txt", "wt")) == NULL){
		printf("file does not exist\n");
		exit(0);
	}
	fwrite(buf, 1, lseek, fp);
	fclose(fp);
	
	printf("the file has been received successfully!\n the total data received is %d bytes\n", (int)lseek);
}
