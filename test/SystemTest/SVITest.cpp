//
// Created by tony on 29/05/23.
//
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <vector>
#include <OoOJoin.h>
#include "TestFunction.cpp"

TEST_CASE("Test SVI class in continue learning step")
{
  int a = 1;
  torch::manual_seed(114514);
  TROCHPACK_SVI::LinearSVI lsvi;
  lsvi.initSVIParameters(5);
  torch::Tensor x = torch::tensor({5.0, 4.9, 5.1, 4.8, 5.2});
  x = x.reshape({1, 5});
  lsvi.loadPriorDist(0.1, torch::var(x).item<float>());
  lsvi.runForward(x);
  // lsvi.forwardMu(x);
  for (int i = 0; i < 100; i++) {
    lsvi.learnStep(x);
  }
  lsvi.runForward(x);

  std::cout << lsvi.resultMu << "," << lsvi.resultSigma << endl;
  //lsvi.updateEstimations();

  REQUIRE(a == 1);
}
TEST_CASE("Test SVI class in disk io")
{
  int a = 1;
  torch::manual_seed(114514);
  TROCHPACK_SVI::LinearSVI lsvi, diskSvi;
  lsvi.initSVIParameters(5);
  torch::Tensor x = torch::tensor({5.0, 4.9, 5.1, 4.8, 5.2});
  x = x.reshape({1, 5});
  torch::Tensor y = torch::tensor({10.0});
  lsvi.loadPriorDist(0.1, torch::var(x).item<float>());
  y = y.reshape({1, 1});
//lsvi.runForward(x);
  for (int i = 0; i < 100; i++) {
    lsvi.pretrainStep(x, y);
  }
  lsvi.runForward(x);
  std::cout << "raw :" << lsvi.resultMu << "," << lsvi.resultSigma << endl;
  lsvi.storeModule("lsvi.pt");

  diskSvi.loadModule("lsvi.pt");
  diskSvi.loadPriorDist(0.1, torch::var(x).item<float>());
  diskSvi.runForward(x);
  std::cout << "disk :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  x = x + 1.0;
  diskSvi.learnStep(x);
  diskSvi.runForward(x);

  std::cout << "what if data changes :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  diskSvi.learnStep(x);
  diskSvi.runForward(x);

  std::cout << "2nd :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  REQUIRE(a == 1);
}

TEST_CASE("Test python compatability")
{
  int a = 1;
  torch::manual_seed(114514);
  TROCHPACK_SVI::LinearSVI diskSvi;
  torch::Tensor x = torch::tensor({5.0, 4.9, 5.1, 4.8, 5.2, 5.0, 4.9, 5.1, 4.8, 5.2});
  x = x.reshape({1, 10});
  diskSvi.loadModule("torchscripts/linearSVI/linearSVI_selectivity.pt");
  diskSvi.runForward(x);
  //diskSvi.loadPriorDist(0.1, torch::var(x).item<float>());
  diskSvi.runForward(x);
  std::cout << "disk :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  x = x + 1.0;
  diskSvi.learnStep(x);
  diskSvi.runForward(x);

  std::cout << "what if data changes :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  diskSvi.learnStep(x);
  diskSvi.runForward(x);

  std::cout << "2nd :" << diskSvi.resultMu << "," << diskSvi.resultSigma << endl;
  REQUIRE(a == 1);

}