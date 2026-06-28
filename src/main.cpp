#include "core/Application.h"

MisigmaApp app;

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    static auto WndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (message)
        {
        case WM_SIZE:
        case WM_DPICHANGED:
            if (app.is_window_active)
                app.SetWindow(hWnd);
            break;
        case WM_CHAR:
            switch (wParam)
            {
            case VK_BACK:
                wi::gui::TextInputField::DeleteFromInput();
                break;
            case VK_RETURN:
                break;
            default:
                wi::gui::TextInputField::AddInput((wchar_t)wParam);
                break;
            }
            break;
        case WM_INPUT:
            wi::input::rawinput::ParseMessage((void*)lParam);
            break;
        case WM_KILLFOCUS:
            app.is_window_active = false;
            break;
        case WM_SETFOCUS:
            app.is_window_active = true;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    };

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW wcex   = {};
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"MisigmaWindow";
    RegisterClassExW(&wcex);

    HWND hWnd = CreateWindowW(
        wcex.lpszClassName, L"Misigma",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1280, 720,
        nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, SW_SHOWDEFAULT);

    app.SetWindow(hWnd);
    wi::arguments::Parse(lpCmdLine);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            app.Run();
        }
    }

    wi::jobsystem::ShutDown();
    return (int)msg.wParam;
}