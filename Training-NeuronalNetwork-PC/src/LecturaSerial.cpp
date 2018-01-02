#include "LecturaSerial.h"

const short LecturaSerial::TIEMPO_ESPERA_ARDUINO = 2000;

LecturaSerial::LecturaSerial(string nombrePuerto)
    : handle(CreateFileA(static_cast<LPCSTR>(&nombrePuerto[0]), GENERIC_READ | GENERIC_WRITE
        , 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
{
    if (!estaConectado()) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            cerr << "El puerto " << nombrePuerto << " no se encuentra disponible." << endl;
        }
        else {
            cerr << "No fue posible conectarse al puerto " << nombrePuerto << "." << endl;
        }
    }
    else {
        DCB parametrosPuerto;

        if (!GetCommState(handle, &parametrosPuerto)) {
            cerr << "No fue posible obtener los parametros del puerto " << nombrePuerto << "." << endl;
        }
        else {
            parametrosPuerto.BaudRate = CBR_9600;
            parametrosPuerto.ByteSize = 8;
            parametrosPuerto.StopBits = ONESTOPBIT;
            parametrosPuerto.Parity = NOPARITY;
            parametrosPuerto.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(handle, &parametrosPuerto)) {
                cerr << "No fue posible establecer los parametros del puerto " << nombrePuerto << "." << endl;
            }

            else {
                cout << "Conectado al puerto " << nombrePuerto << "." << endl;
                PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(TIEMPO_ESPERA_ARDUINO);
            }
        }
    }
}

LecturaSerial::~LecturaSerial()
{
    if (estaConectado()) {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }
}

bool LecturaSerial::estaConectado() const
{
    return handle != INVALID_HANDLE_VALUE;
}

void LecturaSerial::leer()
{
    DWORD bytesLeidos, errores;
    COMSTAT estado;
    char caracterLeido;
    bool banderaInicio = false;

    do {
        do {
            ClearCommError(handle, &errores, &estado);
        }
        while (estado.cbInQue == 0) ;

        if (!ReadFile(handle, &caracterLeido, sizeof caracterLeido, &bytesLeidos, NULL)) {
            cerr << "Ocurrio un error al intentar leer un caracter." << endl;
        }
        else {
            if (banderaInicio) {
                parser.agregarCaracter(caracterLeido);
            }
            else if (caracterLeido == '[') {
                parser.agregarCaracter(caracterLeido);
                banderaInicio = true;
            }
        }
    }
    while (caracterLeido != ']');
}

void LecturaSerial::leerDatosPrueba(double* nivelAgua, double* variacionNivel)
{
    DWORD bytesLeidos, errores;
    COMSTAT estado;
    char caracterLeido;

    do {
        do {
            ClearCommError(handle, &errores, &estado);
        }
        while (estado.cbInQue == 0) ;

        if (!ReadFile(handle, &caracterLeido, sizeof caracterLeido, &bytesLeidos, NULL)) {
            cerr << "Ocurrio un error al intentar leer un caracter." << endl;
        }
        else {
            parser.agregarCaracter(caracterLeido);
        }
    }
    while (caracterLeido != '}');

    parser.leerElemento(nivelAgua, variacionNivel);
    parser.limpiar();
}

void LecturaSerial::enviarProporcionAbertura(double proporcionCalculada)
{
    DWORD bytesEscritos, errores;
    COMSTAT estado;

    char buffer[8];
    snprintf(buffer, sizeof buffer, "%.04f ", proporcionCalculada);

    if (WriteFile(handle, buffer, (sizeof buffer) - 1, &bytesEscritos, 0)) {
        ClearCommError(handle, &errores, &estado);
    }
    else {
        cerr << "Ocurrio un error al intentar escribir." << endl;
    }
    cout << "Fin funcion miembro." << endl;
}

ParserJson* LecturaSerial::obtenerParser()
{
    return &parser;
}
