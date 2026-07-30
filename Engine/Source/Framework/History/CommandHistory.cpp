#include <Framework/History/CommandHistory.hpp>
#include <Framework/Scene/Scene.hpp>

TransformCommand::TransformCommand(
  std::shared_ptr<GameObject> pObject,
  const Transform& OldTransform,
  const Transform& NewTransform
) : 
  m_pObject(std::move(pObject)),
  m_OldTransform(OldTransform),
  m_NewTransform(NewTransform) 
{}

void TransformCommand::Execute() {
  if (m_pObject != nullptr) {
    m_pObject->GetTransform() = m_NewTransform;
  }
}

void TransformCommand::Undo() {
  if (m_pObject != nullptr) {
    m_pObject->GetTransform() = m_OldTransform;
  }
}

MultiTransformCommand::MultiTransformCommand(
  std::vector<std::shared_ptr<GameObject>> pObjects,
  std::vector<Transform> OldTransforms,
  std::vector<Transform> NewTransforms
) :
  m_pObjects(std::move(pObjects)),
  m_OldTransforms(std::move(OldTransforms)),
  m_NewTransforms(std::move(NewTransforms))
{}

void MultiTransformCommand::Execute() {
  for (size_t i = 0; i < m_pObjects.size(); ++i) {
    if (m_pObjects[i] != nullptr) {
      m_pObjects[i]->GetTransform() = m_NewTransforms[i];
    }
  }
}

void MultiTransformCommand::Undo() {
  for (size_t i = 0; i < m_pObjects.size(); ++i) {
    if (m_pObjects[i] != nullptr) {
      m_pObjects[i]->GetTransform() = m_OldTransforms[i];
    }
  }
}

DeleteCommand::DeleteCommand(
  Scene* pScene,
  std::vector<std::shared_ptr<GameObject>> pObjects
) :
  m_pScene(pScene),
  m_pObjects(std::move(pObjects))
{}

void DeleteCommand::Execute() {
  if (m_pScene != nullptr) {
    for (const auto& pObj : m_pObjects) {
      if (pObj != nullptr) {
        m_pScene->RemoveGameObject(pObj->GetObjectName());
      }
    }
  }
}

void DeleteCommand::Undo() {
  if (m_pScene != nullptr) {
    for (const auto& pObj : m_pObjects) {
      if (pObj != nullptr) {
        m_pScene->GetGameObjects()[pObj->GetObjectName()] = pObj;
      }
    }
  }
}

void CommandHistory::ExecuteCommand(std::unique_ptr<ICommand> pCommand) {
  pCommand->Execute();

  m_UndoStack.push_back(
    std::move(pCommand)
  );
  m_RedoStack.clear();
}

void CommandHistory::Undo() {
  if (!m_UndoStack.empty()) {
    auto& pCommand = m_UndoStack.back();

    pCommand->Undo();

    m_RedoStack.push_back(
      std::move(pCommand)
    );
    m_UndoStack.pop_back();
  }
}

void CommandHistory::Redo() {
  if (!m_RedoStack.empty()) {
    auto& pCommand = m_RedoStack.back();

    pCommand->Execute();

    m_UndoStack.push_back(
      std::move(pCommand)
    );
    m_RedoStack.pop_back();
  }
}
