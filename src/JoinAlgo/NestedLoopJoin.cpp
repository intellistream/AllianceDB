#include <JoinAlgo/NestedLoopJoin.h>

using namespace AllianceDB;
size_t NestedLoopJoin::join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads) {
  assert(threads > 0);
  size_t result = 0;
  for (size_t i = 0; i < tsLen; i++) {
    if (ts[i]->key == tr->key) {
      //cout<<to_string(ts[i]->subKey)+","+ to_string(tr->subKey)<<endl;
      result++;
    }
  }
  // cout<<result<<endl;
  return result;
}
size_t NestedLoopJoin::join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads) {
  size_t result = 0;
  for (size_t i = 0; i < trLen; i++) {
    result += join(ts, tr[i], tsLen, threads);
  }
  return result;
}
size_t NestedLoopJoin::join(TuplePtrQueue ts, TuplePtr tr, int threads) {
  cout << "NL JOIN" << endl;
  return join(ts->front(), tr, ts->size(), threads);
}

size_t NestedLoopJoin::join(TuplePtrQueue ts, TuplePtrQueue tr, int threads) {
  return join(ts->front(), tr->front(), ts->size(), tr->size(), threads);
}