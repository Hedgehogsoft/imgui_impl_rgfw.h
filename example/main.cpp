#ifdef _MSC_VER
    #define TRAP() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define TRAP() __builtin_trap()
#else
    #error "Unknown trap intrinsic for this compiler."
#endif
#define ASSERT_ALWAYS(x) do { if (!(x)) { TRAP(); } } while(0)

#define RGFW_ASSERT ASSERT_ALWAYS
#define RGFW_DEBUG /* for debug logs */
#define RGFW_OPENGL

/*
    Alternative: imgui_impl_rgfw.h includes RGFW.h internally
    if you don't need to include RGFW.h explicitly here, you can condense this block into:
    #define RGFW_IMPLEMENTATION
    #define RGFW_IMGUI_IMPLEMENTATION
    #include "../imgui_impl_rgfw.h"
*/
#define RGFW_IMPLEMENTATION
#include "RGFW.h"

#define IM_ASSERT ASSERT_ALWAYS
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#define RGFW_IMGUI_IMPLEMENTATION
#include "../imgui_impl_rgfw.h"

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

int main() {
    RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    hints->major = 3;
    hints->minor = 3;

    RGFW_windowFlags window_flags = RGFW_windowCenter | RGFW_windowScaleToMonitor | RGFW_windowOpenGL;
    RGFW_window* window = RGFW_createWindow("rgfw-imgui", 0, 0, 800, 600, window_flags);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#ifdef RGFW_IMGUI_DOCKING_EXAMPLE
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   /* (Unsupported) Enable Multi-Viewport / Platform Windows */
#endif

    ImGui::StyleColorsDark();

    ImGui_ImplRgfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_pollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplRgfw_NewFrame();
        ImGui::NewFrame();

        /* Adjust ImGui UI for HiDPI screens */
        {
            float old_scale = io.FontGlobalScale;
            float content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(window);
            io.FontGlobalScale = content_scale;
            ImGui::GetStyle().ScaleAllSizes(content_scale / old_scale);
        }

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");
                ImGui::Text("This is some useful text.");
                ImGui::Checkbox("Demo Window", &show_demo_window);
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);

                if (ImGui::Button("Button")) {
                    counter++;
                }
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        if (show_another_window) {
            ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me")) {
                    show_another_window = false;
                }
            ImGui::End();
        }

        ImGui::Render();
        i32 pixel_width, pixel_height;
        RGFW_window_getSizeInPixels(window, &pixel_width, &pixel_height);
        glViewport(0, 0, pixel_width, pixel_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef RGFW_IMGUI_DOCKING_EXAMPLE
        /* (Unsupported) Update and render additional Platform Windows */
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            RGFW_window_makeCurrentWindow_OpenGL(window);
        }
#endif

        RGFW_window_swapBuffers_OpenGL(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplRgfw_Shutdown();
    ImGui::DestroyContext();
    RGFW_window_close(window);
    return 0;
}
