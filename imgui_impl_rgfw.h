/*
    dear imgui RGFW backend
    This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
*/

#pragma once

#ifndef RGFW_IMGUI_H
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

#ifndef IMGUI_DISABLE

#define RGFW_IMGUI_H

#include <stdbool.h>
#include <chrono>

typedef struct RGFW_window RGFW_window;

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
IMGUI_IMPL_API void     ImGui_ImplRgfw_WindowFocusCallback(RGFW_window* window, u8 inFocus);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplRgfw_CursorEnterCallback(RGFW_window* window, i32 x, i32 y, u8 status);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplRgfw_CursorPosCallback(RGFW_window* window, i32 x, i32 y, float vecX, float vecY);   // Since 1.87
IMGUI_IMPL_API void     ImGui_ImplRgfw_MouseButtonCallback(RGFW_window* window, u8 button, u8 pressed);
IMGUI_IMPL_API void     ImGui_ImplRgfw_MouseScrollCallback(RGFW_window* window, float xoffset, float yoffset);
IMGUI_IMPL_API void     ImGui_ImplRgfw_KeyCallback(RGFW_window* window, u8 keycode, u8 keyChar, u8 modState, u8 repeat, u8 pressed);
IMGUI_IMPL_API void     ImGui_ImplRgfw_CharCallback(RGFW_window* window, unsigned int c);
#endif /* ifndef RGFW_IMGUI_H */

#ifdef RGFW_IMGUI_IMPLEMENTATION

#define RGFWDEF
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
    RGFW_focusfunc      PrevUserCallbackWindowFocus;
    RGFW_mousePosfunc        PrevUserCallbackCursorPos;
    RGFW_mouseNotifyfunc      PrevUserCallbackCursorEnter;
    RGFW_mouseButtonfunc      PrevUserCallbackMousebutton;
    RGFW_mouseScrollfunc      PrevUserCallbackMouseScroll;
    RGFW_keyfunc              PrevUserCallbackKey;

    ImGui_ImplRgfw_Data()   { memset(static_cast<void*>(this), 0, sizeof(*this)); }
};

static ImGui_ImplRgfw_Data* ImGui_ImplRgfw_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplRgfw_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

char* clipboard_str = nullptr;

// Functions
static const char* ImGui_ImplRgfw_GetClipboardText(void* user_data)
{
    RGFW_UNUSED(user_data);

    size_t size;
    return RGFW_readClipboard(&size);
}

static void ImGui_ImplRgfw_SetClipboardText(void* user_data, const char* text)
{
    RGFW_UNUSED(user_data);
    RGFW_UNUSED(text);
    RGFW_writeClipboard(text, static_cast<u32>(strlen(text)));
}

