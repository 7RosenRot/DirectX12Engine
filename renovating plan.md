Engine/
│
├── Core/
│   │
│   ├── Logger/
│   │   └── Logger   *(class)*
│   │
│   ├── Math/
│   │   └── Math.hpp *(inline lib)*
│   │
│   └── EventSystem/
│       ├── EventBus *(class)*
│       └── EventProcessor *(class)*
│
├── Application/
│   │
│   ├── Kernel/
│   │   └── Kernel   *(class)*
│   ├── Window/
│   │   └── Window   *(class)*
│   └── Input/
│       └── Input    *(class)*
│
├── Framework/
│   │
│   ├── AssetManager/
│   │   ├── IResourceLoader.hpp *(interface)*
│   │   ├── Loaders/
│   │   │   ├── ObjLoader      *(class)*
│   │   │   └── PngLoader  *(class)*
│   │   └── Resources/
│   │       ├── IMesh.hpp       *(interface)*
│   │       └── IMaterial.hpp   *(interface)*
│   │
│   ├── ECS/
│   │   ├── Core/
│   │   │   ├── Coordinator      *(class)*
│   │   │   ├── EntityManager    *(class)*
│   │   │   ├── ComponentManager *(class)*
│   │   │   └── SystemManager    *(class)*
│   │   ├── Components/
│   │   │   ├── Transform.hpp    *(component)*
│   │   │   ├── Mesh.hpp         *(component)*
│   │   │   ├── Material.hpp     *(component)*
│   │   │   └── Camera.hpp       *(component)*
│   │   └── Systems/
│   │       ├── Render.hpp
│   │       └── Camera.hpp
│   │
│   └── UI/
│       ├── UI *(class)*
│       ├── Panels/
│       │   ├── Viewport.hpp *(component)*
│       │   ├── Browser.hpp  *(component)*
│       │   └── Overview.hpp *(component)*
│       └── Commands/
│           └── CommandHistory/
│               ├──ICommand              *(interface)*
│               ├──TransformCommand      *(class)*
│               ├──MultiTransformCommand *(class)*
│               ├──DeleteCommand         *(class)*
│               └──CommandHistory        *(class)*
│
├── Renderer/
│   ├── IRenderer.hpp
│   ├── ICommandContext.hpp
│   └── Backend/
│       ├── D3D12/
│       │   ├── D3D12Renderer.cpp
│       │   ├── D3D12Model.cpp
│       │   ├── D3D12Texture.cpp
│       │   └── RHI/
│       └── VK3D/
│
└── main.cpp

## Описание компонентов и модулей
### Core
#### Logger
- Класс логирования, уже есть, ничего не меняем.
#### Math
- inline-библиотека, обертка над DirectXMath. Мы полностью отказываемся от использования #include <DirectXMath> внтури движка. Необходимо реализовать свои классы `Rose::Math::Vector`/`Rose::Math::Matrix4x4` и так далее.
#### EventSystem
- Теперь любое событие внутри движка - Изменения размера окна/Пользовательский ввод - попадают на внутреннюю шину событий `EventBus`. Обрабочик `EventProcessor` забирает последнее события с шины и обрабатывает его - вызывает `Resize(newWidth, newHeight)`, `Input(Key, true)`

### Application
#### Kernel
- Полностью очистить от лишней инициализации и владений. Никаких больше `std::unique_ptr<AssetManager>` и других. Теперь мы позволяем ядру владеть только главными модулями движка: Window, UI, Renderer. Ядро инициализирует модули - создает окно, передает хэндл, инициализирует одно из API используя паттерн фабрики. Любые вызовы API производятся через IRenderer.
#### Window
- Создание окна, прослушивание ивентов в WndProc. Уже есть, ничего писать не нужно, максимум корректировать
#### Input
- Вычистить всю лишнюю логику событий - теперь за это отвечает EventSystem модуль

### Framework
#### Assets
- Сюда мигрирует AssetManager. Мы переделываем всю систему загрузки и хранения с нуля.
1. *Вызов AssetManager*
1. 1. Допустим, UI обработал нажатие кнопки `Import`, он обратился к AssetManager как `m_pAssetManager->GetMesh("/path/to/(.obj, .fbx, .png, ...)")`.

2. *Обход ресурсов*
2. 1. AssetManger хранит `std::unordered_map<std::string, uint32_t>` - список уже загруженных ресурсов `<"/path/to/(.obj, .fbx, .png, ...)", ID>`.
2. 2. Если есть такой ресурс - AssetManager тут же вернет его ID, что позволит быстро вызвать ресурс из памяти.s
2. 3. Если такого ресурса пока нет - AssetManger должен загрузить его.

