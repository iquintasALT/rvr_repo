#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador: public Serializable
{
private:

  int16_t pos_x;
  int16_t pos_y;
  
  static const size_t MAX_NAME = 20;

  char name[MAX_NAME];


public:
    Jugador(const char * _n, int16_t _x, int16_t _y):pos_x(_x),pos_y(_y)
    {
        strncpy(name, _n, MAX_NAME);
    };

    virtual ~Jugador(){};

    //getters
    int16_t getPosX() const { return pos_x; }
    int16_t getPosY() const { return pos_y; }
    //rellenar info que va a ser mandada (binario)
    void to_bin()
    {
        _size = (sizeof(int16_t) * 2) + (sizeof(char) * MAX_NAME);
        alloc_data(_size);
        char* mydata = _data;
        memcpy(mydata, name, MAX_NAME * sizeof(char));
        mydata += MAX_NAME * sizeof(char);
        memcpy(mydata, &pos_x, sizeof(int16_t));
        mydata += sizeof(int16_t);
        memcpy(mydata, &pos_y, sizeof(int16_t));
    }

    int from_bin(char * data)
    {
        char* mydata = data;
        memcpy(name, mydata, MAX_NAME * sizeof(char));
        mydata += MAX_NAME * sizeof(char);
        memcpy(&pos_x, mydata, sizeof(int16_t));
        mydata += sizeof(int16_t);
        memcpy(&pos_y, mydata, sizeof(int16_t));

        return 0;
    }
};

int main(int argc, char **argv)
{
    //jugador que recibe info
    Jugador one_r("", 0, 0);
    //jugador del que se guarda info
    Jugador one_w("Player_ONE", 123, 987);

    // 1. Serializar el objeto one_w
    // 2. Escribir la serialización en un fichero
    // 3. Leer el fichero
    // 4. "Deserializar" en one_r
    // 5. Mostrar el contenido de one_r

    // 1. Guardamos representacion binaria
    one_w.to_bin();
    // 2. Guardamos esa informacion en un fichero con los permisos correspondientes (si no da problemas)
    int fd = open("./Player_ONE.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
    
    ssize_t bytes = write(fd, one_w.data(), one_w.size());
    if(bytes != one_w.size()){
        std::cout << "Error, los bytes no coinciden";
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

