#ifndef PHYSICS_H__
#define PHYSICS_H__

#include <spirit/Physics/spPhysicsWorld.h>
#include <spirit/Physics/spBulletWorld.h>
#include <spirit/spSettings.h>

class Physics {
 public:
  Physics();
  ~Physics();
  void Create(const spPhyEngineType phy_type);
  void AddObject(spCommonObject& obj);
  void Iterate();

 private:
  std::shared_ptr<spPhysicsWorld> phyworld_;
};

#endif  // PHYSICS_H__