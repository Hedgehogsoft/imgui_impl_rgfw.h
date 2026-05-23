#include "imgui_unity.h"
#include "imgui_unity.cpp"

#define RGFW_ASSERT IM_ASSERT
#define RGFW_OPENGL

/*
    Alternative: imgui_impl_rgfw.h includes RGFW internally
    if you don't need to include RGFW.h explicity here, you can condense this block into:
    #define RGFW_IMPLEMENTATION
    #define RGFW_IMGUI_IMPLEMENTATION
    #include "../imgui_impl_rgfw.h"
*/
#define RGFW_IMPLEMENTATION
#include "RGFW.h"
#undef RGFW_IMPLEMENTATION /* undef so the backend won't implement it again */

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; /* Enable Keyboard Controls */
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  /* Enable Gamepad Controls */
#ifdef RGFW_IMGUI_DOCKING_EXAMPLE
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     /* Enable Docking */
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   /* (Unsupported) Enable Multi-Viewport / Platform Windows */
#endif

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    /* Setup Platform/Renderer backends */
    ImGui_ImplRgfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_pollEvents();

        /* Start the Dear ImGui Frame */
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

        /* 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!). */
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        /* 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window. */
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                              /* Create a window called "Hello, world!" and append into it */
                ImGui::Text("This is some useful text.");               /* Display some text (you can use a format strings too) */
                ImGui::Checkbox("Demo Window", &show_demo_window);      /* Edit bools storing our window open/close state */
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            /* Edit 1 float using a slider from 0.0f to 1.0f */
                ImGui::ColorEdit3("clear color", (float*)&clear_color); /* Edit 3 floats representing a color */

                if (ImGui::Button("Button"))                            /* Buttons return true when clicked (most widgets return true when edited/activated) */
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        /* 3. Show another simple window. */
        if (show_another_window) {
            ImGui::Begin("Another Window", &show_another_window); /* Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked) */
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
            ImGui::End();
        }

        /* Rendering */
        ImGui::Render();
        i32 pixel_width, pixel_height;
        RGFW_window_getSizeInPixels(window, &pixel_width, &pixel_height);
        glViewport(0, 0, pixel_width, pixel_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef RGFW_IMGUI_DOCKING_EXAMPLE
        /* (Unsupported) Update and render additional Platform Windows */
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     RGFW_window_makeCurrentWindow_OpenGL(window);
        // }
#endif

        RGFW_window_swapBuffers_OpenGL(window);
    }

    /* Cleanup */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplRgfw_Shutdown();
    ImGui::DestroyContext();
    RGFW_window_close(window);
    return 0;
}
