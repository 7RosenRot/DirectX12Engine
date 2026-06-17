#include <Framework/Scene/Scene.hpp>

#include <Framework/GameObject/GameObject.hpp>
#include <Framework/AssetManager/AssetManager.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>
#include <fstream>
#include <sstream>

void Scene::Initialize(AssetManager* pAssetManager) {
  m_pAssetManager = pAssetManager;
}

void Scene::AddGameObject(const std::string& FilePath) {
  if (m_pAssetManager == nullptr) {
    return;
  }

  std::string ObjectName = std::filesystem::path(FilePath).stem().string();

  auto pNewObject = std::make_shared<GameObject>(m_pAssetManager, ObjectName);

  pNewObject->LoadModel(FilePath);

  m_GameObjects[ObjectName] = pNewObject;
}

void Scene::UpdateScene(const float MovementSpeed, const float MouseSensivity, bool BlockCameraInput) {
  if (!BlockCameraInput) {
    m_ActiveCamera.InputProcessing(MovementSpeed, MouseSensivity);
  }
}

void Scene::RenderScene(D3D12Engine::DirectX12Graphics& rRenderer, std::shared_ptr<GameObject> pSelectedObject) {
  DirectX::XMMATRIX View = m_ActiveCamera.GetTransform().GetMatrixView();
  DirectX::XMMATRIX Projection = m_ActiveCamera.GetMatrixProjection();

  for (auto& [ObjectName, pGameObject] : m_GameObjects) {
    pGameObject->UpdateModelMatrix(View, Projection);

    bool isSelected = (pGameObject == pSelectedObject);
    rRenderer.DrawFrame(
      *pGameObject->GetModel(),
      *pGameObject->GetTexture(),
      pGameObject->GetModelMatrix(),
      isSelected
    );
  }
}

void Scene::SaveScene(const std::string& FilePath) {
  std::ofstream File(FilePath);
  if (!File.is_open()) return;

  for (auto& [Name, pObj] : m_GameObjects) {
    File << "GAMEOBJECT " << Name << "\n";
    File << "MODEL " << pObj->GetModelPath() << "\n";
    File << "TEXTURE " << pObj->GetTexturePath() << "\n";
    
    auto pos = pObj->GetTransform().GetPosition();
    File << "POSITION " << pos.x << " " << pos.y << " " << pos.z << "\n";
    
    auto rot = pObj->GetTransform().GetRotation();
    File << "ROTATION " << rot.x << " " << rot.y << " " << rot.z << "\n";
    
    auto scale = pObj->GetTransform().GetScale();
    File << "SCALE " << scale.x << " " << scale.y << " " << scale.z << "\n";
  }
}

void Scene::LoadScene(const std::string& FilePath) {
  std::ifstream File(FilePath);
  if (!File.is_open()) return;

  m_GameObjects.clear();

  std::string Line;
  std::shared_ptr<GameObject> pCurrentObj = nullptr;

  while (std::getline(File, Line)) {
    if (Line.empty()) continue;
    
    std::istringstream Iss(Line);
    std::string Token;
    Iss >> Token;

    if (Token == "GAMEOBJECT") {
      std::string Name;
      std::getline(Iss >> std::ws, Name);
      pCurrentObj = std::make_shared<GameObject>(m_pAssetManager, Name);
      m_GameObjects[Name] = pCurrentObj;
    } else if (pCurrentObj) {
      if (Token == "MODEL") {
        std::string Path;
        std::getline(Iss >> std::ws, Path);
        if (!Path.empty()) pCurrentObj->LoadModel(Path);
      } else if (Token == "TEXTURE") {
        std::string Path;
        std::getline(Iss >> std::ws, Path);
        if (!Path.empty()) pCurrentObj->LoadTexture(Path);
      } else if (Token == "POSITION") {
        float x, y, z;
        Iss >> x >> y >> z;
        pCurrentObj->GetTransform().SetPosition(x, y, z);
      } else if (Token == "ROTATION") {
        float x, y, z;
        Iss >> x >> y >> z;
        pCurrentObj->GetTransform().SetRotation(x, y, z);
      } else if (Token == "SCALE") {
        float x, y, z;
        Iss >> x >> y >> z;
        pCurrentObj->GetTransform().SetScale(x, y, z);
      }
    }
  }
}