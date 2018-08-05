#pragma once

#include <strsafe.h>	
#include <string>
#include <deque>
#include <iostream>    
#include <sstream>  
#include "resource.h"
#include <ShlObj.h>								// For File Browsing
#include <objidl.h>
#include <ctime>								// For Running Timer
#include <Shlwapi.h>
#pragma warning( push )
#pragma warning( disable : 4458)
#include <gdiplus.h>
#pragma warning( pop )
using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")
#pragma comment (lib, "shlwapi.lib")
#pragma warning(disable:4996)					// Disable warnings about uses of mbstowcs

using namespace std;

void CopyJPEGToFavorite(void);
bool CreateFavoritePicturesPath(void);
void DrawJPEG(HDC, const std::wstring & fileName);
bool FindAllImageFiles(void);
void ResetTimer(UINT_PTR timerID, UINT elapseTime);
void SetViewOrderMenuState(bool enabled);
void SetViewOrderCheckedItem(UINT itemID);
void SetDisplayTimeCheckedItem(UINT itemID);
void AddImageCount(void);
void UpdateDisplayTime(UINT timeID, UINT newDisplayTime);
