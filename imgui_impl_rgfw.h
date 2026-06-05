/*
    dear imgui: Platform backend for RGFW
    This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
*/

/*
    Implemented features:
     [X] Platform: Clipboard support.
     [X] Platform: Mouse support.
     [X] Platform: Keyboard support.
     [X] Platform: Mouse cursor shape and visibility (ImGuiBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
    Missing features or issues:
     [ ] Platform: Gamepad support.
     [ ] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.
*/

#ifndef RGFW_IMGUI_H
#define RGFW_IMGUI_H

#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

#include <stdbool.h>

typedef struct RGFW_window RGFW_window;
typedef union RGFW_event RGFW_event;

/* Basic API */
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForOpenGL(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForVulkan(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplRgfw_InitForOther(RGFW_window* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplRgfw_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplRgfw_NewFrame();

/*
    RGFW callbacks install
    - When calling Init with 'install_callbacks=true': ImGui_ImplRgfw_InstallCallbacks() is called. RGFW callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
    - When calling Init with 'install_callbacks=false': RGFW callbacks won't be installed. You will need to call individual function yourself from your own RGFW callbacks.
*/
IMGUI_IMPL_API void     ImGui_ImplRgfw_InstallCallbacks(RGFW_window* window);
IMGUI_IMPL_API void     ImGui_ImplRgfw_RestoreCallbacks(RGFW_window* window);

/*
    RGFW callbacks options:
    - Set 'chain_for_all_windows=true' to enable chaining callbacks for all windows (including secondary viewports created by backends or by user)
*/
IMGUI_IMPL_API void     ImGui_ImplRgfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

/* RGFW callbacks (to call yourself if you didn't install callbacks) */
IMGUI_IMPL_API void     ImGui_ImplRgfw_UberCallback(const RGFW_event* e);

/* RGFW helpers */
IMGUI_IMPL_API float    ImGui_ImplRgfw_GetContentScaleForWindow(RGFW_window *window);
IMGUI_IMPL_API float    ImGui_ImplRgfw_GetContentScaleForMonitor(RGFW_monitor *monitor);

#endif /* #ifndef IMGUI_DISABLE */
#endif /* #ifndef RGFW_IMGUI_H */

#ifdef RGFW_IMGUI_IMPLEMENTATION
#ifndef IMGUI_DISABLE

#ifndef RGFW_HEADER
    #include "RGFW.h"
#endif

#ifndef RGFW_NATIVE
    #ifdef RGFW_WINDOWS
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <windows.h>
    #elif defined(RGFW_WASM)
        #include <emscripten.h>
        #include <emscripten/html5.h>
    #else
        #include <time.h>
    #endif
    #ifdef RGFW_MACOS
        #include <mach/mach_time.h>
    #endif
#endif

enum RgfwClientApi {
    RgfwClientApi_Unknown,
    RgfwClientApi_OpenGL,
    RgfwClientApi_Vulkan,
};

struct ImGui_ImplRgfw_Data {
    RGFW_window*            Window;
    RgfwClientApi           ClientApi;
    double                  Time;
    RGFW_window*            MouseWindow;
    ImVec2                  LastValidMousePos;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;

    /* Chain RGFW callbacks: our callbacks will call the user's previously installed callbacks, if any. */
    RGFW_callbacks          PrevCallbacks;

    ImGui_ImplRgfw_Data()   { memset(static_cast<void*>(this), 0, sizeof(*this)); }
};

static ImGui_ImplRgfw_Data* ImGui_ImplRgfw_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplRgfw_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

/* Functions */
static const char* ImGui_ImplRgfw_GetClipboardText(ImGuiContext* ctx) {
    IM_UNUSED(ctx);
    size_t size;
    return RGFW_readClipboard(&size);
}

static void ImGui_ImplRgfw_SetClipboardText(ImGuiContext* ctx, const char* text) {
    IM_UNUSED(ctx);
    IM_UNUSED(text);
    RGFW_writeClipboard(text, static_cast<u32>(strlen(text)));
}

static ImGuiKey ImGui_ImplRgfw_KeyToImGuiKey(int key) {
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
        case RGFW_keyReturn:       return ImGuiKey_Enter;  /* Same as RGFW_keyEnter */
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
        case RGFW_keyF25:          return ImGuiKey_None;  /* No ImGuiKey_F25 */
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
        case RGFW_keyWorld1:       return ImGuiKey_None;  /* Not supported */
        case RGFW_keyWorld2:       return ImGuiKey_None;  /* Not supported */
        default:                return ImGuiKey_None;
    }
}

static bool ImGui_ImplRgfw_ShouldChainCallback(RGFW_window* window) {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void ImGui_ImplRgfw_UberCallback(const RGFW_event* e) {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplRgfw_InitForXXX()?");
    if (bd->PrevCallbacks.arr[e->type] != nullptr && ImGui_ImplRgfw_ShouldChainCallback(e->common.win)) {
        bd->PrevCallbacks.arr[e->type](e);
    }

    ImGuiIO& io = ImGui::GetIO();
    switch (e->type) {
        case RGFW_windowFocusIn:
        case RGFW_windowFocusOut: {
            io.AddFocusEvent(e->type == RGFW_windowFocusIn);
            break;
        }
        /*
            Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
            so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
        */
        case RGFW_mouseEnter:
        case RGFW_mouseLeave: {
            RGFW_window* window = e->mouse.win;
            RGFW_bool entered = e->mouse.inWindow;
            if (entered) {
                bd->MouseWindow = window;
                io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
            }
            else if (!entered && bd->MouseWindow == window) {
                bd->LastValidMousePos = io.MousePos;
                bd->MouseWindow = nullptr;
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            }
            break;
        }
        case RGFW_mousePosChanged: {
            i32 x = e->mouse.x;
            i32 y = e->mouse.y;
            io.AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
            bd->LastValidMousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
            break;
        }
        case RGFW_mouseButtonPressed:
        case RGFW_mouseButtonReleased: {
            RGFW_mouseButton button = e->button.value;
            ImGuiMouseButton imguiButton = ImGuiMouseButton_COUNT;
            if (button == RGFW_mouseLeft) { imguiButton = ImGuiMouseButton_Left; }
            else if (button == RGFW_mouseRight) { imguiButton = ImGuiMouseButton_Right; }
            else if (button == RGFW_mouseMiddle) { imguiButton = ImGuiMouseButton_Middle; }
            if (imguiButton < ImGuiMouseButton_COUNT) {
                io.AddMouseButtonEvent(imguiButton, e->type == RGFW_mouseButtonPressed);
            }
            break;
        }
        case RGFW_mouseScroll: {
            float xDelta = e->delta.x;
            float yDelta = e->delta.y;
            io.AddMouseWheelEvent(xDelta, yDelta);
            break;
        }
        case RGFW_keyPressed:
        case RGFW_keyReleased: {
            RGFW_keymod modState = e->key.mod;
            io.AddKeyEvent(ImGuiMod_Ctrl, (modState & RGFW_modControl) != 0);
            io.AddKeyEvent(ImGuiMod_Shift, (modState & RGFW_modShift) != 0);
            io.AddKeyEvent(ImGuiMod_Alt,  (modState & RGFW_modAlt) != 0);
            io.AddKeyEvent(ImGuiMod_Super, (modState & RGFW_modSuper) != 0);
            ImGuiKey imguiKey = ImGui_ImplRgfw_KeyToImGuiKey(e->key.value);
            io.AddKeyEvent(imguiKey, e->type == RGFW_keyPressed);
            break;
        }
        case RGFW_keyChar: {
            u32 c = e->keyChar.value;
            io.AddInputCharacter(c);
            break;
        }
        default: break;
    }
}

void ImGui_ImplRgfw_InstallCallbacks(RGFW_window* window) {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    IM_ASSERT(bd->Window == window);

    RGFW_setAllEventCallbacks(ImGui_ImplRgfw_UberCallback, &bd->PrevCallbacks);
    bd->InstalledCallbacks = true;
}

void ImGui_ImplRgfw_RestoreCallbacks(RGFW_window* window) {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    IM_ASSERT(bd->Window == window);

    for (u32 i = RGFW_eventNone + 1; i < RGFW_eventCount; i++) {
        RGFW_setEventCallback(static_cast<RGFW_eventType>(i), bd->PrevCallbacks.arr[i]);
    }

    bd->InstalledCallbacks = false;
    memset(&bd->PrevCallbacks, 0, sizeof(bd->PrevCallbacks));
}

/*
    Set to 'true' to enable chaining installed callbacks for all windows (including secondary viewports created by backends or by user.
    This is 'false' by default meaning we only chain callbacks for the main viewport.
    We cannot set this to 'true' by default because user callbacks code may be not testing the 'window' parameter of their callback.
    If you set this to 'true' your user callback code will need to make sure you are testing the 'window' parameter.
*/
void ImGui_ImplRgfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows) {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

#ifdef __EMSCRIPTEN__
EM_JS(void, ImGui_ImplRgfw_EmscriptenOpenURL, (char const* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif

static bool ImGui_ImplRgfw_Init(RGFW_window* window, bool install_callbacks, RgfwClientApi client_api) {
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    /* Setup backend capabilities flags */
    ImGui_ImplRgfw_Data* bd = IM_NEW(ImGui_ImplRgfw_Data)();
    io.BackendPlatformUserData = static_cast<void*>(bd);
    io.BackendPlatformName = "imgui_impl_rgfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; /* We can honor GetMouseCursor() values (optional) */
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  /* We can honor io.WantSetMousePos requests (optional, rarely used) */

    bd->Window = window;
    bd->Time = 0.0;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = ImGui_ImplRgfw_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = ImGui_ImplRgfw_GetClipboardText;
    platform_io.Platform_ClipboardUserData = bd->Window;
#ifdef __EMSCRIPTEN__
    platform_io.PlatformOpenInShellFn = [](ImGuiContext*, const char* url) { ImGui_ImplRgfw_EmscriptenOpenURL(url); return true; };
#endif

    /* Chain RGFW callbacks: our callbacks will call the user's previously installed callbacks, if any. */
    if (install_callbacks) {
        ImGui_ImplRgfw_InstallCallbacks(window);
    }

    /* Set platform dependent data in viewport */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->PlatformHandle = static_cast<void*>(bd->Window);
#if defined(_WIN32) || defined(__APPLE__)
    mainViewport->PlatformHandleRaw = bd->Window->src.window;
#else
    IM_UNUSED(mainViewport);
#endif

    bd->ClientApi = client_api;
    return true;
}

bool ImGui_ImplRgfw_InitForOpenGL(RGFW_window* window, bool install_callbacks) {
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_OpenGL);
}

bool ImGui_ImplRgfw_InitForVulkan(RGFW_window* window, bool install_callbacks) {
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_Vulkan);
}

bool ImGui_ImplRgfw_InitForOther(RGFW_window* window, bool install_callbacks) {
    return ImGui_ImplRgfw_Init(window, install_callbacks, RgfwClientApi_Unknown);
}

void ImGui_ImplRgfw_Shutdown() {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    if (bd->InstalledCallbacks) {
        ImGui_ImplRgfw_RestoreCallbacks(bd->Window);
    }

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos);
    platform_io.ClearPlatformHandlers();
    IM_DELETE(bd);
}

static void ImGui_ImplRgfw_UpdateMouseData() {
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    /* (those braces are here to reduce diff with multi-viewports support in 'docking' branch) */
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
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || (bd->Window->internal.flags & (1L<<2))) {
        return;
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    /* (those braces are here to reduce diff with multi-viewports support in 'docking' branch) */
    {
        RGFW_window* window = bd->Window;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            RGFW_window_showMouse(window, 0);
        }
        else {
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

static void ImGui_ImplRgfw_GetWindowSizeAndFramebufferScale(RGFW_window* window, ImVec2* out_size, ImVec2* out_framebuffer_scale) {
    i32 winW, winH;
    RGFW_window_getSize(window, &winW, &winH);

    i32 displayW, displayH;
    RGFW_window_getSizeInPixels(window, &displayW, &displayH);
    float xScale = winW > 0 ? static_cast<float>(displayW) / static_cast<float>(winW) : 1.0f;
    float yScale = winH > 0 ? static_cast<float>(displayH) / static_cast<float>(winH) : 1.0f;

    if (out_size != nullptr) {
        *out_size = ImVec2(static_cast<float>(winW), static_cast<float>(winH));
    }
    if (out_framebuffer_scale != nullptr) {
        *out_framebuffer_scale = ImVec2(xScale, yScale);
    }
}

static u64 ImGui_ImplRgfw_GetTimerFreq() {
    u64 freq;
#ifdef RGFW_WINDOWS
    static u64 cachedFreq = 0;
    if (cachedFreq == 0) {
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&cachedFreq));
    }
    freq = cachedFreq;
#elif defined(RGFW_MACOS)
    static u64 cachedFreq = 0;
    if (cachedFreq == 0) {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        cachedFreq = static_cast<u64>((info.denom * 1e9) / info.numer);
    }
    freq = cachedFreq;
#elif defined(RGFW_WASM)
    freq = 1000llu;
#else
    freq = 1000000000llu;
#endif
    return freq;
}

static u64 ImGui_ImplRgfw_GetTimerValue() {
    u64 value;
#ifdef RGFW_WINDOWS
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&value));
#elif defined(RGFW_WASM)
        value = static_cast<u64>(emscripten_get_now() * 1e+6);
#elif defined(RGFW_MACOS)
        value = static_cast<u64>(mach_absolute_time());
#else
        static i32 selectedClock = -1;
        struct timespec ts;
        if (selectedClock == -1) {
#if defined(_POSIX_MONOTONIC_CLOCK)
            if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
                selectedClock = CLOCK_MONOTONIC;
            } else {
                selectedClock = CLOCK_REALTIME;
            }
#else
            selectedClock = CLOCK_REALTIME;
#endif
        }
        clock_gettime(selectedClock, &ts);
        value = static_cast<u64>(ts.tv_sec) * ImGui_ImplRgfw_GetTimerFreq() + static_cast<u64>(ts.tv_nsec);
