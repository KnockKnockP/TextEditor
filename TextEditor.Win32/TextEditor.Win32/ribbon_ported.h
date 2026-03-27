#ifndef TEXTEDITOR_RIBBON_PORTED_H
#define TEXTEDITOR_RIBBON_PORTED_H

#include <Windows.h>

#ifndef REFPROPERTYKEY
#define REFPROPERTYKEY const PROPERTYKEY * __MIDL_CONST
#endif

namespace TextEditor {
namespace ribbon {

enum ViewType {
    kViewTypeRibbon = 1
};

enum ViewVerb {
    kViewVerbCreate,
    kViewVerbDestroy,
    kViewVerbSize,
    kViewVerbError
};

enum CommandType {
    kCommandTypeAction
};

enum ExecutionVerb {
    kExecutionVerbExecute,
    kExecutionVerbPreview,
    kExecutionVerbCancelPreview
};

struct IUISimplePropertySet : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetValue(PROPERTYKEY *key, PROPVARIANT *value) = 0;
};

struct IUICommandHandler : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Execute(UINT32 command_id,
                                              ExecutionVerb verb,
                                              const PROPERTYKEY *key,
                                              const PROPVARIANT *current_value,
                                              IUISimplePropertySet *command_execution_properties) = 0;
    virtual HRESULT STDMETHODCALLTYPE UpdateProperty(UINT32 command_id,
                                                     REFPROPERTYKEY key,
                                                     const PROPVARIANT *current_value,
                                                     PROPVARIANT *new_value) = 0;
};

struct IUIApplication : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE OnViewChanged(UINT32 view_id,
                                                    ViewType type_id,
                                                    IUnknown *view,
                                                    ViewVerb verb,
                                                    INT32 reason_code) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnCreateUICommand(UINT32 command_id,
                                                        CommandType type_id,
                                                        IUICommandHandler **command_handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnDestroyUICommand(UINT32 command_id,
                                                         CommandType type_id,
                                                         IUICommandHandler *command_handler) = 0;
};

struct IUIFramework : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Initialize(HWND window, IUIApplication *application) = 0;
    virtual HRESULT STDMETHODCALLTYPE Destroy() = 0;
    virtual HRESULT STDMETHODCALLTYPE LoadUI(HINSTANCE instance, LPCWSTR resource_name) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetView(UINT32 view_id, REFIID riid, void **view) = 0;
};

struct IUIRibbon : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetHeight(UINT32 *height) = 0;
    virtual HRESULT STDMETHODCALLTYPE LoadSettingsFromStream(IStream *stream) = 0;
    virtual HRESULT STDMETHODCALLTYPE SaveSettingsToStream(IStream *stream) = 0;
};

extern const GUID IID_IUIFramework;
extern const GUID IID_IUIApplication;
extern const GUID IID_IUIRibbon;
extern const GUID IID_IUICommandHandler;
extern const GUID CLSID_UIRibbonFramework;

}  // namespace ribbon
}  // namespace TextEditor

#endif
