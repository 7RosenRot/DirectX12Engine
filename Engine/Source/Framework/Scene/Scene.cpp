#include <Framework/Scene/Scene.hpp>

#include <Framework/GameObject/GameObject.hpp>
#include <Framework/AssetManager/AssetManager.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

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

void Scene::UpdateScene(const float MovementSpeed, const float MouseSensivity, bool BlockCameraInput, const DirectX::XMFLOAT3& TargetPoint) {
  if (!BlockCameraInput) {
    m_ActiveCamera.InputProcessing(MovementSpeed, MouseSensivity, TargetPoint);
  }
}

void Scene::RenderScene(D3D12Engine::DirectX12Graphics& rRenderer, const std::vector<std::shared_ptr<GameObject>>& selectedObjects) {
  DirectX::XMMATRIX View = m_ActiveCamera.GetTransform().GetMatrixView();
  DirectX::XMMATRIX Projection = m_ActiveCamera.GetMatrixProjection();

  for (auto& [ObjectName, pGameObject] : m_GameObjects) {
    pGameObject->UpdateModelMatrix(View, Projection);

    bool isSelected = (
      std::find(
        selectedObjects.begin(), selectedObjects.end(), pGameObject) != selectedObjects.end()
    );
    
    rRenderer.DrawNormal(
      *pGameObject->GetModel(),
      *pGameObject->GetTexture(),
      pGameObject->GetModelMatrix(),
      isSelected
    );

    if (isSelected) {
      rRenderer.DrawStencil(
        *pGameObject->GetModel(),
        pGameObject->GetModelMatrix()
      );

      rRenderer.DrawOutline(
        *pGameObject->GetModel(),
        pGameObject->GetModelMatrix()
      );
    }
  }
}

void Scene::SaveScene(const std::string& FilePath) {
  std::ofstream File(FilePath);
  if (!File.is_open()) return;

  for (auto& [ObjectName, pObject] : m_GameObjects) {
    File << "GAMEOBJECT " << ObjectName << "\n";
    File << "MODEL " << pObject->GetModelPath() << "\n";
    File << "TEXTURE " << pObject->GetTexturePath() << "\n";
    
    auto pos = pObject->GetTransform().GetPosition();
    File << "POSITION " << pos.x << " " << pos.y << " " << pos.z << "\n";
    
    auto rot = pObject->GetTransform().GetRotation();
    File << "ROTATION " << rot.x << " " << rot.y << " " << rot.z << "\n";
    
    auto scale = pObject->GetTransform().GetScale();
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
      std::string ObjectName;
      std::getline(Iss >> std::ws, ObjectName);
      pCurrentObj = std::make_shared<GameObject>(m_pAssetManager, ObjectName);
      m_GameObjects[ObjectName] = pCurrentObj;
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
        pCurrentObj->GetTransform().SetRotation(
          DirectX::XMConvertToDegrees(x),
          DirectX::XMConvertToDegrees(y),
          DirectX::XMConvertToDegrees(z)
        );
      } else if (Token == "SCALE") {
        float x, y, z;
        Iss >> x >> y >> z;
        pCurrentObj->GetTransform().SetScale(x, y, z);
      }
    }
  }
}