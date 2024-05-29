/*! \file ObservationGroup.h*/
//
// Created by tony on 30/05/23.
//

#ifndef INTELLISTREAM_INCLUDE_COMMON_OBSERVATIONGROUP_HPP_
#define INTELLISTREAM_INCLUDE_COMMON_OBSERVATIONGROUP_HPP_
#include <torch/torch.h>
#include <torch/script.h>
#include <Utils/IntelliLog.h>
#include <optional>
#include <filesystem>
namespace OBSERVAYION_GROUP {
/**
  * @class ObservationGroup Operator/LinearSVIOperator.h
  * @ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
  * @brief THe class of tensors, buffere management to keep observations
  */
class ObservationGroup {
 public:
  ObservationGroup() = default;
  ~ObservationGroup() = default;
  float finalObservation = 0.0;
  float scalingFactor = 0.0;
  uint64_t xCols, xRows = 0;
  bool buffFull = false;
  uint64_t fullCnt = 0;
  bool noNormalize = false;
  /**
   * @brief xTensor is the observation, yTensor is the label.
   */
  torch::Tensor xTensor, yTensor, tempXTensor;
  uint64_t bufferLen = 0, observationCnt = 0;
  void initObservationBuffer(uint64_t _bufferLen) {
    bufferLen = _bufferLen;
    xTensor = torch::zeros({1, (long) bufferLen});
    tempXTensor = torch::zeros({1, (long) bufferLen});
  }
  /**
   * @brief to append a new observation to tensor X
   * @param newX
   */
  void appendX(float newX) {
    if (observationCnt == bufferLen - 1) {
      buffFull = true;
      fullCnt++;
    } else {
      buffFull = false;
    }

    if (observationCnt >= bufferLen) {
      observationCnt = 0;
      xRows++;
    }
    xTensor[0][observationCnt] = newX;
    observationCnt++;
  }
  /**
   * @brief narrow down the xTensor and ignore all unused elements, and then reshape x to specific number of cols
   */
  void narrowAndReshapeX(uint64_t _xCols) {
    xCols = _xCols;
    //auto b=a.reshape({1,(long)(rows*cols)});
    uint64_t elementsSelected = observationCnt / xCols;
    elementsSelected = elementsSelected * xCols;
    auto b = xTensor.narrow(1, 0, elementsSelected);
    xRows = elementsSelected / xCols;
    b = b.reshape({(long) (elementsSelected / xCols), (long) xCols});
    xTensor = b;
  }
  /**
   * @brief To normalize the x and y tensor
   */
  void normalizeXY() {
    xTensor = xTensor / scalingFactor;
    yTensor = yTensor / scalingFactor;
  }
  /**
   * @brief Generate the Y tensor as labels
   * @param yLabel the label
   */
  void generateY(float yLabel) {
    yTensor = torch::ones({(long) (observationCnt / xCols), 1})
        * yLabel;
  }
  void setFinalObservation(float obs) {
    finalObservation = obs;

  }
  torch::Tensor tryTensor(std::string fileName) {
    if (std::filesystem::exists(fileName)) {
      try {
        torch::Tensor load_tensor;
        // Load the tensor from the file
        torch::load(load_tensor, fileName);
        // If we get here, the file was loaded successfully
        return load_tensor;

      } catch (const std::runtime_error &error) {
        // Handle the error
        //  std::cerr << "Error loading tensor: " << error.what() << std::endl;
        INTELLI_ERROR("the tensor can not be loaded");
        // Return an error code
        return torch::empty({1, 0});;
      }
    }
    INTELLI_WARNING("the tensor DOES NOT exist");
    return torch::empty({1, 0});;
  }
  torch::Tensor saveTensor2File(torch::Tensor ts, std::string ptName) {
    auto oldSelectivityTensorX = tryTensor(ptName);
    torch::Tensor ru;
    if (oldSelectivityTensorX.size(1) != 0) {
      ru = torch::cat({oldSelectivityTensorX, ts}, /*dim=*/0);
    } else {
      ru = ts;
    }
    torch::save({ru}, ptName);
    return ru;
  }
  float tryScalingFactor(std::string ptPrefix) {
    scalingFactor = 1.0;
    auto ts = tryTensor(ptPrefix + "_s.pt");
    if (ts.size(1) != 0) {
      scalingFactor = ts[0][0].item<float>();
    }
    INTELLI_INFO("scaling factor is read as " + to_string(scalingFactor));
    return scalingFactor;
  }
  void tryAndSaveScalingFactor(std::string ptPrefix) {
    auto ts = tryTensor(ptPrefix + "_s.pt");
    if (ts.size(1) != 0) {
      scalingFactor = ts[0][0].item<float>();
      INTELLI_INFO("READ scaling factor as " + to_string(scalingFactor));
    } else {
      // Get the size of tensor A
      //torch::IntArrayRef size = xTensor.sizes();
      int numElements = torch::numel(xTensor);
      if (noNormalize) {
        scalingFactor = 1.0;
      } else {
        float sum = torch::sum(xTensor).item<float>(); // Calculate the sum of all elements in the tensor
        scalingFactor = sum / numElements;
      }
      auto saveScaling = torch::ones({1, 1}) * scalingFactor;
      torch::save({saveScaling}, ptPrefix + "_s.pt");
      INTELLI_WARNING("create scaling factor " + to_string(scalingFactor));
    }

  }

  void saveXYTensors2Files(std::string ptPrefix, uint64_t _xCols) {
    /**
     * @brief 1. generate x
     */
    narrowAndReshapeX(_xCols);

    /**
     * @brief 2. generate y
     *
     */
    generateY(finalObservation);
    /**
     * @brief 3. normalize x and y
     */
    if (noNormalize) {
      scalingFactor = 1.0;
    }
    tryAndSaveScalingFactor(ptPrefix);

    normalizeXY();
    /***
     * @brief 4. sava x and y
     */
    auto tx = saveTensor2File(xTensor, ptPrefix + "_x.pt");

    uint64_t xRows = tx.size(0);
    uint64_t xCols = tx.size(1);
    INTELLI_INFO(
        "Now we have [" + to_string(xRows) + "x" + to_string(xCols) + "] at " + ptPrefix + "_x.pt");
    auto ty = saveTensor2File(yTensor, ptPrefix + "_y.pt");
    uint64_t yRows = ty.size(0);
    uint64_t yCols = ty.size(1);
    INTELLI_INFO(
        "Now we have [" + to_string(yRows) + "x" + to_string(yCols) + "] at " + ptPrefix + "_y.pt");
  }

};
}
#endif //INTELLISTREAM_INCLUDE_COMMON_OBSERVATIONGROUP_HPP_
