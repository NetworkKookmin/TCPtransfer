
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


void printPercent(double percent, int sumfileSize, int fileSize, int *count);
int main(int argc, char *argv[])
{
	struct hostent *hostp;
	struct servent *servp;
	struct sockaddr_in server_addr;
	int sock;
	static struct timeval timeout = { 5, 0 }; /* five seconds */
	fd_set rmask, xmask, mask;
	int nfound, bytesread;
	int sockid, retcode, nread, retval, addrlen;
	size_t numread;
	int sumfileSize = 0;
	int fileSize;
	int count = 0;
	double percent = 0;
	char msg[255];
	char input[255];
	char buf[255];
	char strComplete[10] = "complete";
	char portNumber[20];
	char command[20];
	char file_name[50];
	int randomPortNum = rand() % 1000;
	int BUFSIZE = 64;
	char strConnect[10] = "connect";
	char strPut[10] = "put";
	char strGet[10] = "get";
	char strClose[10] = "close";
	char ipAddress[20];
	char strfileSize[20];
	char strACK[10] = "ACK";
	char strEmpty[10] = "Empty";
	char nameAndSize[50];
	char strFinish[50] = "finish";


	char file_size[50];
	int start = 0;
	int thiscount = 0;
	int sizeCount = 0;


	// argv[1] = portNumber   argv[2] = localhost

	while (1){
		printf("Enter connect [portNumber] [hostAddress] \n"); // ó�� ���ӽ��� ��¹�  connect localhost 10000 ������ �Է� 
		scanf("%s %s %s", input, portNumber, ipAddress); // enter connect 

		if (strcmp(strConnect, input)) {
			(void)fprintf(stderr, "please ");

		}
		else break;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}

	if (isdigit(portNumber[0])) { //,,if first is number
		static struct servent s;
		servp = &s;
		s.s_port = htons((u_short)atoi(portNumber));

	}
	else if ((servp = getservbyname(portNumber, "tcp")) == 0) {

		printf("********* isdigit pass \n");
		fprintf(stderr, "%s: unknown service\n", portNumber);
		exit(1);
	}
	if ((hostp = gethostbyname(ipAddress)) == 0) {
		fprintf(stderr, "%s: unknown host\n", ipAddress);
		exit(1);
	}
	memset((void *)&server_addr, 0, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	memcpy((void *)&server_addr.sin_addr, hostp->h_addr, hostp->h_length);
	server_addr.sin_port = servp->s_port;
	if (connect(sock, (struct sockaddr *)&server_addr, sizeof server_addr) < 0) {
		(void)close(sock);
		perror("connect");
		exit(1);
	}
	printf("********* connect pass \n");
	FD_ZERO(&mask);
	FD_SET(sock, &mask);
	FD_SET(fileno(stdin), &mask);


	while (1){

		printf("----Enter your command.\n put [file_name] | get [file_name] | close\n");
		scanf("%s", command);
		if (!strcmp(strClose, command)){  // close �Է½� ���� 

			strcpy(msg, strClose);
			retcode = write(sock, msg, 20);  //�޼��� ���� 

			break; // ����. 

		}
		scanf("%s", file_name);

		////////////////////////////// put 


		if (!strcmp(strPut, command)){  // put �Է½� 

			printf("----Put! %s \n", file_name);
			FILE *fp = fopen(file_name, "r+");  // fp��� ������ �а��� �������� ����.

			fseek(fp, 0, SEEK_END); //file size check �����ǳ��� �����ϸ鼭 ������ ����� ���ϴ°��� 
			fileSize = ftell(fp);             //���ϻ����� fileSize ������ ����. 
			printf("[%s] (size: %d MB)\n", file_name, fileSize / 1000000);
			fseek(fp, 0, SEEK_SET); //�ٽ� ó���� �����Ѵ�. �����ǳ��� �����ϰ������� ������ �ȵ�. 
			sumfileSize = 0;
			count = 0;
			retcode = 1;

			strcpy(msg, strPut); // put Ŀ��带 server�� �����Ͽ� ���� �������� receive���� �غ� �϶�� ������. 
			retcode = write(sock, msg, 20);  //�޼��� ���� 

			strcpy(nameAndSize, file_name); //
			strcat(nameAndSize, ".");
			sprintf(msg, "%d", fileSize);// ���ϻ������ itoa �� ��ȯ�ؼ� ���� 

			printf("fileSize : %s \n", msg);
			strcat(nameAndSize, msg);

			retcode = write(sock, nameAndSize, 50);  //file_name ���� 




			while (1){

				numread = fread(buf, 1, BUFSIZE, fp); // fp������ �о buf�� �����ϰ� �� ������ ����Ʈ ��ŭ numread�� int������ ���� 
				retcode = write(sock, buf, numread);

				sumfileSize += numread; //byte ������� �����ִ°�. ������� ���� ������ ũ�⸦ ��Ÿ��. 

				percent = ((double)sumfileSize / fileSize) * 100;

				//��� ����.  \r �� �����,  fflush(stdout) �� ������ �Լ�  

				printPercent(percent, sumfileSize, fileSize, &count);

				count++; //count�� ��� �ʱ�ȭ���ָ鼭 �ǽð� ���� ��Ÿ��. 

				if (retcode <= -1) {
					printf("client: sendto failed: %d\n", errno); exit(0);
				}
				if (retcode == 0){  // ���̻� �������� ������ ����.

					retval = read(sock, msg, 50);
					if (strcmp(msg, strFinish) == 0){

						printf("\r[**********] (100% ) %dMB/%dMB\n", fileSize / 1000000, fileSize / 1000000);

						count = 0;
						printf(" File upload OK !! \n ");


						break;
					}

				}
			}
		}

		////////////////////////////////client get

		if (!strcmp(strGet, command)){
			FILE* fp;   //���� ���� ���� 
			fp = fopen(file_name, "w+");     // �����̸��� Ŀ��忡�� �Է��Ѱ����� ���, w+�� ������ ���θ��� �ۼ��Ѵٴ� ����.     
			count = 0;
			percent = 0;
			sumfileSize = 0;
			fileSize = 0;
			retval = 1;
			printf("----Get! %s \n", file_name);

			strcpy(msg, strGet); //get command �� �����ؼ� server�� �˸�. 
			retcode = write(sock, msg, 20);  //�޼��� ���� 
			strcpy(msg, file_name);
			retcode = write(sock, msg, 50);  //file_name ���� 

			//���⼭ �޾ƿ� msg�� �������� �����Ѱ��ε�, fileSize �� sprintf�� ��ȯ�ؼ� string���� ������. 
			nread = read(sock, msg, 20);
			printf("---------------***  %s\n", msg);

			fileSize = atoi(msg); //���� ���ڸ� �ٽ� string ���� int�� ��ȯ�ϱ����� atoi�Լ����. 

			printf("getting file %s : %dMB\n", file_name, fileSize / 1000000);

			while (1){

				retval = read(sock, buf, BUFSIZE);
				fwrite(buf, 1, retval, fp);  // �޾ƿ� buf�� ���Ͽ� �ۼ�.
				sumfileSize += retval;
				percent = ((double)sumfileSize / fileSize) * 100;
				printPercent(percent, sumfileSize, fileSize, &count);
				count++;


				if (sumfileSize >= fileSize){ //���̻� �޾ƿ� ������ ������ �Ϸ�. 
					printf("\r[**********] (100% ) %dMB/%dMB\n", fileSize / 1000000, fileSize / 1000000);
					count = 0;
					fclose(fp);
					break;

				}

			}
			printf(" File download OK.\n");
		} //get ����. 



	}

	close(sock);
}

void printPercent(double percent, int sumfileSize, int fileSize, int* count)
{
	if (percent<10 && *count == 10000){
		printf("[*         ] ( %d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<20 && *count == 10000){
		printf("[**        ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<30 && *count == 10000){
		printf("[***       ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<40 && *count == 10000){
		printf("[****      ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<50 && *count == 10000){
		printf("[*****     ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<60 && *count == 10000){
		printf("[******    ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<70 && *count == 10000){
		printf("[*******   ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<80 && *count == 10000){
		printf("[********  ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else if (percent<100 && *count == 10000){
		printf("[********* ] (%d% ) %dMB/%dMB\r", (int)percent, sumfileSize / 1000000, fileSize / 1000000); fflush(stdout);
		*count = 0;
	}
	else {}


}
