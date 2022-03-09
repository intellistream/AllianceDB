//
// Created by Wang Chenyu on 31/8/21.
//
#include <JoinMethods/CommonFunctions.h>

namespace INTELLI {
void CommonFunction::buildRelation(TuplePtrQueueLocal &relation, const std::string &fileName) {
  std::fstream file;
  std::string buffer;
  file.open(fileName, std::ios::in);
  if (!file.is_open()) INTELLI_ERROR("cannot open the file: " + fileName);
  while (getline(file, buffer)) {
    keyType key;
    size_t ts;
    sscanf(buffer.data(), "%ld,%ld", &key, &ts);
    //keyType key = stoi(buffer);
    TuplePtr tuple = std::make_shared<Tuple>(key);
    tuple->subKey = ts;
    relation.push(tuple);
    //cout<<buffer<<":"<<tuple->key<<","<<tuple->subKey<<endl;
  }
  file.close();
}
}