#include "ScanUI.h"

#pragma comment(lib, "comctl32.lib")

#include <CommCtrl.h>

void InitScanUI(HWND hWnd)
{
    //HWND hwndButton = CreateWindow(
    //    L"BUTTON",  // Predefined class; Unicode assumed 
    //    L"OK",      // Button text 
    //    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    //    10,         // x position 
    //    10,         // y position 
    //    100,        // Button width
    //    100,        // Button height
    //    hWnd,     // Parent window
    //    (HMENU)MY_BUTTON_ID,       // Register button event id.
    //    (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
    //    NULL);      // Pointer not needed.

    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        0, 0,
        rcClient.right - rcClient.left,
        rcClient.bottom - rcClient.top,
        hWnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"name";
    lvc.cx = 100;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvc);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = (LPWSTR)L"항목1";
    ListView_InsertItem(hWndListView, &lvi);
    ListView_SetItemText(hWndListView, 0, 1, (LPWSTR)L"이 것은 첫 번째 항목입니다.");
}