// SlideShow.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SlideShow.h"

#define MAX_LOADSTRING (100)
#define MAX_TIMESTRING (32)
#define DIRECTORY_SEP L"\\"
#define DISPLAY_TIMER 1001								// Timer that controls pacing of window updates. Created right before the Main Message Loop
#define DISPLAYTIME_50MS (50)
#define DISPLAYTIME_100MS (100)
#define DISPLAYTIME_250MS (250)
#define DISPLAYTIME_500MS (500)
#define DISPLAYTIME_1000MS (1000)
#define DISPLAYTIME_2000MS (2000)

#define DEFAULT_DISPLAY_TIME_MS DISPLAYTIME_1000MS
#define DEFAULT_VIEW_DIRECTION ID_NEWEST_FIRST

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

// LOADING FILES
std::deque<wstring> imageFilenameVector;
std::deque<wstring>::iterator dequeImageIterator = imageFilenameVector.begin();

// SAVING FILES
std::wstring favoritesSavePath;		
std::wstring selectedImagePath;

// Defined Globally to prevent value reset on entering the callback
bool pauseSlideShow = true;

unsigned int displayTime = DEFAULT_DISPLAY_TIME_MS;
unsigned int viewDirection = DEFAULT_VIEW_DIRECTION;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	ULONG_PTR token;
	GdiplusStartupInput input = { 0 };
	input.GdiplusVersion = 1;
	GdiplusStartup(&token, &input, NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SLIDESHOW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if ( !InitInstance (hInstance, nCmdShow) )
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SLIDESHOW));

	MSG msg;
	// Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if ( !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SLIDESHOW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SLIDESHOW);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;															// Store instance handle in our global variable
   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
