#include <common/types.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>



int main(int argc,const char** argv){
	if (argc<2){
		return 1;
	}
	unlink(argv[1]);
	struct termios old_config;
	tcgetattr(0,&old_config);
	struct termios new_config=old_config;
	new_config.c_iflag&=~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|IXON);
	new_config.c_iflag|=ICRNL;
	new_config.c_lflag&=~(ECHO|ECHONL|ICANON|ISIG);
	new_config.c_lflag|=IEXTEN;
	tcsetattr(0,TCSANOW,&new_config);
	int server=socket(AF_UNIX,SOCK_STREAM,0);
	struct sockaddr_un address;
	address.sun_family=AF_UNIX;
	strcpy(address.sun_path,argv[1]);
	if (bind(server,(void*)(&address),sizeof(struct sockaddr_un))<0){
		goto _close;
	}
	listen(server,1);
	int client=accept(server,NULL,NULL);
	u8 buffer[4096];
	struct pollfd poll_fds[2]={
		{
			.fd=client,
			.events=POLLIN|POLLERR
		},
		{
			.fd=0,
			.events=POLLIN|POLLERR
		}
	};
	while (1){
		if (poll(poll_fds,2,-1)<0||((poll_fds[0].revents|poll_fds[1].revents)&(POLLERR|POLLHUP))){
			break;
		}
		if (poll_fds[0].revents&POLLIN){
			ssize_t length=read(client,buffer,sizeof(buffer));
			if (length<0||write(1,buffer,length)!=length){
				break;
			}
		}
		if (poll_fds[1].revents&POLLIN){
			ssize_t length=read(0,buffer,sizeof(buffer));
			if (length<0||write(client,buffer,length)!=length){
				break;
			}
		}
	}
_close:
	close(server);
	tcsetattr(0,TCSANOW,&old_config);
	return 0;
}
