#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <CommCtrl.h>
#include <string>
#include <cstdio>
#include "resource.h"


BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//int MaskToPrefix(DWORD dwMask);
//DWORD PrefixToMask(int prefix);

//bool checkBoxPrefix = false;

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

		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	} break;
	case WM_COMMAND:
	{
		HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS_IP);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
		HWND hEditPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		HWND hSpin = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		DWORD dwIpAdddress = 0;
		DWORD dwIpMask = 0;
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS_IP:
		{
			SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIpAdddress);
			if (FIRST_IPADDRESS(dwIpAdddress) < 128) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFF000000);
			else if (FIRST_IPADDRESS(dwIpAdddress) < 192) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFF0000);
			else if (FIRST_IPADDRESS(dwIpAdddress) < 224) SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFFFF00);
		}break;

		case IDC_IPADDRESS_MASK:
		{
			SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIpMask);
			DWORD dwIPprefix = 0;
			for (DWORD iMask = dwIpMask; iMask & 0x80000000; dwIPprefix++) iMask <<= 1;
			CHAR sz_prefix[3];
			sprintf(sz_prefix, "%i", dwIPprefix);
			SendMessage(hEditPrefix, WM_SETTEXT, 0, (LPARAM)sz_prefix);
		} break;


		case IDOK: {} break;
		
		case IDCANCEL:
		{
			EndDialog(hwnd, 0);
		} break;

		} break;
	} break;

	case WM_NOTIFY:
	{
		HWND hEditPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);

		//switch (((NMHDR*)lParam)->idFrom)
		switch (LOWORD(wParam))
		{
		case IDC_SPIN_PREFIX:
		{
			std::cout << "WM_NOTIFE:EDC_SPIN_PREFIX" << std::endl;
			
			DWORD dwPrefix = ((NMUPDOWN*)lParam)->iPos;
			INT iDelta = ((NMUPDOWN*)lParam)->iDelta;
			dwPrefix += iDelta;
			std::cout << dwPrefix << std::endl;
			DWORD dwIPMask = ~(0xFFFFFFFF >> dwPrefix);
			SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPMask);
		} break;
		}
	} break;

	case WM_CLOSE:
		FreeConsole();
		EndDialog(hwnd, 0);
	}
	return FALSE;
}

int MaskToPrefix(DWORD dwMask) 
{
	int prefix = 0;
	DWORD mask = dwMask;

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
	if (prefix < 0 || prefix > 32) return 0xFFFFFFFF; // некоректный префикс возвращаем маску по умолчанию
	return (prefix == 0) ? 0 : (0xFFFFFFFF << (32 - prefix)); // создаём маску с N колво едениц слева
}
