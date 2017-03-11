#pragma once

#include <string>
#include <Windows.h>
#include "BaseWindow.h"
#include "ObjModel.h"
#include "OffscreenBuffer.h"

class MainWindow : public BaseWindow<MainWindow>
{
public:
    PCWSTR ClassName() const override { return L"MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void OpenObjFile();

private:
    ObjModel m_objModel;
};