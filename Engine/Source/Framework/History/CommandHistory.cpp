#include <Framework/History/CommandHistory.hpp>

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
