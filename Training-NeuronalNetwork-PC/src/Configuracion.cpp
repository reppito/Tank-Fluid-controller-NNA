#include "Configuracion.h"

const string Configuracion::NOMBRE_ARCHIVO = "configuracion.txt";

Configuracion::Configuracion() : nombrePuerto("COM3")
{
    ifstream archivo(NOMBRE_ARCHIVO);

    if (archivo) {
        char buffer[32];

        cout << "Archivo de configuracion leido." << endl;
        archivo.seekg(0L, ios::beg);
        archivo.getline(buffer, sizeof buffer);

        nombrePuerto = string(buffer);
    }
    else {
        if (!existeArchivo()) {
            almacenarNombrePuerto();
            cout << "Archivo de configuracion creado." << endl;
        }
        else {
            cerr << "No fue posible abrir el archivo de configuracion." << endl;
        }
    }
}

bool Configuracion::existeArchivo() const
{
    struct stat buffer;
    return stat(NOMBRE_ARCHIVO.c_str(), &buffer) == 0;
}

void Configuracion::almacenarNombrePuerto() const
{
    ofstream archivo(NOMBRE_ARCHIVO, ios::trunc);

    if (archivo) {
        archivo.seekp(0L, ios::beg);
        archivo << nombrePuerto;
    }
    else {
        cerr << "No fue posible escribir sobre el archivo de configuracion." << endl;
    }
}

void Configuracion::fijarNombrePuerto(string nombrePuerto)
{
    this->nombrePuerto = nombrePuerto;
    almacenarNombrePuerto();
}

string Configuracion::obtenerNombrePuerto() const
{
    return nombrePuerto;
}
