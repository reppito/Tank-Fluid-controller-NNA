#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;

class Configuracion
{
private:
    static const string NOMBRE_ARCHIVO;
    string nombrePuerto;
    bool existeArchivo() const;
    void almacenarNombrePuerto() const;

public:
    Configuracion();
    void fijarNombrePuerto(string nombrePuerto);
    string obtenerNombrePuerto() const;
};

#endif // CONFIGURACION_H
