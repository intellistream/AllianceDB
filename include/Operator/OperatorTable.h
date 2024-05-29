/*! \file OperatorTable.h*/
/**
 * @note If new Operator is added, please change this file and its .cpp
 */
//
// Created by tony on 06/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_OPERATORTABLE_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_OPERATORTABLE_H_

#include <Operator/AbstractOperator.h>
#include <map>
#include <utility>

namespace OoOJoin {
/**
 * @class OperatorTable Operator/OperatorTable.h
 * @ingroup ADB_OPERATORS
 * @brief The entity of operatorTable, can used for searching all available operators
 * @note default tags:
 * - "IAWJ": @ref IAWJOperatorPtr (class @ref IAWJOperator)
 * - "MeanAQP": @ref MeanAQPIAWJOperatorPtr (class @ref MeanAQPIAWJOperator)
 * - "IMA": @ref IMAIAWJOperatorPtr (class @ref IMAIAWJOperator)
 */
class OperatorTable {
 protected:
  std::map<std::string, AbstractOperatorPtr> operatorMap;
 public:
  /**
   * @brief The constructing function
   * @note  If new Operator is added, please change this file and its .cpp
   */
  OperatorTable();

  ~OperatorTable() = default;

  /**
 * @brief To register a new operator
 * @param onew The new operator
 * @param tag THe name tag
 */
  void registerNewOperator(AbstractOperatorPtr onew, const std::string &tag) {
    operatorMap[tag] = std::move(onew);
  }

  /**
   * @brief find an operator in the table according to its name
   * @param name The name of operator
   * @return The operator, nullptr if not found
   */
  AbstractOperatorPtr findOperator(const std::string &name) {
    if (operatorMap.count(name)) {
      return operatorMap[name];
    }
    return nullptr;
  }
};

/**
 * @ingroup ADB_OPERATORS
 * @typedef OperatorTablePtr
 * @brief The class to describe a shared pointer to @ref OperatorTable
 */
typedef std::shared_ptr<class OperatorTable> OperatorTablePtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newOperatorTable
 * @brief (Macro) To creat a new @ref OperatorTable under shared pointer.
 */
#define newOperatorTable std::make_shared<OoOJoin::OperatorTable>
} // OoOJoin

#endif //INTELLISTREAM_INCLUDE_OPERATOR_OPERATORTABLE_H_
