// Library.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Library.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <commdlg.h>
#include <map>

using namespace std;

//
// STRUCT: Book
// PURPOSE: Defines the data model for a single book entry.
// NOTE: Uses wstring to support Unicode characters
//
struct Book {
    wstring title;
    wstring author;
    int year;
    wstring genre;
    wstring isbn;
    bool favorite = false;
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wstring currentGenreFilter = L"";               // Tracks the current state of the list view
vector<Book> library;                           // Global vector to store the book catalog
const vector<wstring> GENRES = {                // Predefined list of genres
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

// Dialog Procedures
INT_PTR CALLBACK    LibraryCatalog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    AddBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    EditBookDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    StatsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// File Operations
void                SaveLibraryToFile();
bool                LoadLibraryFromFile(const wstring& filename);
void                LoadLibraryFromDefault();
bool                ImportLibraryFromFile(HWND hDlg);
bool                ExportLibraryToFile(HWND hDlg);

// Data Management & Helpers
void                SortLibrary(int mode);
int                 GetRealIndexFromListBox(HWND hList, int selection);
void                RefreshBookList(HWND hDlg, const wstring& genreFilter = L"");
void                PopulateGenreCombo(HWND combo);

// Graphics
void                DrawBarChart(HDC hdc, RECT rect, const vector<pair<wstring, int>>& data, const wstring& title);

//
//  FUNCTION: SaveLibraryToFile()
//
//  PURPOSE: Saves the current library vector to the default storage file (library.txt).
//
//  COMMENTS:
//      Uses pipe (|) as a delimiter. Saves favorite status as 0 or 1.
//
void SaveLibraryToFile()
{
    wofstream file(L"library.txt");
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

//
//  FUNCTION: LoadLibraryFromFile(const wstring&)
//
//  PURPOSE: Loads book data from a specified file path into the library vector.
//
//  COMMENTS:
//      Clears the existing library before loading.
//
bool LoadLibraryFromFile(const wstring& filename)
{
    wifstream file(filename);
    if (!file.is_open())
        return false;

    library.clear();
    wstring line;

    while (getline(file, line))
    {
        wstringstream ss(line);
        Book b;
        wstring fav;

        getline(ss, b.title, L'|');
        getline(ss, b.author, L'|');

        wstring yearStr;
        getline(ss, yearStr, L'|');
        b.year = _wtoi(yearStr.c_str());

        getline(ss, b.genre, L'|');
        getline(ss, b.isbn, L'|');
        getline(ss, fav, L'|');

        b.favorite = (fav == L"1");

        library.push_back(b);
    }

    return true;
}

//
//  FUNCTION: LoadLibraryFromDefault()
//
//  PURPOSE: Helper function to load the default "library.txt" file on startup.
//
void LoadLibraryFromDefault()
{
    library.clear();
    LoadLibraryFromFile(L"library.txt");
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
    LoadLibraryFromDefault();


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
//  FUNCTION: ImportLibraryFromFile(HWND)
//
//  PURPOSE: Opens a file dialog to import books from a user-selected text file.
//
//  COMMENTS:
//      Checks for duplicate ISBNs to prevent double entries. 
//      Displays a message box with results.
//
bool ImportLibraryFromFile(HWND hDlg)
{
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = L"";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileName(&ofn))
        return false;

    wifstream file(fileName);
    if (!file.is_open())
    {
        MessageBox(hDlg, L"Could not load the selected file.", L"Error", MB_ICONERROR);
        return false;
    }

    int oldSize = (int)library.size();
    wstring line;
    int importedCount = 0;

    while (getline(file, line))
    {
        wstringstream ss(line);
        Book b;
        wstring fav;

        getline(ss, b.title, L'|');
        getline(ss, b.author, L'|');

        wstring yearStr;
        getline(ss, yearStr, L'|');
        b.year = _wtoi(yearStr.c_str());

        getline(ss, b.genre, L'|');
        getline(ss, b.isbn, L'|');
        getline(ss, fav, L'|');

        b.favorite = (fav == L"1");

        bool isDuplicate = false;
        for (const auto& existing : library)
        {
            if (existing.isbn == b.isbn)
            {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate)
        {
            library.push_back(b);
            importedCount++;
        }
    }

    file.close();

    MessageBox(
        hDlg,
        (L"Imported " + to_wstring(importedCount) + L" new books.\n" +
            to_wstring(library.size() - oldSize - importedCount) + L" duplicates skipped.").c_str(),
        L"Import Complete",
        MB_OK | MB_ICONINFORMATION);

    return true;
}

//
//  FUNCTION: ExportLibraryToFile(HWND)
//
//  PURPOSE: Opens a save file dialog to export the current catalog to a file.
//
bool ExportLibraryToFile(HWND hDlg)
{
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = L"library_export.txt";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    if (!GetSaveFileName(&ofn))
        return false;

    wofstream file(fileName);
    if (!file.is_open())
    {
        MessageBox(hDlg, L"Could not create the export file.", L"Error", MB_ICONERROR);
        return false;
    }

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

    file.close();

    MessageBox(
        hDlg,
        (L"Exported " + to_wstring(library.size()) + L" books to:\n" + fileName).c_str(),
        L"Export Complete",
        MB_OK | MB_ICONINFORMATION);

    return true;
}

//
//  FUNCTION: SortLibrary(int)
//
//  PURPOSE: Sorts the global library vector based on the selected mode.
//
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

//
//  FUNCTION: GetRealIndexFromListBox(HWND, int)
//
//  PURPOSE: Retrieves the actual vector index of a book from the ListBox.
//
//  COMMENTS:
//      Because the ListBox might be sorted or filtered differently than the vector,
//      we store the original vector index in ITEMDATA.
//
int GetRealIndexFromListBox(HWND hList, int selection)
{
    if (selection == LB_ERR) return -1;
    return (int)SendMessage(hList, LB_GETITEMDATA, selection, 0);
}

//
//  FUNCTION: RefreshBookList(HWND, const wstring&)
//
//  PURPOSE: Updates the ListBox content based on the library data and genre filter.
//
void RefreshBookList(HWND hDlg, const wstring& genreFilter) {
    HWND hList = GetDlgItem(hDlg, IDC_BOOKLIST);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    int displayedCount = 0;

    for (size_t i = 0; i < library.size(); i++)
    {
        const Book& b = library[i];

        bool filtering = (!genreFilter.empty() && genreFilter != L"All Genres");

        if (filtering && b.genre != genreFilter)
            continue;

        displayedCount++;

        wstring line = (b.favorite ? L"⭐ " : L"") +
            b.title + L" | " + b.author + L" | " +
            to_wstring(b.year) + L" | " + b.genre + L" | " + b.isbn;

        int idx = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        SendMessage(hList, LB_SETITEMDATA, idx, (LPARAM)i);
    }

    wstring countText = L"Showing: " + to_wstring(displayedCount) + L" / " + to_wstring(library.size());
    SetDlgItemText(hDlg, IDC_TOTALBOOKS, countText.c_str());
}

//
//  FUNCTION: PopulateGenreCombo(HWND)
//
//  PURPOSE: Fills a combobox with the predefined list of genres.
//
void PopulateGenreCombo(HWND combo)
{
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    for (const auto& g : GENRES)
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)g.c_str());
}

//
//  FUNCTION: DrawBarChart(HDC, RECT, vector<pair<wstring, int>>, wstring)
//
//  PURPOSE: Draws a GDI bar chart based on provided data pairs.
//
//  COMMENTS:
//      Handles creating fonts, brushes, and pens to draw axes, bars, and labels.
//
void DrawBarChart(HDC hdc, RECT rect, const vector<pair<wstring, int>>& data, const wstring& title)
{
    if (data.empty()) return;

    // Setup Graphics Objects
    HBRUSH blueBrush = CreateSolidBrush(RGB(70, 130, 180));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, blueBrush);
    HPEN blackPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, blackPen);

