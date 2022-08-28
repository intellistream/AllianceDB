//
// Created by tony on 02/03/22.
//
#include <Common/DatasetTool.hpp>
#include <iostream>
#include <fstream>
#include <list>
#include <mutex>
#include <unordered_map>
#include <Common/Types.hpp>
#include <Utils/Logger.hpp>
using namespace AllianceDB;
void DatasetTool::store3VText(TuplePtrQueue &relation, const std::string &fileName) {
  std::fstream file;
  std::string buffer;
  file.open(fileName, std::ios::out);
  if (!file.is_open()) INTELLI_ERROR("cannot open the file: " + fileName);
  size_t i = 0;
  size_t allLen = relation->size();
  for (i = 0; i < allLen; i++) {
    TuplePtr tp = relation->front()[i];
    buffer = std::to_string(tp->key) + "," + std::to_string(tp->payload) + "," + std::to_string(tp->subKey);
    file << buffer << endl;
  }
  file.close();
}
void DatasetTool::load3VText(TuplePtrQueueIn &relation, const std::string &fileName) {
  std::fstream file;
  std::string buffer;
  file.open(fileName, std::ios::in);
  if (!file.is_open()) INTELLI_ERROR("cannot open the file: " + fileName);
  while (getline(file, buffer)) {
    keyType key;
    valueType v;
    size_t ts;
    sscanf(buffer.data(), "%ld,%ld,%ld", &key, &v, &ts);
    //keyType key = stoi(buffer);
    TuplePtr tuple = std::make_shared<Tuple>(key);
    tuple->subKey = ts;
    tuple->payload = v;
    relation.push(tuple);
    //cout<<buffer<<":"<<tuple->key<<","<<tuple->subKey<<endl;
  }
  file.close();
}
void DatasetTool::load3VText(TuplePtrQueue &relation, const std::string &fileName) {
  std::fstream file;
  std::string buffer;
  file.open(fileName, std::ios::in);
  if (!file.is_open()) INTELLI_ERROR("cannot open the file: " + fileName);
  while (getline(file, buffer)) {
    keyType key;
    valueType v;
    size_t ts;
    sscanf(buffer.data(), "%ld,%ld,%ld", &key, &v, &ts);
    //keyType key = stoi(buffer);
    TuplePtr tuple = std::make_shared<Tuple>(key);
    tuple->subKey = ts;
    tuple->payload = v;
    relation->push(tuple);
    //cout<<buffer<<":"<<tuple->key<<","<<tuple->subKey<<endl;
  }
  file.close();
}
void DatasetTool::combine3VVector(TuplePtrQueueIn &relation,
                                  vector<keyType> vk,
                                  vector<valueType> vv,
                                  vector<size_t> vs) {

  size_t i = 0;
  size_t allLen = vk.size();
  for (i = 0; i < allLen; i++) {
    TuplePtr tuple = std::make_shared<Tuple>(vk[i]);
    tuple->payload = vv[i];
    tuple->subKey = vs[i];
    relation.push(tuple);
  }

}
void DatasetTool::combine3VVector(TuplePtrQueue &relation,
                                  vector<keyType> vk,
                                  vector<valueType> vv,
                                  vector<size_t> vs) {

  size_t i = 0;
  size_t allLen = vk.size();
  for (i = 0; i < allLen; i++) {
    TuplePtr tuple = std::make_shared<Tuple>(vk[i]);
    tuple->payload = vv[i];
    tuple->subKey = vs[i];
    relation->push(tuple);
  }
}