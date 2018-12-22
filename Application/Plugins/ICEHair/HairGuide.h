/*-------------------------------------------------
// HairGuide.h
							  _(,__           __),
						(_,d888888888b,d888888888b
						 d888888888888/888888888888b_)
					  (_8888888P'""'`Y8Y`'""'"Y88888b
						Y8888P.-'     `      '-.Y8888b_)
					   ,_Y88P (_(_(        )_)_) d88Y_,
						Y88b,  (o  )      (o  ) d8888P
						`Y888   '-'        '-'  `88Y`
						,d/O\         c         /O\b,
						  \_/'.,______w______,.'\_/
							 .-`             `-.
							/   , d88b  d88b_   \
						   /   / 88888bd88888`\  \
						  /   / \ Y88888888Y   \  \
						  \  \   \ 88888888    /  /
						   `\ `.  \d8888888b, /\\/
							 `.//.d8888888888b; |
							   |/d888888888888b/
							   d8888888888888888b
							,_d88p""q88888p""q888b,
							`""'`\    "`|    /`'""`
								  `.    |===/
									>   |   |
									/   |   |
								   |    |   |
								   |    Y  /
								   \   /  /
							 jgs    | /| /
								   / / / |
								  /=/  |=/
								 `"`   `"`
 */
//-------------------------------------------------
#ifndef __HAIRGUIDE_H_
#define __HAIRGUIDE_H_

#include "HairRegister.h"
#include "HairGeometry.h"
#include "HairGuide.h"
#include <vector>
#include <list>
#include <map>

namespace ICEHAIR {
	class IHVertex;
	class IHVirtualVertex;
	class IHTip;

	//-------------------------------------------------------------
	//
	//	Hair Guide Object
	//
	//	store an array of HairGuideSegment objects
	//	describe ONE PointPosition plus its StrandPosition array
	//
	//-------------------------------------------------------------
	class IHGuide
	{
	public:
		std::vector<IHVertex*> _vertices;				// pointer to array of Vertex(either real or virtual)
		std::vector<IHVirtualVertex*> _virtuals;		// pointer to array of virtual vertices

		bool _walking;									// is this guide walking on mesh or interpolate other guides position
		std::vector<IHGuide*> _guides;					// array of guide objects to interpolate position from
		std::vector<float> _weights;					// array of guide weights to interpolate position from

		void GetDrivingGuides(IHTip* tip);
		void PushVirtualVertices();
		void ClearVirtualVertices();

		ULONG _id;										//	id for this guide
		bool _reached;									//	does this guide reached root(invalid otherwise)
		float GetLastSegmentLength();					// get last segment length
		CVector3f GetLastSegment();					// get last segment length

		IHGuide();										//	constructor
		~IHGuide();									//	destructor

	};
}

#endif