// GUITest.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "GUITest.h"


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                Papcfunc(HWND);
void                Apctest(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GUITEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUITEST));

    MSG msg;
    int ExitCode = 0;

    while (TRUE) {
        DWORD dwR = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
        if (dwR == WAIT_FAILED)
        {
            assert(false);
            ExitCode = -1;
            break;
        }
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return (int)msg.wParam;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return ExitCode;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUITEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GUITEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

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
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
            {
                HWND hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                if (hDlg)
                {
                    ShowWindow(hDlg, SW_SHOW);

                    //Disable parent window to make ours into a modal dialog
                    EnableWindow(hWnd, FALSE);

                    MSG msg;
                    BOOL bStopStop = FALSE;

                    for (; !bStopStop;)
                    {
                        DWORD dwR = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT,
                            MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
                        if (dwR == WAIT_FAILED)
                        {
                            //Error
                            assert(false);
                            break;
                        }

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                            //Hack to ensure processing of EndDialog() calls
                            if (msg.message == WM_NULL && msg.hwnd == hDlg)
                            {
                                //Normal exit
                                bStopStop = true;
                                break;
                            }

                            if (!IsDialogMessage(hDlg, &msg))
                            {
                                if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
                                {
                                    TranslateMessage(&msg);
                                }
                                DispatchMessage(&msg);
                            }
                        }
                    }

                    DestroyWindow(hDlg);
                    hDlg = NULL;

                    //Re-enable parent window
                    EnableWindow(hWnd, TRUE);
                }
                else
                    assert(false);
                break;
            }
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_APC:
                Apctest(hWnd);
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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
        if (LOWORD(wParam) == IDC_APC)
        {
            Apctest(hDlg);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void Papcfunc(
    ULONG_PTR Parameter
)
{
    HWND hWnd = reinterpret_cast<HWND>(Parameter);
    MessageBox(hWnd, L"Papcfunc fired", L"Success", MB_ICONINFORMATION);
}

void Apctest(HWND hwnd) {
    if (!QueueUserAPC(Papcfunc, GetCurrentThread(), reinterpret_cast<ULONG_PTR>(hwnd)))
    {
        MessageBox(hwnd, L"error QueueUserApc", L"Error", MB_ICONERROR);
    }
}