#endif
    return value;
}

static double ImGui_ImplRgfw_GetTime() {
    return static_cast<double>(ImGui_ImplRgfw_GetTimerValue()) / static_cast<double>(ImGui_ImplRgfw_GetTimerFreq());
}

void ImGui_ImplRgfw_NewFrame() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplRgfw_InitForXXX()?");

    /* Setup display size (every frame to accommodate for window resizing) */
    ImGui_ImplRgfw_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);

    /* Setup time step */
    double currentTime = ImGui_ImplRgfw_GetTime();
    if(currentTime <= bd->Time) {
        currentTime = bd->Time + 0.000001;
    }
    io.DeltaTime = bd->Time > 0.0 ? static_cast<float>(currentTime - bd->Time) : 1.0f / 60.0f;
    bd->Time = currentTime;

    ImGui_ImplRgfw_UpdateMouseData();
    ImGui_ImplRgfw_UpdateMouseCursor();
}

float ImGui_ImplRgfw_GetContentScaleForWindow(RGFW_window* window) {
    float contentScale = 1.0f;
#if defined(RGFW_WINDOWS) || defined(RGFW_UNIX)
    RGFW_monitor* mon = RGFW_window_getMonitor(window);
    float xScale, yScale;
    RGFW_monitor_getScale(mon, &xScale, &yScale);
    contentScale = xScale < yScale ? yScale : xScale;
#else
    IM_UNUSED(window);
#endif
    return contentScale;
}

float ImGui_ImplRgfw_GetContentScaleForMonitor(RGFW_monitor* monitor) {
    float contentScale = 1.0f;
#if defined(RGFW_WINDOWS) || defined(RGFW_UNIX)
    float xScale, yScale;
    RGFW_monitor_getScale(monitor, &xScale, &yScale);
    contentScale = xScale < yScale ? yScale : xScale;
#else
    IM_UNUSED(monitor);
#endif
    return contentScale;
}

/* -------------------------------------------------------------------------- */

#endif /* #ifndef IMGUI_DISABLE */
#endif /* #ifdef RGFW_IMGUI_IMPLEMENTATION */
