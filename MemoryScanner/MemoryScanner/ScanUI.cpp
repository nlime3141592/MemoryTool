#include "ScanUI.h"
#include "scanutil.h"

#pragma comment(lib, "comctl32.lib")

#include <CommCtrl.h>

static struct _TextField
{
    HWND txtLabel;
    HWND inputField;

    HWND radioLiteral;
    HWND radioHex;

    HWND radioLittle;
    HWND radioBig;

    HWND radioInclusive;
    HWND radioExclusive;
    HWND radioDisabled;
};

typedef struct _TextField TextField;

#pragma region Global Data
static HWND radioNumberInteger;
static HWND radioNumberFloat;

static HWND radioExact;
static HWND radioMinMax;

static TextField txtField_1;
static TextField txtField_2;

static HWND btnScanNew;
static HWND btnScanContinue;
static HWND btnScanUndefined;

static HWND lvScanned;
static HWND lvPinned;
#pragma endregion

#pragma region Create All HWND GUI
static void CreateScannedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y, int w, int h)
{
    lvScanned = CreateWindow(
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
    ListView_InsertColumn(lvScanned, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Scanned Value";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(lvScanned, 1, &lvc);

    lvc.pszText = (LPWSTR)L"Current Value";
    lvc.cx = 160;
    lvc.iSubItem = 2;
    ListView_InsertColumn(lvScanned, 2, &lvc);
}

static void CreatePinnedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y, int w, int h)
{
    lvPinned = CreateWindow(
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
    ListView_InsertColumn(lvPinned, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Current Value";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(lvPinned, 1, &lvc);

    lvc.pszText = (LPWSTR)L"Description";
    lvc.cx = 160;
    lvc.iSubItem = 2;
    ListView_InsertColumn(lvPinned, 2, &lvc);
}

static HWND CreateNumberRadioButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    HWND hLabel = CreateWindowEx(
        0,
        L"STATIC",
        L"Value Type",
        WS_CHILD | WS_VISIBLE,
        x, y,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    radioNumberInteger = CreateWindowEx(
        0, L"BUTTON", L"Integer",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        x, y + 25, 100, 20,
        hwndParent, (HMENU)RADIO_NUMBER_INTEGER, hInstance, NULL);

    radioNumberFloat = CreateWindowEx(
        0, L"BUTTON", L"Float",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x, y + 50, 100, 20,
        hwndParent, (HMENU)RADIO_NUMBER_FLOAT, hInstance, NULL);

    // 선택 상태로 설정
    SendDlgItemMessage(hwndParent, RADIO_NUMBER_INTEGER, BM_SETCHECK, BST_CHECKED, 0);

    return NULL;
}

static HWND CreateRangeRadioButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    HWND hLabel = CreateWindowEx(
        0,
        L"STATIC",
        L"Range",
        WS_CHILD | WS_VISIBLE,
        x, y,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    radioExact = CreateWindowEx(
        0, L"BUTTON", L"Exact Value",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        x, y + 25, 100, 20,
        hwndParent, (HMENU)RADIO_RANGE_EXACT, hInstance, NULL);

    radioMinMax = CreateWindowEx(
        0, L"BUTTON", L"Min/Max",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x, y + 50, 100, 20,
        hwndParent, (HMENU)RADIO_RANGE_MINMAX, hInstance, NULL);

    // 선택 상태로 설정
    SendDlgItemMessage(hwndParent, RADIO_RANGE_EXACT, BM_SETCHECK, BST_CHECKED, 0);

    return NULL;
}

static HWND CreateTextField(TextField* txtField, HWND hwndParent, HINSTANCE hInstance, LPCWSTR label, int x, int y, int idOffset)
{
    txtField->txtLabel = CreateWindowEx(
        0,                  // 확장 스타일
        L"STATIC",          // 클래스 이름 (라벨은 STATIC)
        label,   // 표시할 텍스트
        WS_CHILD | WS_VISIBLE,  // 기본 스타일
        x, y,             // x, y 위치
        310, 20,            // 너비, 높이
        hwndParent,         // 부모 윈도우 핸들
        NULL,               // 컨트롤 ID (필요하면 (HMENU)ID 지정)
        hInstance,          // 인스턴스 핸들
        NULL                // 추가 매개변수
    );

    txtField->inputField = CreateWindowEx(
        0,                          // 확장 스타일
        L"EDIT",                    // 클래스 이름 (텍스트 박스는 "EDIT")
        L"",                        // 초기 텍스트
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, // 스타일
        x, y + 25,                     // x, y 위치
        310, 20,                    // width, height
        hwndParent,                // 부모 윈도우 핸들
        NULL,               // 컨트롤 ID
        hInstance,                 // 인스턴스 핸들
        NULL                       // 추가 파라미터
    );

    HWND hRadioLabel_1 = CreateWindowEx(
        0,                  // 확장 스타일
        L"STATIC",          // 클래스 이름 (라벨은 STATIC)
        L"Input Format",   // 표시할 텍스트
        WS_CHILD | WS_VISIBLE,  // 기본 스타일
        x, y + 50,             // x, y 위치
        100, 20,            // 너비, 높이
        hwndParent,         // 부모 윈도우 핸들
        NULL,               // 컨트롤 ID (필요하면 (HMENU)ID 지정)
        hInstance,          // 인스턴스 핸들
        NULL                // 추가 매개변수
    );

    txtField->radioLiteral = CreateWindowEx(
        0, L"BUTTON", L"Literal",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        x, y + 75, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_LITERAL_1 + idOffset), hInstance, NULL);

    txtField->radioHex = CreateWindowEx(
        0, L"BUTTON", L"Hex Value",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x, y + 100, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_HEX_1 + idOffset), hInstance, NULL);

    HWND hRadioLabel_2 = CreateWindowEx(
        0,                  // 확장 스타일
        L"STATIC",          // 클래스 이름 (라벨은 STATIC)
        L"Endianness",   // 표시할 텍스트
        WS_CHILD | WS_VISIBLE,  // 기본 스타일
        x + 105, y + 50,             // x, y 위치
        100, 20,            // 너비, 높이
        hwndParent,         // 부모 윈도우 핸들
        NULL,               // 컨트롤 ID (필요하면 (HMENU)ID 지정)
        hInstance,          // 인스턴스 핸들
        NULL                // 추가 매개변수
    );

    txtField->radioLittle = CreateWindowEx(
        0, L"BUTTON", L"Little",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        x + 105, y + 75, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_LITTLE_1 + idOffset), hInstance, NULL);

    txtField->radioBig = CreateWindowEx(
        0, L"BUTTON", L"Big",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x + 105, y + 100, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_BIG_1 + idOffset), hInstance, NULL);

    HWND hRadioLabel_3 = CreateWindowEx(
        0,                  // 확장 스타일
        L"STATIC",          // 클래스 이름 (라벨은 STATIC)
        L"Range Border",   // 표시할 텍스트
        WS_CHILD | WS_VISIBLE,  // 기본 스타일
        x + 210, y + 50,             // x, y 위치
        100, 20,            // 너비, 높이
        hwndParent,         // 부모 윈도우 핸들
        NULL,               // 컨트롤 ID (필요하면 (HMENU)ID 지정)
        hInstance,          // 인스턴스 핸들
        NULL                // 추가 매개변수
    );

    txtField->radioInclusive = CreateWindowEx(
        0, L"BUTTON", L"Inclusive",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        x + 210, y + 75, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_INCLUSIVE_1 + idOffset), hInstance, NULL);

    txtField->radioExclusive = CreateWindowEx(
        0, L"BUTTON", L"Exclusive",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x + 210, y + 100, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_EXCLUSIVE_1 + idOffset), hInstance, NULL);

    txtField->radioDisabled = CreateWindowEx(
        0, L"BUTTON", L"Disabled",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        x + 210, y + 125, 100, 20,
        hwndParent, (HMENU)(RADIO_FIELD_DISABLED_1 + idOffset), hInstance, NULL);

    // 선택 상태로 설정
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITERAL_1 + idOffset), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITTLE_1 + idOffset), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_INCLUSIVE_1 + idOffset), BM_SETCHECK, BST_CHECKED, 0);

    return NULL;
}

