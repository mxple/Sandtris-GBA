#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int sock = 0;
	struct addrinfo hints, *result;
	char msg[256];

	if (argc < 2) {
		return -1;
	}

	realpath(argv[1], msg);

	if (argc > 2) {
		strcat(msg, " GDB");
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	if (getaddrinfo("host.docker.internal", "2346", &hints, &result)) {
		return -1;
	}

	if ((sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0) {
		return -1;
	}

	if (connect(sock, (struct sockaddr*)result->ai_addr, result->ai_addrlen) < 0) {
		return -1;
	}

	send(sock, msg, strlen(msg), 0);
	close(sock);
	freeaddrinfo(result);
	return 0;
}