    SetBkMode(hdc, TRANSPARENT);

    // Create Fonts
    // Title Font
    HFONT hTitleFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");

    // Number Font
    HFONT hNumFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");

    // Axis Label Font
    HFONT hLabelFont = CreateFont(12, 0, 900, 900, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");

    // Draw Title
    HFONT oldFont = (HFONT)SelectObject(hdc, hTitleFont); // Save original system font
    SetTextAlign(hdc, TA_CENTER | TA_TOP);
    TextOut(hdc, rect.left + (rect.right - rect.left) / 2, rect.top + 10, title.c_str(), (int)title.length());

    // Calculate Chart Dimensions
    int chartTop = rect.top + 50;
    int chartBottom = rect.bottom - 80;
    int chartLeft = rect.left + 60;
    int chartRight = rect.right - 20;
    int chartHeight = chartBottom - chartTop;
    int chartWidth = chartRight - chartLeft;

    // Draw Axes
    MoveToEx(hdc, chartLeft, chartTop, NULL);
    LineTo(hdc, chartLeft, chartBottom);
    LineTo(hdc, chartRight, chartBottom);

    // Calculate Scale
    int maxValue = 0;
    for (const auto& item : data)
        if (item.second > maxValue) maxValue = item.second;
    if (maxValue == 0) maxValue = 1;

    int barCount = (int)data.size();
    int barSpacing = 10;
    int totalSpacing = barSpacing * (barCount + 1);
    int barWidth = (chartWidth - totalSpacing) / barCount;
    if (barWidth < 5) barWidth = 5;

    // Loop to draw Bars, Numbers, and Labels
    for (size_t i = 0; i < data.size(); i++)
    {
        int x = chartLeft + barSpacing + (int)i * (barWidth + barSpacing);
        int barHeight = (int)((double)data[i].second / maxValue * chartHeight);
        int y = chartBottom - barHeight;

        // Draw the Bar Rectangle
        Rectangle(hdc, x, y, x + barWidth, chartBottom);

		// Draw number above bar
        SelectObject(hdc, hNumFont);        
        SetTextAlign(hdc, TA_CENTER | TA_TOP);

        wstring valueStr = to_wstring(data[i].second);
        TextOut(hdc, x + barWidth / 2, y - 20, valueStr.c_str(), (int)valueStr.length());

		// Draw label below bar
        SelectObject(hdc, hLabelFont);
        SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
        TextOut(hdc, x + barWidth / 2 + 5, chartBottom + 5, data[i].first.c_str(), (int)data[i].first.length());
    }

    // Cleanup
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldFont);