#pragma warning( disable : 4459)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_TIMER:														// Entry point for timer signals
		{	
			switch (wParam)
			{
				case DISPLAY_TIMER:
				{
					if (dequeImageIterator != imageFilenameVector.end())
					{
						dequeImageIterator++;
						UpdateWindow(hWnd);
					}
					else if (!imageFilenameVector.empty() && dequeImageIterator == imageFilenameVector.end())
					{
						KillTimer(hWnd, DISPLAY_TIMER);
						MessageBox(hWnd, L"Reached End of Images", L"Done", MB_OK);
						// Allowed to change direction once done going through all images
						SetViewOrderMenuState(true);
					}
					else
					{
						// nothing to do
					}
				}
			}
		}
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
			case ID_DISPLAYTIME_50MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_50MS);
				break;
			case ID_DISPLAYTIME_100MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_100MS);
				break;
			case ID_DISPLAYTIME_250MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_250MS);
				break;
			case ID_DISPLAYTIME_500MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_500MS);
				break;
			case ID_DISPLAYTIME_1000MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_1000MS);
				break;
			case ID_DISPLAYTIME_2000MS:
				UpdateDisplayTime(wParam, DISPLAYTIME_2000MS);
				break;
			case IDM_OPEN:
			{
				LPITEMIDLIST pidlRoot = NULL;
				SHGetFolderLocation(hWnd, CSIDL_DESKTOP, NULL, NULL, &pidlRoot);

				// BROWSER
				BROWSEINFO bi = { 0 };
				TCHAR szDisplayName[MAX_PATH + 1];
				szDisplayName[0] = NULL;
				bi.hwndOwner = hWnd;
				bi.pidlRoot = pidlRoot;
				bi.pszDisplayName = szDisplayName;
				bi.lpszTitle = L"Please select the folder of pictures (JPEG):";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				bi.lParam = NULL;
				bi.iImage = 0;
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if (NULL != pidl)
				{
					TCHAR loadingPathFromBrowser[MAX_PATH + 1];
					if (SHGetPathFromIDList(pidl, loadingPathFromBrowser) == FALSE)
					{
						return 0;
					}

					pauseSlideShow = false;

					selectedImagePath = loadingPathFromBrowser;
					if (!FindAllImageFiles())
					{
						return 0;
					}

					dequeImageIterator = imageFilenameVector.begin();
					UpdateWindow(hWnd);
					//SetTimer(hWnd, DISPLAY_TIMER, displayTime, NULL);
				}
				break;
			}
			case ID_OLDEST_FIRST:
			{
				SetViewOrderCheckedItem(ID_OLDEST_FIRST);
				viewDirection = ID_OLDEST_FIRST;
				break;
			}
			case ID_NEWEST_FIRST:
			{
				SetViewOrderCheckedItem(ID_NEWEST_FIRST);
				viewDirection = ID_NEWEST_FIRST;
				break;
			}
			case IDM_EXIT:
			{
				KillTimer(hWnd, DISPLAY_TIMER);								// Destroys timer object to prevent memory leak.
				PostQuitMessage(0);
			}
			break;
			case ID_HELP:
			{
				MessageBox(hWnd, L"YOUR MOM!", L"Help", MB_OK | MB_ICONWARNING);
				break;
			}
			}
		}
		case WM_PAINT:														// Called with UpdateWindow()
		{
			// Resizing hack: If window is being resized, this triggers a WM_PAINT, which bypasses the timer
			// The correct fix is to move to a timer callback instead of issuing an UpdateWindow.
			// But this works for our purposes, and also serves to let LMB indirectly pause playback
			if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
			{
				// Assume window being dragged
				return 0;
			}

			if (!pauseSlideShow && dequeImageIterator != imageFilenameVector.end())
			{
				InvalidateRect(hWnd, FALSE, TRUE);
			}

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			if (!pauseSlideShow && !imageFilenameVector.empty() && dequeImageIterator != imageFilenameVector.end())
			{
				KillTimer(hWnd, DISPLAY_TIMER);
				std::wstring fileName = selectedImagePath + std::wstring(DIRECTORY_SEP) + (*dequeImageIterator);
				DrawJPEG(hdc, fileName);
				EndPaint(hWnd, &ps);
				SetTimer(hWnd, DISPLAY_TIMER, displayTime, NULL);
			}
			break;
		}
		case WM_ERASEBKGND:													// To aid in reducing the flash between image loads
		{
			return true;
		}
		case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				KillTimer(hWnd, DISPLAY_TIMER);
				CopyJPEGToFavorite();
				if (dequeImageIterator != imageFilenameVector.end())
				{
					SetTimer(hWnd, DISPLAY_TIMER, displayTime, NULL);
				}			
			}
			else if (wParam == VK_SPACE)
			{
				if (dequeImageIterator == imageFilenameVector.end())
				{
					MessageBox(hWnd, L"Cannot pause when at end of images", L"Already stopped", MB_OK);
					return 0;
				}
				pauseSlideShow = !pauseSlideShow;
				pauseSlideShow ? KillTimer(hWnd, DISPLAY_TIMER) : SetTimer(hWnd, DISPLAY_TIMER, displayTime, NULL);
			}
			else if (wParam == VK_ESCAPE)
			{
				if (dequeImageIterator == imageFilenameVector.end())
				{
					MessageBox(hWnd, L"Cannot exit when at end of images", L"Already stopped", MB_OK);
					return 0;
				}
				pauseSlideShow = false;
				KillTimer(hWnd, DISPLAY_TIMER);
				SetViewOrderMenuState(true);
				MessageBox(hWnd, L"Image playback stopped", L"Full stop", MB_OK);
				imageFilenameVector.clear();
				dequeImageIterator = imageFilenameVector.begin();
			}
			break;
		}
		case WM_DESTROY:
		{
			KillTimer(hWnd, DISPLAY_TIMER);
			PostQuitMessage(0);
			break;
		}
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
    return 0;
}

void DrawJPEG(HDC hdc, const std::wstring & fileName)
{
	// Must re-evaluate the window size on each draw since the window can be resized
	// Optimization point: only recalculate if we detect window size change message
	Graphics graphics(hdc);
	RECT windowRect = { 0 };
	GetWindowRect(hWnd, &windowRect);
	Rect destRect(0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top - 59);
	Image image(fileName.c_str());
	graphics.DrawImage(&image, destRect);
}

//
//  FUNCTION: CopyJPEGToFavorite()
//
//	Purpose: Copies the JPEG from the loaded folder into a desktop subdirectory
//  
//

