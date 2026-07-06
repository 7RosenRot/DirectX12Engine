#pragma once

#include <memory>
#include <vector>
#include <Framework/GameObject/GameObject.hpp>
#include <Framework/Transform/Transform.hpp>

class Scene;

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

class MultiTransformCommand : public ICommand {
 public:
  MultiTransformCommand(
    std::vector<std::shared_ptr<GameObject>> pObjects,
    std::vector<Transform> OldTransforms,
    std::vector<Transform> NewTransforms
  );
  ~MultiTransformCommand() override = default;

  void Execute() override;
  void Undo() override;

 private:
  std::vector<std::shared_ptr<GameObject>> m_pObjects;
  std::vector<Transform> m_OldTransforms;
  std::vector<Transform> m_NewTransforms;
};

class DeleteCommand : public ICommand {
 public:
  DeleteCommand(
    Scene* pScene,
    std::vector<std::shared_ptr<GameObject>> pObjects
  );
  ~DeleteCommand() override = default;

  void Execute() override;
  void Undo() override;

 private:
  Scene* m_pScene;
  std::vector<std::shared_ptr<GameObject>> m_pObjects;
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