    DeleteObject(blueBrush);
    DeleteObject(blackPen);
    DeleteObject(hTitleFont);
    DeleteObject(hNumFont);
    DeleteObject(hLabelFont);
}

//
//  FUNCTION: StatsDlg(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the Statistics dialog box.
//
//  COMMENTS:
//      Handles WM_PAINT to draw charts and WM_COMMAND to switch between views (Genre/Year).
//
INT_PTR CALLBACK StatsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool showByGenre = true;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        CheckRadioButton(hDlg, IDC_RADIO_GENRE, IDC_RADIO_YEAR, IDC_RADIO_GENRE);
        showByGenre = true;
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RADIO_GENRE:
            showByGenre = true;
            InvalidateRect(hDlg, NULL, TRUE);
            return (INT_PTR)TRUE;

        case IDC_RADIO_YEAR:
            showByGenre = false;
            InvalidateRect(hDlg, NULL, TRUE);
            return (INT_PTR)TRUE;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        RECT clientRect;
        GetClientRect(hDlg, &clientRect);

        RECT chartRect = clientRect;
        chartRect.bottom -= 60;

        if (showByGenre)
        {
            map<wstring, int> genreCount;
            for (const auto& b : library)
            {
                if (!b.genre.empty())
                    genreCount[b.genre]++;
            }

            vector<pair<wstring, int>> data(genreCount.begin(), genreCount.end());

            sort(data.begin(), data.end(),
                [](const pair<wstring, int>& a, const pair<wstring, int>& b) {
                    return a.second > b.second;
                });

            DrawBarChart(hdc, chartRect, data, L"Books by Genre");
        }
        else
        {
            map<int, int> yearCount;
            for (const auto& b : library)
            {
                if (b.year > 0)
                    yearCount[b.year]++;
            }

            vector<pair<wstring, int>> data;
            for (const auto& item : yearCount)
                data.push_back(make_pair(to_wstring(item.first), item.second));

            sort(data.begin(), data.end(),
                [](const pair<wstring, int>& a, const pair<wstring, int>& b) {
                    return _wtoi(a.first.c_str()) < _wtoi(b.first.c_str());
                });

            DrawBarChart(hdc, chartRect, data, L"Books by Year");
        }

        EndPaint(hDlg, &ps);
        return (INT_PTR)TRUE;
    }
    }

    return (INT_PTR)FALSE;
}

