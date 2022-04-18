#include <netdb.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

const size_t MESSAGE_MAX_SIZE = 100;

int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset((void *)&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;     //Devolver 0.0.0.0
    hints.ai_family =  AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    
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

    int b = bind(sd, (struct sockaddr *) result->ai_addr, result->ai_addrlen);
    if(b == -1){
        std::cerr << "Error asignando addr a socket\n";
		return -1;
    }

    freeaddrinfo(result);

    if(listen(sd, 1) == -1){
        std::cerr << "Error en listen()\n" << gai_strerror(sd) << std::endl;
        return -1;
    }

    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    struct sockaddr cliente;
    socklen_t cliente_len = sizeof(struct sockaddr);
    int cliente_sd = accept(sd, (struct sockaddr *)&cliente, &cliente_len);

    if(cliente_sd == -1)
    {
        std::cerr << "Error en la conexiñón TCP \n" << gai_strerror(sd) << std::endl;
        close(cliente_sd);
        close(sd);
        return -1;
    
    } 

    getnameinfo((struct sockaddr *)&cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    printf("Conexión desde Host:%s Puerto:%s\n",host, serv);

    bool exit = false;
    char buffer[MESSAGE_MAX_SIZE];
    while (!exit)
    {
        ssize_t bytesRecieved = recv(cliente_sd, (void *) buffer, (MESSAGE_MAX_SIZE -1 )* sizeof(char), 0);

        if( bytesRecieved <= 0 )
        {
            std::cout << "Conexion terminada\n";
            exit = true;
            continue;
        }

        send(cliente_sd, (void *) buffer, bytesRecieved, 0);
    }

    // Cerramos todo
    close(cliente_sd);
    close(sd);

    return 0;
}