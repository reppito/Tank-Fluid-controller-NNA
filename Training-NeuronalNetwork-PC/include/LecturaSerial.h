#ifndef LECTURASERIAL_H
#define LECTURASERIAL_H

#include <cstdio>
#include <ElementoEntrenamiento.h>
#include <iostream>
#include <ParserJson.h>
#include <string>
#include <windows.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

class LecturaSerial
{
private:
    HANDLE handle;
    static const short TIEMPO_ESPERA_ARDUINO;
    ParserJson parser;

public:
    LecturaSerial(string nombrePuerto);
    ~LecturaSerial();
    bool estaConectado() const;
    void leer();
    ParserJson* obtenerParser();
    void leerDatosPrueba(double* nivelAgua, double* variacionNivel);
    void enviarProporcionAbertura(double proporcionCalculada);
};

#endif // LECTURASERIAL_H
