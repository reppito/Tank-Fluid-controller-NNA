#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <Configuracion.h>
#include <ConjuntoEntrenamiento.h>
#include <iostream>
#include <LecturaSerial.h>
#include <fstream>
#include <MLP.h>

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;

void modoEntrenamiento();
void modoPrueba();

int main();

#endif // MAIN_H_INCLUDED
