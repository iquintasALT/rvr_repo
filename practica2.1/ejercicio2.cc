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

    struct addrinfo *tiempo;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &tiempo);
    if(rc != 0) {
        std::cerr << "Error getaddrinfo " << gai_strerror(rc) << std::endl;
        return -1;
    }

    int sd = socket(tiempo->ai_family, tiempo->ai_socktype, 0);
    if (sd == -1) {
		std::cerr << "Error, socket no creado\n";
		return -1;
	}

    int b = bind(sd, (struct sockaddr *) tiempo->ai_addr, tiempo->ai_addrlen);
    if(b == -1){
        std::cerr << "Error asignando addr a socket\n";
		return -1;
    }

    freeaddrinfo(tiempo);

    bool exit = false;
    char buffer[BUFFER_MAX];
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    while(!exit){
        struct sockaddr_in cliente;
        socklen_t cliente_len = sizeof(struct sockaddr_in);

        int bytes = recvfrom(sd, buffer, BUFFER_MAX - 1, 0, (struct sockaddr *)&cliente, &cliente_len);

        if (bytes == -1) {
			std::cerr << "Error en la recepcion de bytes\n";
			close(sd);
            return -1;
		}
        buffer[bytes] = '\0';

        getnameinfo((struct sockaddr *) &cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        
        printf("Conexión desde Host:%s Puerto:%s\n",host, serv);
        printf("\tMensaje(%i): %s\n", bytes, buffer);

        time_t tiempo;
        char hora[12];

        if(buffer[0] == 't' && buffer[1] == '\n') {
            time(&tiempo);
            tm* time = localtime(&tiempo);
            size_t tam = strftime(hora, sizeof(hora), "%r", time); // "%r" es equivalente a "%I:%M:%S %p"
            hora[tam] = '\n';
            sendto(sd, hora, sizeof(hora), 0, (struct sockaddr *) &cliente, cliente_len);
        }
        else if(buffer[0] == 'd' && buffer[1] == '\n'){
            time(&tiempo);
            tm* time = localtime(&tiempo);
            size_t tam = strftime(hora, sizeof(hora), "%F", time); // "%F" es equivalente a "%Y:%m:%d"
 
            hora[tam] = '\n';
            sendto(sd, hora, sizeof(hora), 0, (struct sockaddr *) &cliente, cliente_len);
        }

        else if(buffer[0] == 'q' && buffer[1] == '\n'){
            printf("Saliendo...\n");
            exit = true;
        }
        else printf("Comando no soportado %s", buffer);
    }

    return 0;
}