static ImGuiKey ImGui_ImplRgfw_KeyToImGuiKey(int key)
{
    switch(key) {
        case RGFW_escape:       return ImGuiKey_Escape;
        case RGFW_backtick:     return ImGuiKey_GraveAccent;
        case RGFW_0:            return ImGuiKey_0;
        case RGFW_1:            return ImGuiKey_1;
        case RGFW_2:            return ImGuiKey_2;
        case RGFW_3:            return ImGuiKey_3;
        case RGFW_4:            return ImGuiKey_4;
        case RGFW_5:            return ImGuiKey_5;
        case RGFW_6:            return ImGuiKey_6;
        case RGFW_7:            return ImGuiKey_7;
        case RGFW_8:            return ImGuiKey_8;
        case RGFW_9:            return ImGuiKey_9;
        case RGFW_minus:        return ImGuiKey_Minus;
        case RGFW_equals:       return ImGuiKey_Equal;
        case RGFW_backSpace:    return ImGuiKey_Backspace;
        case RGFW_tab:          return ImGuiKey_Tab;
        case RGFW_space:        return ImGuiKey_Space;
        case RGFW_a:            return ImGuiKey_A;
        case RGFW_b:            return ImGuiKey_B;
        case RGFW_c:            return ImGuiKey_C;
        case RGFW_d:            return ImGuiKey_D;
        case RGFW_e:            return ImGuiKey_E;
        case RGFW_f:            return ImGuiKey_F;
        case RGFW_g:            return ImGuiKey_G;
        case RGFW_h:            return ImGuiKey_H;
        case RGFW_i:            return ImGuiKey_I;
        case RGFW_j:            return ImGuiKey_J;
        case RGFW_k:            return ImGuiKey_K;
        case RGFW_l:            return ImGuiKey_L;
        case RGFW_m:            return ImGuiKey_M;
        case RGFW_n:            return ImGuiKey_N;
        case RGFW_o:            return ImGuiKey_O;
        case RGFW_p:            return ImGuiKey_P;
        case RGFW_q:            return ImGuiKey_Q;
        case RGFW_r:            return ImGuiKey_R;
        case RGFW_s:            return ImGuiKey_S;
        case RGFW_t:            return ImGuiKey_T;
        case RGFW_u:            return ImGuiKey_U;
        case RGFW_v:            return ImGuiKey_V;
        case RGFW_w:            return ImGuiKey_W;
        case RGFW_x:            return ImGuiKey_X;
        case RGFW_y:            return ImGuiKey_Y;
        case RGFW_z:            return ImGuiKey_Z;
        case RGFW_period:       return ImGuiKey_Period;
        case RGFW_comma:        return ImGuiKey_Comma;
        case RGFW_slash:        return ImGuiKey_Slash;
        case RGFW_bracket:      return ImGuiKey_LeftBracket;
        case RGFW_closeBracket: return ImGuiKey_RightBracket;
        case RGFW_semicolon:    return ImGuiKey_Semicolon;
        case RGFW_apostrophe:   return ImGuiKey_Apostrophe;
        case RGFW_backSlash:    return ImGuiKey_Backslash;
        case RGFW_return:       return ImGuiKey_Enter;  // Same as RGFW_enter
        case RGFW_delete:       return ImGuiKey_Delete;
        case RGFW_F1:           return ImGuiKey_F1;
        case RGFW_F2:           return ImGuiKey_F2;
        case RGFW_F3:           return ImGuiKey_F3;
        case RGFW_F4:           return ImGuiKey_F4;
        case RGFW_F5:           return ImGuiKey_F5;
        case RGFW_F6:           return ImGuiKey_F6;
        case RGFW_F7:           return ImGuiKey_F7;
        case RGFW_F8:           return ImGuiKey_F8;
        case RGFW_F9:           return ImGuiKey_F9;
        case RGFW_F10:          return ImGuiKey_F10;
        case RGFW_F11:          return ImGuiKey_F11;
        case RGFW_F12:          return ImGuiKey_F12;
        case RGFW_F13:          return ImGuiKey_F13;
        case RGFW_F14:          return ImGuiKey_F14;
        case RGFW_F15:          return ImGuiKey_F15;
        case RGFW_F16:          return ImGuiKey_F16;
        case RGFW_F17:          return ImGuiKey_F17;
        case RGFW_F18:          return ImGuiKey_F18;
        case RGFW_F19:          return ImGuiKey_F19;
        case RGFW_F20:          return ImGuiKey_F20;
        case RGFW_F21:          return ImGuiKey_F21;
        case RGFW_F22:          return ImGuiKey_F22;
        case RGFW_F23:          return ImGuiKey_F23;
        case RGFW_F24:          return ImGuiKey_F24;
        case RGFW_F25:          return ImGuiKey_None;  // No ImGuiKey_F25
        case RGFW_capsLock:     return ImGuiKey_CapsLock;
        case RGFW_shiftL:       return ImGuiKey_LeftShift;
        case RGFW_controlL:     return ImGuiKey_LeftCtrl;
        case RGFW_altL:         return ImGuiKey_LeftAlt;
        case RGFW_superL:       return ImGuiKey_LeftSuper;
        case RGFW_shiftR:       return ImGuiKey_RightShift;
        case RGFW_controlR:     return ImGuiKey_RightCtrl;
        case RGFW_altR:         return ImGuiKey_RightAlt;
        case RGFW_superR:       return ImGuiKey_RightSuper;
        case RGFW_up:           return ImGuiKey_UpArrow;
        case RGFW_down:         return ImGuiKey_DownArrow;
        case RGFW_left:         return ImGuiKey_LeftArrow;
        case RGFW_right:        return ImGuiKey_RightArrow;
        case RGFW_insert:       return ImGuiKey_Insert;
        case RGFW_menu:         return ImGuiKey_Menu;
        case RGFW_end:          return ImGuiKey_End;
        case RGFW_home:         return ImGuiKey_Home;
        case RGFW_pageUp:       return ImGuiKey_PageUp;
        case RGFW_pageDown:     return ImGuiKey_PageDown;
        case RGFW_numLock:      return ImGuiKey_NumLock;
        case RGFW_kpSlash:      return ImGuiKey_KeypadDivide;
        case RGFW_kpMultiply:   return ImGuiKey_KeypadMultiply;
        case RGFW_kpPlus:       return ImGuiKey_KeypadAdd;
        case RGFW_kpMinus:      return ImGuiKey_KeypadSubtract;
        case RGFW_kpEqual:      return ImGuiKey_KeypadEqual;
        case RGFW_kp1:          return ImGuiKey_Keypad1;
        case RGFW_kp2:          return ImGuiKey_Keypad2;
        case RGFW_kp3:          return ImGuiKey_Keypad3;
        case RGFW_kp4:          return ImGuiKey_Keypad4;
        case RGFW_kp5:          return ImGuiKey_Keypad5;
        case RGFW_kp6:          return ImGuiKey_Keypad6;
        case RGFW_kp7:          return ImGuiKey_Keypad7;
        case RGFW_kp8:          return ImGuiKey_Keypad8;
        case RGFW_kp9:          return ImGuiKey_Keypad9;
        case RGFW_kp0:          return ImGuiKey_Keypad0;
        case RGFW_kpPeriod:     return ImGuiKey_KeypadDecimal;
        case RGFW_kpReturn:     return ImGuiKey_KeypadEnter;
        case RGFW_scrollLock:   return ImGuiKey_ScrollLock;
        case RGFW_printScreen:  return ImGuiKey_PrintScreen;
        case RGFW_pause:        return ImGuiKey_Pause;
        case RGFW_world1:       return ImGuiKey_None;  // Not supported
        case RGFW_world2:       return ImGuiKey_None;  // Not supported
        default:                return ImGuiKey_None;
    }
}

