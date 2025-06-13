#include "ScanUI.h"
#include "scanutil.h"

#pragma region Global Data
static HWND radioNumberInteger;
static HWND radioNumberFloat;

static HWND radioExact;
static HWND radioMinMax;

static TextField txtField_1;
static TextField txtField_2;

static HWND btnScanNew;
static HWND btnScanContinue;

static HWND btnProcRefresh;
static HWND btnProcSelect;
static HWND btnProcClose;

static HWND lvScanned;
static HWND lvPinned;
static HWND lvPid;

static HWND btnMod;
static HWND txtModAddress;
static HWND txtModValue;

static int lvidxScanned = -1;
static int lvidxPinned = -1;
static int lvidxPid = -1;

static HANDLE hProcess = NULL;

static int ridxNumber = 0;
static int ridxRange = 0;

static int ridxFormat1 = 0;
static int ridxEndian1 = 0;

PrimitiveData srcData1 = { 0 };
PrimitiveData srcData2 = { 0 };

PrimitiveData srcModAddress = { 0 };
PrimitiveData srcModValue = { 0 };
#pragma endregion

#pragma region Create All HWND GUI
static void CreateScannedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    lvScanned = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y,
        525, 465,
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

    ListView_SetExtendedListViewStyle(lvScanned, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

static void CreatePinnedListView(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    lvPinned = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y,
        525, 165,
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

    ListView_SetExtendedListViewStyle(lvPinned, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

static void CreateNumberRadioButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    HWND hLabel = CreateWindowEx(
        0,
        L"STATIC",
        L"Value Type",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
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
}

static void CreateRangeRadioButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    HWND hLabel = CreateWindowEx(
        0,
        L"STATIC",
        L"Range",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
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
}

static void CreateTextField(TextField* txtField, HWND hwndParent, HINSTANCE hInstance, LPCWSTR label, int x, int y, int idOffset)
{
    txtField->txtLabel = CreateWindowEx(
        0,
        L"STATIC",
        label,
        WS_CHILD | WS_VISIBLE,
        x, y,
        310, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    txtField->inputField = CreateWindowEx(
        0,
        L"EDIT",
        L"", // 초기 텍스트
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        x, y + 25,
        310, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    txtField->radioLabelInput = CreateWindowEx(
        0,
        L"STATIC",
        L"Input Format",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y + 50,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
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

    txtField->radioLabelEndianness = CreateWindowEx(
        0,
        L"STATIC",
        L"Endianness",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x + 105, y + 50,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
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

    txtField->radioLabelRange = CreateWindowEx(
        0,
        L"STATIC",
        L"Range Border",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x + 210, y + 50,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
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
}

static void CreatePidMonitor(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    HWND hLabel = CreateWindowEx(
        0,
        L"STATIC",
        L"PID Monitor",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y,
        100, 25,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    lvPid = CreateWindow(
        WC_LISTVIEW,
        L"",
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
        x, y + 35,
        435, 165,
        hwndParent,
        NULL,
        hInstance,
        NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"Process Name";
    lvc.cx = 225;
    lvc.iSubItem = 0;
    ListView_InsertColumn(lvPid, 0, &lvc);

    lvc.pszText = (LPWSTR)L"PID";
    lvc.cx = 160;
    lvc.iSubItem = 1;
    ListView_InsertColumn(lvPid, 1, &lvc);

    btnProcRefresh = CreateWindow(
        L"BUTTON",
        L"Refresh",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x + 125, y,
        100, 25,
        hwndParent,
        (HMENU)PID_REFRESH,
        (HINSTANCE)hInstance,
        NULL
    );

    btnProcSelect = CreateWindow(
        L"BUTTON",
        L"Select",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x + 230, y,
        100, 25,
        hwndParent,
        (HMENU)PID_SELECT,
        (HINSTANCE)hInstance,
        NULL
    );

    btnProcClose = CreateWindow(
        L"BUTTON",
        L"Close",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x + 335, y,
        100, 25,
        hwndParent,
        (HMENU)PID_CLOSE,
        (HINSTANCE)hInstance,
        NULL
    );

    ListView_SetExtendedListViewStyle(lvPid, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

static void CreateNavigationButton(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    btnScanNew = CreateWindow(
        L"BUTTON",
        L"Scan New",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y,
        100, 25,
        hwndParent,
        (HMENU)NAVIGATOR_SCAN_NEW,
        (HINSTANCE)hInstance,
        NULL
    );

    btnScanContinue = CreateWindow(
        L"BUTTON",
        L"Scan Continue",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y + 30,
        100, 25,
        hwndParent,
        (HMENU)NAVIGATOR_SCAN_CONTINUE,
        (HINSTANCE)hInstance,
        NULL
    );
}

static void CreateModifyingNavigator(HWND hwndParent, HINSTANCE hInstance, int x, int y)
{
    btnMod = CreateWindow(
        L"BUTTON",
        L"Modify",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y + 60,
        100, 25,
        hwndParent,
        (HMENU)MOD_MODIFY,
        (HINSTANCE)hInstance,
        NULL
    );

    HWND labelAddress = CreateWindowEx(
        0,
        L"STATIC",
        L"Target Address",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    txtModAddress = CreateWindowEx(
        0,
        L"EDIT",
        L"", // 초기 텍스트
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        x + 105, y,
        205, 20,
        hwndParent,
        (HMENU)MOD_ADDRESS,
        hInstance,
        NULL
    );

    HWND labelValue = CreateWindowEx(
        0,
        L"STATIC",
        L"Value",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y + 30,
        100, 20,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    txtModValue = CreateWindowEx(
        0,
        L"EDIT",
        L"", // 초기 텍스트
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        x + 105, y + 30,
        205, 20,
        hwndParent,
        (HMENU)MOD_VALUE,
        hInstance,
        NULL
    );
}
#pragma endregion

#pragma region Event Handlers
static void OnInitializeScanUI(HWND hwndParent)
{
    // 선택 상태로 설정
    SendDlgItemMessage(hwndParent, RADIO_NUMBER_INTEGER, BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, RADIO_RANGE_EXACT, BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITERAL_1), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITTLE_1), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_INCLUSIVE_1), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITERAL_2), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_LITTLE_2), BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(hwndParent, (RADIO_FIELD_INCLUSIVE_2), BM_SETCHECK, BST_CHECKED, 0);

    // 선택 상태로 설정 후 이벤트 Publishing
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_NUMBER_INTEGER, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_NUMBER_INTEGER));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_RANGE_EXACT, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_RANGE_EXACT));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_LITERAL_1, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_LITERAL_1));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_LITTLE_1, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_LITTLE_1));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_INCLUSIVE_1, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_INCLUSIVE_1));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_LITERAL_2, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_LITERAL_2));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_LITTLE_2, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_LITTLE_2));
    SendMessage(hwndParent, WM_COMMAND, MAKELONG(RADIO_FIELD_INCLUSIVE_2, BN_CLICKED), (LPARAM)GetDlgItem(hwndParent, RADIO_FIELD_INCLUSIVE_2));

}

