#include "IRSmooth.h"

namespace ICERIG {
  void SmoothPoint::PushNeighbors(long i) { _neighbors.push_back(i); };
  void SmoothPoint::PushWeights(float w) { _weights.push_back(w); };

  void SmoothPoint::Interpolate()
  {
    for (int w = 0; w < _weights.size(); w++) {
      float tmp = _weights[w] * (1.0f - _smooth * 0.5f) + _tmp[w] * (_smooth * 0.5f);
      _weights[w] = tmp;
    }
  };

  void Smooth::CleanUp()
  {
    std::vector<SmoothPoint*>::iterator it = _points.begin();
    for (; it < _points.end(); it++) {
      delete(*it);
    }
    _points.clear();
  }

  void Smooth::Init(long nbPoints, long nbIterations)
  {
    _points.resize(nbPoints);
    for (long x = 0; x < nbPoints; x++) {
      SmoothPoint* p = new SmoothPoint();
      _points[x] = p;
    }
    iterations = nbIterations;
    _init = true;
  }

  void Smooth::Compute()
  {
    for (int x = 0; x < iterations; x++) {
      std::vector<SmoothPoint*>::iterator it = _points.begin();
      for (; it < _points.end(); it++) {
        (*it)->_tmp.resize((*it)->_weights.size());
        for (int w = 0; w < (*it)->_weights.size(); w++) {
          (*it)->_tmp[w] = 0;
          long nbn = (long)(*it)->_neighbors.size();
          float average = 0;

          for (long z = 0; z < nbn; z++) {
            average += _points[(*it)->_neighbors[z]]->_weights[w];
          }
          average /= (float)nbn;
          (*it)->_tmp[w] = average;
        }
      }

      for (it = _points.begin(); it < _points.end(); it++) {
        (*it)->Interpolate();
      }
    }
  }
} // end namespace ICERIG
