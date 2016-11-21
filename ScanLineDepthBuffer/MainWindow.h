#pragma once

#include <string>
#include <Windows.h>
#include "BaseWindow.h"
#include "ObjModel.h"

#define BYTES_PER_PIXEL 4

struct OffscreenBuffer
{
    BITMAPINFO info;
    LPVOID memory;
    INT32 width;
    INT32 height;
    INT32 pitch;
};

class MainWindow : public BaseWindow<MainWindow>
{
public:
    MainWindow() : m_buffer(), m_objModel() { }
    PCWSTR ClassName() const { return L"MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OpenObjFile();

private:
    OffscreenBuffer m_buffer;
    ObjModel m_objModel;

    void Resize(INT32 width, INT32 height);
    void Render();
};