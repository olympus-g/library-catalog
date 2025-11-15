// Library.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Library.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

struct Book {
    wstring title;
    wstring author;
    int year;
    wstring genre;
    wstring isbn;
    bool favorite = false;
};

vector<Book> library;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    LibraryCatalog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    EditBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void                RefreshBookList(HWND hDlg);
INT_PTR CALLBACK    AddBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


void SaveLibraryToFile()
{
    std::wofstream file(L"library.txt");
    if (!file.is_open()) return;

    for (const auto& b : library)
    {
        file
            << b.title << L"|"
            << b.author << L"|"
            << b.year << L"|"
            << b.genre << L"|"
            << b.isbn << L"|"
            << (b.favorite ? 1 : 0) << L"\n";
    }
}

void LoadLibraryFromFile()
{
    std::wifstream file(L"library.txt");
    if (!file.is_open()) return;

    library.clear();
    std::wstring line;

    while (std::getline(file, line))
    {
        std::wstringstream ss(line);
        Book b;
        std::wstring fav;

        std::getline(ss, b.title, L'|');
        std::getline(ss, b.author, L'|');

        std::wstring yearStr;
        std::getline(ss, yearStr, L'|');
        b.year = _wtoi(yearStr.c_str());

        std::getline(ss, b.genre, L'|');
        std::getline(ss, b.isbn, L'|');
        std::getline(ss, fav, L'|');

        b.favorite = (fav == L"1");

        library.push_back(b);
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LIBRARY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    LoadLibraryFromFile();


    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LIBRARY));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LIBRARY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LIBRARY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

void RefreshBookList(HWND hDlg) {
    HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    for (const auto& book : library)
    {
        wstring line;
        if (book.favorite)
            line = L"⭐ " + book.title + L" - " + book.author;
        else
            line = book.title + L" - " + book.author;

        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }

    wstring count = L"Total books: " + to_wstring(library.size());
    SetDlgItemText(hDlg, IDC_TOTALBOOKS, count.c_str());
}


INT_PTR CALLBACK AddBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            Book newBook;
            wchar_t buffer[256];

            GetDlgItemText(hDlg, IDC_EDIT_TITLE, buffer, 256);
            newBook.title = buffer;

            GetDlgItemText(hDlg, IDC_EDIT_AUTHOR, buffer, 256);
            newBook.author = buffer;

            GetDlgItemText(hDlg, IDC_EDIT_GENRE, buffer, 256);
            newBook.genre = buffer;

            GetDlgItemText(hDlg, IDC_EDIT_ISBN, buffer, 256);
            newBook.isbn = buffer;

            BOOL success = FALSE;
            int year = GetDlgItemInt(hDlg, IDC_EDIT_YEAR, &success, FALSE);
            newBook.year = success ? year : 0;

            for (const auto& b : library) {
                if (b.isbn == newBook.isbn) {
                    MessageBox(hDlg, L"Book with this ISBN already exists.", L"Duplicate ISBN", MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
            }
            library.push_back(newBook);
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK EditBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int selectedIndex = -1;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_TITLE);

        for (size_t i = 0; i < library.size(); i++)
        {
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)library[i].title.c_str());
        }

        selectedIndex = -1;
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_COMBO_TITLE:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_TITLE);
                selectedIndex = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);

                if (selectedIndex >= 0)
                {
                    const Book& b = library[selectedIndex];

                    SetDlgItemText(hDlg, IDC_EDIT_TITLE, b.title.c_str());
                    SetDlgItemText(hDlg, IDC_EDIT_AUTHOR, b.author.c_str());
                    SetDlgItemInt(hDlg, IDC_EDIT_YEAR, b.year, FALSE);
                    SetDlgItemText(hDlg, IDC_EDIT_GENRE, b.genre.c_str());
                    SetDlgItemText(hDlg, IDC_EDIT_ISBN, b.isbn.c_str());
                }
            }
            return (INT_PTR)TRUE;

        case IDOK:
            if (selectedIndex >= 0)
            {
                Book& b = library[selectedIndex];
                wchar_t buffer[256];

                GetDlgItemText(hDlg, IDC_EDIT_TITLE, buffer, 256);
                b.title = buffer;

                GetDlgItemText(hDlg, IDC_EDIT_AUTHOR, buffer, 256);
                b.author = buffer;

                BOOL success = FALSE;
                int year = GetDlgItemInt(hDlg, IDC_EDIT_YEAR, &success, FALSE);
                b.year = success ? year : 0;

                GetDlgItemText(hDlg, IDC_EDIT_GENRE, buffer, 256);
                b.genre = buffer;

                GetDlgItemText(hDlg, IDC_EDIT_ISBN, buffer, 256);
                b.isbn = buffer;
            }

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }

    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK LibraryCatalog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        RefreshBookList(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_BTN_ADD:
            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_DIALOG), hDlg, AddBookDlg) == IDOK)
            {
                RefreshBookList(hDlg);
            }
            return (INT_PTR)TRUE;

        case IDC_BTN_EDIT:
            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIALOG), hDlg, EditBookDlg) == IDOK)
            {
                RefreshBookList(hDlg);
            }
            return (INT_PTR)TRUE;
        case IDC_BTN_DELETE:
        {
            HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (sel == LB_ERR)
            {
                MessageBox(hDlg, L"Please select a book to delete.", L"No selection", MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            if (MessageBox(hDlg, L"Are you sure you want to delete this book?", L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                library.erase(library.begin() + sel);
                RefreshBookList(hDlg);
            }

            return (INT_PTR)TRUE;
        }
        case IDC_BTN_FAV:
        {
            HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (sel == LB_ERR)
            {
                MessageBox(hDlg, L"Please select a book to favorite.", L"No Selection", MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            library[sel].favorite = !library[sel].favorite;

            RefreshBookList(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        }
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_LIB:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_LIBRARY_DIALOG), hWnd, LibraryCatalog);
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        SaveLibraryToFile();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