static void CreateNavigationButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    btnScanNew = CreateWindow(
        L"BUTTON",
        L"Scan New",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y,
        100, 20,
        hwndParent,
        (HMENU)NAVIGATOR_SCAN_NEW,
        (HINSTANCE)hInstance,
        NULL
    );

    btnScanContinue = CreateWindow(
        L"BUTTON",
        L"Scan Continue",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x + 105, y,
        100, 20,
        hwndParent,
        (HMENU)NAVIGATOR_SCAN_CONTINUE,
        (HINSTANCE)hInstance,
        NULL
    );

    btnScanUndefined = CreateWindow(
        L"BUTTON",
        L"<Undefined>",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x + 210, y,
        100, 20,
        hwndParent,
        (HMENU)NAVIGATOR_SCAN_UNDEFINED,
        (HINSTANCE)hInstance,
        NULL
    );
}
#pragma endregion

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

void InitScanUI(HWND hWnd)
{
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

    CreateScannedListView(hWnd, hInstance, 10, 10, 525, 465);
    CreatePinnedListView(hWnd, hInstance, 10, 485, 525, 165);
    CreateNumberRadioButton(hWnd, hInstance, 545, 10);
    CreateRangeRadioButton(hWnd, hInstance, 545, 180);
    CreateTextField(&txtField_1, hWnd, hInstance, L"Min Value", 670, 10, 0);
    CreateTextField(&txtField_2, hWnd, hInstance, L"Max Value", 670, 180, 10000);
    CreateNavigationButton(hWnd, hInstance, 670, 455);

    // 리스트 뷰에 데이터를 추가하는 방법을 보여줌.
    //UINT8 temp[16] = { 12,13,45,46,78,79,123,456,789,31,64,97,21,54,87,10 };
    //AddScannedAddress(lvScanned, 0x1234, temp, 4);
    //AddPinnedAddress(lvPinned, 0x5678, temp, 8);
}