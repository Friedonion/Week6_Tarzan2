#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "UnrealClient.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/EditorEngine.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Renderer/StaticMeshRenderPass.h"
#include "Renderer/BillboardRenderPass.h"
#include "Renderer/DepthBufferDebugPass.h"
#include "Renderer/FogRenderPass.h"
#include "Renderer/GizmoRenderPass.h"
#include "Renderer/LineRenderPass.h"
#include "Engine/EditorEngine.h"
#include "World/World.h"
#include "atomic"
#include "thread"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

FGraphicsDevice FEngineLoop::GraphicDevice;
FRenderer FEngineLoop::Renderer;
UPrimitiveDrawBatch FEngineLoop::PrimitiveDrawBatch;
FResourceMgr FEngineLoop::ResourceManager;
uint32 FEngineLoop::TotalAllocationBytes = 0;
uint32 FEngineLoop::TotalAllocationCount = 0;
std::atomic<bool> g_bShaderThreadExit(false);

FEngineLoop::FEngineLoop()
    : AppWnd(nullptr)
    , UIMgr(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
    , bufferManager(nullptr)
{
}

int32 FEngineLoop::PreInit()
{
    return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
    /* must be initialized before window. */
    WindowInit(hInstance);

    UnrealEditor = new UnrealEd();

    bufferManager = new FDXDBufferManager();

    UIMgr = new UImGuiManager;

    AppMessageHandler = std::make_unique<FSlateAppMessageHandler>();

    LevelEditor = new SLevelEditor();

    UnrealEditor->Initialize();

    GraphicDevice.Initialize(AppWnd);

    bufferManager->Initialize(GraphicDevice.Device, GraphicDevice.DeviceContext);

    Renderer.Initialize(&GraphicDevice, bufferManager);

    PrimitiveDrawBatch.Initialize(&GraphicDevice);

    UIMgr->Initialize(AppWnd, GraphicDevice.Device, GraphicDevice.DeviceContext);

    ResourceManager.Initialize(&Renderer, &GraphicDevice);

    LevelEditor->Initialize();

    GEngine = FObjectFactory::ConstructObject<UEditorEngine>(nullptr);
    GEngine->Init();

    return 0;
}


void FEngineLoop::Render() const
{
    GraphicDevice.Prepare(LevelEditor->GetActiveViewportClient());
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> viewportClient = GetLevelEditor()->GetActiveViewportClient();
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetViewportClient(i);
            Renderer.PrepareRender();
            Renderer.Render(LevelEditor->GetActiveViewportClient());
        }
        GetLevelEditor()->SetViewportClient(viewportClient);
    }
    else
    {
        Renderer.PrepareRender();
        Renderer.Render(LevelEditor->GetActiveViewportClient());
    }
}

void FEngineLoop::Tick()
{
    LARGE_INTEGER frequency;
    const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER startTime, endTime;
    double elapsedTime = 0.0;

    std::thread hotReloadThread(&FEngineLoop::HotReloadShader, this, L"Shaders");

    while (bIsExit == false)
    {
        QueryPerformanceCounter(&startTime);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
            DispatchMessage(&msg);  // 메시지를 WndProc에 전달

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        float DeltaTime = elapsedTime / 1000.f;

        GEngine->Tick(DeltaTime);
        LevelEditor->Tick(DeltaTime);
        Render();
        UIMgr->BeginFrame();
        UnrealEditor->Render();

        Console::GetInstance().Draw();

        UIMgr->EndFrame();

        // Pending 처리된 오브젝트 제거
        GUObjectArray.ProcessPendingDestroyObjects();

        GraphicDevice.SwapBuffer();

        // 쉐이더 파일 변경 감지, 핫 리로드
        if (bShaderChanged)
        {
            Renderer.ShaderManager->ReleaseAllShader();
            Renderer.StaticMeshRenderPass->CreateShader();
            Renderer.BillboardRenderPass->CreateShader();
            Renderer.GizmoRenderPass->CreateShader();
            Renderer.DepthBufferDebugPass->CreateShader();
            Renderer.FogRenderPass->CreateShader();
            Renderer.LineRenderPass->CreateShader();
            Renderer.ChangeViewMode(LevelEditor->GetActiveViewportClient()->GetViewMode());
            bShaderChanged = false;
        }
        
        do
        {
            Sleep(0);
            QueryPerformanceCounter(&endTime);
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.f / frequency.QuadPart;
        } while (elapsedTime < targetFrameTime);
    }

    // 메인스레드 종료전에 파일 탐색 스레드 종료
    g_bShaderThreadExit = true;
    hotReloadThread.join();
}

float FEngineLoop::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEngineLoop::HotReloadShader(const std::wstring& dir)
{
    // 감시할 디렉토리의 변경 이벤트 핸들 얻기
    HANDLE hChange = FindFirstChangeNotificationW(
        dir.c_str(),
        TRUE,   // 하위 디렉토리 포함
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE
    );

    if (hChange == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"FindFirstChangeNotification failed: " << GetLastError() << std::endl;
        return;
    }

    std::wcout << L"Monitoring directory with FindFirstChangeNotification: " << dir << std::endl;

    while (!g_bShaderThreadExit.load())
    {
        // 변경 이벤트가 발생할 때까지 대기
        DWORD waitStatus = WaitForSingleObject(hChange, 1000);
        if (waitStatus == WAIT_OBJECT_0)
        {
            std::wcout << L"Directory change detected!" << std::endl;

            // 쉐이더 리로드
            bShaderChanged = true;

            // 변경 이벤트 핸들을 재설정
            if (!FindNextChangeNotification(hChange))
            {
                std::wcerr << L"FindNextChangeNotification failed: " << GetLastError() << std::endl;
                break;
            }
        }
        else if (waitStatus == WAIT_TIMEOUT)
        {
            continue;
        }
        else
        {
            std::wcerr << L"WaitForSingleObject error: " << GetLastError() << std::endl;
            break;
        }
    }

    FindCloseChangeNotification(hChange);
}

void FEngineLoop::Exit()
{
    LevelEditor->Release();
    UIMgr->Shutdown();
    delete UIMgr;
    ResourceManager.Release(&Renderer);
    Renderer.Release();
    GraphicDevice.Release();
}


void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
    WCHAR WindowClass[] = L"JungleWindowClass";

    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wndclass{};
    wndclass.lpfnWndProc = AppWndProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = WindowClass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClassW(&wndclass);

    AppWnd = CreateWindowExW(
        0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr
    );
}

LRESULT CALLBACK FEngineLoop::AppWndProc(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
    {
        return true;
    }

    switch (Msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            auto LevelEditor = GEngineLoop.GetLevelEditor();
            if (LevelEditor)
            {
                FEngineLoop::GraphicDevice.OnResize(hWnd);
                //UGraphicsDevice 객체의 OnResize 함수 호출
                LevelEditor->ResizeLevelEditor();
            }
        }
        Console::GetInstance().OnResize(hWnd);
        // ControlPanel::GetInstance().OnResize(hWnd);
        // PropertyPanel::GetInstance().OnResize(hWnd);
        // Outliner::GetInstance().OnResize(hWnd);
        // ViewModeDropdown::GetInstance().OnResize(hWnd);
        // ShowFlags::GetInstance().OnResize(hWnd);
        if (GEngineLoop.GetUnrealEditor())
        {
            GEngineLoop.GetUnrealEditor()->OnResize(hWnd);
        }
        ViewportTypePanel::GetInstance().OnResize(hWnd);
        break;
    default:
        GEngineLoop.AppMessageHandler->ProcessMessage(hWnd, Msg, wParam, lParam);
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return 0;
}
