#ifndef CONJUNTOENTRENAMIENTO_H
#define CONJUNTOENTRENAMIENTO_H

#include <ElementoEntrenamiento.h>
#include <Sample.h>
#include <vector>

using std::vector;

class ConjuntoEntrenamiento
{
private:
    vector<TrainingSample> trainingSamples;

public:
    ConjuntoEntrenamiento(vector<ElementoEntrenamiento*>* ee);
    vector<TrainingSample>* obtenerTrainingSamples();
};

#endif // CONJUNTOENTRENAMIENTO_H
