#pragma once

#include <memory>
#include <vector>
#include <Framework/GameObject/GameObject.hpp>
#include <Framework/Transform/Transform.hpp>

class ICommand {
 public:
  virtual ~ICommand() = default;

  virtual void Execute() = 0;
  virtual void Undo() = 0;
};

class TransformCommand : public ICommand {
 public:
  TransformCommand(
    std::shared_ptr<GameObject> pObject,
    const Transform& OldTransform,
    const Transform& NewTransform
  );
  ~TransformCommand() override = default;

  void Execute() override;
  void Undo() override;

 private:
  std::shared_ptr<GameObject> m_pObject;
  Transform m_OldTransform;
  Transform m_NewTransform;
};

class CommandHistory {
 public:
  CommandHistory() = default;
  ~CommandHistory() = default;

  void ExecuteCommand(
    std::unique_ptr<ICommand> pCommand
  );
  
  void Undo();
  void Redo();

 private:
  std::vector<std::unique_ptr<ICommand>> m_UndoStack;
  std::vector<std::unique_ptr<ICommand>> m_RedoStack;
};
