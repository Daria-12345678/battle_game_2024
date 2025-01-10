#pragma once
#include "battle_game/core/unit.h"

namespace battle_game::unit {
class Soldier : public Unit {
 public:
  Soldier(GameCore *game_core, uint32_t id, uint32_t player_id);
  void Render() override;
  void Update() override;
  [[nodiscard]] bool IsHit(glm::vec2 position) const override;

 protected:
  void SoldierMove(float move_speed, float rotate_angular_speed);
  void TurretRotate();
  void Fire();
  [[nodiscard]] const char *UnitName() const override;
  [[nodiscard]] const char *Author() const override;

  uint32_t steps_moved_{0};  // Keep track of how many key presses have been made
  float turret_rotation_{0.0f};
  uint32_t fire_count_down_{0};
};
}  // namespace battle_game::unit