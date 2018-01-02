#include "ConjuntoEntrenamiento.h"

ConjuntoEntrenamiento::ConjuntoEntrenamiento(vector<ElementoEntrenamiento*>* ee)
{
    for (auto& elemento : *ee) {
        vector<double> inputVector = { elemento->nivelAgua, -elemento->variacionNivel };
        vector<double> outputVector = { elemento->proporcionAbertura };
        TrainingSample tSample(inputVector, outputVector);
        trainingSamples.push_back(tSample);
    }
}

vector<TrainingSample>* ConjuntoEntrenamiento::obtenerTrainingSamples()
{
    return &trainingSamples;
}
