#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAXCLIENTS 5 
#define MAXMEMORY 512 
#define LEN 150
int quit=0;
int shut=10;
void timeout()
{
 quit=1;
}

int main(int argc,char **argv) 
{
 int sock,csock[MAXMEMORY],connect[MAXCLIENTS],j,i,max=0,index=0,top=-1,clen,nbytes,reuse;
 struct sockaddr_in svr,clt;
 struct tm *loc_time;
 char buf[LEN];	
 time_t curtime;
 struct hostent *cp[MAXMEMORY];
 char rbuf[1024],usernames[MAXCLIENTS][128],ips[MAXCLIENTS][128],nametag[130],temp[128];
 fd_set rfds; 	
 struct timeval tv;	

 //状態s1
 if ((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
 		perror("socket");exit(1);
 }
 reuse=1;
 if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0) {
	perror("setsockopt");exit(1);
 }
 /* client受付用ソケットの情報設定*/
 bzero(&svr,sizeof(svr));
 svr.sin_family=AF_INET;
 svr.sin_addr.s_addr=htonl(INADDR_ANY); 
 /*受付側のIPアドレスは任意*/
 svr.sin_port=htons(10140);  
 /*ポート番号10140を介して受け付ける*//*ソケットにソケットアドレスを割り当てる*/
 if(bind(sock,(struct sockaddr *)&svr,sizeof(svr))<0) {
	perror("bind");
	exit(1);
 }
 /*待ち受けクライアント数の設定*/
 if (listen(sock,MAXCLIENTS)<0){  /*待ち受け数に5を指定*/
 	perror("listen");
	exit(1);
 }

 for(i=0;i<MAXCLIENTS;i++){connect[i]=0; }
 if(signal(SIGINT,timeout) == SIG_ERR) {
 	perror("signal failed."); exit(1); }

 //状態s2
 do{
	/* 入力を監視するファイル記述子の集合を変数 rfds にセットする */
 	FD_ZERO(&rfds);	
 	 /* rfds を空集合に初期化 */
 	FD_SET(0,&rfds); 
	FD_SET(sock,&rfds); 
	tv.tv_sec = 1;tv.tv_usec = 0; 
	max=sock;
	for(i=0;i<=top;i++){
		FD_SET(csock[connect[i]],&rfds);
		if(csock[connect[i]]>max){max=csock[connect[i]];}
		if(quit==1&&shut>0){
			sprintf(temp,"Server: Shutdown in %d seconds\n",shut);
			write(csock[connect[i]],temp,strlen(temp));
		}
		else if(quit==1&&shut==0){
			write(csock[connect[i]],"Server: Shutdown Complete\n",strlen("Server: Shutdown Complete\n"));
		}
	}
	if(quit==1&&(top+1)==0){break;}
	else if(quit==1&&shut>0){printf("Server: Shutdown in %d seconds\n",shut);shut--;}
	else if(quit==1&&shut==0){break;}

 	if(select(max+1,&rfds,NULL,NULL,&tv)>0)
        {
		//状態s3
 		if(FD_ISSET(0,&rfds)) { 
 			// serverから読み込み
 			if ( ( nbytes = read(0,rbuf,sizeof(rbuf)) ) < 0) {
				perror("read server");continue;	
			}
			for(i=0;i<=top;i++){
				if(nbytes>0){write(csock[connect[i]],"Server: ",strlen("Server: "));
					write(csock[connect[i]],rbuf,nbytes);}
			}
		}
		for(i=0;i<=top;i++){
			 if(FD_ISSET(csock[connect[i]],&rfds)) { 
 				//状態s6
 				if ( ( nbytes = read(csock[connect[i]],rbuf,sizeof(rbuf)) ) < 0) {
					perror("read client");continue;	
				}
				if(nbytes==0){//状態s7
					close(csock[connect[i]]);
					sprintf(temp,"%s has left the chat. %d users online.\n",usernames[i],top);
					for ( j = i; j <top; j++) {
      						connect[j] = connect[j+1]; 
						strncpy(usernames[j],usernames[j+1],strlen(usernames[j+1])); 
						strncpy(ips[j],ips[j+1],strlen(ips[j+1]) ); 
   					}
					top--;
					write(1,temp,strlen(temp));
					for(j=0;j<=top;j++){
						write(csock[connect[j]],temp,strlen(temp));
					}
				}
				else if(nbytes==6&&memcmp(rbuf,"/list\n",6)==0){
					write(csock[connect[i]],"Server: ",strlen("Server: "));
					for(j=0;j<=top;j++){
						write(csock[connect[i]],usernames[j],strlen(usernames[j]));
						write(csock[connect[i]]," ; ",3);
					}
				}
				else if(nbytes>1){
					curtime = time (NULL); //Getting current time of system
 					loc_time = localtime (&curtime);// Converting current time to local time
 					strftime (buf, LEN, " [%I:%M %p]", loc_time);

					sprintf(nametag,"%s: ",usernames[i]);
					sprintf(temp,"[%s]\n",ips[i]);
					write(1,nametag,strlen(nametag));
					write(1,rbuf,nbytes-1);
					write(1,buf,strlen(buf));
					write(1,temp,strlen(temp));

					if(memcmp(rbuf,"/send ",6)==0){
						char* token;
						token = strtok(rbuf, " ");
						token = strtok(NULL, " ");	
							
						for(j=0;j<=top;j++){
							if((strlen(token)==strlen(usernames[j]))&&memcmp(token,usernames[j],MIN(strlen(token),strlen(usernames[j])))==0){
								break;
							}
						}
						token = strtok(NULL, "");
						write(csock[connect[j]],nametag,strlen(nametag));
						write(csock[connect[j]],token,nbytes-8-strlen(usernames[j]));
						write(csock[connect[j]],buf,strlen(buf));
						write(csock[connect[j]],temp,strlen(temp));
					}
					else{
						for(j=0;j<=top;j++){
							if(j!=i){
								write(csock[connect[j]],nametag,strlen(nametag));
								write(csock[connect[j]],rbuf,nbytes-1);
								write(csock[connect[j]],buf,strlen(buf));
								write(csock[connect[j]],temp,strlen(temp));
							}
						}
						
					}
				}
			 }
		}
		if(FD_ISSET(sock,&rfds)) { 
			clen = sizeof(clt);  
			if ( ( csock[index] = accept(sock,(struct sockaddr *)&clt,&clen) ) <0 ||top>=MAXCLIENTS-1 || index>=MAXMEMORY-1) {
				write(csock[index],"REQUEST REJECTED\n",17);close(csock[index]);
			} 
			else {//クライアントの受付ができる場合
		
 				//クライアントのホスト情報の取得
				cp[index] = gethostbyaddr((char *)&clt.sin_addr,sizeof(struct in_addr),AF_INET);
				write(csock[index],"REQUEST ACCEPTED\n",17);
				printf("Connected to: [%s]\n",cp[index]->h_name);
				//状態s5
				int namereject=0;
				strcpy(rbuf,"");
				if ( ( nbytes = read(csock[index],rbuf,sizeof(rbuf)) ) < 0) {perror("read");}	
				else{
				   nbytes--;
				   for(i=0;i<=top;i++){
					if(nbytes==strlen(usernames[i]) && memcmp(rbuf,usernames[i],MIN(nbytes,strlen(usernames[i])))==0){
						write(csock[index],"USERNAME REJECTED\n",18);close(csock[index]);	
						printf("Username was rejected;[%s] disconnected\n",cp[index]->h_name);
						namereject=1;break;
					}
			 	   }
			 	   if(namereject==0){
					//クライアントの受付完了	
					write(csock[index],"USERNAME REGISTERED\n",20);
					top++;
					connect[top]=index;
					strncpy(usernames[top],rbuf,nbytes);
					strncpy(ips[top],inet_ntoa(clt.sin_addr),strlen(inet_ntoa(clt.sin_addr)));
					printf("[%s] chose username %s \n",cp[index]->h_name,usernames[top]);
					index++;
			 	   } 
				}
			}	
		}
	}
	//状態s4
	/*clen = sizeof(clt);  
	printf("b4 if\n");*/
	 sprintf(rbuf," ");
 } 
 while(1);
 for(i=0;i<=top;i++){
 	close(csock[connect[i]]);  
 }
 sprintf(rbuf," ");
 
}
