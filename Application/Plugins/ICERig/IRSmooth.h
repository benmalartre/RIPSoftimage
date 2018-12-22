#ifndef IR_SMOOTH_H
#define IR_SMOOTH_H
#include <vector>

namespace ICERIG {
	class SmoothPoint
	{
	public:
		std::vector<float> _weights;
		std::vector<float> _tmp;
		std::vector<long> _neighbors;
		float _smooth;

		void PushNeighbors(long i);
		void PushWeights(float w);
		void Interpolate();
	};

	class Smooth
	{
	public:
		std::vector<SmoothPoint*> _points;
		long iterations;
		bool _init;

		void CleanUp();
		void Init(long nbPoints, long nbIterations);
		void DoIt();
	};
}
#endif