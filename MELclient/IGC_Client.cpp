/*
 * The file contains the Client side code.
 * Which connects to android server.
 * Android server IP to be passed as command line argument.
 * Port number used for communication is 3000.
 */

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

using namespace std;
#define SERV_PORT 3000 /*port*/

//the thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{

    int sockfd = 0, n = 0;
    char recvBuff[1024];
    char sendBuff[1024];
    struct sockaddr_in serv_addr;
    int sentBytes = 0;
    int connectReturnValue = 0;
    int readBytes = 0;
    pthread_t Recv_Thread;
    int *Rx_Sock = (int*)malloc(1);

    if(argc < 2)
    {
    	printf("\n Please Enter IP address of server as commandline argument \n\n");
        return 1;
    }
    printf("\n IP address of server u entered is = %s\n",argv[1]);

    memset(recvBuff,'0',sizeof(recvBuff));
    memset(sendBuff,'0',sizeof(sendBuff));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
  //  close(sockfd);

    memset(&serv_addr,'0',sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    serv_addr.sin_addr.s_addr= inet_addr(argv[1]);

    connectReturnValue = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if( connectReturnValue < 0)
    {
        printf("\n Error : connectReturnValue = %d\n",connectReturnValue);
        printf("\n Error : IS = %s\n",strerror(errno));

        return 1;
    }
    *Rx_Sock = sockfd;
    if( pthread_create( &Recv_Thread , NULL ,  connection_handler , (void*) Rx_Sock) < 0)
    {
    	printf("could not create thread");
        return 1;
    }
    pthread_detach(Recv_Thread);
    sleep(1);
    cout << " \n Enter data to be send to Android Server \n";
    cout << " \n To QUIT enter character Q and hit enter key \n";
    while(1)
    {
    	fgets (sendBuff,200,stdin);
    	if(strcmp(sendBuff,"close\n") == 0)
    	{
    		printf("connection close request received ");
    		close(sockfd);
    		return 1;
    	}
    	if((strcmp(sendBuff,"q\n")!=0) && (strcmp(sendBuff,"Q\n")!=0))
    	{
    		sentBytes = send(sockfd, sendBuff, strlen(sendBuff) + 1, 0);
            if(sentBytes < 0) {
            	printf("Cannot Send Data ");
            }
            else
            {
            	 printf("\n Client Data Sent : %s \n",sendBuff);
            }
    	}
    	else
    	{
    		printf("connection close request received ");
    		close(sockfd);
    		return 1;
    	}
    }
    pthread_exit(0);
    free(Rx_Sock);
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char recvBuff[1024];
    int recvNumber = 0;
	int SIZE = 1;
    FILE* fp = fopen("RxData.txt", "a+");
  //  char buff[100];
    memset(recvBuff,0,sizeof(recvBuff));

    cout << "\n Reader thread started \n";
    while((recvNumber = recv(sock,recvBuff,(sizeof(recvBuff)),0)) > 0)
    {
    	recvBuff[recvNumber] = 0;

    	if(fp == NULL)
    	{
    		printf("\n************************ File cannot be opened for wartting !! ******************* \n");
    	}
    	else
    	{
    		fp = fopen("RxData.txt", "a+");
    		//fwrite(recvBuff,SIZE,strlen(recvBuff),fp);
    		printf("\n Data Received = %s\n",recvBuff);
    		printf("\n It is been written to file \n");
    		fprintf(fp,"\n **************************************************************************** \n");
    		fprintf(fp,"\n  %s  \n",recvBuff);
    		fprintf(fp,"\n ***************************************************************************** \n");
    		fclose(fp);
    		memset(recvBuff,0,sizeof(recvBuff));
    	}

    }
    if(recvNumber == -1)
    {
    	printf("receive failed");
		printf("\n Could not read data at Client \n");
		fprintf(fp,"\n *********************** Could not read data at Client ***************************** \n");
		fclose(fp);
    }
    return 0;
}
