#pragma once

#include <Windows.h>

/*
 * Abstract base window class
 */
template <typename DERIVED_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                                       WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE *pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
            pThis = static_cast<DERIVED_TYPE *>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA,
                             reinterpret_cast<LONG_PTR>(pThis));

            pThis->m_hwnd = hwnd;
            // NOTE(jaege): The above line of code is needed when using
            //     `WM_NCCREATE` message to get the pointer to the concrete
            //     class object for the first time. However, if the `WM_CREATE`
            //     message is used here instead, then this line can be removed.
        }
        else
        {
            pThis = reinterpret_cast<DERIVED_TYPE *>(GetWindowLongPtr(
                hwnd, GWLP_USERDATA));
        }

        if (pThis) { return pThis->HandleMessage(uMsg, wParam, lParam); }
        else { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
    }

    BaseWindow() : m_hwnd(NULL) { }

    BOOL Create(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle = 0,
                int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
                int nWidth = CW_USEDEFAULT, int nHeight = CW_USEDEFAULT,
                HWND hWndParent = NULL, HMENU hMenu = NULL)
    {
        WNDCLASS wc = { };

        wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();
        // Force redraw whole window when any change happens.
        wc.style = CS_HREDRAW | CS_VREDRAW;

        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle,
                                x, y, nWidth, nHeight, hWndParent, hMenu,
                                GetModuleHandle(NULL), this);

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const { return m_hwnd; }

protected:

    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;
};