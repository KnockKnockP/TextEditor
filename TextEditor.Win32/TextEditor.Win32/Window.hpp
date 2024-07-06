/*
#pragma once

#include <Windows.h>

class Window {
protected:
    static Window *singleton;

    HWND hwnd = NULL;

    static Window *GetSingleton(void);
    static LRESULT CALLBACK Callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual ATOM Register(LPCTSTR name) const;

public:
    virtual bool Create(void);
    void Show(void) const;
    HWND GetHwnd(void) const;
};
*/