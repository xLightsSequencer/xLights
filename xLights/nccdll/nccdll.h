// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NCCDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NCCDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NCCDLL_EXPORTS
#define NCCDLL_API __declspec(dllexport)
#else
#define NCCDLL_API __declspec(dllimport)
#endif

// This class is exported from the nccdll.dll
class NCCDLL_API Cnccdll {
public:
	Cnccdll(void);
	// TODO: add your methods here.
};

extern NCCDLL_API int nnccdll;

NCCDLL_API int fnnccdll(void);
