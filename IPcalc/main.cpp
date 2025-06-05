#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include "resource.h"


BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int MaskToPrefix(DWORD dwMask);
DWORD PrefixToMask(int prefix);


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: 
	{
		HWND hPrefix = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		SendMessage(hPrefix, UDM_SETRANGE, 0, MAKELPARAM(32, 0));
		SetFocus(GetDlgItem(hwnd, IDC_IPADDRESS_IP));
	} break;
	case WM_COMMAND:
	{
		HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS_IP);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
		HWND hEditPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		HWND hSpin = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		DWORD dwIPadddress = 0;
		DWORD dwIPmask = 0;
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS_IP:
		{
			SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPadddress);
			if (FIRST_IPADDRESS(dwIPadddress) == 0) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0x00000000);
			else if (FIRST_IPADDRESS(dwIPadddress) < 128) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFF000000);
			else if (FIRST_IPADDRESS(dwIPadddress) < 192) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFF0000);
			else if (FIRST_IPADDRESS(dwIPadddress) < 224) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFFFF00);
		}break;

		case IDC_IPADDRESS_MASK:
		{
			/*SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
			int prefix = MaskToPrefix(dwIPmask);
			if (prefix != -1)
			{
				SetWindowText(hEditPrefix, std::to_wstring(prefix).c_str());
			}*/
		} break;
		// создать новую кнопку для выпора режима работы префикса или через IP

		case IDC_EDIT_PREFIX:
		{
			
			int prefix = (int)SendMessage(hSpin, UDM_GETPOS32, 0, 0);
			if (prefix < 0 || prefix > 32) return -1;
			DWORD mask = PrefixToMask(prefix);
			SendMessage(hIPmask, IPM_SETADDRESS, 0, mask);
		} break;

		case IDOK: {} break;
		
		case IDCANCEL:
		{
			EndDialog(hwnd, 0);
		} break;

		} break;
	} break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
	}
	return FALSE;
}

int MaskToPrefix(DWORD dwMask) 
{
	int prefix = 0;
	DWORD mask = dwMask;

	// подсчёт количества единиц в маске
	while (mask & 0x80000000) 
	{
		prefix++;
		mask <<= 1;
	}

	// если после единиц идут нули, это валидная маска
	if (mask != 0) return -1; // некорректная маска (ошибка)

	return prefix;
}

DWORD PrefixToMask(int prefix) 
{
	if (prefix < 0 || prefix > 32) return 0xFFFFFFFF;
	return (prefix == 0) ? 0 : (0xFFFFFFFF << (32 - prefix));
}
