// OpenVRHapticsTest.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "winmm.lib")
#include "framework.h"
#include "OpenVRHapticsTest.h"

#define MAX_LOADSTRING 100

#include "openvr.h"

#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <ostream>
#include <string>
#include <sstream>

// C RunTime Header Files
#include <Windows.h>
#include <timeapi.h>
#include "targetver.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <mmsystem.h>

#define MAX_LOADSTRING 100

// Global Variables:

// canvas size and magnification
const int PIXEL_SIZE = 8; // magnification factor
const int CANVAS_WIDTH = 192; // width of the canvas in (magnified) pixels
const int CANVAS_HEIGHT = 48; // height of the canvas in (magnified) pixels

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

vr::IVRSystem* vr_system = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	vr::EVRInitError error = vr::VRInitError_None;
	vr_system = vr::VR_Init(&error, vr::VRApplication_Background);

	if (error != vr::VRInitError_None) {
		return FALSE;
	}

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_OPENVRHAPTICSTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OPENVRHAPTICSTEST));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OPENVRHAPTICSTEST));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_OPENVRHAPTICSTEST);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, L"Vibrate Test", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1200, 500, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// Create a bitmap for the drawing area
HBITMAP hBitmap = nullptr;

// Global handles for buttons
HWND hButtonClear = nullptr; // "Clear" button
HWND hButtonSave = nullptr;  // "Save" button
HWND hButtonVibrate = nullptr;
HWND hTextInput = nullptr;     // Edit control (input text box)
HWND hMaxPowerInput = nullptr;
HWND hKeyframes = nullptr;
HWND hButtonSelectAll = nullptr;

// Control identifiers
#define BUTTON_CLEAR_ID 1001
#define BUTTON_SAVE_ID 1002
#define BUTTON_VIB_ID 1003
#define BUTTON_SELECTALL_ID 1004

