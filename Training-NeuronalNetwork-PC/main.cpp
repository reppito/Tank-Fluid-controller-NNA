#include "main.h"

//#define MODO_ARCHIVO
//#define MODO_ENTRENAMIENTO

void modoEntrenamiento()
{
    Configuracion conf;
    LecturaSerial sp(&conf.obtenerNombrePuerto()[0]);

    #ifndef MODO_ARCHIVO
    if (sp.estaConectado()) {
        sp.leer();
        sp.obtenerParser()->leerElementos();
        sp.obtenerParser()->almacenarArchivo();
        #else
        sp.obtenerParser()->cargarArchivo();
        #endif // MODO_ARCHIVO

        auto elementosLeidos = *sp.obtenerParser()->obtenerElementosLeidos();
        unsigned i = 0;

        for (auto& elementoLeido : elementosLeidos) {
            i += 1;
            cout << "Elemento #" << i << endl;
            cout << "nivelAgua: " << elementoLeido->nivelAgua << endl;
            cout << "variacionNivel: " << -elementoLeido->variacionNivel << endl;
            cout << "proporcionAbertura: " << elementoLeido->proporcionAbertura << endl;
            cout << endl;
        }

        ConjuntoEntrenamiento ce(sp.obtenerParser()->obtenerElementosLeidos());

        for (auto& tSample : *ce.obtenerTrainingSamples()) {
            tSample.AddBiasValue(1);
        }

        auto numEntradas = ce.obtenerTrainingSamples()->at(0).GetInputVectorSize();
        auto numSalidas = ce.obtenerTrainingSamples()->at(0).GetOutputVectorSize();

        cout << "numEntradas: " << numEntradas << endl;
        cout << "numSalidas: " << numSalidas << endl;

        MLP mlp({ numEntradas, 5,  numSalidas}, { "sigmoid", "linear" });

        cout << "Entrenando..." << endl;
        mlp.Train(*ce.obtenerTrainingSamples(), 0.03, 1000, 0.05);
        cout << "Entrenamiento finalizado." << endl;

        auto capaOculta = mlp.m_layers.at(0);

        ofstream archivo("resultados.txt", ios::trunc);

        if (archivo) {
            cout << "Almacenando resultados..." << endl;
            i = 0;
            for (auto& ts : *ce.obtenerTrainingSamples()) {
                i += 1;
                archivo << "Prueba #" << i << endl;
                archivo << "Nivel de agua: " << ts.input_vector().at(1) << endl;
                archivo << "Variacion de nivel: " << ts.input_vector().at(2) << endl;
                archivo << "P. esperada: " << ts.output_vector().at(0) << endl;

                vector<double> out;
                mlp.GetOutput(ts.input_vector(), &out);

                archivo << "P. obtenida: " << out.at(0) << endl;
                archivo << endl;
            }
            cout << "Resultados almacenados." << endl;
        }
        else {
            cerr << "No fue posible leer lacava.txt." << endl;
        }
        string nombreArchivo = "pesos.mlp";

        mlp.SaveMLPNetwork(nombreArchivo);
    #ifndef MODO_ARCHIVO
    }
    #endif // MODO_ARCHIVO
}

void modoPrueba()
{
    Configuracion conf;
    LecturaSerial ls(&conf.obtenerNombrePuerto()[0]);

    MLP mlp({ 3, 20, 1}, { "sigmoid", "linear" });
    string nombreArchivo = "pesos.mlp";
    mlp.LoadMLPNetwork(nombreArchivo);

    if (ls.estaConectado()) {
        cout << "Inicio de recepcion de datos:" << endl;
        while (true) {
            vector<double> in = { 1, 0, 0 };
            ls.leerDatosPrueba(&in.at(1), &in.at(2));
            cout << "nivelAgua: " << in.at(1) << " variacionNivel: " << in.at(2);
            vector<double> out = {0};

            if (in.at(2) > 0) {
                mlp.GetOutput(in, &out);
            }
            else if (in.at(2) <= 0) {
                out[0] = 0;
            }

            if (out[0] < 0) {
                out[0] = 0;
            }

            if (out[0] > 1.0) {
                out[0] = 1.0;
            }

            cout << " proporcionAbertura: " << out.at(0) << endl;
            ls.enviarProporcionAbertura(out.at(0));
        }
    }

}

int main()
{
    #ifdef MODO_ENTRENAMIENTO
    modoEntrenamiento();
    #else
    modoPrueba();
    #endif // MODO_ENTRENAMIENTO
    return 0;
}
