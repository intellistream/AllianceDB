//
// Created by 86183 on 2023/4/18.
//

#include <limits>
#include <climits>
#include "Operator/PECJ/VariationalInference.h"

auto VariationalInference::getE(std::vector<double> pX) -> double {
  double E = 0.0;
  for (int i = 0; i < pX.size(); i++) {
    E += pX[i] * i;
  }
  return E;
}

auto VariationalInference::cavi(u_int64_t key) -> void {
  double elbo_prev = -std::numeric_limits<double>::infinity();
  double elbo_curr;
  double elbo_diff;
//
//    double tolerance = 1e-6;

  // Update local variational parameters phi
  for (auto &it : Z[key]) {
    //indicate it is a validate random variable
    if (it != INT_MIN) {
      pZ[key][it] = 0;
    }
  }

  // Update global variational parameters U
  for (auto &it : U[key]) {
    //indicate it is a validate random variable
    if (it != INT_MIN) {
      pU[key][it] = 0;
    }
  }

  // Compute ELBO
//    elbo_curr = compute_elbo(/* Necessary parameters */);

  // Check for convergence
  elbo_diff = std::abs(elbo_curr - elbo_prev);
//    if (elbo_diff < tolerance) {
//        break;
//    }

  elbo_prev = elbo_curr;
}

auto VariationalInference::getSigma(std::vector<double> pX) -> double {
  double mean = 0, sigma = 0;
  int n = pX.size();
  // avg
  for (double x : pX) {
    mean += x;
  }
  mean /= n;

  //sigma
  for (double x : pX) {
    sigma += (x - mean) * (x - mean);
  }
  sigma /= n;

  return sigma;
}
