#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <android/log.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBYTES 200 /*max text line length*/
#define SERV_PORT 3000
#define LISTENQ 8 /*maximum number of client connections */
#undef LOG_TAG
#define LOG_TAG "serverConnection"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
int connfd; // global variable to share connection link with Tx and Rx handler

struct readThreadParams {
	JNIEnv *env;
	jobject obj;
};

extern "C" {
		JNIEXPORT jstring JNICALL
		Java_android_igc_server_MySyncTask_startCommunication
		(JNIEnv *env, jobject obj)
		{
			 int listenfd, n;
			 char receiveBuffer[MAXBYTES];
			 struct sockaddr_in clientAddress, servaddr;
			 char returnNOTok[]="NOT_OK";
			 memset(&clientAddress,'0',sizeof(clientAddress));
			 memset(&servaddr,'0',sizeof(servaddr));
			 readThreadParams *params = (readThreadParams*)malloc(sizeof(readThreadParams));
			 params->env = env;
			 params->obj = obj;
			 int sockoptval = 1;
			 /* master file descriptor list */
			 fd_set master;
			 /* temp file descriptor list for select() */
			 fd_set read_fds;
 		     /* maximum file descriptor number */
			 int fdmax;
			 /* listening socket descriptor */
			 int listener;
			 /* newly accept()ed socket descriptor */
			 int newfd;
			 /* for setsockopt() SO_REUSEADDR, below */
			 int addrlen;
		 	 int receivedBytes;
			 int socketFD, recieveSocketFD;
			 jobject result;
			 /* clear the master and temp sets */
			 FD_ZERO(&master);
			 FD_ZERO(&read_fds);
			 //creation of the socket
			 listenfd = socket (AF_INET, SOCK_STREAM, 0);

			 //preparation of the socket address
			 servaddr.sin_family = AF_INET;
			 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			 servaddr.sin_port = htons(SERV_PORT);
			 memset(&(servaddr.sin_zero), '\0', 8);
			 setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

			 if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
			 {
				 LOGI("server bind() error !! \n");
			    exit(1);
			 }
			 LOGI("server bind() is OK...\n");
			 if(listen(listenfd, LISTENQ)==-1)
			 {
				 LOGI("server Listen Error");
			 }
			 LOGI("server listen() is OK...\n");
			 LOGI("server listenfd = %d \n", listenfd);
			 /* add the listener to the master set */
 			 FD_SET(listenfd, &master);
			 /* keep track of the biggest file descriptor */
 			 fdmax = listenfd; /* so far, it's this one*/

			 jstring jstr = params->env->NewStringUTF("This comes from jni.");
			 jclass clazz = params->env->FindClass("android/igc/server/MySyncTask");
			 jmethodID DisplayData = params->env->GetMethodID(clazz, "DisplayData", "(Ljava/lang/String;)Ljava/lang/String;");
			 LOGI("server running...waiting for connections");

  			 for ( ; ; )
			 {
  				read_fds = master; // initiase read_fds with master as master is by default assigned with listener fd
  				if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
  				{
  				    LOGI("server select() error !!");
  				    exit(1);
  				}
  				LOGI("server select() is OK...\n");
  			    for(socketFD = 0; socketFD <= fdmax; socketFD++)
				{
  			    	if(FD_ISSET(socketFD, &read_fds))
				    { /* we got one... */
					    if(socketFD == listenfd)
					     {
				         /* handle new connections */
    				        addrlen = sizeof(clientAddress);
    				        if((newfd = accept(listenfd, (struct sockaddr *)&clientAddress, &addrlen)) == -1)
    				        {
    				        	LOGI("server accept() error !!");
    				        }
    				        else
    				        {
    				        	LOGI("server accepted client request ");
    				        	FD_SET(newfd, &master); /* add to master set */
    				        	connfd = newfd; // storing in global variable
    				        	if(newfd > fdmax)
    				        	{ /* keep track of the maximum */
    				        		fdmax = newfd;
    				        	}
								jstr = env->NewStringUTF("Connection Established");
								result = env->CallObjectMethod(obj, DisplayData, jstr);

    				        	LOGI("server : New connection from %s on socket %d\n",  inet_ntoa(clientAddress.sin_addr), newfd);
    				        }
					     }
					     else
					     {
					    	 LOGI("server Handling received data \n");
					      /* handle data from a client */
							  if((receivedBytes = recv(socketFD, receiveBuffer, sizeof(receiveBuffer), 0)) <= 0)
							  {
							   /* got error or connection closed by client */
								  if(receivedBytes == 0)
								  {
								/* connection closed */
									  LOGI("server : Client has closed the connection on socket %d \n", socketFD);
									  jstr = env->NewStringUTF("Client Terminated");
									  result = env->CallObjectMethod(obj, DisplayData, jstr);

									  connfd = 0;
								  }
								  else
									  LOGI("server data receive error !!!");
								  /* close it... */
								  close(socketFD);
								  /* remove from master set */
								  FD_CLR(socketFD, &master);
							  }
							  else
							  {
								 // we got some data from a client
								  LOGI("server Data received = %s from Client on Socket = %d\n", receiveBuffer, socketFD);
								  jstr = env->NewStringUTF(receiveBuffer);
								  LOGI("server Updating Android Application ");
								  result = env->CallObjectMethod(obj, DisplayData, jstr);
								  memset(receiveBuffer,0,sizeof(receiveBuffer)); // resetting receive buffer
							  } // sending data to UI
					     } // else clause, handling received data from client
				    } // if clause, checking events on fd
				} // for loop, running through all fd's
			 } // for loop
		} // Java_android_igc_server_MySyncTask_stringFromJNI

		jstring Java_android_igc_server_MainActivity_Send2Client
		(JNIEnv *env, jobject obj, jstring TxData)
		{
			  char returnValue[MAXBYTES]="True";
			  jstring jstr = env->NewStringUTF("This comes from jni.");
			  const char *TxData2Send = (env)->GetStringUTFChars(TxData, 0);
			  if(connfd != 0)
			  {
				  send(connfd, TxData2Send, strlen(TxData2Send), 0);
				  LOGI("server Data sent =  %s",TxData2Send);
			  }
			  else
			  {
				  LOGI("server client connection not Established !!!!");
			  }
			  //release all the passed strings after using them
			  (env)->ReleaseStringUTFChars(TxData, TxData2Send);
			  return jstr;
		}// Java_android_igc_server_MainActivity_Send2Client

} // extern "C"
