cmake_minimum_required(VERSION 3.9)
project(AllianceDB)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

add_executable(AllianceDB main.cpp joins/SymmetricHashJoin.cpp joins/SymmetricHashJoin.h
        storage/DbIterator.cpp storage/DbIterator.h storage/Tuple.cpp storage/Tuple.h storage/TupleDesc.cpp storage/TupleDesc.h storage/TupleIterator.cpp storage/TupleIterator.h Utility.cpp Utility.h storage/type/Type.cpp storage/type/Type.h storage/Field.cpp storage/Field.h storage/IntField.cpp storage/IntField.h storage/type/INT_TYPE.cpp storage/type/INT_TYPE.h)