void CopyJPEGToFavorite(void)
{
	if (favoritesSavePath.empty())
	{
		CreateFavoritePicturesPath();
	}

	std::deque<wstring>::iterator tempIterator = dequeImageIterator == imageFilenameVector.end() ? dequeImageIterator - 1: dequeImageIterator;

	if (CopyFile((selectedImagePath + std::wstring(DIRECTORY_SEP) + (*tempIterator)).c_str(), (favoritesSavePath + std::wstring(DIRECTORY_SEP) + (*tempIterator)).c_str(), TRUE) == FALSE)
	{
		MessageBox(hWnd, L"Failed to save image to favorites", L"Error", MB_OK | MB_ICONERROR);
	}
}

//
//  FUNCTION: CreateFavoritePicturesDirectory()
//
//	Purpose: create directory on desktop to save files
//  
//

bool CreateFavoritePicturesPath()
{
	wchar_t toss_MDY[MAX_TIMESTRING];
	wchar_t toss_HOUR[MAX_TIMESTRING];
	wchar_t toss_MINUTES[MAX_TIMESTRING];
	wchar_t toss_SECONDS[MAX_TIMESTRING];

	time_t t = time(0);
	struct tm time = { 0 };
	localtime_s(&time, &t);
	char conversionTemp[MAX_TIMESTRING];
	strftime(conversionTemp, MAX_TIMESTRING, "%F", &time);
	mbstowcs(toss_MDY, conversionTemp, strlen(conversionTemp) + 1);
	strftime(conversionTemp, MAX_TIMESTRING, "%H", &time);
	mbstowcs(toss_HOUR, conversionTemp, strlen(conversionTemp) + 1);
	strftime(conversionTemp, MAX_TIMESTRING, "%M", &time);
	mbstowcs(toss_MINUTES, conversionTemp, strlen(conversionTemp) + 1);
	strftime(conversionTemp, MAX_TIMESTRING, "%S", &time);
	mbstowcs(toss_SECONDS, conversionTemp, strlen(conversionTemp) + 1);

	TCHAR desktopPath[MAX_PATH] = { 0 };
	if (SHGetSpecialFolderPath(hWnd, desktopPath, CSIDL_DESKTOPDIRECTORY, FALSE) == FALSE)
	{
		MessageBox(hWnd, L"Failed to get your Desktop folder", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	favoritesSavePath = std::wstring(desktopPath) + DIRECTORY_SEP + L"Favorite_Game_Camera_Images" + DIRECTORY_SEP;
	if (PathFileExists(favoritesSavePath.c_str()) == FALSE)
	{
		if (CreateDirectory(favoritesSavePath.c_str(), NULL) == FALSE)
		{
			MessageBox(hWnd, L"Failed to create favorites directory on your desktop. Check if it already exists", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	favoritesSavePath += std::wstring(toss_MDY) + L"__" + toss_HOUR + L"." + toss_MINUTES + L"." + toss_SECONDS + DIRECTORY_SEP;

	if (CreateDirectory(favoritesSavePath.c_str(), NULL) == FALSE)
	{
		MessageBox(hWnd, L"Failed to create subset directory on your desktop", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

bool FindAllImageFiles(void)
{
	imageFilenameVector.clear();
	WIN32_FIND_DATA ffd = { 0 };
	HANDLE hFind = FindFirstFile((selectedImagePath + std::wstring(L"//*jpg")).c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd, L"No JPEG file(s) found", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	imageFilenameVector.push_back(ffd.cFileName);

	while (FindNextFile(hFind, &ffd) == TRUE)
	{
		if (viewDirection == ID_NEWEST_FIRST)
		{
			imageFilenameVector.push_front(ffd.cFileName);
		}
		else if(viewDirection == ID_OLDEST_FIRST)
		{
			imageFilenameVector.push_back(ffd.cFileName);
		}
		else
		{
			MessageBox(hWnd, L"Unknown view direction!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	// Not allowed to change direction when going through images
	SetViewOrderMenuState(false);
	AddImageCount();

	return true;
}

void ResetTimer(UINT_PTR timerID, UINT elapseTime)
{
	// Don't reset the timer if we do not have images
	if (!imageFilenameVector.empty())
	{
		KillTimer(hWnd, timerID);
		SetTimer(hWnd, timerID, elapseTime, NULL);
	}
}

void AddImageCount(void)
{
	HMENU menu = GetMenu(hWnd);
	if (menu == NULL)
	{
		return;
	}
	HMENU subMenu = GetSubMenu(menu, 3);
	if (subMenu == NULL)
	{
		return;
	}

	HMENU hmenuPopup = CreatePopupMenu();
	MENUITEMINFO itemInfo = { 0 };
	itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask = MIIM_STRING | MIIM_DATA | MIIM_SUBMENU;
	itemInfo.fType = MFT_STRING;
	itemInfo.wID = 0;
	itemInfo.hSubMenu = hmenuPopup;

	std::wstringstream imageCount;
	imageCount << "Viewing "<< imageFilenameVector.size() << " Images";
	wstring imageCountString = imageCount.str();
	LPWSTR imageCountStr = const_cast<LPWSTR>(imageCountString.c_str());
	itemInfo.dwTypeData = imageCountStr;

	if (InsertMenu(subMenu, 1, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, 1, imageCountStr) == TRUE)
	{
		// This will remove any previous entry, we'll fail silently since there will not be an entry on first run
		RemoveMenu(subMenu, 2, MF_BYPOSITION);
		DrawMenuBar(hWnd);
	}
}

void SetViewOrderMenuState(bool enabled)
{
	HMENU menu = GetMenu(hWnd);
	if (menu == NULL)
	{
		return;
	}
	HMENU subMenu = GetSubMenu(menu, 2);
	if (subMenu == NULL)
	{
		return;
	}
	DWORD result = EnableMenuItem(subMenu, ID_OLDEST_FIRST, enabled ? MF_ENABLED : MF_GRAYED);
	result = EnableMenuItem(subMenu, ID_NEWEST_FIRST, enabled ? MF_ENABLED : MF_GRAYED);
}

void SetViewOrderCheckedItem(UINT itemID)
{
	HMENU menu = GetMenu(hWnd);
	if (menu == NULL)
	{
		return;
	}
	HMENU subMenu = GetSubMenu(menu, 2);
	if (subMenu == NULL)
	{
		return;
	}
	switch (itemID)
	{
	case ID_OLDEST_FIRST:
		CheckMenuItem(subMenu, ID_OLDEST_FIRST, MF_CHECKED);
		CheckMenuItem(subMenu, ID_NEWEST_FIRST, MF_GRAYED);
		break;
	case ID_NEWEST_FIRST:
		CheckMenuItem(subMenu, ID_NEWEST_FIRST, MF_CHECKED);
		CheckMenuItem(subMenu, ID_OLDEST_FIRST, MF_GRAYED);
		break;
	default:
		MessageBox(hWnd, L"Unknown View Order Menu Item ID!", L"Programmer Oops", MB_OK | MB_ICONERROR);
	}
}

void SetDisplayTimeCheckedItem(UINT itemID)
{
	HMENU menu = GetMenu(hWnd);
	if (menu == NULL)
	{
		return;
	}
	HMENU subMenu = GetSubMenu(menu, 1);
	if (subMenu == NULL)
	{
		return;
	}
	switch (itemID)
	{
	case ID_DISPLAYTIME_50MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_CHECKED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_GRAYED);
		break;
	case ID_DISPLAYTIME_100MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_CHECKED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_GRAYED);
		break;
	case ID_DISPLAYTIME_250MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_CHECKED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_GRAYED);
		break;
	case ID_DISPLAYTIME_500MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_CHECKED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_GRAYED);
		break;
	case ID_DISPLAYTIME_1000MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_CHECKED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_GRAYED);
		break;
	case ID_DISPLAYTIME_2000MS:
		CheckMenuItem(subMenu, ID_DISPLAYTIME_50MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_100MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_250MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_500MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_1000MS, MF_GRAYED);
		CheckMenuItem(subMenu, ID_DISPLAYTIME_2000MS, MF_CHECKED);
		break;
	default:
		MessageBox(hWnd, L"Unknown Display Time Menu Item ID!", L"Programmer Oops", MB_OK | MB_ICONERROR);
	}
}

void UpdateDisplayTime(UINT timeID, UINT newDisplayTime)
{
	displayTime = newDisplayTime;
	SetDisplayTimeCheckedItem(timeID);
	if (dequeImageIterator != imageFilenameVector.end())
	{
		ResetTimer(DISPLAY_TIMER, displayTime);
	}
}