static bool ImGui_ImplRgfw_ShouldChainCallback(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void ImGui_ImplRgfw_MouseButtonCallback(RGFW_window* window, u8 button, u8 pressed)
{
    if (button == RGFW_mouseMiddle) button = RGFW_mouseRight;
    else if (button == RGFW_mouseRight) button = RGFW_mouseMiddle;

    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackMousebutton != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackMousebutton(window, button, pressed);

    ImGuiIO& io = ImGui::GetIO();
    if (button < ImGuiMouseButton_COUNT) {
        io.AddMouseButtonEvent(button, pressed);
    }
}

void ImGui_ImplRgfw_MouseScrollCallback(RGFW_window* window, float xoffset, float yoffset)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackMouseScroll != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackMouseScroll(window, xoffset, yoffset);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent(xoffset, yoffset);
}

void ImGui_ImplRgfw_KeyCallback(RGFW_window* window, u8 key, u8 keyChar, u8 modState, RGFW_bool repeat, RGFW_bool pressed)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackKey != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackKey(window, key, keyChar, modState, repeat, pressed);

    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, modState & RGFW_modControl);
    io.AddKeyEvent(ImGuiMod_Shift, modState & RGFW_modShift);
    io.AddKeyEvent(ImGuiMod_Alt,  modState & RGFW_modAlt);
    io.AddKeyEvent(ImGuiMod_Super, modState & RGFW_modSuper);
    io.AddKeyEvent(ImGuiMod_Super, modState & RGFW_modSuper);

    if (pressed == RGFW_TRUE) {
        ImGui_ImplRgfw_CharCallback(window, keyChar);
    }

    ImGuiKey imgui_key = ImGui_ImplRgfw_KeyToImGuiKey(key);
    io.AddKeyEvent(imgui_key, pressed);
    //io.SetKeyEventNativeData(imgui_key, keyChar, key); // To support legacy indexing (<1.87 user code)
}

