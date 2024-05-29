/*! \file LinearSVI.h*/
//Copyright (C) 2022 by the IntelliStream team (https://github.com/intellistream)
// Created by tony on 03/03/23.
//
#ifndef _SVI_LINEARSVI_HPP_
#define _SVI_LINEARSVI_HPP_
#pragma once
#include <torch/torch.h>
#include <iostream>
#include <torch/script.h>
#include <string>
#include <memory>
namespace TROCHPACK_SVI {
#define  newtorchMethod std::make_shared<torch::jit::Method>
typedef std::shared_ptr<torch::jit::Method> torchMethodPtr;
#define  newtorchOptimiser std::make_shared<torch::optim::Adam>
typedef std::shared_ptr<torch::optim::Adam> torchOptimiserPtr;
/**@ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
 * @{
* @defgroup TROCHPACK_SVI The SVI classes
* @{
 * This package covers SVI classes
*/
/**
 * @ingroup TROCHPACK_SVI
 * @class LinearSVI  LinearSVI.hpp
 * @brief The SVI Modeled under linear distortion
*/
class LinearSVI : public torch::nn::Module {
 private:
  /* data */
  uint64_t inputDimension, latentDimension;
  //torchMethodPtr getDimensionMethod,getMuEstimationMethod,loadPriorDistMethod;
  //torchMethodPtr lossUnderNormalMethod,lossUnderPretrain;
  void getDimension();
  void getMuEstimation();
  torchOptimiserPtr myOpt;
  int latent_dim_;
  torch::Tensor mu, irMu;
  //torch::Tensor logVar;
  torch::Tensor tau, irTau;
  torch::Tensor latentZ;

  //
 public:
  LinearSVI() {

  }
  ~LinearSVI() {

  }
  void setEvalMode() {
    //module.eval();
  }
  void setTrainMode() {
    //module.train();
  }
  torch::Tensor forwardMu(torch::Tensor data);
  /**
   *  @brief To reset the learning rate to lr
   * @param lr the learning rate
   */
  void resetLearningRate(double lr);
  /**
 * @brief To compute the -elbo given observation x and return
 * @return the elbo
 */
  torch::Tensor minusELBO(torch::Tensor &x);
  void updateEstimations();
  /**
   * @brief init the parameters and register them to be recognized by optimizer
   * @param latentD The dimension of latent variables, must be the same as that of X
   */
  void initSVIParameters(uint64_t latentD);
  /**
   *  @brief load the module from [path]
   * @param path the string to indicate the loaded path
  */
  void loadModule(std::string path);
  /**
  *  @brief store the module to [path]
  * @param path the string to indicate the loaded path
 */
  void storeModule(std::string path);
  /**
   * @brief run an NN forward on data
   * @param data the data stored in plain std vector, will be automatically converted to the suitable shape
   * @note please read the @ref resultMu and @ref resultSigma after this
  */
  void runForward(std::vector<float> data);
  /**
* @brief run an NN forward on data
* @param data the data stored in tensor
* @note please read the @ref resultMu and @ref resultSigma after this
*/
  void runForward(torch::Tensor data);
  float resultMu, resultSigma;
  /**
 * @brief load the prior distribution
 * @param pmu
 * @param psigma
*/
  void loadPriorDist(float pmu, float psigma);
  /**
  * @brief to perform one step learning on data
  * @param data the data stored in plain std vector, will be automatically converted to the suitable shape
 */
  void learnStep(std::vector<float> data);
  /**
   * @brief to perform one step learning on data
   * @param data the data stored as tensor, should check size outside
  */
  void learnStep(torch::Tensor data);
  /**
* @brief to perform one step pretrain on data
* @param data the data stored in plain std vector, will be automatically converted to the suitable shape
*/
  void pretrainStep(std::vector<float> data, std::vector<float> label);
  /**
   * @brief to perform one step pretrain on data
   * @param data the data stored as tensor, should check size outside
  */
  void pretrainStep(torch::Tensor data, torch::Tensor label);
  float resultLoss;
  /***
   * @brief return the value of inputDimension
   * @return copy of inputDimension
   */
  uint64_t getXDimension() { return inputDimension; }
};

/**
 *@}
 *@}
*/

}

#endif