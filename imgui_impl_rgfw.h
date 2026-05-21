/*
    dear imgui RGFW backend
    This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
*/


#ifndef IMGUI_DISABLE

#ifndef RGFW_IMGUI_H
#define RGFW_IMGUI_H

#include "imgui.h"      // IMGUI_IMPL_API

#ifdef _MSC_VER
    #if _MSC_VER < 600
        #define RGFW_USE_INT
    #endif
#endif

#ifndef RGFW_INT_DEFINED
    #ifdef RGFW_USE_INT /* optional for any system that might not have stdint.h */
        typedef unsigned char 	u8;
        typedef signed char		i8;
        typedef unsigned short  u16;
        typedef signed short 	i16;
        typedef unsigned long int 	u32;
        typedef signed long int		i32;
        typedef unsigned long long	u64;
        typedef signed long long		i64;
    #else /* use stdint standard types instead of c ""standard"" types */
        #include <stdint.h>

        typedef uint8_t     u8;
        typedef int8_t      i8;
        typedef uint16_t   u16;
        typedef int16_t    i16;
        typedef uint32_t   u32;
        typedef int32_t    i32;
        typedef uint64_t   u64;
        typedef int64_t    i64;
    #endif
    #define RGFW_INT_DEFINED
#endif

#include <stdbool.h>

typedef struct RGFW_window RGFW_window;
typedef union RGFW_event RGFW_event;

