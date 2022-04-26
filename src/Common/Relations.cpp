#include <Common/Relations.hpp>
std::shared_ptr<INTELLI::Relations> INTELLI::Relations::create() {
  return std::make_shared<INTELLI::Relations>();
}