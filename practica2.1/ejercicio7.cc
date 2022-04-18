#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>

const size_t BUFFER_MAX = 80;
#define MAXLISTEN 3

std::condition_variable numCv;
std::mutex numMutex;
int numListen = 0;
const size_t MESSAGE_MAX_SIZE = 100;

class MyThread {
    
private:
	int clientesd_;
	int id_;
public:
	MyThread(int clientesd, int id) : clientesd_(clientesd), id_(id) {};
	
	void connections() {
		bool activo = true;

		while(activo) {
			char buffer[BUFFER_MAX];

	                int bytes = recv(clientesd_, (void *) buffer, BUFFER_MAX - 1, '\0');
			if (bytes < 1) {
				std::cout << "Conexión terminada\n";
				activo = false;
			}
			buffer[bytes] = '\0';

			send(clientesd_, (void *) buffer, bytes, 0);
		}

		close(clientesd_);

		numMutex.lock();
		numListen--;
		
		if(numListen < MAXLISTEN){
			numCv.notify_all();
		}

		numMutex.unlock();
	}
};


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

    while(true) {
		struct sockaddr cliente;
		socklen_t cliente_len = sizeof(struct sockaddr);

		{
			std::unique_lock<std::mutex> lck(numMutex);
		        while(numListen >= MAXLISTEN){
				std::cout << "Error, el servidor está lleno...\n";
				numCv.wait(lck);
			}
		}

		int clientesd = accept(sd, (struct sockaddr *) &cliente, &cliente_len);
		if (clientesd == -1) {
	                std::cerr << "Error en el accept\n";
			close(sd);
	                return -1;
		}

		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		getnameinfo((struct sockaddr *) &cliente, cliente_len, host, NI_MAXHOST,
			serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
		std::cout << "Conexión desde " << host << " " << serv << "\n";

		numMutex.lock();
		numListen++;
		numMutex.unlock();

		MyThread* thrd = new MyThread(clientesd, numListen);
		std::thread([&thrd]() { thrd->connections(); delete thrd; }).detach();
	}

	close(sd);
	return 0;
}