3. *Вызов загрузчика ресурса*
3. 1. AssetManger обязан сам определить, какой именно класс-загрузчик вызвать.
3. 2. Для этого AssetManger хранит `std::list<IResourceLoader> m_ResourceLoader`, а сам IResourceLoader имеет `virtual std::string GetPattern() const = 0;`, который отдельно реализован в каждом из классов загрузчиков (ObjLoader/PngLoader/FbxLoader/...) как например `std::string GetPattern() const override { return "*.obj"; }` для ObjLoader класса.
3. 3. AssetManger в цикле по всем элментам списка вызывает метод `GetPattern()` и, найдя нужный, обращается к методу `LoadResource("/path/to/(.obj, .fbx, .png, ...)")` и попадает на нужный, благодаря виртуализации.

4. *Загрузка ресурса*
4. 1. Внтури метода LoadResource() мы только читаем файл и переводим данные либо в массив вершин, либо в матрицу текстуры и так далее в ОЗУ. Однако, классы (ObjLoader/PngLoader/FbxLoader/...) полностью изолированы от API и не должны знать о низком уровне.
4. 2. Внутри метода LoadResource() идет обращение через `IRenderer->CreateMesh(Verticies)` (метод из D3D12/VK3DMesh)
4. 3. Сам метод CreateMesh() при этом имеет тип возвращаемого значения `std::shared_ptr<IMesh> CreateMesh(...)`
4. 4. Виртуальный класс IMesh соджержит методы, необходимые для доступа к загруженным данным, так например `virtual uint32_t GetVertexCount() const = 0;` и `virtual uint32_t GetIndexCount() const = 0;`, а сами же эти методы реализованы внутри D3D12Mesh или VK3DMesh как `uint32_t GetIndexCount() const override { return m_IndexCount; }` и `uint32_t GetVertexCount() const override { return m_VertexCount; }`

5. *Возврат изапись ресурса в реестр*
5. 1. Сам метод LoadResource() возвращает либо `std::shared_ptr<IMesh>`, либо `std::shared_ptr<IMaterial>`, либо другой тип, если необходимо.
5. 2. AssetManager записывает полученный ресурс в `std::unordered_map<path, ID>` и добавляет его `в ResourceRegistry<IMesh>`

Хорошо, еще раз сверим пайплайн загрузки и работы с ассетами:
--- Слой UI/
1. Обработка "Import", вызов `m_pAssetManger->UploadResource(path);`

--- Слой AssetManger/
Система хранения выглядит как:
```C++
  // хранит список поддерживаемых расширений
  enum class ResourceType : uint32_t { Mesh, Texture, Audio, Animation, Unknown };
  // структура для быстрого доступа к ресурсам
  struct ResourceDesc {
    ResourceType Type = ResourceType::Unknown;
    uint32_t Index = -1;
  };
  // мапа для быстрого доступа к нужному элементу по пути
  std::unordered_map<std::string, ResourceDesc> ResourceMap;
  // вектор всех когда-либо загруженных ассетов
  std::vector<std::shared_ptr<IMesh>> m_pMeshes;
  std::vector<std::shared_ptr<ITextrue>> m_pTextures;
```

Как это работает?

*Внутри `uint32_t UploadResource(std::string Path) {...}`* 
1. Проверяем `std::unordered_map<Path, Index> ResourceMap`
1. 1. Запись есть - вернуть Index (сигнал успеха)
```C++
  if (ResourceMap.contains(Path)) {
    return ResourceMap[Path].Index;
  }
```
1. 2. Записи нет - идем дальше

2. Загрузка ресурса
2. 1. Вычленяем расширение
```C++
  std::string FileExtension = std::filesystem::path(path).extension().string();
```
2. 2. Определяем загручик и выполняем загрузку
```C++
  // *Алгоритм загрузки* //
  std::shared_ptr<IResource> pResource;
  
  for (const auto& ResourceLoader : m_pResourceLoaders) {
    if (ResourceLoader->GetPattern() == FileExtension) {
      pResource = ResourceLoader->LoadResource(Path);
    }
  }
```

3. Записываем ресрурс в реестр
```C++
  ResourceDesc newResource = {};
  
  if (FileExtension == "obj") {
    std::shared_ptr<IMesh> pMesh = std::static_pointer_cast<pResource>;

    m_pMeshes.push_back(pMesh);

    uint32_t Index = static_cast<uint32_t>(m_pMeshes.size() - 1);

    newResource = { ResourceType::Mesh, Index };

    ResourceMap.emplace(Path, newResource);
    
    return newResource;
  }

  if (FileExtension == "png") {...}

  if (...) {...}

  return -1;
```

