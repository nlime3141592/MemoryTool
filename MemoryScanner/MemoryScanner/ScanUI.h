#pragma once

#include "framework.h"

#pragma comment(lib, "comctl32.lib")

#include <CommCtrl.h>
#include <Windows.h>
#include <Psapi.h>
#include <string>

#include "scan.h"

#define MY_BUTTON_ID 999

#define RADIO_NUMBER_INTEGER 1001
#define RADIO_NUMBER_FLOAT 1002

#define RADIO_RANGE_EXACT 1011
#define RADIO_RANGE_MINMAX 1012



#define RADIO_FIELD_LITERAL_1 11001
#define RADIO_FIELD_HEX_1 11002

#define RADIO_FIELD_LITTLE_1 11011
#define RADIO_FIELD_BIG_1 11012

#define RADIO_FIELD_INCLUSIVE_1 11021
#define RADIO_FIELD_EXCLUSIVE_1 11022
#define RADIO_FIELD_DISABLED_1 11023



#define RADIO_FIELD_LITERAL_2 21001
#define RADIO_FIELD_HEX_2 21002

#define RADIO_FIELD_LITTLE_2 21011
#define RADIO_FIELD_BIG_2 21012

#define RADIO_FIELD_INCLUSIVE_2 21021
#define RADIO_FIELD_EXCLUSIVE_2 21022
#define RADIO_FIELD_DISABLED_2 21023



#define NAVIGATOR_SCAN_NEW 2001
#define NAVIGATOR_SCAN_CONTINUE 2002



#define PID_REFRESH 2011
#define PID_SELECT 2012
#define PID_CLOSE 2013



#define MOD_MODIFY 2021
#define MOD_ADDRESS 2022
#define MOD_VALUE 2023



#pragma region Structures
static struct _TextField
{
    HWND txtLabel;
    HWND inputField;

    HWND radioLabelInput;
    HWND radioLiteral;
    HWND radioHex;

    HWND radioLabelEndianness;
    HWND radioLittle;
    HWND radioBig;

    HWND radioLabelRange;
    HWND radioInclusive;
    HWND radioExclusive;
    HWND radioDisabled;
};
#pragma endregion



#pragma region Type Definition
typedef struct _TextField TextField;
#pragma endregion




#pragma region Event Handlers
static void ShowTextField(TextField* txtField, int nCmdShow);

void OnRadioIntegerSelected();
void OnRadioFloatSelected();

void OnRadioExactSelected();
void OnRadioMinMaxSelected();

void OnPidRefreshClicked();
void OnPidSelectClicked();
void OnPidCloseClicked();

void OnScanNewClicked();
void OnScanContinueClicked();

void OnNotifyListView(LPARAM lParam);

void OnModModifyClicked();
#pragma endregion

void AddScannedAddress(HWND hwndListView, UINT64 address, LPCVOID data, SIZE_T dataLength);

void InitScanUI(HWND hWnd);