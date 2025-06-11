#include "ScanUI.h"
#include "scanutil.h"

#pragma comment(lib, "comctl32.lib")

#include <CommCtrl.h>

static HWND InitScannedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y, int w, int h)
{
    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y,
        w, h,
        hwndParent,
        NULL,
        hInstance,
        NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"Address";
    lvc.cx = 160;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Scanned Value";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &lvc);

    lvc.pszText = (LPWSTR)L"Current Value";
    lvc.cx = 160;
    lvc.iSubItem = 2;
    ListView_InsertColumn(hWndListView, 2, &lvc);

    return hWndListView;
}

void AddScannedAddress(HWND hwndListView, UINT64 address, LPCVOID data, SIZE_T dataLength)
{
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;

    int i = ListView_GetItemCount(hwndListView);

    WCHAR szAddress[32];
    swprintf(szAddress, 32, L"%016llX", address);

    WCHAR wrData[256] = L"";
    ByteStreamToWideString(wrData, data, dataLength);
    
    lvi.iItem = i;
    lvi.iSubItem = 0;
    lvi.pszText = szAddress;

    ListView_InsertItem(hwndListView, &lvi);
    ListView_SetItemText(hwndListView, i, 1, wrData);
    ListView_SetItemText(hwndListView, i, 2, wrData);
}

static HWND InitPinnedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y, int w, int h)
{
    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y,
        w, h,
        hwndParent,
        NULL,
        hInstance,
        NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"Address";
    lvc.cx = 160;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Current Value";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &lvc);

    lvc.pszText = (LPWSTR)L"Description";
    lvc.cx = 160;
    lvc.iSubItem = 2;
    ListView_InsertColumn(hWndListView, 2, &lvc);

    return hWndListView;
}

void AddPinnedAddress(HWND hwndListView, UINT64 address, LPCVOID data, SIZE_T dataLength)
{
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;

    int i = ListView_GetItemCount(hwndListView);

    WCHAR szAddress[32];
    swprintf(szAddress, 32, L"%016llX", address);

    WCHAR wrData[256] = L"";
    ByteStreamToWideString(wrData, data, dataLength);

    lvi.iItem = i;
    lvi.iSubItem = 0;
    lvi.pszText = szAddress;

    ListView_InsertItem(hwndListView, &lvi);
    ListView_SetItemText(hwndListView, i, 1, wrData);
    ListView_SetItemText(hwndListView, i, 2, wrData);
}

static HWND InitTestListView(HWND hwndParent, HINSTANCE hInstance, int x, int y, int w, int h)
{
    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y,
        w, h,
        hwndParent,
        NULL,
        hInstance,
        NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"Address";
    lvc.cx = 160;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Scanned Value";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &lvc);

    lvc.pszText = (LPWSTR)L"Current Value";
    lvc.cx = 160;
    lvc.iSubItem = 2;
    ListView_InsertColumn(hWndListView, 1, &lvc);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;

    for (int i = 0; i < 20; ++i)
    {
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = (LPWSTR)L"Address";
        ListView_InsertItem(hWndListView, &lvi);
        ListView_SetItemText(hWndListView, i, 1, (LPWSTR)L"0 - 이 것은 첫 번째 항목입니다.");
        ListView_SetItemText(hWndListView, i, 2, (LPWSTR)L"0 - 이 것은 두 번째 항목입니다.");
    }

    return hWndListView;
}

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

    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DummyClass";

    RegisterClassExW(&wc);

    // 반드시 로드되어야 리스트 뷰를 쓸 수 있음.
    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    HWND scanned = InitScannedListView(hWnd, hInstance, 10, 10, 525, 465);
    HWND pinned = InitPinnedListView(hWnd, hInstance, 10, 485, 525, 165);

    UINT8 temp[16] = { 12,13,45,46,78,79,123,456,789,31,64,97,21,54,87,10 };
    AddScannedAddress(scanned, 0x1234, temp, 4);
    AddPinnedAddress(pinned, 0x5678, temp, 8);

    //InitTestListView(hWnd, hInstance, 10, 10, 525, 465);
    //InitTestListView(hWnd, hInstance, 10, 485, 525, 165);

    //InitTestListView(hWnd, hInstance, 540, 0, 540, 480);
    //InitTestListView(hWnd, hInstance, 540, 480, 540, 240);
}