static void ShowTextField(TextField* txtField, int nCmdShow)
{
    ShowWindow(txtField->txtLabel, nCmdShow);
    ShowWindow(txtField->inputField, nCmdShow);

    ShowWindow(txtField->radioLabelInput, nCmdShow);
    ShowWindow(txtField->radioLiteral, nCmdShow);
    ShowWindow(txtField->radioHex, nCmdShow);

    ShowWindow(txtField->radioLabelEndianness, nCmdShow);
    ShowWindow(txtField->radioLittle, nCmdShow);
    ShowWindow(txtField->radioBig, nCmdShow);
    
    ShowWindow(txtField->radioLabelRange, nCmdShow);
    ShowWindow(txtField->radioInclusive, nCmdShow);
    ShowWindow(txtField->radioExclusive, nCmdShow);
    ShowWindow(txtField->radioDisabled, nCmdShow);
}

void OnRadioIntegerSelected()
{
    ridxNumber = 0;
}

void OnRadioFloatSelected()
{
    ridxNumber = 1;
}

void OnRadioExactSelected()
{
    SetWindowText(txtField_1.txtLabel, L"Value");

    ShowTextField(&txtField_2, SW_HIDE);

    ShowWindow(txtField_1.radioLabelRange, SW_HIDE);
    ShowWindow(txtField_1.radioInclusive, SW_HIDE);
    ShowWindow(txtField_1.radioExclusive, SW_HIDE);
    ShowWindow(txtField_1.radioDisabled, SW_HIDE);

    ridxRange = 0;
}

