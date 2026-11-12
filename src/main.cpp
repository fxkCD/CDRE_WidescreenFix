#include <windows.h>
#include <memory>
#include <algorithm>
using std::min;

#include "injector/injector.hpp"

float AspectWidth;
float AspectHeight;

float scale_general = 4.0f / 3.0f;
double projection_one = 1.0;

int window_x;
int window_y;
uintptr_t base;

int gcd(int a, int b)
{
	int result = min(a, b);
	while (result > 0)
	{
		if (a % result == 0 && b % result == 0)
		{
			break;
		}
		result--;
	}
	return result;
}

DWORD WINAPI MainTHREAD(LPVOID)
{
	injector::WriteMemory<DWORD>(base + 0x27CAC1, 0x05100FF3, true);
	injector::WriteMemory<uintptr_t>(base + 0x27CAC5,
		reinterpret_cast<uintptr_t>(&scale_general), true);
	injector::WriteMemory<uintptr_t>(base + 0x27CAD1,
		reinterpret_cast<uintptr_t>(&projection_one), true);
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	while (true)
	{
		if (*(DWORD*)(base + 0x470260))
		{
			window_x = static_cast<int>(injector::ReadMemory<float>(injector::ReadMemory<DWORD>(base + 0x470260) + 0x1C, true));
			window_y = static_cast<int>(injector::ReadMemory<float>(injector::ReadMemory<DWORD>(base + 0x470260) + 0x20, true));

			if ((window_x > 0) && (window_y > 0))
			{
				AspectWidth = window_x / gcd(window_x, window_y);
				AspectHeight = window_y / gcd(window_x, window_y);

				scale_general = (AspectWidth / AspectHeight);
			}
		}
		Sleep(16);
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
		if (*(DWORD*)(base + 0x160) == 0x37FA8E &&
			injector::ReadMemory<DWORD>(base + 0x27CAC1) == 0x87100FF3 &&
			injector::ReadMemory<DWORD>(base + 0x27CAC5) == 0xE4 &&
			injector::ReadMemory<uintptr_t>(base + 0x27CAD1) == base + 0x408F50)
		{
			HANDLE thread = CreateThread(0, 0, MainTHREAD, 0, 0, 0);
			if (!thread)
				return FALSE;
			CloseHandle(thread);
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}