void ImGui_ImplRgfw_WindowFocusCallback(RGFW_window* window, RGFW_bool inFocus)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackWindowFocus != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackWindowFocus(window, inFocus);

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(inFocus != 0);
}

void ImGui_ImplRgfw_CursorPosCallback(RGFW_window* window, i32 x, i32 y, float vecX, float vecY)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackCursorPos != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackCursorPos(window, x, y, vecX, vecY);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
    bd->LastValidMousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
void ImGui_ImplRgfw_CursorEnterCallback(RGFW_window* window, i32 x, i32 y, RGFW_bool status)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    if (bd->PrevUserCallbackCursorEnter != nullptr && ImGui_ImplRgfw_ShouldChainCallback(window))
        bd->PrevUserCallbackCursorEnter(window, x, y, status);

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

void ImGui_ImplRgfw_CharCallback(RGFW_window* window, unsigned int c)
{
    RGFW_UNUSED(window);
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void ImGui_ImplRgfw_InstallCallbacks(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    IM_ASSERT(bd->Window == window);

    /*
        TODO: RGFW doesn't have anyway to do this yet
        update this when I add it to RGFW
    */

    bd->PrevUserCallbackWindowFocus = RGFW_setFocusCallback(ImGui_ImplRgfw_WindowFocusCallback);
    bd->PrevUserCallbackCursorEnter = RGFW_setMouseNotifyCallback(ImGui_ImplRgfw_CursorEnterCallback);
    bd->PrevUserCallbackCursorPos = RGFW_setMousePosCallback(ImGui_ImplRgfw_CursorPosCallback);
    bd->PrevUserCallbackMousebutton = RGFW_setMouseButtonCallback(ImGui_ImplRgfw_MouseButtonCallback);
    bd->PrevUserCallbackMouseScroll = RGFW_setMouseScrollCallback(ImGui_ImplRgfw_MouseScrollCallback);
    bd->PrevUserCallbackKey = RGFW_setKeyCallback(ImGui_ImplRgfw_KeyCallback);
    bd->InstalledCallbacks = true;
}

void ImGui_ImplRgfw_RestoreCallbacks(RGFW_window* window)
{
    ImGui_ImplRgfw_Data* bd = ImGui_ImplRgfw_GetBackendData();
    IM_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    IM_ASSERT(bd->Window == window);

    RGFW_setFocusCallback(bd->PrevUserCallbackWindowFocus);
    RGFW_setMouseNotifyCallback(bd->PrevUserCallbackCursorEnter);
    RGFW_setMousePosCallback(bd->PrevUserCallbackCursorPos);
    RGFW_setMouseButtonCallback(bd->PrevUserCallbackMousebutton);
    RGFW_setKeyCallback(bd->PrevUserCallbackKey);
    bd->InstalledCallbacks = false;
    bd->PrevUserCallbackWindowFocus = nullptr;
    bd->PrevUserCallbackCursorEnter = nullptr;
    bd->PrevUserCallbackCursorPos = nullptr;
    bd->PrevUserCallbackMousebutton = nullptr;
    bd->PrevUserCallbackKey = nullptr;
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

    io.SetClipboardTextFn = ImGui_ImplRgfw_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplRgfw_GetClipboardText;
    io.ClipboardUserData = bd->Window;
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

#endif // #ifndef IMGUI_DISABLE

#endif /* RGFW_IMGUI_IMPLEMENTATION */
