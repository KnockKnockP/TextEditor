#include <leak_checker.h>

#include <ribbon_host.h>

#include <windows_helper.h>

namespace TextEditor {

namespace {

class ComObjectBase {
public:
    ComObjectBase()
        : reference_count_(1) {
    }

    ULONG AddReference() {
        return ++reference_count_;
    }

    ULONG ReleaseReference() {
        const ULONG remaining = --reference_count_;
        if (!remaining) {
            delete this;
        }
        return remaining;
    }

protected:
    virtual ~ComObjectBase() {
    }

private:
    ULONG reference_count_;
};

}  // namespace

class RibbonHost::CommandHandler : public ribbon::IUICommandHandler, private ComObjectBase {
public:
    explicit CommandHandler(Delegate *delegate)
        : delegate_(delegate) {
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **object) {
        if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, ribbon::IID_IUICommandHandler)) {
            *object = static_cast<ribbon::IUICommandHandler *>(this);
            AddRef();
            return S_OK;
        }

        *object = NULL;
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() {
        return AddReference();
    }

    virtual ULONG STDMETHODCALLTYPE Release() {
        return ReleaseReference();
    }

    virtual HRESULT STDMETHODCALLTYPE Execute(UINT32 command_id,
                                              ribbon::ExecutionVerb verb,
                                              const PROPERTYKEY *key,
                                              const PROPVARIANT *current_value,
                                              ribbon::IUISimplePropertySet *command_execution_properties) {
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(current_value);
        UNREFERENCED_PARAMETER(command_execution_properties);

        if (verb == ribbon::kExecutionVerbExecute && delegate_) {
            delegate_->OnRibbonCommand(command_id);
            return S_OK;
        }

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE UpdateProperty(UINT32 command_id,
                                                     REFPROPERTYKEY key,
                                                     const PROPVARIANT *current_value,
                                                     PROPVARIANT *new_value) {
        UNREFERENCED_PARAMETER(command_id);
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(current_value);
        UNREFERENCED_PARAMETER(new_value);
        return E_NOTIMPL;
    }

private:
    Delegate *delegate_;
};

class RibbonHost::Application : public ribbon::IUIApplication, private ComObjectBase {
public:
    explicit Application(RibbonHost *owner)
        : owner_(owner) {
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **object) {
        if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, ribbon::IID_IUIApplication)) {
            *object = static_cast<ribbon::IUIApplication *>(this);
            AddRef();
            return S_OK;
        }

        *object = NULL;
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() {
        return AddReference();
    }

    virtual ULONG STDMETHODCALLTYPE Release() {
        return ReleaseReference();
    }

    virtual HRESULT STDMETHODCALLTYPE OnViewChanged(UINT32 view_id,
                                                    ribbon::ViewType type_id,
                                                    IUnknown *view,
                                                    ribbon::ViewVerb verb,
                                                    INT32 reason_code) {
        UNREFERENCED_PARAMETER(view_id);
        UNREFERENCED_PARAMETER(reason_code);

        if (type_id == ribbon::kViewTypeRibbon && verb == ribbon::kViewVerbSize && owner_) {
            ribbon::IUIRibbon *ribbon_view = NULL;
            if (SUCCEEDED(view->QueryInterface(ribbon::IID_IUIRibbon, reinterpret_cast<void **>(&ribbon_view))) && ribbon_view) {
                UINT32 height = 0;
                if (SUCCEEDED(ribbon_view->GetHeight(&height))) {
                    owner_->SetHeight(height);
                }
                ribbon_view->Release();
            }
        }

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnCreateUICommand(UINT32 command_id,
                                                        ribbon::CommandType type_id,
                                                        ribbon::IUICommandHandler **command_handler) {
        UNREFERENCED_PARAMETER(command_id);
        UNREFERENCED_PARAMETER(type_id);

        if (!owner_) {
            *command_handler = NULL;
            return E_FAIL;
        }

        CommandHandler *handler = owner_->GetOrCreateCommandHandler();
        handler->AddRef();
        *command_handler = handler;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnDestroyUICommand(UINT32 command_id,
                                                         ribbon::CommandType type_id,
                                                         ribbon::IUICommandHandler *command_handler) {
        UNREFERENCED_PARAMETER(command_id);
        UNREFERENCED_PARAMETER(type_id);
        UNREFERENCED_PARAMETER(command_handler);
        return S_OK;
    }

private:
    RibbonHost *owner_;
};

RibbonHost::RibbonHost()
    : framework_(NULL),
      application_(NULL),
      command_handler_(NULL),
      delegate_(NULL),
      height_(0) {
}

RibbonHost::~RibbonHost() {
    Destroy();
}

bool RibbonHost::Initialize(HWND window, Delegate *delegate) {
    Destroy();

    delegate_ = delegate;
    height_ = 0;

    HRESULT result = CoCreateInstance(ribbon::CLSID_UIRibbonFramework,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      ribbon::IID_IUIFramework,
                                      reinterpret_cast<void **>(&framework_));
    if (FAILED(result) || !framework_) {
        framework_ = NULL;
        return false;
    }

    application_ = new Application(this);
    result = framework_->Initialize(window, application_);
    if (FAILED(result)) {
        Destroy();
        return false;
    }

    result = framework_->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
    if (FAILED(result)) {
        Destroy();
        return false;
    }

    return true;
}

void RibbonHost::Destroy() {
    if (framework_) {
        framework_->Destroy();
        framework_->Release();
        framework_ = NULL;
    }

    if (command_handler_) {
        command_handler_->Release();
        command_handler_ = NULL;
    }

    if (application_) {
        application_->Release();
        application_ = NULL;
    }

    delegate_ = NULL;
    height_ = 0;
}

bool RibbonHost::available() const {
    return framework_ != NULL;
}

UINT32 RibbonHost::height() const {
    return height_;
}

RibbonHost::CommandHandler *RibbonHost::GetOrCreateCommandHandler() {
    if (!command_handler_) {
        command_handler_ = new CommandHandler(delegate_);
    }

    return command_handler_;
}

void RibbonHost::SetHeight(UINT32 ribbon_height) {
    height_ = ribbon_height;
    if (delegate_) {
        delegate_->OnRibbonHeightChanged(ribbon_height);
    }
}

}  // namespace TextEditor
