cmake_minimum_required(VERSION 3.9)
project(AllianceDB)

set(CMAKE_CXX_STANDARD 11)

add_executable(AllianceDB main.cpp joins/SymmetricHashJoin.cpp joins/SymmetricHashJoin.h
        storage/DbIterator.cpp storage/DbIterator.h storage/Tuple.cpp storage/Tuple.h storage/TupleDesc.cpp storage/TupleDesc.h storage/TupleIterator.cpp storage/TupleIterator.h)