//
//  FUNCTION: AddBookDlg(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the "Add Book" dialog box.
//
//  COMMENTS:
//      Validates input and checks for duplicate ISBNs before adding to library.
//
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

//
//  FUNCTION: EditBookDlg(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the "Edit Book" dialog box.
//
//  COMMENTS:
//      Allows user to select a book from a dropdown, then updates fields for editing.
//
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

//
//  FUNCTION: LibraryCatalog(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main Library Catalog dialog.
//
//  COMMENTS:
//      Handles CRUD buttons (Add, Edit, Delete), Sorting, Filtering, and Import/Export.
//
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

        HWND hGenre = GetDlgItem(hDlg, IDC_GENRE);
        PopulateGenreCombo(hGenre);
        SendMessage(hGenre, CB_INSERTSTRING, 0, (LPARAM)L"All Genres");
        SendMessage(hGenre, CB_SETCURSEL, 0, 0);
        currentGenreFilter = L"All Genres";

        SortLibrary(6);
        RefreshBookList(hDlg, currentGenreFilter);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_BTN_ADD:
            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_DIALOG), hDlg, AddBookDlg) == IDOK)
            {
                RefreshBookList(hDlg, currentGenreFilter);
            }
            return (INT_PTR)TRUE;

        case IDC_BTN_EDIT:
            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIALOG), hDlg, EditBookDlg) == IDOK)
            {
                RefreshBookList(hDlg, currentGenreFilter);
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
                RefreshBookList(hDlg, currentGenreFilter);
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
            RefreshBookList(hDlg, currentGenreFilter);
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
                    RefreshBookList(hDlg, currentGenreFilter);
                }
            }
            return (INT_PTR)TRUE;
        }
        case IDC_GENRE:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                HWND hGenre = GetDlgItem(hDlg, IDC_GENRE);
                int sel = (int)SendMessage(hGenre, CB_GETCURSEL, 0, 0);

                if (sel != CB_ERR)
                {
                    wchar_t buffer[256];
                    SendMessage(hGenre, CB_GETLBTEXT, sel, (LPARAM)buffer);

                    currentGenreFilter = buffer;
                    RefreshBookList(hDlg, currentGenreFilter);
                }
            }
            return (INT_PTR)TRUE;
        }
        case IDC_BTN_IMPORT:
        {
            if (ImportLibraryFromFile(hDlg))
            {
                HWND hSort = GetDlgItem(hDlg, IDC_SORT);
                int sortMode = (int)SendMessage(hSort, CB_GETCURSEL, 0, 0);

                SortLibrary(sortMode);
                RefreshBookList(hDlg, currentGenreFilter);
            }
            return (INT_PTR)TRUE;
        }

        case IDC_BTN_EXPORT:
        {
            ExportLibraryToFile(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDC_BTN_STATS:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_STATS_DIALOG), hDlg, StatsDlg);
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