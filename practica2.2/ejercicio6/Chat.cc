#include "Chat.h"
#include <memory.h>
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* myData = _data;
    memcpy(myData, &type, sizeof(uint8_t));
    myData += sizeof(uint8_t);
    memcpy(myData, nick.c_str(), sizeof(char) * 8);
    myData += sizeof(char) * 8;
    memcpy(myData, message.c_str(), sizeof(char) * 80);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    char* myData = _data;
    memcpy(&type, myData, sizeof(uint8_t));
    myData += sizeof(uint8_t);
    nick = myData;
    myData += sizeof(char) * 8;
    message = myData;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)



        ChatMessage msg;
        Socket *s;
        int sd = socket.recv(msg, s);
        if(sd == -1){
            std::cerr << "Error en socket.recv()\n";
            return;
        }

        switch(msg.type) {
	    case ChatMessage::LOGIN:
        {
	        std::cout << "Login de jugador:  " << msg.nick << "\n";
	        clients.push_back(std::move(std::make_unique<Socket>(*s)));
            break;
        }
	    case ChatMessage::LOGOUT:
        {
            auto it = clients.begin();
	        while (it != clients.end() && (**it != *s))
		        ++it;

	        if (it == clients.end())
		        std::cout << "Error, logout de jugador que no estaba conectado\n";
	        else {
		        std::cout << "Logout de jugador:  " << msg.nick << "\n";
                clients.erase(it);
		        Socket *delSock = (*it).release();
		        delete delSock; 
	        }
	        break;
        }
	    case ChatMessage::MESSAGE:
        {
	        for (auto it = clients.begin(); it != clients.end(); ++it) {
		    if (**it !=  *s)
		        s->send(msg, **it);
	        }
	        break;
        }
	    default:
	        std::cerr << "Error, tipo de mensaje no reconocido\n";
            break;
	    }

    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}

