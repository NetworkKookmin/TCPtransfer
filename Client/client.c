
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
		printf("Enter connect [portNumber] [hostAddress] \n"); // 처음 접속시의 출력문  connect localhost 10000 순으로 입력 
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
		if (!strcmp(strClose, command)){  // close 입력시 종료 

			strcpy(msg, strClose);
			retcode = write(sock, msg, 20);  //메세지 전송 

			break; // 종료. 

		}
		scanf("%s", file_name);

		////////////////////////////// put 


		if (!strcmp(strPut, command)){  // put 입력시 

			printf("----Put! %s \n", file_name);
			FILE *fp = fopen(file_name, "r+");  // fp라는 파일을 읽고쓰기 형식으로 연다.

			fseek(fp, 0, SEEK_END); //file size check 파일의끝을 참고하면서 파일의 사이즈를 구하는것임 
			fileSize = ftell(fp);             //파일사이즈 fileSize 변수에 저장. 
			printf("[%s] (size: %d MB)\n", file_name, fileSize / 1000000);
			fseek(fp, 0, SEEK_SET); //다시 처음을 보게한다. 파일의끝을 보게하고있으면 전송이 안됨. 
			sumfileSize = 0;
			count = 0;
			retcode = 1;

			strcpy(msg, strPut); // put 커멘드를 server에 전송하여 이제 서버에서 receive받을 준비를 하라고 말해줌. 
			retcode = write(sock, msg, 20);  //메세지 전송 

			strcpy(nameAndSize, file_name); //
			strcat(nameAndSize, ".");
			sprintf(msg, "%d", fileSize);// 파일사이즈는 itoa 로 변환해서 전달 

			printf("fileSize : %s \n", msg);
			strcat(nameAndSize, msg);

			retcode = write(sock, nameAndSize, 50);  //file_name 전송 




			while (1){

				numread = fread(buf, 1, BUFSIZE, fp); // fp파일을 읽어서 buf에 저장하고 그 저장한 바이트 만큼 numread에 int값으로 저장 
				retcode = write(sock, buf, numread);

				sumfileSize += numread; //byte 수를계속 더해주는것. 현재까지 보낸 파일의 크기를 나타냄. 

				percent = ((double)sumfileSize / fileSize) * 100;

				//출력 포멧.  \r 은 덮어쓰기,  fflush(stdout) 은 덮어쓰기용 함수  

				printPercent(percent, sumfileSize, fileSize, &count);

				count++; //count를 계속 초기화해주면서 실시간 상태 나타냄. 

				if (retcode <= -1) {
					printf("client: sendto failed: %d\n", errno); exit(0);
				}
				if (retcode == 0){  // 더이상 보낼것이 없으면 종료.

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
			FILE* fp;   //받을 파일 저장 
			fp = fopen(file_name, "w+");     // 파일이름을 커멘드에서 입력한것으로 사용, w+는 없으면 새로만들어서 작성한다는 형식.     
			count = 0;
			percent = 0;
			sumfileSize = 0;
			fileSize = 0;
			retval = 1;
			printf("----Get! %s \n", file_name);

			strcpy(msg, strGet); //get command 를 전송해서 server에 알림. 
			retcode = write(sock, msg, 20);  //메세지 전송 
			strcpy(msg, file_name);
			retcode = write(sock, msg, 50);  //file_name 전송 

			//여기서 받아온 msg는 서버에서 전송한것인데, fileSize 를 sprintf로 변환해서 string으로 보낸것. 
			nread = read(sock, msg, 20);
			printf("---------------***  %s\n", msg);

			fileSize = atoi(msg); //받은 문자를 다시 string 에서 int로 변환하기위해 atoi함수사용. 

			printf("getting file %s : %dMB\n", file_name, fileSize / 1000000);

			while (1){

				retval = read(sock, buf, BUFSIZE);
				fwrite(buf, 1, retval, fp);  // 받아온 buf를 파일에 작성.
				sumfileSize += retval;
				percent = ((double)sumfileSize / fileSize) * 100;
				printPercent(percent, sumfileSize, fileSize, &count);
				count++;


				if (sumfileSize >= fileSize){ //더이상 받아온 정보가 없을때 완료. 
					printf("\r[**********] (100% ) %dMB/%dMB\n", fileSize / 1000000, fileSize / 1000000);
					count = 0;
					fclose(fp);
					break;

				}

			}
			printf(" File download OK.\n");
		} //get 종료. 



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
