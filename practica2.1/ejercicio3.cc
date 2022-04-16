#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
const size_t BUFFER_MAX = 80;

int main(int argc, char **argv){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE; // Devolver 0.0.0.0
    hints.ai_family = AF_UNSPEC; // Coge direcciones de cualquier familia (ipv4, ipv6...)
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *result;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);
    if(rc != 0) {
        std::cerr << "Error getaddrinfo " << gai_strerror(rc) << std::endl;
        return -1;
    }

    int sd = socket(result->ai_family, result->ai_socktype, 0);
    if (sd == -1) {
		std::cerr << "Error, socket no creado\n";
		return -1;
	}

    char buffer[BUFFER_MAX];

    if (sendto(sd, argv[3], strlen(argv[3]) + 1, 0, result->ai_addr, result->ai_addrlen) == -1) {
		std::cerr << "Error en sendTo()\n";
		close(sd);
		return -1;
	}

    
	if (recvfrom(sd, buffer, BUFFER_MAX - 1, 0, result->ai_addr, &result->ai_addrlen) == -1) {
		std::cerr << "Error en recvfrom()\n";      
		close(sd);
        return -1;
        }

	std::cout << buffer << "\n";

    freeaddrinfo(result);
    close(sd);
    return 0;
}