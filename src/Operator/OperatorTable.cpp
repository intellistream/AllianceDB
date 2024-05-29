//
// Created by tony on 06/12/22.
//

#include <Operator/OperatorTable.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include <Operator/IMAIAWJOperator.h>
#include <Operator/MSWJOperator.h>
#include <Operator/IAWJOperator.h>
#include <Operator/AIOperator.h>
#include <Operator/LinearSVIOperator.h>
#include <Operator/IAWJSelOperator.h>
#include <Operator/LazyIAWJSelOperator.h>
#include <Operator/RawPRJOperator.h>
#include <Operator/RawSHJOperator.h>
namespace OoOJoin {
OperatorTable::OperatorTable() {
  operatorMap["IAWJ"] = newIAWJOperator();
  operatorMap["MeanAQP"] = newMeanAQPIAWJOperator();
  operatorMap["IMA"] = newIMAIAWJOperator();
  operatorMap["MSWJ"] = newMSWJOperator();
  operatorMap["AI"] = newAIOperator();
  operatorMap["LinearSVI"] = newLinearSVIOperator();
  operatorMap["IAWJSel"] = newIAWJSelOperator();
  operatorMap["LazyIAWJSel"] = newLazyIAWJSelOperator();
  operatorMap["SHJ"] = newRawSHJOperator();
  operatorMap["PRJ"] = newRawPRJOperator();
}
} // OoOJoin