/* basic api */
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForOpenGL(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForVulkan(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForOther(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplRgfw_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplRgfw_NewFrame();

// RGFW callbacks install
// - When calling Init with 'install_callbacks=true': ImGui_ImplRgfw_InstallCallbacks() is called. RGFW callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': RGFW callbacks won't be installed. You will need to call individual function yourself from your own RGFW callbacks.
IMGUI_IMPL_API void     ImGui_ImplRgfw_InstallCallbacks(RGFW_window* window);
IMGUI_IMPL_API void     ImGui_ImplRgfw_RestoreCallbacks(RGFW_window* window);

// RGFW callbacks options:
// - Set 'chain_for_all_windows=true' to enable chaining callbacks for all windows (including secondary viewports created by backends or by user)
IMGUI_IMPL_API void     ImGui_ImplRgfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

typedef struct {int x; int y;} impoint;
#define RGFW_point impoint

// RGFW callbacks (individual callbacks to call yourself if you didn't install callbacks)
IMGUI_IMPL_API void     ImGui_ImplRgfw_WindowFocusCallback(const RGFW_event* e); // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplRgfw_CursorEnterCallback(const RGFW_event* e); // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplRgfw_CursorPosCallback(const RGFW_event* e);   // Since 1.87
IMGUI_IMPL_API void     ImGui_ImplRgfw_MouseButtonCallback(const RGFW_event* e);
IMGUI_IMPL_API void     ImGui_ImplRgfw_MouseScrollCallback(const RGFW_event* e);
IMGUI_IMPL_API void     ImGui_ImplRgfw_KeyCallback(const RGFW_event* e);
IMGUI_IMPL_API void     ImGui_ImplRgfw_CharCallback(const RGFW_event* e);
#endif /* ifndef RGFW_IMGUI_H */

#ifdef RGFW_IMGUI_IMPLEMENTATION

#include <chrono>

#ifndef RGFWDEF
    #define RGFWDEF
#endif
#include "RGFW.h"

// RGFW data
enum RgfwClientApi
{
    RgfwClientApi_Unknown,
    RgfwClientApi_OpenGL,
    RgfwClientApi_Vulkan,
};

struct ImGui_ImplRgfw_Data
{
    RGFW_window*             Window;
    RgfwClientApi           ClientApi;
    double                  Time;
    RGFW_window*             MouseWindow;
    ImVec2                  LastValidMousePos;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;

    // Chain RGFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    RGFW_callbacks          PrevCallbacks;

    ImGui_ImplRgfw_Data()   { memset(static_cast<void*>(this), 0, sizeof(*this)); }
};

static ImGui_ImplRgfw_Data* ImGui_ImplRgfw_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplRgfw_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

char* clipboard_str = nullptr;

// Functions
static const char* ImGui_ImplRgfw_GetClipboardText(ImGuiContext* ctx)
{
    RGFW_UNUSED(ctx);

    size_t size;
    return RGFW_readClipboard(&size);
}

static void ImGui_ImplRgfw_SetClipboardText(ImGuiContext* ctx, const char* text)
{
    RGFW_UNUSED(ctx);
    RGFW_UNUSED(text);
    RGFW_writeClipboard(text, static_cast<u32>(strlen(text)));
}

static ImGuiKey ImGui_ImplRgfw_KeyToImGuiKey(int key)
{
    switch(key) {
        case RGFW_keyEscape:       return ImGuiKey_Escape;
        case RGFW_keyBacktick:     return ImGuiKey_GraveAccent;
        case RGFW_key0:            return ImGuiKey_0;
        case RGFW_key1:            return ImGuiKey_1;
        case RGFW_key2:            return ImGuiKey_2;
        case RGFW_key3:            return ImGuiKey_3;
        case RGFW_key4:            return ImGuiKey_4;
        case RGFW_key5:            return ImGuiKey_5;
        case RGFW_key6:            return ImGuiKey_6;
        case RGFW_key7:            return ImGuiKey_7;
        case RGFW_key8:            return ImGuiKey_8;
        case RGFW_key9:            return ImGuiKey_9;
        case RGFW_keyMinus:        return ImGuiKey_Minus;
        case RGFW_keyEquals:       return ImGuiKey_Equal;
        case RGFW_keyBackSpace:    return ImGuiKey_Backspace;
        case RGFW_keyTab:          return ImGuiKey_Tab;
        case RGFW_keySpace:        return ImGuiKey_Space;
        case RGFW_keyA:            return ImGuiKey_A;
        case RGFW_keyB:            return ImGuiKey_B;
        case RGFW_keyC:            return ImGuiKey_C;
        case RGFW_keyD:            return ImGuiKey_D;
        case RGFW_keyE:            return ImGuiKey_E;
        case RGFW_keyF:            return ImGuiKey_F;
        case RGFW_keyG:            return ImGuiKey_G;
        case RGFW_keyH:            return ImGuiKey_H;
        case RGFW_keyI:            return ImGuiKey_I;
        case RGFW_keyJ:            return ImGuiKey_J;
        case RGFW_keyK:            return ImGuiKey_K;
        case RGFW_keyL:            return ImGuiKey_L;
        case RGFW_keyM:            return ImGuiKey_M;
        case RGFW_keyN:            return ImGuiKey_N;
        case RGFW_keyO:            return ImGuiKey_O;
        case RGFW_keyP:            return ImGuiKey_P;
        case RGFW_keyQ:            return ImGuiKey_Q;
        case RGFW_keyR:            return ImGuiKey_R;
        case RGFW_keyS:            return ImGuiKey_S;
        case RGFW_keyT:            return ImGuiKey_T;
        case RGFW_keyU:            return ImGuiKey_U;
        case RGFW_keyV:            return ImGuiKey_V;
        case RGFW_keyW:            return ImGuiKey_W;
        case RGFW_keyX:            return ImGuiKey_X;
        case RGFW_keyY:            return ImGuiKey_Y;
        case RGFW_keyZ:            return ImGuiKey_Z;
        case RGFW_keyPeriod:       return ImGuiKey_Period;
        case RGFW_keyComma:        return ImGuiKey_Comma;
        case RGFW_keySlash:        return ImGuiKey_Slash;
        case RGFW_keyBracket:      return ImGuiKey_LeftBracket;
        case RGFW_keyCloseBracket: return ImGuiKey_RightBracket;
        case RGFW_keySemicolon:    return ImGuiKey_Semicolon;
        case RGFW_keyApostrophe:   return ImGuiKey_Apostrophe;
        case RGFW_keyBackSlash:    return ImGuiKey_Backslash;
        case RGFW_keyReturn:       return ImGuiKey_Enter;  // Same as RGFW_keyEnter
        case RGFW_keyDelete:       return ImGuiKey_Delete;
        case RGFW_keyF1:           return ImGuiKey_F1;
        case RGFW_keyF2:           return ImGuiKey_F2;
        case RGFW_keyF3:           return ImGuiKey_F3;
        case RGFW_keyF4:           return ImGuiKey_F4;
        case RGFW_keyF5:           return ImGuiKey_F5;
        case RGFW_keyF6:           return ImGuiKey_F6;
        case RGFW_keyF7:           return ImGuiKey_F7;
        case RGFW_keyF8:           return ImGuiKey_F8;
        case RGFW_keyF9:           return ImGuiKey_F9;
        case RGFW_keyF10:          return ImGuiKey_F10;
        case RGFW_keyF11:          return ImGuiKey_F11;
        case RGFW_keyF12:          return ImGuiKey_F12;
        case RGFW_keyF13:          return ImGuiKey_F13;
        case RGFW_keyF14:          return ImGuiKey_F14;
        case RGFW_keyF15:          return ImGuiKey_F15;
        case RGFW_keyF16:          return ImGuiKey_F16;
        case RGFW_keyF17:          return ImGuiKey_F17;
        case RGFW_keyF18:          return ImGuiKey_F18;
        case RGFW_keyF19:          return ImGuiKey_F19;
        case RGFW_keyF20:          return ImGuiKey_F20;
        case RGFW_keyF21:          return ImGuiKey_F21;
        case RGFW_keyF22:          return ImGuiKey_F22;
        case RGFW_keyF23:          return ImGuiKey_F23;
        case RGFW_keyF24:          return ImGuiKey_F24;
        case RGFW_keyF25:          return ImGuiKey_None;  // No ImGuiKey_F25
        case RGFW_keyCapsLock:     return ImGuiKey_CapsLock;
        case RGFW_keyShiftL:       return ImGuiKey_LeftShift;
        case RGFW_keyControlL:     return ImGuiKey_LeftCtrl;
        case RGFW_keyAltL:         return ImGuiKey_LeftAlt;
        case RGFW_keySuperL:       return ImGuiKey_LeftSuper;
        case RGFW_keyShiftR:       return ImGuiKey_RightShift;
        case RGFW_keyControlR:     return ImGuiKey_RightCtrl;
        case RGFW_keyAltR:         return ImGuiKey_RightAlt;
        case RGFW_keySuperR:       return ImGuiKey_RightSuper;
        case RGFW_keyUp:           return ImGuiKey_UpArrow;
        case RGFW_keyDown:         return ImGuiKey_DownArrow;
        case RGFW_keyLeft:         return ImGuiKey_LeftArrow;
        case RGFW_keyRight:        return ImGuiKey_RightArrow;
        case RGFW_keyInsert:       return ImGuiKey_Insert;
        case RGFW_keyMenu:         return ImGuiKey_Menu;
        case RGFW_keyEnd:          return ImGuiKey_End;
        case RGFW_keyHome:         return ImGuiKey_Home;
        case RGFW_keyPageUp:       return ImGuiKey_PageUp;
        case RGFW_keyPageDown:     return ImGuiKey_PageDown;
        case RGFW_keyNumLock:      return ImGuiKey_NumLock;
        case RGFW_keyPadSlash:     return ImGuiKey_KeypadDivide;
        case RGFW_keyPadMultiply:  return ImGuiKey_KeypadMultiply;
        case RGFW_keyPadPlus:      return ImGuiKey_KeypadAdd;
        case RGFW_keyPadMinus:     return ImGuiKey_KeypadSubtract;
        case RGFW_keyPadEqual:     return ImGuiKey_KeypadEqual;
        case RGFW_keyPad1:         return ImGuiKey_Keypad1;
        case RGFW_keyPad2:         return ImGuiKey_Keypad2;
        case RGFW_keyPad3:         return ImGuiKey_Keypad3;
        case RGFW_keyPad4:         return ImGuiKey_Keypad4;
        case RGFW_keyPad5:         return ImGuiKey_Keypad5;
        case RGFW_keyPad6:         return ImGuiKey_Keypad6;
        case RGFW_keyPad7:         return ImGuiKey_Keypad7;
        case RGFW_keyPad8:         return ImGuiKey_Keypad8;
        case RGFW_keyPad9:         return ImGuiKey_Keypad9;
        case RGFW_keyPad0:         return ImGuiKey_Keypad0;
        case RGFW_keyPadPeriod:    return ImGuiKey_KeypadDecimal;
        case RGFW_keyPadReturn:    return ImGuiKey_KeypadEnter;
        case RGFW_keyScrollLock:   return ImGuiKey_ScrollLock;
        case RGFW_keyPrintScreen:  return ImGuiKey_PrintScreen;
        case RGFW_keyPause:        return ImGuiKey_Pause;
        case RGFW_keyWorld1:       return ImGuiKey_None;  // Not supported
        case RGFW_keyWorld2:       return ImGuiKey_None;  // Not supported
        default:                return ImGuiKey_None;
    }
}

static bool ImGui_ImplRgfw_ShouldChainCallback(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void ImGui_ImplRgfw_MouseButtonCallback(const RGFW_event* e)
{
    u8 button = e->button.value;
    u8 pressed = e->button.state;

    if (button == RGFW_mouseMiddle) button = RGFW_mouseRight;
    else if (button == RGFW_mouseRight) button = RGFW_mouseMiddle;

    ImGuiIO& io = ImGui::GetIO();
    if (button < ImGuiMouseButton_COUNT) {
        io.AddMouseButtonEvent(button, pressed != 0);
    }
}

void ImGui_ImplRgfw_MouseScrollCallback(const RGFW_event* e)
{
    float xoffset = e->delta.x;
    float yoffset = e->delta.y;

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent(xoffset, yoffset);
}

void ImGui_ImplRgfw_KeyCallback(const RGFW_event* e)
{
    RGFW_key key = e->key.value;
    RGFW_keymod modState = e->key.mod;
    RGFW_bool pressed = e->key.state;

    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (modState & RGFW_modControl) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (modState & RGFW_modShift) != 0);
    io.AddKeyEvent(ImGuiMod_Alt,  (modState & RGFW_modAlt) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (modState & RGFW_modSuper) != 0);

    ImGuiKey imgui_key = ImGui_ImplRgfw_KeyToImGuiKey(key);
    io.AddKeyEvent(imgui_key, pressed != 0);
}

void ImGui_ImplRgfw_WindowFocusCallback(const RGFW_event* e)
{
    RGFW_bool inFocus = e->focus.state;

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(inFocus != 0);
}

void ImGui_ImplRgfw_CursorPosCallback(const RGFW_event* e)
{
    i32 x = e->mouse.x;
    i32 y = e->mouse.y;

    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
    bd->LastValidMousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
void ImGui_ImplRgfw_CursorEnterCallback(const RGFW_event* e)
{
    RGFW_window* window = e->common.win;
    RGFW_bool status = e->mouse.inWindow;

    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    if (status)
    {
        bd->MouseWindow = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
    }
    else if (!status && bd->MouseWindow == window)
    {
        bd->LastValidMousePos = io.MousePos;
        bd->MouseWindow = nullptr;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}

void ImGui_ImplRgfw_CharCallback(const RGFW_event* e)
{
    unsigned int c = (unsigned int)e->keyChar.value;

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void ImGui_ImplRgfw_MasterCallback(const RGFW_event* e)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd == nullptr) return;

    switch (e->type)
    {
        case RGFW_windowFocusIn:
        case RGFW_windowFocusOut:
            ImGui_ImplRgfw_WindowFocusCallback(e);
            break;
        case RGFW_mouseEnter:
        case RGFW_mouseLeave:
            ImGui_ImplRgfw_CursorEnterCallback(e);
            break;
        case RGFW_mousePosChanged:
            ImGui_ImplRgfw_CursorPosCallback(e);
            break;
        case RGFW_mouseButtonPressed:
        case RGFW_mouseButtonReleased:
            ImGui_ImplRgfw_MouseButtonCallback(e);
            break;
        case RGFW_mouseScroll:
            ImGui_ImplRgfw_MouseScrollCallback(e);
            break;
        case RGFW_keyPressed:
        case RGFW_keyReleased:
            ImGui_ImplRgfw_KeyCallback(e);
            break;
        case RGFW_keyChar:
            ImGui_ImplRgfw_CharCallback(e);
            break;
        default:
            break;
    }

    if (bd->PrevCallbacks.arr[e->type] != nullptr && ImGui_ImplRgfw_ShouldChainCallback(e->common.win))
        bd->PrevCallbacks.arr[e->type](e);
}

void ImGui_ImplRgfw_InstallCallbacks(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    IM_ASSERT(bd->Window == window);
    (void)window;

    /*
        TODO: RGFW doesn't have anyway to do this yet
        update this when I add it to RGFW
    */

    RGFW_setAllEventCallbacks(ImGui_ImplRgfw_MasterCallback, &bd->PrevCallbacks);
    bd->InstalledCallbacks = true;
}

void ImGui_ImplRgfw_RestoreCallbacks(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    IM_ASSERT(bd->Window == window);
    (void)window;

    for (u32 i = RGFW_eventNone + 1; i < RGFW_eventCount; i++)
        RGFW_setEventCallback((RGFW_eventType)i, bd->PrevCallbacks.arr[i]);

    bd->InstalledCallbacks = false;
    memset(&bd->PrevCallbacks, 0, sizeof(bd->PrevCallbacks));
}

// Set to 'true' to enable chaining installed callbacks for all windows (including secondary viewports created by backends or by user.
// This is 'false' by default meaning we only chain callbacks for the main viewport.
// We cannot set this to 'true' by default because user callbacks code may be not testing the 'window' parameter of their callback.
// If you set this to 'true' your user callback code will need to make sure you are testing the 'window' parameter.
void ImGui_ImplRgfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

#ifdef __EMSCRIPTEN__
EM_JS(void, ImGui_ImplRgfw_EmscriptenOpenURL, (char const* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif

static bool ImGui_ImplRgfw_Init(RGFW_window* window, bool install_callbacks, RgfwClientApi client_api)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplRgfw_Data* bd = IM_NEW(ImGui_ImplRgfw_Data)();
    io.BackendPlatformUserData = static_cast<void*>(bd);
    io.BackendPlatformName = "imgui_impl_rgfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->Window = window;
    bd->Time = 0.0;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = ImGui_ImplRgfw_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = ImGui_ImplRgfw_GetClipboardText;
    platform_io.Platform_ClipboardUserData = bd->Window;
#ifdef __EMSCRIPTEN__
      // io.PlatformOpenInShellFn = [](ImGuiContext*, const char* url) { ImGui_ImplRgfw_EmscriptenOpenURL(url); return true; };
#endif
    // Chain RGFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    if (install_callbacks)
        ImGui_ImplRgfw_InstallCallbacks(window);

    // Set platform dependent data in viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = static_cast<void*>(bd->Window);
#if defined(_WIN32) || defined(__APPLE__)
    main_viewport->PlatformHandleRaw = bd->Window->src.window;
#else
    IM_UNUSED(main_viewport);
#endif

    bd->ClientApi = client_api;
    return true;
}

bool ImGui_ImplRgfw_InitForOpenGL(RGFW_window* window, bool install_callbacks)
{
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_OpenGL);
}

bool ImGui_ImplRgfw_InitForVulkan(RGFW_window* window, bool install_callbacks)
{
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_Vulkan);
}

bool ImGui_ImplRgfw_InitForOther(RGFW_window* window, bool install_callbacks)
{
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_Unknown);
}

void ImGui_ImplRgfw_Shutdown()
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    if (bd->InstalledCallbacks)
        ImGui_ImplRgfw_RestoreCallbacks(bd->Window);

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos);
    IM_DELETE(bd);
}