void OnRadioMinMaxSelected()
{
    SetWindowText(txtField_1.txtLabel, L"Min Value");
    SetWindowText(txtField_2.txtLabel, L"Max Value");

    ShowTextField(&txtField_2, SW_SHOW);

    ShowWindow(txtField_1.radioLabelRange, SW_SHOW);
    ShowWindow(txtField_1.radioInclusive, SW_SHOW);
    ShowWindow(txtField_1.radioExclusive, SW_SHOW);
    ShowWindow(txtField_1.radioDisabled, SW_SHOW);

    ridxRange = 1;
}

void OnPidRefreshClicked()
{
    ListView_DeleteAllItems(lvPid);

    // 리스트 뷰에 데이터를 추가하는 방법을 보여줌.
    //UINT8 temp[16] = { 12,13,45,46,78,79,123,456,789,31,64,97,21,54,87,10 };
    //AddScannedAddress(lvScanned, 0x1234, temp, 4);
    //AddPinnedAddress(lvPinned, 0x5678, temp, 8);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;

    DWORD processes[1024];
    DWORD needed;
    DWORD count;

    if (!EnumProcesses(processes, sizeof(processes), &needed))
    {
        // EnumProcesses 실패
        return;
    }

    count = needed / sizeof(DWORD);

    //std::string strcnt = std::to_string(count);
    //MessageBoxA(lvPid, strcnt.c_str(), strcnt.c_str(), MB_OK);

    for (DWORD i = 0; i < count; ++i)
    {
        DWORD pid = processes[i];

        if (pid == 0)
            continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        if (hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;
            TCHAR szProcessName[256] = TEXT("<unknown>");

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
            }

            int j = ListView_GetItemCount(lvPid);
            lvi.iItem = j;
            lvi.iSubItem = 0;
            lvi.pszText = szProcessName;

            WCHAR pidData[16] = L"";
            swprintf(pidData, 16, L"%u", pid);

            ListView_InsertItem(lvPid, &lvi);
            ListView_SetItemText(lvPid, j, 1, pidData);

            CloseHandle(hProcess);
        }
    }
}

void OnPidSelectClicked()
{
    WCHAR pidStr[64];
    ListView_GetItemText(lvPid, lvidxPid, 1, pidStr, 64);
    MessageBoxW(lvPid, pidStr, L"선택된 PID", MB_OK);

    int pid = _wtoi(pidStr);

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (hProc == NULL)
    {
        // 프로세스를 열 수 없음.
        return;
    }

    hProcess = hProc;

    EnableWindow(btnProcClose, TRUE);
    EnableWindow(btnProcSelect, FALSE);
}

void OnPidCloseClicked()
{
    hProcess = NULL;

    if (hProcess == NULL)
    {
        EnableWindow(btnProcClose, FALSE);
        EnableWindow(btnProcSelect, FALSE);
    }
}

void OnModModifyClicked()
{
    WCHAR dataBuffer1[256];
    WCHAR dataBuffer2[256];

    GetWindowText(txtModAddress, dataBuffer1, 255);
    GetWindowText(txtModValue, dataBuffer2, 255);

    srcModAddress.udata64 = wcstoull(dataBuffer1, NULL, 16); // 16진수 입력임.
    srcModValue.data32 = _wtoi(dataBuffer2);

    Modify(hProcess, (LPVOID)(srcModAddress.udata64), (LPCVOID)(&srcModValue), 4);
}

static void ReadFromTextField()
{
    WCHAR dataBuffer1[256];
    WCHAR dataBuffer2[256];

    GetWindowText(txtField_1.inputField, dataBuffer1, 255);
    GetWindowText(txtField_2.inputField, dataBuffer2, 255);

    srcData1.data32 = _wtoi(dataBuffer1);
    srcData2.data32 = _wtoi(dataBuffer2);
}

