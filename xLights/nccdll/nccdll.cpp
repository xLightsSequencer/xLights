// nccdll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nccdll.h"


// This is an example of an exported variable
NCCDLL_API int nnccdll=0;

// This is an example of an exported function.
NCCDLL_API int fnnccdll(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see nccdll.h for the class definition
Cnccdll::Cnccdll()
{
	return;
}
