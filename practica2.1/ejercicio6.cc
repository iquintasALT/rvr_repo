#include <netdb.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>

#define MAXTHREAD 5

const size_t BUFFER_MAX = 100;

class MessageThread{
    private:
        int sd, id;
    public:
    MessageThread(int sd_, int id_) : sd(sd_), id(id_){}
    void createMessage(){
		time_t tiempo;
		size_t tam;
		char buffer[BUFFER_MAX];
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];
		
		struct sockaddr cliente;	
		socklen_t cliente_len = sizeof(struct sockaddr);

		while(true){
			 sleep(3);

			int bytes = recvfrom(sd, (void *) buffer, BUFFER_MAX - 1,
				'\0', &cliente, &cliente_len);
			
			if (bytes == -1) {
                std::cerr << "Error en la recepcion de bytes\n";
                close(sd);
                return;
		}
	
			buffer[bytes] = '\0';
	
			getnameinfo(&cliente, cliente_len, host, NI_MAXHOST,
					serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	
			std::cout << bytes << " bytes de " << host << ":" << serv << "\n";
			std::cout << "Thread id: " << std::this_thread::get_id() << "\n";
	
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
        else printf("Comando no soportado %s", buffer);
		}
	}
};


int main(int argc, char** argv){
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags    = AI_PASSIVE; //Devolver 0.0.0.0
    hints.ai_family   = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;

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

     for(int i = 0; i < MAXTHREAD; i++){
        MessageThread *message = new MessageThread(sd, i);
        std::thread([&message]() {message->createMessage(); delete message; }).detach();
	 }

	std::string q = "_";
    while (q!="Q")
    {
        std::cin >> q;
    }

    close(sd);
    return 0;
}