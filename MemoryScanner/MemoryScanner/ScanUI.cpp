#include "ScanUI.h"

#pragma comment(lib, "comctl32.lib")

#include <CommCtrl.h>

static HWND InitTestListView(HWND hwndParent, HINSTANCE hInstance, int w, int h)
{
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);

    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        0, 0,
        w, h,
        //rcClient.right - rcClient.left,
        //rcClient.bottom - rcClient.top,
        hwndParent,
        NULL,
        hInstance,
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

    return hWndListView;
}

static HWND InitDataFormatContainer(HWND hwndParent, HINSTANCE hInstance)
{
    HWND container = CreateWindow(
        0,
        L"DummyClass", // dummy
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        540, 480, 540, 240,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    InitTestListView(container, hInstance, 540, 240);

    return container;
}

static HWND InitTargetDataContainer(HWND hwndParent, HINSTANCE hInstance)
{
    HWND container = CreateWindow(
        0,
        L"DummyClass", // dummy
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        540, 0, 540, 480,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    InitTestListView(container, hInstance, 540, 480);

    return container;
}

static HWND InitPinnedDataContainer(HWND hwndParent, HINSTANCE hInstance)
{
    HWND container = CreateWindow(
        0,
        L"DummyClass", // dummy
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        0, 540, 540, 240,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    InitTestListView(container, hInstance, 540, 240);

    return container;
}

static HWND InitScannedDataContainer(HWND hwndParent, HINSTANCE hInstance)
{
    HWND container = CreateWindow(
        0,
        L"DummyClass", // dummy
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        0, 0, 540, 480,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    InitTestListView(container, hInstance, 540, 480);

    return container;
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

    InitScannedDataContainer(hWnd, hInstance);
    //InitPinnedDataContainer(hWnd, hInstance);
    //InitTargetDataContainer(hWnd, hInstance);
    //InitDataFormatContainer(hWnd, hInstance);
}