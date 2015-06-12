#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
 
int main(int argc, char *argv[])
{
    struct servent *servp;
    struct sockaddr_in my_addr, client_addr;
    int request_sock, new_sock;
    int nfound, fd, maxfd, bytesread;
    fd_set rmask, mask;
    static struct timeval timeout = { 5, 0 }; /* 5 seconds */
 
int sockid=0, nread=0, addrlen=0 , retval=0;
    int retcode;
    size_t numread;       //소켓에서 받아온 데이터 크기를 저장하는 변수들

    
    char msg[255];
    char buf[255];
    char file_name[50];
    char strPut[10] ="put";        // client 명령 비교하기 위해 (strcmp 사용)
    char strGet[10] ="get";
    char strClose[10] ="close";
    char strACK[10]="ACK";
    char strEmpty[10]="Empty";
    int fileSize=0;              //파일의 전체 사이즈를 나타내는 변수
    int SendBUFSIZE=64;
    int RecvBUFSIZE=64;
    int sumfileSize=0;           //받아오거나 보내는 파일의 현재 사이즈를 나타내는 변수
    int secondCount=0;           // 1초를 나타내기 위한 count 단위
    double percent;    
    char file_size[50];
    int start=0;
    int thiscount=0;
    int sizeCount=0;
    char nameAndSize[50];
    char strFinish[50]="finish";

    char strSendrate[20]="sendrate";
    char strRecvrate[20]="recvrate";
    char Sendrate[20];
    char Recvrate[20];


    if (argc != 2) {
        (void) fprintf(stderr,"usage: %s service|port\n",argv[0]);
        exit(1);
    }
    if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    if (isdigit(argv[1][0])) {
        static struct servent s;
        servp = &s;
        s.s_port = htons((u_short)atoi(argv[1]));
    } else if ((servp = getservbyname(argv[1], "tcp")) == 0) {
        fprintf(stderr,"%s: unknown service\n", "tcp");
        exit(1);
    }
    memset((void *) &my_addr, 0, sizeof my_addr);
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = servp->s_port;
    if (bind(request_sock, (struct sockaddr *)&my_addr, sizeof my_addr) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(request_sock, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }
    FD_ZERO(&mask);
    FD_SET(request_sock, &mask);
    maxfd = request_sock;

 while(1){
            /* a new connection is available on the connetion socket */
            addrlen = sizeof(client_addr);
            new_sock = accept(request_sock,(struct sockaddr *)&client_addr, (socklen_t*) &addrlen);
            if (new_sock < 0) {
                perror("accept");
                exit(1);
            }
            printf("connection from host %s, port %d, socket %d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                new_sock);
            FD_SET(new_sock, &mask);
            if (new_sock > maxfd) maxfd = new_sock;
            FD_CLR(request_sock, &rmask);
        break;
     

}

while(1){

        

        printf("wait message!\n");
        nread = read(new_sock,msg,20);    
        // client 에서 보낸 command명령 받음

        ////////////////////////////////client put
        
        if(!strcmp(strClose,msg)){  
            break; 
        } 
        
        if(!strcmp(strSendrate,msg)){
            read(new_sock,Recvrate,20);
            RecvBUFSIZE=atoi(Recvrate);
            printf("Set RecvRate : %d \n",RecvBUFSIZE);


        }
        if(!strcmp(strRecvrate,msg)){
            read(new_sock,Sendrate,20);
            SendBUFSIZE=atoi(Sendrate);
            printf("Set SendRate : %d \n",SendBUFSIZE);


        }

        if(!strcmp(strPut,msg)){

            nread = read(new_sock,msg,50);
            //recive file_name & file_Size  client에서 파일이름 받아옴
            sizeCount=0;
            thiscount=0;
            fileSize=0;

            bzero(file_size,50);
            bzero(file_name,50);

            strcpy(nameAndSize,msg);
            for(start=0; start<strlen(nameAndSize);start++){
                if(nameAndSize[start]=='.') thiscount=start; 
                
            }

            for(start=0; start<thiscount; start++){
              file_name[sizeCount]=nameAndSize[start];
                sizeCount++;
            }
            file_name[thiscount]='\0';
            sizeCount=0;
            
            for(start=thiscount+1; start<strlen(msg); start++){
              file_size[sizeCount]=nameAndSize[start];
                sizeCount++;
            }

            printf("file name : %s \n",file_name);

            fileSize=atoi(file_size);
            //recive fileSize    client에서 파일 사이즈 받아옴                    

            printf("file size : %d \n",fileSize);


            FILE* fp;                   //파일 지정 후 받아온 파일이름으로 오픈.
            fp=fopen(file_name,"w+");   // w+ 쓰기+읽기모드로 파일이 없는 경우에 생성함.
            
            percent=0;            // 출력인자 초기화
            sumfileSize=0;
            retval=1;
            //client put start

            printf("Transfer status: recv[%s] [ 0% , 0MB/%dMB]\n",file_name,fileSize/1000000); //출력포멧. 0%인 경우.
              strcpy(msg,strEmpty);
            while(1){

                        retval = read(new_sock,buf,RecvBUFSIZE);
                sumfileSize+= retval; //그것들을 현재받는파일의 사이즈에 더해준다 // 현재 크기를 나타냄.

                fwrite(buf,1,retval,fp);
                
                 //이건 받아온 버퍼를 fp라는 FILE 포인터 변수에 저장하는데. 이것이 파일로 써지는것.
                percent=((double)sumfileSize/fileSize)*100; // 전체크기로 나눠서 퍼센트로 나타냄.

                if(secondCount==80000){ // 약 1초가 되는경우에 출력함.
                    printf("Transfer status: recv[%s] [%d% , %dMB/%dMB]\n",file_name,(int)percent,sumfileSize/1000000,fileSize/1000000);
                    secondCount=0;
                    
                }
                
                if(sumfileSize==fileSize ){  // 더이상 보낼 것이 없으면 종료한다.
                    printf(" receive file : %d\n ",sumfileSize);
                    fclose(fp);

            printf("Transfer status: recv[%s] [100% , %dMB/%dMB]\n",file_name,fileSize/1000000,fileSize/1000000);

            printf(" File download OK !!\n"); //완료시 출력
                    retval = write(new_sock,strFinish,50);
                    break;
                }

                secondCount++; // 시간을 나타내기위한 단위 80000이되면 다시 초기화됨.
                

            }  //close while

                    strcpy(msg,strEmpty);

        }
        ////////////////////////////////client put






////////////////////////////////client 에서 get명령 호출시 아래와 같이 실행 
 
        if (!strcmp(strGet,msg)){ 
            

            nread = read(new_sock,msg,50);  
     
            //recive file_name 파일의 이름을 받아온다. 
 
            strcpy(file_name,msg); //파일 이름을 msg--> file_name 으로 복사한다. 
 
            printf("sending file %s \n", msg);  //파일 이름 출력 
 
            FILE* fp; 
            fp=fopen(file_name,"r+");    //file open     
            percent=0; 
            sumfileSize=0; 
            retcode=1; 
            fseek(fp,0,SEEK_END); // fseek 함수는 파일의 끝을 확인해서 그 파일의 사이즈를 볼 수 있게해줌. 
            fileSize=ftell(fp); //파일 사이즈를 저장한다. 
            printf("file size %d\n",fileSize);            
            printf("Transfer status: send[%s] [ 0% , 0MB/%dMB]\n",file_name,fileSize/1000000); 
            sprintf(msg,"%d",fileSize); // sprintf는 itoa 의 대신하는 리눅스의 인트를 문자로 변경하는 함수.
 
            retcode = write(new_sock,msg,20);  
            //fileSize를 클라이언트에 전송함 
 
            fseek(fp,0,SEEK_SET); // start reference 다시 시작을 가리키게 변경 
            secondCount=0; 
     
            while(1){             
 
 
                numread = fread(buf,1,SendBUFSIZE,fp); 
 
                //파일을 읽어서 buf에 저장하고, 그 읽은 만큼의 바이트 크기를 numread에 저장. 
 
                retcode = write(new_sock,buf,numread);   
                // buf에있는정보를 client에 전달하고 , 그 전달한 만큼의 바이트 크기를 retcode에 저장. 
                 
                sumfileSize+=retcode; //현재 보내는 파일의 크기를 sumfileSize에 계속 추가로 더함. 
                 
                percent=((double)sumfileSize/fileSize)*100; //퍼센트 정의 
                 
 
                if(secondCount==80000){ 
                    printf("Transfer status: send[%s] [%d% , %dMB/%dMB]\n",file_name,(int)percent,sumfileSize/1000000,fileSize/1000000); 
                    secondCount=0;                 
                } 
 
                secondCount++; 
                if (retcode <= -1) { 
                    printf("client: sendto failed: %d\n",errno); 
                    exit(0); 
                } 
                if(sumfileSize>=fileSize){ 
                printf("sumfilesize : %d numread : %d , retcode : %d \n",sumfileSize,numread,retcode);
    printf("Transfer status: send[%s] [100% , %dMB/%dMB]\n",file_name,fileSize/1000000,fileSize/1000000); 
                    printf(" File upload OK  !! \n "); 
                    break;
                } 
 
            }  //close while 
 
        } //close get if 



          }

    printf("Finished Server\n " );
}


      

 