// Function to create buttons and canvas control
void CreateControls(HWND hWnd)
{
	// Create a static control for the canvas
	HWND hCanvas = CreateWindow(
		"STATIC",
		NULL,
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		0, 0,
		CANVAS_WIDTH * PIXEL_SIZE,
		CANVAS_HEIGHT * PIXEL_SIZE,
		hWnd,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	// Create "Clear" button
	hButtonClear = CreateWindow(
		"BUTTON",
		"Clear",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, // x-position
		CANVAS_HEIGHT * PIXEL_SIZE + 10, // y-position
		60, // width
		30, // height
		hWnd,
		(HMENU)BUTTON_CLEAR_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	// Create "Vibrate" button
	hButtonVibrate = CreateWindow(
		"BUTTON",
		"Vibrate",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		120, // x-position
		CANVAS_HEIGHT * PIXEL_SIZE + 10, // y-position
		60, // width
		30, // height
		hWnd,
		(HMENU)BUTTON_VIB_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	CreateWindow(
		"STATIC",
		"Period (ms)", // Initial text
		WS_CHILD | WS_VISIBLE | SS_LEFT, // Styles for left-aligned, visible child
		200,  // x-coordinate
		CANVAS_HEIGHT * PIXEL_SIZE + 10,  // y-coordinate
		80, // Width
		30,  // Height
		hWnd,
		nullptr,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		nullptr
	);
	// input box
	hTextInput = CreateWindow(
		"EDIT",
		"5",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		280,
		CANVAS_HEIGHT * PIXEL_SIZE + 10,
		30,
		30,
		hWnd,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);
	CreateWindow(
		"STATIC",
		"Max Power", // Initial text
		WS_CHILD | WS_VISIBLE | SS_LEFT, // Styles for left-aligned, visible child
		320,  // x-coordinate
		CANVAS_HEIGHT * PIXEL_SIZE + 10,  // y-coordinate
		80, // Width
		30,  // Height
		hWnd,
		nullptr,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		nullptr
	);
	hMaxPowerInput = CreateWindow(
		"EDIT",
		"4000",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		400,
		CANVAS_HEIGHT * PIXEL_SIZE + 10,
		40,
		30,
		hWnd,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	hKeyframes = CreateWindow(
		"EDIT",
		"{}", // Initial text
		WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL | SS_LEFT, // Styles for left-aligned, visible child
		10,  // x-coordinate
		CANVAS_HEIGHT * PIXEL_SIZE + 50,  // y-coordinate
		800, // Width
		30,  // Height
		hWnd,
		nullptr,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		nullptr
	);

	// Create a button that selects all text
	hButtonSelectAll = CreateWindow(
		"BUTTON",
		"COPY",  // Button text
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		700,  // x-position
		CANVAS_HEIGHT * PIXEL_SIZE + 10,   // y-position
		100,  // width
		30,   // height
		hWnd,
		(HMENU)BUTTON_SELECTALL_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

}

int HapticStrengthScale(int max, int in) {
	return (CANVAS_HEIGHT - in) * (max / CANVAS_HEIGHT) - 1;
}

struct RGB {
	unsigned char r, g, b;
};

int mybitmap[CANVAS_HEIGHT][CANVAS_WIDTH] = {};

int FindLength() {
	int length = 0;
	for (int c = 0; c < CANVAS_WIDTH; c++) {
		for (int i = 0; i < CANVAS_HEIGHT; i++) {
			if (mybitmap[i][c]) length = c;
		}
	}
	return length;
}

std::mutex vibrate_lock;

void DoVibration() {
	std::scoped_lock lock(vibrate_lock);

	int length = FindLength();

	char textBuffer[256];
	GetWindowText(hTextInput, textBuffer, sizeof(textBuffer));
	int period = 5;
	try {
		period = std::stoi(textBuffer);
	}
	catch (...) {
	};

	GetWindowText(hMaxPowerInput, textBuffer, sizeof(textBuffer));
	int maxpower = 4000;
	try {
		maxpower = std::stoi(textBuffer);
	}
	catch (...) {
	};

	auto lhand = vr_system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	auto rhand = vr_system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	if (maxpower < 100) { maxpower = 4000; }
	if (period > 22) { period = 22; } if (period < 1) { period = 1; }

	std::ostringstream oss;
	oss << "{";
	for (int col = 0; col <= length; col++) {
		int duration = 0;
		for (int row = 0; row < CANVAS_HEIGHT; row++) {
			mybitmap[row][col];
			if (mybitmap[row][col]) {
				duration = HapticStrengthScale(maxpower, row);
				vr_system->TriggerHapticPulse(lhand, 0, duration);
				vr_system->TriggerHapticPulse(rhand, 5, duration);
				oss << duration << ", ";
				break;
			}
		}
		if (!duration) { oss << "0, "; }

		timeBeginPeriod(1);
		std::this_thread::sleep_for(std::chrono::milliseconds(period));
		timeEndPeriod(1);

	}
	oss << "}" << "\n";

	auto finalstr = oss.str();
	SetWindowText(hKeyframes, finalstr.c_str());
}

void SetPixelsInColumn(HDC hdc, int x, int y) {
	for (int i = 0; i < CANVAS_HEIGHT; i++) {
		if (i == y) {
			SetPixel(hdc, x, i, RGB(0, 0, 0));
			mybitmap[i][x] = 1;
		}
		else
		{
			SetPixel(hdc, x, i, RGB(200, 200, 200));
			mybitmap[i][x] = 0;
		}
	}
}
void ClearPixelsInColumn(HDC hdc, int x) {
	for (int i = 0; i < CANVAS_HEIGHT; i++) {
		SetPixel(hdc, x, i, RGB(200, 200, 200));
		mybitmap[i][x] = 0;
	}
}

bool isMouseDown = false;
bool RightMouseDown = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		{
			// Create a bitmap for the drawing area (off-screen)
			HDC hdc = GetDC(hWnd);
			hBitmap = CreateCompatibleBitmap(hdc, CANVAS_WIDTH, CANVAS_HEIGHT);
			ReleaseDC(hWnd, hdc);
			HDC memDC = CreateCompatibleDC(NULL);
			SelectObject(memDC, hBitmap);
			RECT rect = { 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT };
			FillRect(memDC, &rect, (HBRUSH)(COLOR_BACKGROUND)); // White
			DeleteDC(memDC);
			InvalidateRect(hWnd, NULL, TRUE);

			// Create controls (canvas and buttons)
			CreateControls(hWnd);
		}
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case BUTTON_SELECTALL_ID:
		{
			int length = GetWindowTextLength(hKeyframes);
			char* buffer = new char[length + 1];
			GetWindowText(hKeyframes, buffer, length + 1); // Retrieve the text

			if (OpenClipboard(NULL)) {
				EmptyClipboard();

				// Allocate global memory for the text
				size_t length = strlen(buffer) + 1; // Include the null terminator
				HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, length);
				if (hGlobal) {
					// Lock the global memory and copy the text
					char* pGlobal = (char*)GlobalLock(hGlobal);
					strcpy_s(pGlobal, length, buffer); // Copy the text
					GlobalUnlock(hGlobal);

					// Set the clipboard data with CF_TEXT format
					SetClipboardData(CF_TEXT, hGlobal);
				}

				// Close the clipboard
				CloseClipboard();
			}



			delete[] buffer; // Clean up allocated memory
		}
		break;
		case BUTTON_CLEAR_ID:
		{
			// Clear the bitmap
			HDC memDC = CreateCompatibleDC(NULL);
			SelectObject(memDC, hBitmap);
			RECT rect = { 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT };
			FillRect(memDC, &rect, (HBRUSH)(COLOR_BACKGROUND)); // White
			DeleteDC(memDC);
			InvalidateRect(hWnd, NULL, TRUE);
			memset(mybitmap, 0, sizeof(mybitmap));
			break;
		}
		case BUTTON_VIB_ID:
		{
			auto p = std::thread(DoVibration);
			p.detach();
		}
		break;
		default:
			break;
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		{
			if (LOWORD(lParam) < CANVAS_WIDTH * PIXEL_SIZE && HIWORD(lParam) < CANVAS_HEIGHT * PIXEL_SIZE)
			{
				isMouseDown = true;

				int x = LOWORD(lParam) / PIXEL_SIZE; // scale down to canvas coordinates
				int y = HIWORD(lParam) / PIXEL_SIZE;

				HDC hdc = GetDC(hWnd);
				HDC memDC = CreateCompatibleDC(hdc);
				SelectObject(memDC, hBitmap);
				SetPixelsInColumn(memDC, x, y);
				DeleteDC(memDC);
				ReleaseDC(hWnd, hdc);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	}
	case WM_MOUSEMOVE:  // Mouse is moving
		if (LOWORD(lParam) < CANVAS_WIDTH * PIXEL_SIZE && HIWORD(lParam) < CANVAS_HEIGHT * PIXEL_SIZE &&
			LOWORD(lParam) > 0 && HIWORD(lParam) > 0)
		{
			if (isMouseDown) {  // Only paint if the mouse is down
				int x = LOWORD(lParam) / PIXEL_SIZE;
				int y = HIWORD(lParam) / PIXEL_SIZE;

				HDC hdc = GetDC(hWnd);
				HDC memDC = CreateCompatibleDC(hdc);
				SelectObject(memDC, hBitmap);
				SetPixelsInColumn(memDC, x, y);
				InvalidateRect(hWnd, NULL, TRUE);
				DeleteDC(memDC);
				ReleaseDC(hWnd, hdc);
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else if (RightMouseDown) {
				int x = LOWORD(lParam) / PIXEL_SIZE;
				int y = HIWORD(lParam) / PIXEL_SIZE;

				HDC hdc = GetDC(hWnd);
				HDC memDC = CreateCompatibleDC(hdc);
				SelectObject(memDC, hBitmap);
				ClearPixelsInColumn(memDC, x);
				InvalidateRect(hWnd, NULL, TRUE);
				DeleteDC(memDC);
				ReleaseDC(hWnd, hdc);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else {
			isMouseDown = false;
		}
		break;

	case WM_RBUTTONDOWN:
	{
		if (LOWORD(lParam) < CANVAS_WIDTH * PIXEL_SIZE && HIWORD(lParam) < CANVAS_HEIGHT * PIXEL_SIZE)
		{
			RightMouseDown = true;
			int x = LOWORD(lParam) / PIXEL_SIZE;
			int y = HIWORD(lParam) / PIXEL_SIZE;

			HDC hdc = GetDC(hWnd);
			HDC memDC = CreateCompatibleDC(hdc);
			SelectObject(memDC, hBitmap);
			ClearPixelsInColumn(memDC, x);
			InvalidateRect(hWnd, NULL, TRUE);

			DeleteDC(memDC);
			ReleaseDC(hWnd, hdc);
		}
		break;
	}
	case WM_RBUTTONUP:
		RightMouseDown = false;
		break;
	case WM_LBUTTONUP:
		isMouseDown = false;
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// Create a compatible DC and select the off-screen bitmap
		HDC memDC = CreateCompatibleDC(hdc);
		SelectObject(memDC, hBitmap);

		// Stretch the bitmap to fill the window with magnification
		StretchBlt(
			hdc,
			0, 0,
			CANVAS_WIDTH * PIXEL_SIZE,
			CANVAS_HEIGHT * PIXEL_SIZE,
			memDC,
			0, 0,
			CANVAS_WIDTH,
			CANVAS_HEIGHT,
			SRCCOPY
		);

		DeleteDC(memDC);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		DeleteObject(hBitmap); // clean up bitmap
		PostQuitMessage(0);
		vr::VR_Shutdown();
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
