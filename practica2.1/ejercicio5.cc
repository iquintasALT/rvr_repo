#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <unistd.h>

const size_t BUFFER_MAX = 100;

int main(int argc, char** argv){
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags    = AI_PASSIVE; //Devolver 0.0.0.0
    hints.ai_family   = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

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

    int server = connect(sd, (struct sockaddr *) result->ai_addr, result->ai_addrlen);

    if(server == -1){
        std::cerr << "Error en la conexion.\n";
		close(server);
        close(sd);
		return -1;
    }

	freeaddrinfo(result);

    char buffer[BUFFER_MAX];
    bool exit = false;
    int i = 0; 

    while(!exit){
        std::cin >> buffer;
        send(server, buffer, sizeof(buffer), 0);
        recv(server, buffer, sizeof(buffer), 0);

        if (buffer[0] == 'Q' && buffer[1] == '\0')
        {
            exit = true;
        }
        else{
            std::cout << buffer << "\n";
        }
    }

    close(server);
    close(sd);
    return 0;
}