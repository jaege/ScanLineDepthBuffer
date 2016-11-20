#pragma once

#include <string>
#include <Windows.h>
#include "BaseWindow.h"

#define BYTES_PER_PIXEL 4

struct OffscreenBuffer
{
    BITMAPINFO info;
    void *memory;
    INT32 width;
    INT32 height;
    INT32 pitch;
};

class MainWindow : public BaseWindow<MainWindow>
{
public:
    MainWindow() : m_buffer(), m_objFilePath() { }
    PCWSTR ClassName() const { return L"MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OpenObjFile();

private:
    OffscreenBuffer m_buffer;
    std::wstring m_objFilePath;

    void Resize(UINT32 width, UINT32 height);
    void Render();
};