void OnScanNewClicked()
{
    if (ridxNumber == 0)
    {
        ReadFromTextField();

        SIZE_T length = -1;

        if (ridxRange == 0)
        {
            length = ScanIntegerNewly(hProcess, (LPCVOID)(&srcData1), (LPCVOID)(&srcData1), 4, ENDIAN_LITTLE, SIGN_SIGNED, TRUE, TRUE, TRUE);
        }
        else if (ridxRange == 1)
        {
            length = ScanIntegerNewly(hProcess, (LPCVOID)(&srcData1), (LPCVOID)(&srcData2), 4, ENDIAN_LITTLE, SIGN_SIGNED, TRUE, TRUE, TRUE);
        }

        std::string cntstr = std::to_string(length);
        MessageBoxA(lvPid, cntstr.c_str(), "발견한 메모리 주소 갯수", MB_OK);

        ListView_DeleteAllItems(lvScanned);

        if (length == 0)
        {
            // 찾은 메모리 공간이 없음.
            MessageBoxW(lvScanned, L"메모리 주소를 찾을 수 없습니다.", L"Error", 0);

            return;
        }

        for (SIZE_T i = 0; i < length; ++i)
        {
            // 리스트에 값 추가
            AddScannedAddress(lvScanned, (UINT64)adrBuffer[i], (LPCVOID)(&srcData1), 4);
        }
    }

    if (ridxNumber == 1)
    {
        //ScanFloatNewly(hProcess, inputData1, inputData2, 4, ridxEndian1, SIGN_SIGNED, ridxEndian1, ridxEndian1);
    }
}

void OnScanContinueClicked()
{
    if (ridxNumber == 0)
    {
        ReadFromTextField();

        SIZE_T length = -1;

        if (ridxRange == 0)
        {
            length = ScanIntegerContinuously(hProcess, (LPCVOID)(&srcData1), (LPCVOID)(&srcData1), 4, ENDIAN_LITTLE, SIGN_SIGNED, TRUE, TRUE, TRUE);
        }
        else if (ridxRange == 1)
        {
            length = ScanIntegerContinuously(hProcess, (LPCVOID)(&srcData1), (LPCVOID)(&srcData2), 4, ENDIAN_LITTLE, SIGN_SIGNED, TRUE, TRUE, TRUE);
        }

        std::string cntstr = std::to_string(length);
        MessageBoxA(lvPid, cntstr.c_str(), "발견한 메모리 주소 갯수", MB_OK);

        ListView_DeleteAllItems(lvScanned);

        if (length == 0)
        {
            // 찾은 메모리 공간이 없음.
            MessageBoxW(lvScanned, L"메모리 주소를 찾을 수 없습니다.", L"Error", 0);

            return;
        }

        for (SIZE_T i = 0; i < length; ++i)
        {
            // 리스트에 값 추가
            AddScannedAddress(lvScanned, (UINT64)adrBuffer[i], (LPCVOID)(&srcData1), 4);
        }
    }
}

static void OnNotifyScannedListView(LPNMHDR pnmh, NMLISTVIEW* pnmv)
{
    //MessageBox(pnmh->hwndFrom, L"ok1", L"ok01", 0);

    if (!(pnmv->uChanged & LVIF_STATE))
        return;

    BOOL wasSelected = (pnmv->uOldState & LVIS_SELECTED);
    BOOL isSelected = (pnmv->uNewState & LVIS_SELECTED);

    if (!wasSelected && isSelected)
    {
        // 새로 선택된 항목에 대해서만 처리
        lvidxScanned = pnmv->iItem;

        WCHAR pidStr[64];
        ListView_GetItemText(lvPid, lvidxScanned, 1, pidStr, 64); // PID 열 = 1

        //MessageBoxW(pnmh->hwndFrom, pidStr, L"선택된 PID", MB_OK);

        EnableWindow(btnProcSelect, TRUE);
    }
    else if (wasSelected && !isSelected)
    {
        // 선택 해제된 경우
        lvidxScanned = -1;
        EnableWindow(btnProcSelect, FALSE);
    }
}