static void ImGui_ImplRgfw_UpdateMouseData()
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // (those braces are here to reduce diff with multi-viewports support in 'docking' branch)
    {
        RGFW_window* window = bd->Window;
        const bool is_window_focused = RGFW_window_isInFocus(window);
        if (is_window_focused)
        {
            // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
            if (io.WantSetMousePos)
                RGFW_window_moveMouse(window, static_cast<i32>(io.MousePos.x), static_cast<i32>(io.MousePos.y));

            // (Optional) Fallback to provide mouse position when focused (ImGui_ImplRgfw_CursorPosCallback already provides this when hovered or captured)
            if (bd->MouseWindow == nullptr)
            {
                i32 x, y;
                RGFW_window_getMouse(window, &x, &y);
                bd->LastValidMousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
                io.AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
            }
        }
    }
}

static void ImGui_ImplRgfw_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || (bd->Window->internal.flags & (1L<<2)))
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    // (those braces are here to reduce diff with multi-viewports support in 'docking' branch)
    {
        RGFW_window* window = bd->Window;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            RGFW_window_showMouse(window, 0);
        }
        else
        {
            // Show OS mouse cursor
            // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with RGFW 3.2, but 3.3 works here.

            static const u8 imgui_mouse_cursors[] = {
                RGFW_mouseNormal,
                RGFW_mouseIbeam,
                RGFW_mouseResizeAll,
                RGFW_mouseResizeNS,
                RGFW_mouseResizeEW,
                RGFW_mouseResizeNESW,
                RGFW_mouseResizeNWSE,
                RGFW_mousePointingHand,
                RGFW_mouseNotAllowed,
            };

            if (imgui_cursor < (ImGuiMouseCursor)sizeof(imgui_mouse_cursors)) {
                RGFW_window_setMouseStandard(window, imgui_mouse_cursors[imgui_cursor]);
                RGFW_window_showMouse(window, 1);
            }
        }
    }
}

void ImGui_ImplRgfw_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplRgfw_InitForXXX()?");

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2(static_cast<float>(bd->Window->w), static_cast<float>(bd->Window->h));

    // Setup time step
    using namespace std::chrono;
    double current_time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() / 1000.0f;
    if(current_time <= bd->Time) {
        current_time = bd->Time + 0.000001;
    }
    io.DeltaTime = bd->Time == 0.0 ? static_cast<float>(1.0f / 60.0f) : static_cast<float>(current_time - bd->Time);
    bd->Time = current_time;

    ImGui_ImplRgfw_UpdateMouseData();
    ImGui_ImplRgfw_UpdateMouseCursor();
}

//-----------------------------------------------------------------------------

#endif /* RGFW_IMGUI_IMPLEMENTATION */

#endif /* #ifndef IMGUI_DISABLE */
