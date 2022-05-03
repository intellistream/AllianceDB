#include <Common/DatasetTool.hpp>
#include <fstream>
#include <Common/Types.hpp>
#include <Utils/Logger.hpp>

using namespace INTELLI;

void DatasetTool::LoadData(Relation &relation, const std::string &fileName) {
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
    relation.push_back(tuple);
//    cout << buffer << ":" << tuple->key << "," << tuple->subKey << endl;
  }
  file.close();
}