static void OnNotifyPinnedListView(LPNMHDR pnmh, NMLISTVIEW* pnmv)
{
    //MessageBox(pnmh->hwndFrom, L"ok2", L"ok02", 0);

    if (!(pnmv->uChanged & LVIF_STATE))
        return;

    BOOL wasSelected = (pnmv->uOldState & LVIS_SELECTED);
    BOOL isSelected = (pnmv->uNewState & LVIS_SELECTED);

    if (!wasSelected && isSelected)
    {
        // 새로 선택된 항목에 대해서만 처리
        lvidxPinned = pnmv->iItem;

        WCHAR pidStr[64];
        ListView_GetItemText(lvPid, lvidxPinned, 1, pidStr, 64); // PID 열 = 1

        //MessageBoxW(pnmh->hwndFrom, pidStr, L"선택된 PID", MB_OK);

        EnableWindow(btnProcSelect, TRUE);
    }
    else if (wasSelected && !isSelected)
    {
        // 선택 해제된 경우
        lvidxPinned = -1;
        EnableWindow(btnProcSelect, FALSE);
    }
}

static void OnNotifyPidListView(LPNMHDR pnmh, NMLISTVIEW* pnmv)
{
    if (!(pnmv->uChanged & LVIF_STATE))
        return;

    BOOL wasSelected = (pnmv->uOldState & LVIS_SELECTED);
    BOOL isSelected = (pnmv->uNewState & LVIS_SELECTED);

    if (!wasSelected && isSelected)
    {
        // 새로 선택된 항목에 대해서만 처리
        lvidxPid = pnmv->iItem;

        WCHAR pidStr[64];
        ListView_GetItemText(lvPid, lvidxPid, 1, pidStr, 64); // PID 열 = 1

        //MessageBoxW(pnmh->hwndFrom, pidStr, L"선택된 PID", MB_OK);

        EnableWindow(btnProcSelect, TRUE);
    }
    else if (wasSelected && !isSelected)
    {
        // 선택 해제된 경우
        lvidxPid = -1;
        EnableWindow(btnProcSelect, FALSE);
    }
}

void OnNotifyListView(LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;
    HWND source = pnmh->hwndFrom;

    if (source == lvScanned && pnmh->code == LVN_ITEMCHANGED)
    {
        OnNotifyScannedListView(pnmh, (NMLISTVIEW*)lParam);
    }
    if (source == lvPinned && pnmh->code == LVN_ITEMCHANGED)
    {
        OnNotifyPinnedListView(pnmh, (NMLISTVIEW*)lParam);
    }
    if (source == lvPid && pnmh->code == LVN_ITEMCHANGED)
    {
        OnNotifyPidListView(pnmh, (NMLISTVIEW*)lParam);
    }
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

void InitScanUI(HWND hwnd)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

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
    GetClientRect(hwnd, &rcClient);

    CreateScannedListView(hwnd, hInstance, 10, 10);
    CreatePinnedListView(hwnd, hInstance, 10, 485);
    CreateNumberRadioButton(hwnd, hInstance, 545, 10);
    CreateRangeRadioButton(hwnd, hInstance, 545, 180);
    CreateTextField(&txtField_1, hwnd, hInstance, L"Min Value", 670, 10, 0);
    CreateTextField(&txtField_2, hwnd, hInstance, L"Max Value", 670, 180, 10000);
    CreateNavigationButton(hwnd, hInstance, 545, 350);
    CreateModifyingNavigator(hwnd, hInstance, 670, 350);
    CreatePidMonitor(hwnd, hInstance, 545, 450);

    OnInitializeScanUI(hwnd);

    EnableWindow(btnProcClose, FALSE);
    EnableWindow(btnProcSelect, FALSE);

    // 리스트 뷰에 데이터를 추가하는 방법을 보여줌.
    //UINT8 temp[16] = { 12,13,45,46,78,79,123,456,789,31,64,97,21,54,87,10 };
    //AddScannedAddress(lvScanned, 0x1234, temp, 4);
    //AddPinnedAddress(lvPinned, 0x5678, temp, 8);
}