#include "ParserJson.h"

const string ParserJson::NOMBRE_ARCHIVO = "conjunto_entrenamiento.txt";

ParserJson::~ParserJson()
{
    limpiarElementos();
}

void ParserJson::agregarCaracter(char c)
{
    if (c == '{' && json.find("{", 0) != string::npos) {
       json += ",{";
    }
    else {
       json += c;
    }
}

void ParserJson::limpiar()
{
    json.clear();
}

void ParserJson::leerElemento(double* nivelAgua, double* variacionNivel)
{
    auto e = json::parse(json);
    *nivelAgua = e["nivelAgua"];
    *variacionNivel = e["variacionNivel"];
}

void ParserJson::leerElementos()
{
    auto parsedJson = json::parse(json);

    for (auto& e : parsedJson) {
        ElementoEntrenamiento* nuevoElemento = new ElementoEntrenamiento;
        nuevoElemento->nivelAgua = e["nivelAgua"];
        nuevoElemento->variacionNivel = e["variacionNivel"];
        nuevoElemento->proporcionAbertura = e["proporcionAbertura"];
        elementosLeidos.push_back(nuevoElemento);
    }
}

vector<ElementoEntrenamiento*>* ParserJson::obtenerElementosLeidos()
{
    return &elementosLeidos;
}

void ParserJson::limpiarElementos()
{
    for (auto& e : elementosLeidos) {
        delete e;
    }
    elementosLeidos.clear();
}

void ParserJson::almacenarArchivo()
{
    ofstream archivo(NOMBRE_ARCHIVO, ios::trunc);

    if (archivo) {
        archivo.seekp(0L, ios::beg);
        for (auto& e : elementosLeidos) {
            archivo << e->nivelAgua << " " << e->variacionNivel<< " " << e->proporcionAbertura << endl;
        }
        cout << "Archivo almacenado correctamente." << endl;
    }
    else {
        cerr << "No se logro abrir el archivo " << NOMBRE_ARCHIVO << "." << endl;
    }
}

void ParserJson::cargarArchivo()
{
    ifstream archivo(NOMBRE_ARCHIVO);

    if (archivo) {
        archivo.seekg(0L, ios::beg);
        limpiarElementos();

        double h, dh, pa;

        while (archivo >> h >> dh >> pa) {
            ElementoEntrenamiento* ee = new ElementoEntrenamiento;
            ee->nivelAgua = h;
            ee->variacionNivel = dh;
            ee->proporcionAbertura = pa;
            elementosLeidos.push_back(ee);
        }

        cout << "Archivo cargado correctamente." << endl;
    }
    else {
        cerr << "No se logro abrir el archivo " << NOMBRE_ARCHIVO << "." << endl;
    }
}
