#ifndef TEXTEDITOR_RIBBON_HOST_H
#define TEXTEDITOR_RIBBON_HOST_H

#include <Windows.h>

#include <ribbon_ported.h>

namespace TextEditor {

class RibbonHost {
public:
    class Delegate {
    public:
        virtual ~Delegate() {
        }

        virtual void OnRibbonCommand(UINT32 command_id) = 0;
        virtual void OnRibbonHeightChanged(UINT32 ribbon_height) = 0;
    };

    RibbonHost();
    ~RibbonHost();

    bool Initialize(HWND window, Delegate *delegate);
    void Destroy();

    bool available() const;
    UINT32 height() const;

private:
    class CommandHandler;
    class Application;

    CommandHandler *GetOrCreateCommandHandler();
    void SetHeight(UINT32 ribbon_height);

    ribbon::IUIFramework *framework_;
    Application *application_;
    CommandHandler *command_handler_;
    Delegate *delegate_;
    UINT32 height_;
};

}  // namespace TextEditor

#endif
