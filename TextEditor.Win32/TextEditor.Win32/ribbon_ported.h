#ifndef RIBBON_PORTED_H
#define RIBBON_PORTED_H

#include <windows.h>

extern const GUID IID_IUnknown;
extern const GUID IID_IUIFramework;
extern const GUID IID_IUIApplication;
extern const GUID IID_IUIRibbon;
extern const GUID CLSID_UIRibbonFramework;

typedef enum UI_VIEWTYPE {
    UI_VIEWTYPE_RIBBON = 1
} UI_VIEWTYPE;

typedef enum UI_VIEWVERB {
    UI_VIEWVERB_CREATE,
    UI_VIEWVERB_DESTROY,
    UI_VIEWVERB_SIZE,
    UI_VIEWVERB_ERROR
} UI_VIEWVERB;

typedef enum UI_COMMANDTYPE {
    UI_COMMANDTYPE_ACTION
} UI_COMMANDTYPE;

typedef struct IUIApplication IUIApplication;
typedef struct IUIFramework IUIFramework;
typedef struct IUIRibbon IUIRibbon;

typedef struct IUIApplicationVtbl {
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(IUIApplication *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(IUIApplication *This);
    ULONG(STDMETHODCALLTYPE *Release)(IUIApplication *This);
    HRESULT(STDMETHODCALLTYPE *OnViewChanged)(IUIApplication *This, UINT32 viewId, UI_VIEWTYPE typeId, IUnknown *view, UI_VIEWVERB verb, INT32 uReasonCode);
    HRESULT(STDMETHODCALLTYPE *OnCreateUICommand)(IUIApplication *This, UINT32 commandId, UI_COMMANDTYPE typeId, IUnknown **commandHandler);
    HRESULT(STDMETHODCALLTYPE *OnDestroyUICommand)(IUIApplication* This, UINT32 commandId, UI_COMMANDTYPE typeId, IUnknown *commandHandler);
} IUIApplicationVtbl;

struct IUIApplication {
    const IUIApplicationVtbl *lpVtbl;
};

typedef struct IUIFrameworkVtbl {
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(IUIFramework *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(IUIFramework *This);
    ULONG(STDMETHODCALLTYPE *Release)(IUIFramework *This);
    HRESULT(STDMETHODCALLTYPE *Initialize)(IUIFramework *This, HWND hwnd, IUIApplication *application);
    HRESULT(STDMETHODCALLTYPE *Destroy)(IUIFramework *This);
    HRESULT(STDMETHODCALLTYPE *LoadUI)(IUIFramework *This, HINSTANCE instance, LPCWSTR resourceName);
    HRESULT(STDMETHODCALLTYPE *GetView)(IUIFramework *This, UINT32 viewId, REFIID riid, void **ppv);
} IUIFrameworkVtbl;

struct IUIFramework {
    const IUIFrameworkVtbl *lpVtbl;
};

typedef struct IUIRibbonVtbl {
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(IUIRibbon *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE* AddRef)(IUIRibbon *This);
    ULONG(STDMETHODCALLTYPE* Release)(IUIRibbon *This);
    HRESULT(STDMETHODCALLTYPE* GetHeight)(IUIRibbon *This, UINT32 *cy);
    HRESULT(STDMETHODCALLTYPE* LoadSettingsFromStream)(IUIRibbon *This, IStream *pStream);
    HRESULT(STDMETHODCALLTYPE* SaveSettingsToStream)(IUIRibbon *This, IStream *pStream);
} IUIRibbonVtbl;

struct IUIRibbon {
    const IUIRibbonVtbl *lpVtbl;
};
#endif