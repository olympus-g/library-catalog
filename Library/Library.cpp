// Library.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Library.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

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
const vector<wstring> GENRES = {
    L"Fantasy",
    L"Romance",
    L"Science Fiction",
    L"Literary Fiction",
    L"Horror",
    L"Nonfiction",
    L"Historical",
    L"Mystery",
    L"Thriller",
    L"Biography"
};

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



void SortLibrary(int mode)
{
    switch (mode)
    {
    case 0: // Title ascending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.title < b.title;
            });
        break;

    case 1: // Title descending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.title > b.title;
            });
        break;

    case 2: // Author ascending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.author < b.author;
            });
        break;

    case 3: // Author descending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.author > b.author;
            });
        break;

    case 4: // Year ascending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.year < b.year;
            });
        break;

    case 5: // Year descending
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                return a.year > b.year;
            });
        break;

    case 6: // Favorites first, then by title
        sort(library.begin(), library.end(),
            [](const Book& a, const Book& b) {
                if (a.favorite != b.favorite)
                    return a.favorite > b.favorite;
                return a.title < b.title;
            });
        break;
    }

}

int GetRealIndexFromListBox(HWND hList, int selection)
{
    if (selection == LB_ERR) return -1;
    return (int)SendMessage(hList, LB_GETITEMDATA, selection, 0);
}

void RefreshBookList(HWND hDlg) {
    HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    for (size_t i = 0; i < library.size(); i++)
    {
        const Book& b = library[i];
        wstring line = (b.favorite ? L"⭐ " : L"") +
            b.title + L" | " + b.author + L" | " +
            to_wstring(b.year) + L" | " + b.genre + L" | " + b.isbn;

        int idx = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        SendMessage(hList, LB_SETITEMDATA, idx, (LPARAM)i);
    }

    SetDlgItemText(hDlg, IDC_TOTALBOOKS,
        (L"Total books: " + to_wstring(library.size())).c_str());
}

void PopulateGenreCombo(HWND combo)
{
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    for (const auto& g : GENRES)
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)g.c_str());
}


INT_PTR CALLBACK AddBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        PopulateGenreCombo(GetDlgItem(hDlg, IDC_GENRE));
        SendMessage(GetDlgItem(hDlg, IDC_GENRE), CB_SETCURSEL, 0, 0);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }

        if (LOWORD(wParam) == IDOK)
        {
            Book newBook;
            wchar_t buffer[256];

            GetDlgItemText(hDlg, IDC_EDIT_TITLE, buffer, 256);
            newBook.title = buffer;

            GetDlgItemText(hDlg, IDC_EDIT_AUTHOR, buffer, 256);
            newBook.author = buffer;

            int genreSel = (int)SendMessage(GetDlgItem(hDlg, IDC_GENRE), CB_GETCURSEL, 0, 0);
            if (genreSel >= 0) newBook.genre = GENRES[genreSel];

            GetDlgItemText(hDlg, IDC_EDIT_ISBN, buffer, 256);
            newBook.isbn = buffer;

            BOOL success;
            newBook.year = GetDlgItemInt(hDlg, IDC_EDIT_YEAR, &success, FALSE);

            for (const auto& b : library)
            {
                if (b.isbn == newBook.isbn)
                {
                    MessageBox(hDlg, L"Book with this ISBN already exists.", L"Duplicate ISBN", MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
            }

            library.push_back(newBook);
            EndDialog(hDlg, IDOK);
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
        HWND hComboTitle = GetDlgItem(hDlg, IDC_COMBO_TITLE);
        for (size_t i = 0; i < library.size(); i++)
        {
            int idx = (int)SendMessage(hComboTitle, CB_ADDSTRING, 0, (LPARAM)library[i].title.c_str());
            SendMessage(hComboTitle, CB_SETITEMDATA, idx, (LPARAM)i);
        }

        PopulateGenreCombo(GetDlgItem(hDlg, IDC_GENRE));
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
                int comboIndex = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                selectedIndex = (int)SendMessage(hCombo, CB_GETITEMDATA, comboIndex, 0);

                if (selectedIndex >= 0 && selectedIndex < (int)library.size())
                {
                    const Book& b = library[selectedIndex];

                    SetDlgItemText(hDlg, IDC_EDIT_TITLE, b.title.c_str());
                    SetDlgItemText(hDlg, IDC_EDIT_AUTHOR, b.author.c_str());
                    SetDlgItemInt(hDlg, IDC_EDIT_YEAR, b.year, FALSE);
                    SetDlgItemText(hDlg, IDC_EDIT_ISBN, b.isbn.c_str());

                    auto it = find(GENRES.begin(), GENRES.end(), b.genre);
                    if (it != GENRES.end())
                    {
                        int genreIdx = (int)distance(GENRES.begin(), it);
                        SendMessage(GetDlgItem(hDlg, IDC_GENRE), CB_SETCURSEL, genreIdx, 0);
                    }
                }
            }
            return (INT_PTR)TRUE;

        case IDOK:
            if (selectedIndex >= 0 && selectedIndex < (int)library.size())
            {
                Book& b = library[selectedIndex];
                wchar_t buffer[256];

                GetDlgItemText(hDlg, IDC_EDIT_TITLE, buffer, 256);
                b.title = buffer;

                GetDlgItemText(hDlg, IDC_EDIT_AUTHOR, buffer, 256);
                b.author = buffer;

                BOOL success;
                b.year = GetDlgItemInt(hDlg, IDC_EDIT_YEAR, &success, FALSE);

                int genreSel = (int)SendMessage(GetDlgItem(hDlg, IDC_GENRE), CB_GETCURSEL, 0, 0);
                if (genreSel >= 0) b.genre = GENRES[genreSel];

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
    {
        HWND hSort = GetDlgItem(hDlg, IDC_SORT);

        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Title (asc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Title (desc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Author (asc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Author (desc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Year (asc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Year (desc.)");
        SendMessage(hSort, CB_ADDSTRING, 0, (LPARAM)L"Favourites First");

        SendMessage(hSort, CB_SETCURSEL, 6, 0);

        SortLibrary(6); 

        RefreshBookList(hDlg);
        return (INT_PTR)TRUE;
    }
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
            int realIndex = GetRealIndexFromListBox(hList, sel);

            if (realIndex < 0)
            {
                MessageBox(hDlg, L"Please select a book to delete.", L"No selection", MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            if (MessageBox(hDlg, L"Are you sure you want to delete this book?",
                L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                library.erase(library.begin() + realIndex);
                RefreshBookList(hDlg);
            }
            return (INT_PTR)TRUE;
        }

        case IDC_BTN_FAV:
        {
            HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
            int realIndex = GetRealIndexFromListBox(hList, sel);

            if (realIndex < 0)
            {
                MessageBox(hDlg, L"Please select a book to favorite.", L"No Selection", MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            library[realIndex].favorite = !library[realIndex].favorite;
            SortLibrary(6);
            RefreshBookList(hDlg);
            return (INT_PTR)TRUE;
        }
        case IDC_SORT:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                HWND hSort = GetDlgItem(hDlg, IDC_SORT);
                int mode = (int)SendMessage(hSort, CB_GETCURSEL, 0, 0);

                if (mode != CB_ERR && mode >= 0)
                {
                    SortLibrary(mode);
                    RefreshBookList(hDlg);
                }
            }
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
