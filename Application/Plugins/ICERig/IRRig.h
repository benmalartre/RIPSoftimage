/*-----------------------------------------------------------------
File : IRRig.h
-------------------------------------------------------------------*/
#ifndef IR_RIG_H
#define IR_RIG_H

#include "IRRegister.h"
#include "IRJoint.h"
#include "IRElement.h"

namespace ICERIG {
	class IRRig
	{
	public:
		IRRig() {};
		~IRRig() {};
	private:
		std::vector<IRElement*> m_elements;
	};
}		// ICERIG
#endif	// IR_RIG_H