--- Слой AssetManger/Loaders
4. Загружаем ресурс
4. 1. Внтури метода `LoadResource()` мы только читаем файл и переводим данные либо в массив вершин, либо в матрицу текстуры и так далее в ОЗУ. Однако, классы (ObjLoader/PngLoader/FbxLoader/...) полностью изолированы от API и не должны знать о низком уровне.
4. 2. Внутри метода `LoadResource()` идет обращение через `IRenderer->CreateMesh(Verticies)` (метод из D3D12/VK3DMesh)
--- Слой Renderer
4. 3. Сам метод CreateMesh() при этом имеет тип возвращаемого значения `std::shared_ptr<IMesh> CreateMesh(...)`
4. 4. Виртуальный класс IMesh соджержит методы, необходимые для доступа к загруженным данным, так например `virtual uint32_t GetVertexCount() const = 0;` и `virtual uint32_t GetIndexCount() const = 0;`, а сами же эти методы реализованы внутри D3D12Mesh или VK3DMesh как `uint32_t GetIndexCount() const override { return m_IndexCount; }` и `uint32_t GetVertexCount() const override { return m_VertexCount; }`

--- Слой AssetManger/Loaders
5. Возврат изапись ресурса в реестр
5. 1. Сам метод LoadResource() возвращает либо `std::shared_ptr<IMesh>`, либо `std::shared_ptr<IMaterial>`, либо другой тип, если необходимо.

#### ECS
#### UI

Механики движка:
### Новый пайплан загрузки объектов
1. Я хочу добавить на сцену *"car.obj"*
  - Я нажимая клавишу *"Import"* в верхней панели браузера или клавишу *"Файл+"* прямо в списке доступных (уже загруженнных) объектов (эта клавиша всегда в конце списка).
  - Затем я прохожу по ФС и нахожу нужный `.obj` файл и добавляю.
  - Теперь объект есть в списке доступных и я могу добавить его на сцену.
  - Чтобы поместить его на сцену я нажимаю на него ЛКМ и тяну в Viewport, таким образом он оказывается на сцене в нуле координат.
- *(!)* Таким образом файл проходит путь: Нет выделенной памяти -> Память выделена один раз, объект не отрисован -> Обращение к одному и тому же участку памяти и отрисовка объекта n-раз.

### Добавление Event Bus системы
2. Я хочу изменить размер всего окна приложения/окна Viewport *(изменение любой из панелей также ведет к изменению Viewport)*
  - Существует соответственно два *listener*. Первый в `Window.cpp` и слушает изменение самого окна приложения, а второй в `EngineUI.cpp` и слушает измение Viewport.
  - Если *listener* определяет изменение, он создает сообщение в `Events.hpp`.
- *(!)* Таким образом необходжимо предусмотреть *reader*, который считывает такое сообщение (берет последнее из очереди) и исполняет либо `OnResize`, либо обработку ввода и так далее.

3. Пользователь совершил ввод (WASD/Mouse Move - LMB, MMB, RMB)
  - Логика точно такая же как и в случае с ресайзом, так как мы используем механизм ивентов и шины ивентов.

### Приветственный экран и сохранение/сохарнение в файл/открытие проекта
4. Как теперь выглядит взаимодействие с движком (относительно проекта):
  - Я запускаю движок. Сам движок еще не запускается, вместо него я вижу приветсвенный экран (как у Visual Studio 2022). Есть три пути развития:
  - *Создание нового проекта*. Нажал на одноименную кнопку, открылся чистый проект. В папке temp создается файл Untitled-1.rose. Например я добавил кубик на сцену и должен идти по делам - я просто нажимаю Project -> Save и состояние сцены записывается в Untitled-1.rose. Потом я вернулся, снова открылся приветсвенный экран и я просто выбираю Untitled-1.rose из недавних - и возвращаюсь к пректу
  - *Открытие готового проекта*. Вернемся к моменту сохранения - я нажал не Save, а Save As и например выбрал рабочий стол для проекта. Теперь я могу нажать клавишу Open Project и выбрать свой созхраненный проект
  - *Открытие проекта .rose*. Я просто сразу на рабочем столе дабл-клик по проекту Scene.rose - и сразу открывается движок с загруженной сценой моего проекта Scene.rose