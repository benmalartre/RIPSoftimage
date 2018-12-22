// sPointCache.h
//------------------------------------
#ifndef _SPOINTCACHE_H_
#define _SPOINTCACHE_H_

#include "bbppPointCache.h"
#include "sRegister.h"

// sCPointCache Class
//------------------------------------
class sPointCache  
{  
public:
	sPointCache(){};
	~sPointCache(){};

	CStatus Init(const CString& in_filepath,long in_nbpoints );
	CStatus Update(OperatorContext&);
	bbppPointCache _reader;
};

#endif
