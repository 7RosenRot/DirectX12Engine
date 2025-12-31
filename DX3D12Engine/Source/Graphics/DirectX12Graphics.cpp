#include <Include/Graphics/DirectX12Graphics.hpp>
#include <stdexcept>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName) :
  InterfaceDirectX12(WindowHeight, WindowWidth, WindowName)
  // Добавить соответсвующие поля, которые будут проинициализированы конструктором класса по умолчанию
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  // Подготовливаем среду для работы
  LoadPipeline();
  
  // Устанавливаем контракт между нашим кодом и шейдерами
  LoadAssets();
}

void D3D12Engine::DirectX12Graphics::OnRender() {
  // Выполняем отрисовку
}

void D3D12Engine::DirectX12Graphics::OnUpdate() {
  // Обновляем кадр, сели встречено какое-то изменение
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  // Дожидаемся отрисовки последнего кадра и завершаем конвейер
}

void D3D12Engine::DirectX12Graphics::LoadPipeline() {
  //Если сборка проекта Debug - включаем слой отладки (в случае ошибок - Debug Layer сдеает запись в лог)
  UINT DXGIFactoryFlags{0};
#if defined(_DEBUG)
  {
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
      DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif
  //Создаём главную фабрику (Менеджер). Задачи - поиск и перечисление всех адаптеров, создание Swap Chain, отслеживание, на какой монитор выводится изображение, ...
  Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
  CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&factory4));

  // Выбираем адаптер, который будет рисовать графику: Warp (Software) - нагрузку возьмёт на себя CPU, Hardware - нагрузку берёт на себя GPU
  if (m_useWarpAdapter) {
    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    
    factory4->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
  }
  else {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    
    getHardwareAdapter(factory4.Get(), &hardwareAdapter, true);
    D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
  }

  // Создаём дескриптор очереди комнад GPU. Очередь команд - точка входа в GPU
  D3D12_COMMAND_QUEUE_DESC cmdQueueDescriptor{};
  cmdQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;                     // Нет флагов - стнадартное поведение
  cmdQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;                     // Тип - использовать для отрисовки, рендера, вчислений
  m_device->CreateCommandQueue(&cmdQueueDescriptor, IID_PPV_ARGS(&m_cmdQueue)); // Мы создали объект структуры, проинициализировали его поля, теперь "скопируем" его в подготовленное поле нашего класса

  // Создаём дескриптор Swap Chain, фактически - чертёж, как должен выглядеть механизм вывода изображения 
  DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
  swapChainDescriptor.BufferCount = m_frameCount;                    // Количество буферов (холстов): 2 - двойная буферизация
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Используем наши буферы как место, КУДА отрисовываются пиксели
  swapChainDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // Формат пискеля: RGBA, 8 бит/канал
  swapChainDescriptor.SampleDesc.Count = 1;                          // Отключение MSAA на уровне Swap Chain
  swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // Путь кадра: Нарисовали -> Показали -> Удалили (самый современный и производительный способ)
  swapChainDescriptor.Height = m_WindowHeight;                       // Высота буфера
  swapChainDescriptor.Width = m_WindowWidth;                         // Ширина буфера

  // Создаем временный Swap Chain ver.1 - Почему? CreateSwapChainForHwnd() - не может принять SC ver.3 
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  factory4->CreateSwapChainForHwnd(
    m_cmdQueue.Get(),               // Привязка к нашей очереди команд GPU
    D3D12Engine::Window::getHwnd(), // Дескриптор созданного окна HWND
    &swapChainDescriptor,           // Настройки Swap Chain
    nullptr, nullptr,               // Доп. параметры
    &swapChain                      // Записываем во врменный Swap Chain
  );

  factory4->MakeWindowAssociation(
    D3D12Engine::Window::getHwnd(),  // Дескриптор созданного окна HWND
    DXGI_MWA_NO_ALT_ENTER            // Не используем Alt+Enter - это будет реализовано программно
  );
  
  swapChain.As(&m_swapChain);                              // Обновляем Swap Chain ver.1 -> ver.3
  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex(); // Получаем индекс актуального (отрисовываемого) буфера

  // Создаём дескриптор кучи дескрипторов - блоки данных, описывающие какой-либо ресурс
  D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeap{};
  rtvDescriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Делаем кучу невидимой для шейдеров HLSL
  rtvDescriptorHeap.NodeMask = 0;                            // Если в системе несколько GPU, выбираем только нулевую
  rtvDescriptorHeap.NumDescriptors = m_frameCount;           // Выделяем место ровно под два "холста"
  rtvDescriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;   // Используем эту кучу только для дескрипторов RTV (Render Target View) - "холстов"

  // Выделяем непрерывный участок в памяти видеокарты, где будут храниться описатели (дескрипторы) наших буферов
  m_device->CreateDescriptorHeap(&rtvDescriptorHeap, IID_PPV_ARGS(&m_rtvDescriptorHeap));
  
  // Поскольку размер дескриптора зависит от модели видеокарты, то, чтобы проходиться по массиву дескрипторов, нам нужно знать - где заканчивается один дескриптор и начинается другой, для этого выясняем размер одного RTV дескриптора
  m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // Получаем указатель на начало кучи/массива RTV дескрипторов
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT frame = 0; frame < m_frameCount; frame += 1) {
    m_swapChain->GetBuffer(frame, IID_PPV_ARGS(&m_renderTargets[frame]));               // Получаем из Swap Chain указатель на буфер №frame
    m_device->CreateRenderTargetView(m_renderTargets[frame].Get(), nullptr, rtvHandle); // Возьми текстуру m_renderTargets[frame] и создай на неё описание в ячейке, куда указывает rtvHandle
    rtvHandle.Offset(1, m_rtvDescriptorSize);                                           // Используя ранее полученный размер дескрипотра, сдвигаем казатель на 1 десриптор вперёд
  }

  // Создаём Command Allocator - физическое хранилище команд в байтах
  m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator));
}

void D3D12Engine::DirectX12Graphics::LoadAssets() {}

void D3D12Engine::DirectX12Graphics::FillCommandList() {}

void D3D12Engine::DirectX12Graphics::WaitForPreviousFrame() {}