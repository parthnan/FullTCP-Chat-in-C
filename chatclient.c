#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#define SA struct sockaddr 
#define NUMPROCS 1024
#define TIMEOUT 60
//check for zombie process with ps -el
int quit=0;
void myalarm(int sec) {
	static int children[NUMPROCS];
	static int parent;
	static int j=0,org=0;
	int i,pid, status;
	if(org==0){parent=getpid();
		org=1;}
	for (i=0; i<j; i++) {
 		if (kill(children[i],SIGTERM) == -1) {
 			perror("kill failed.");
 			exit(1);}
 	}
	time_t td,start=clock();
	if ((pid=fork())== -1) {
 		perror("fork failed.");exit(1); 
 	}
 	if (pid == 0) { /* Child process */
 		do{
			td=clock()-start;
		}while(td<sec*CLOCKS_PER_SEC);
		for (i=0; i<j; i++) {
 			if (kill(children[i],SIGTERM) == -1)
 			{  perror("kill failed.");	 exit(1);}
 		}
		kill(parent,SIGALRM);
 		exit(0);
 	} else {
 		children[j++] = pid;
 	}
}

void timeout()
{
 quit=1;
}
int main(int argc,char **argv) 
{
 int sock,csock,reuse,error=0,recurse=0,nbytes;
 struct sockaddr_in host;
 struct hostent *hp;
 char rbuf[1024],hname[256],temp[101];
 char quitcmd[1024]="!exit";
 char abandon[1024]="!abandon";
 fd_set rfds; 		/* select() で用いるファイル記述子集合 */
 struct timeval tv;	/* select() が返ってくるまでの待ち時間を指定する変数 */
 if (argc != 3) {
 	fprintf(stderr,"Usage: %s hostname username(0-9,-,_)\n",argv[0]);
 	exit(1);}
  //状態c1
 if ((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
 	perror("socket");
	exit(1);
 }
 reuse=1;
 if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0) {
	perror("setsockopt");
	exit(1);
 }

 /* client送信用ソケットの情報設定*/
 bzero(&host,sizeof(host));
 host.sin_family=AF_INET;
 host.sin_port=htons(10140); 
 /*ポート番号10140を介して送信*/

 // connect the client socket to server socket
 if ( ( hp = gethostbyname(argv[1]) ) == NULL ) 
 { 
 	fprintf(stderr,"unknown host %s\n",argv[1]);
 	exit(1);//状態c6
 }

 bcopy(hp->h_addr,&host.sin_addr,hp->h_length); 
 if ( connect(sock, (SA*)&host, sizeof(host) )  ==1 ) 
 { 
 	fprintf(stderr,"connection with host failed! %s\n",argv[1]);
 	exit(1);//状態c6
 }
 //状態c2
if((nbytes = read(sock,rbuf,sizeof(rbuf)))<0){
	perror("REQUEST NOT ACCEPTED:");close(sock);exit(1);
 }
 if(nbytes!=17||memcmp(rbuf,"REQUEST ACCEPTED",16)!=0){
	printf("REQUEST NOT ACCEPTED: *%s\n",rbuf);close(sock);exit(1);
 }
 printf("Connected to: [%s]\n",argv[1]);
 sprintf(hname,"%s: ",argv[1]);

  //状態c3
 sprintf(temp,"%s\n",argv[2]);
 if ( (write(sock,temp,strlen(temp)) ) < 0) {
	perror("USERNAME NOT REGISTERED");close(sock);exit(1);
 } 
 if ( ( nbytes = read(sock,rbuf,sizeof(rbuf)) ) < 0) {
	perror("USERNAME NOT REGISTERED");close(sock);exit(1);
 } 
 if(memcmp(rbuf,"USERNAME REGISTERED",19)!=0){
	perror("USERNAME NOT REGISTERED\n");close(sock);exit(1);
 }
 printf("Username registered with server; auto-timeout after 60 seconds inactivity\n");
 if(signal(SIGALRM,timeout) == SIG_ERR) {
 	perror("signal failed."); exit(1); }
 myalarm(TIMEOUT);
  //状態c4
 do{
 		/* 入力を監視するファイル記述子の集合を変数 rfds にセットする */
 	FD_ZERO(&rfds);	
 	FD_SET(0,&rfds);   /* rfds を空集合に初期化 */
 	FD_SET(sock,&rfds); 
 	tv.tv_sec = 1;tv.tv_usec = 0;  /* 監視する待ち時間を 1 秒に設定 */
	if(quit==1){
		printf("You are Timed Out.\n");
		break;
	}

 		/* 標準入力とソケットからの受信を同時に監視する */
 	if(select(sock+1,&rfds,NULL,NULL,&tv)>0) {
		 
 		if(FD_ISSET(0,&rfds)) { 
			int c;
 			/* 標準入力から読み込みクライアントに送信 */
 			if ( ( nbytes = read(0,rbuf,sizeof(rbuf)) ) < 0) {
				if(error==0){perror("read");error=1;break;}
			} 
			else if((c=rbuf[0])==EOF){printf("You have left the chat.\n");quit=1;
						break;}
			else{error=0;}
			write(sock,rbuf,nbytes);
			myalarm(TIMEOUT);
 		}
 		if(FD_ISSET(sock,&rfds)) { 
 			/* serverから読み込み端末に出力 */
 			if ( ( nbytes = read(sock,rbuf,sizeof(rbuf)) ) < 0) {
				if(error==0){perror("read");error=1;break;}
				
			}
			write(1,rbuf,nbytes);
		}
 	}
 	sprintf(rbuf," ");
 } 
 while(1); /* 繰り返す */
 //状態c5
 close(sock);
 exit(0);
}
