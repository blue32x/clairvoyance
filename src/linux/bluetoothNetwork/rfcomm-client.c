#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
	struct sockaddr_rc addr = { 0 };
	int s, status, bytes_read;
	char dest[18] = "00:1A:7D:DA:71:11";
	char buf[1024] = "hello!";

	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( dest, &addr.rc_bdaddr );

	// connect to server
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

	// send a message
	if( status == 0 ) {
		status = write(s, buf, sizeof(buf));
	}
	if( status < 0 ) {
		perror("uh oh");
	}
	else {
		printf("sended [%s]\n", buf);
	}

	// read data from the server
	bytes_read = read(s, buf, sizeof(buf));
	if( bytes_read > 0 ) {
		printf("received [%s]\n", buf);
	}

	close(s);
	return 0;
}
