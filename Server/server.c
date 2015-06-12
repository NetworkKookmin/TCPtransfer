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
    size_t numread;       //���Ͽ��� �޾ƿ� ������ ũ�⸦ �����ϴ� ������

    
    char msg[255];
    char buf[255];
    char file_name[50];
    char strPut[10] ="put";        // client ��� ���ϱ� ���� (strcmp ���)
    char strGet[10] ="get";
    char strClose[10] ="close";
    char strACK[10]="ACK";
    char strEmpty[10]="Empty";
    int fileSize=0;              //������ ��ü ����� ��Ÿ���� ����
    int SendBUFSIZE=64;
    int RecvBUFSIZE=64;
    int sumfileSize=0;           //�޾ƿ��ų� ������ ������ ���� ����� ��Ÿ���� ����
    int secondCount=0;           // 1�ʸ� ��Ÿ���� ���� count ����
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
        // client ���� ���� command��� ����

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
            //recive file_name & file_Size  client���� �����̸� �޾ƿ�
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
            //recive fileSize    client���� ���� ������ �޾ƿ�                    

            printf("file size : %d \n",fileSize);


            FILE* fp;                   //���� ���� �� �޾ƿ� �����̸����� ����.
            fp=fopen(file_name,"w+");   // w+ ����+�б���� ������ ���� ��쿡 ������.
            
            percent=0;            // ������� �ʱ�ȭ
            sumfileSize=0;
            retval=1;
            //client put start

            printf("Transfer status: recv[%s] [ 0% , 0MB/%dMB]\n",file_name,fileSize/1000000); //�������. 0%�� ���.
              strcpy(msg,strEmpty);
            while(1){

                        retval = read(new_sock,buf,RecvBUFSIZE);
                sumfileSize+= retval; //�װ͵��� ����޴������� ����� �����ش� // ���� ũ�⸦ ��Ÿ��.

                fwrite(buf,1,retval,fp);
                
                 //�̰� �޾ƿ� ���۸� fp��� FILE ������ ������ �����ϴµ�. �̰��� ���Ϸ� �����°�.
                percent=((double)sumfileSize/fileSize)*100; // ��üũ��� ������ �ۼ�Ʈ�� ��Ÿ��.

                if(secondCount==80000){ // �� 1�ʰ� �Ǵ°�쿡 �����.
                    printf("Transfer status: recv[%s] [%d% , %dMB/%dMB]\n",file_name,(int)percent,sumfileSize/1000000,fileSize/1000000);
                    secondCount=0;
                    
                }
                
                if(sumfileSize==fileSize ){  // ���̻� ���� ���� ������ �����Ѵ�.
                    printf(" receive file : %d\n ",sumfileSize);
                    fclose(fp);

            printf("Transfer status: recv[%s] [100% , %dMB/%dMB]\n",file_name,fileSize/1000000,fileSize/1000000);

            printf(" File download OK !!\n"); //�Ϸ�� ���
                    retval = write(new_sock,strFinish,50);
                    break;
                }

                secondCount++; // �ð��� ��Ÿ�������� ���� 80000�̵Ǹ� �ٽ� �ʱ�ȭ��.
                

            }  //close while

                    strcpy(msg,strEmpty);

        }
        ////////////////////////////////client put






////////////////////////////////client ���� get��� ȣ��� �Ʒ��� ���� ���� 
 
        if (!strcmp(strGet,msg)){ 
            

            nread = read(new_sock,msg,50);  
     
            //recive file_name ������ �̸��� �޾ƿ´�. 
 
            strcpy(file_name,msg); //���� �̸��� msg--> file_name ���� �����Ѵ�. 
 
            printf("sending file %s \n", msg);  //���� �̸� ��� 
 
            FILE* fp; 
            fp=fopen(file_name,"r+");    //file open     
            percent=0; 
            sumfileSize=0; 
            retcode=1; 
            fseek(fp,0,SEEK_END); // fseek �Լ��� ������ ���� Ȯ���ؼ� �� ������ ����� �� �� �ְ�����. 
            fileSize=ftell(fp); //���� ����� �����Ѵ�. 
            printf("file size %d\n",fileSize);            
            printf("Transfer status: send[%s] [ 0% , 0MB/%dMB]\n",file_name,fileSize/1000000); 
            sprintf(msg,"%d",fileSize); // sprintf�� itoa �� ����ϴ� �������� ��Ʈ�� ���ڷ� �����ϴ� �Լ�.
 
            retcode = write(new_sock,msg,20);  
            //fileSize�� Ŭ���̾�Ʈ�� ������ 
 
            fseek(fp,0,SEEK_SET); // start reference �ٽ� ������ ����Ű�� ���� 
            secondCount=0; 
     
            while(1){             
 
 
                numread = fread(buf,1,SendBUFSIZE,fp); 
 
                //������ �о buf�� �����ϰ�, �� ���� ��ŭ�� ����Ʈ ũ�⸦ numread�� ����. 
 
                retcode = write(new_sock,buf,numread);   
                // buf���ִ������� client�� �����ϰ� , �� ������ ��ŭ�� ����Ʈ ũ�⸦ retcode�� ����. 
                 
                sumfileSize+=retcode; //���� ������ ������ ũ�⸦ sumfileSize�� ��� �߰��� ����. 
                 
                percent=((double)sumfileSize/fileSize)*100; //�ۼ�Ʈ ���� 
                 
 
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


      

 
