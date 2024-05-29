#include <TestBench/FileDataLoader.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

using namespace INTELLI;
using namespace OoOJoin;

bool FileDataLoader::setConfig(ConfigMapPtr cfg) {
  cfgGlobal = cfg;
  string fnameR, fnameS;
  fnameR = cfg->tryString("fileDataLoader_rFile", "../../benchmark/datasets/rTuple.csv", true);
  fnameS = cfg->tryString("fileDataLoader_sFile", "../../benchmark/datasets/sTuple.csv", true);
  sTuple = loadDataFromCsv(fnameS);
  rTuple = loadDataFromCsv(fnameR);
  return true;
}

vector<TrackTuplePtr> FileDataLoader::getTupleVectorS() {
  return sTuple;
}

vector<TrackTuplePtr> FileDataLoader::getTupleVectorR() {
  return rTuple;
}

std::vector<TrackTuplePtr> OoOJoin::FileDataLoader::loadDataFromCsv(std::string fname,
                                                                    std::string separator,
                                                                    std::string newLine) {
  std::vector<TrackTuplePtr> ru;
  ifstream ins;
  ins.open(fname);
  assert(separator.data());
  assert(newLine.data());
  if (ins.fail()) {
    INTELLI_ERROR("Can't open file [" + fname + "]");
    return ru;
  }
  std::string readStr;
  /**
   * header is of << "key,value,eventTime,arrivalTime,processedTime\n";
   * should read the first line
   */
  std::getline(ins, readStr, newLine.data()[0]);
  vector<std::string> cols;
  // readStr.erase(readStr.size()-1);
  spilt(readStr, separator, cols);
  size_t idxKey = 0, idxValue = 0, idxEventTime = 0, idxArrivalTime = 0;
  size_t validCols = 4;
  size_t validRows = 0;
  if (cols.size() < validCols) {
    INTELLI_ERROR("Invalid csv header [" + fname + "] return nothing");
    return ru;
  }

  /**
   * @note parase the first row here
   */
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "key") {
      idxKey = i;
    }
    if (cols[i] == "value") {
      idxValue = i;
    }
    if (cols[i] == "eventTime") {
      idxEventTime = i;
    }
    if (cols[i] == "arrivalTime" || cols[i] == "arriveTime" || cols[i] == "arrivalTime\r" ||
        cols[i] == "arriveTime\r") {
      idxArrivalTime = i;
    }
  }
  /**
   * @note deciding the valid rows
   */
  while (std::getline(ins, readStr, newLine.data()[0])) {
    if (cols.size() >= validCols) {
      validRows++;
    }
  }
  INTELLI_INFO("valid rows=" + to_string(validRows));
  // cout<<"valid rows="<<validRows<<endl;
  ru = std::vector<TrackTuplePtr>(validRows);
  /**
   * re-open file
   */
  ins.close();
  ins.open(fname);
  /**
   * jump the header,
   */
  size_t loadRow = 0;
  std::getline(ins, readStr, newLine.data()[0]);
  while (std::getline(ins, readStr, newLine.data()[0])) {
    vector<std::string> cols2;
    // readStr.erase(readStr.size()-1);
    spilt(readStr, separator, cols2);
    if (cols.size() >= validCols) {
      keyType k;
      valueType v;
      tsType et, at;
      istringstream k_ss(cols2[idxKey]);
      k_ss >> k;
      istringstream v_ss(cols2[idxValue]);
      v_ss >> v;
      istringstream et_ss(cols2[idxEventTime]);
      et_ss >> et;
      istringstream at_ss(cols2[idxArrivalTime]);
      at_ss >> at;
      TrackTuplePtr tp = newTrackTuple(k, v, et, at);
      tp->processedTime = 0;
      ru[loadRow] = tp;
      //  cout<<ru[loadRow]->toString()<<endl;
      loadRow++;
    }
  }
  ins.close();
  return ru;
}

