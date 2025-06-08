#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <CommCtrl.h>
#include <string>
#include <cstdio>
#include "resource.h"

#define BUFFER_SIZE 1024

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID PrintInfo(HWND hwnd);
CHAR* IPtoString(DWORD dwIpAddres, CHAR sz_IpAddress[]);

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
		SendMessage(hPrefix, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
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
			if (FIRST_IPADDRESS(dwIpAdddress) < 128) SendMessage(hEditPrefix, WM_SETTEXT, 0, (LPARAM)"8");
			else if (FIRST_IPADDRESS(dwIpAdddress) < 192) SendMessage(hEditPrefix, WM_SETTEXT, 0, (LPARAM)"16");
			else if (FIRST_IPADDRESS(dwIpAdddress) < 224) SendMessage(hEditPrefix, WM_SETTEXT, 0, (LPARAM)"24");
		}break;

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

		case IDC_EDIT_PREFIX:
		{
			CHAR sz_prefix[3] = {};
			SendMessage(hEditPrefix, WM_GETTEXT, 3, (LPARAM)sz_prefix);
			DWORD dwPrefix = atoi(sz_prefix);
			DWORD dwIPMask = ~(0xFFFFFFFF >> dwPrefix);
			SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPMask);
			PrintInfo(hwnd);
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
		DWORD dwIpMask = 0;
		
		switch (((NMHDR*)lParam)->idFrom)
		//switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS_MASK:
		{
			SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIpMask);
			DWORD dwIPprefix = 0;
			for (DWORD iMask = dwIpMask; iMask & 0x80000000; dwIPprefix++) iMask <<= 1;
			CHAR sz_prefix[3];
			sprintf(sz_prefix, "%i", dwIPprefix);
			SendMessage(hEditPrefix, WM_SETTEXT, 0, (LPARAM)sz_prefix);
		} break;

		} //switch
	} break;

	case WM_CLOSE:
		FreeConsole();
		EndDialog(hwnd, 0);
	}
	return FALSE;
}

VOID PrintInfo(HWND hwnd)
{
	HWND hIpAddress = GetDlgItem(hwnd, IDC_IPADDRESS_IP);
	HWND hIpMask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
	HWND hStaticInfo = GetDlgItem(hwnd, IDC_STATIC_INFO);
	DWORD dwIpAddress = 0;
	DWORD dwIpMask = 0;
	
	SendMessage(hIpAddress, IPM_GETADDRESS, 0, (LPARAM)&dwIpAddress);
	SendMessage(hIpMask, IPM_GETADDRESS, 0, (LPARAM)&dwIpMask);
	DWORD dwNetworkAddress = dwIpAddress & dwIpMask;
	DWORD dwBroadcastAddress = dwIpAddress | ~dwIpMask;
	DWORD dwNumberOfAddresses = ~dwIpMask + 1;
	DWORD dwNumberOfHost = ~dwIpMask - 1;

	CHAR sz_NetworkAddress[16] = {};
	CHAR sz_BroadcastAddress[16] = {};
	CHAR sz_info[BUFFER_SIZE] = {};
	sprintf
	(
		sz_info,
		"Info:\nАдрес сети:\t\t\t%s;\nШироковещательный адрес:\t%s;\nКоличество Ip-адресов:\t%i;\nКоличество узлов:\t\t%i",
		IPtoString(dwNetworkAddress, sz_NetworkAddress),
		IPtoString(dwBroadcastAddress, sz_BroadcastAddress),
		dwNumberOfAddresses,
		dwNumberOfHost
	);
	SendMessage(hStaticInfo, WM_SETTEXT, 0, (LPARAM)sz_info);
}

CHAR* IPtoString(DWORD dwIpAddres, CHAR sz_IpAddress[])
{
	sprintf
	(
		sz_IpAddress, 
		"%i.%i.%i.%i",
		FIRST_IPADDRESS(dwIpAddres),
		SECOND_IPADDRESS(dwIpAddres),
		THIRD_IPADDRESS(dwIpAddres),
		FOURTH_IPADDRESS(dwIpAddres)
	);
	return sz_IpAddress;
}
