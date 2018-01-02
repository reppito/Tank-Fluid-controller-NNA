#ifndef PARSERJSON_H
#define PARSERJSON_H

#include <ElementoEntrenamiento.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>
#include <vector>

using std::ios;
using std::endl;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using json = nlohmann::json;
using std::ofstream;
using std::string;
using std::vector;

class ParserJson
{
private:
    string json;
    vector<ElementoEntrenamiento*> elementosLeidos;
    void limpiarElementos();
    static const string NOMBRE_ARCHIVO;

public:
    ~ParserJson();
    void agregarCaracter(char c);
    void limpiar();
    void leerElementos();
    vector<ElementoEntrenamiento*>* obtenerElementosLeidos();
    void almacenarArchivo();
    void cargarArchivo();
    void leerElemento(double* nivelAgua, double* variacionNivel);
};

#endif // PARSERJSON_H
