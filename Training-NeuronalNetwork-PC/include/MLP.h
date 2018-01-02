//============================================================================
// Name : MLP.cpp
// Author : David Nogueira
//============================================================================
#ifndef MLP_H
#define MLP_H

#include "Layer.h"
#include "Sample.h"
#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

class MLP {
public:
  std::vector<Layer> m_layers;
  //desired call sintax :  MLP({64*64,20,4}, {"sigmoid", "linear"},
  MLP(const std::vector<uint64_t> & layers_nodes,
      const std::vector<std::string> & layers_activfuncs,
      bool use_constant_weight_init = false,
      double constant_weight_init = 0.5);
  MLP(std::string & filename);
  ~MLP();

  void SaveMLPNetwork(std::string & filename)const;
  void LoadMLPNetwork(std::string & filename);

  void GetOutput(const std::vector<double> &input,
                 std::vector<double> * output,
                 std::vector<std::vector<double>> * all_layers_activations = nullptr) const;
  void GetOutputClass(const std::vector<double> &output, size_t * class_id) const;

  void Train(const std::vector<TrainingSample> &training_sample_set_with_bias,
                       double learning_rate,
                       int max_iterations = 5000,
                       double min_error_cost = 0.001,
                       bool output_log = true);
protected:
  void UpdateWeights(const std::vector<std::vector<double>> & all_layers_activations,
                     const std::vector<double> &error,
                     double learning_rate);
private:
  void CreateMLP(const std::vector<uint64_t> & layers_nodes,
                 const std::vector<std::string> & layers_activfuncs,
                 bool use_constant_weight_init,
                 double constant_weight_init = 0.5);
  int m_num_inputs{ 0 };
  int m_num_outputs{ 0 };
  int m_num_hidden_layers{ 0 };
  std::vector<uint64_t> m_layers_nodes;
};

#endif //MLP_H
