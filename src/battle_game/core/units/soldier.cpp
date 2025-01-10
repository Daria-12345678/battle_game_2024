#include "soldier.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t soldier_body_model_index = 0xffffffffu;
uint32_t soldier_turret_model_index = 0xffffffffu;
}  // namespace

Soldier::Soldier(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~soldier_body_model_index) {
    auto mgr = AssetsManager::GetInstance();
    // Soldier Body Model (simple cube)
    soldier_body_model_index = mgr->RegisterModel(
        {
            {{-0.5f, 0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        },
        {0, 1, 2, 1, 2, 3});
    // Soldier Turret Model (rectangle = gun)
    std::vector<ObjectVertex> turret_vertices;
    std::vector<uint32_t> turret_indices;
    turret_vertices.push_back(
        {{0.0f, 1.2f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
    turret_vertices.push_back(
        {{0.2f, 1.2f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
    turret_vertices.push_back(
        {{-0.2f, 1.2f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
    turret_indices = {0, 1, 2};
    soldier_turret_model_index =
        mgr->RegisterModel(turret_vertices, turret_indices);
  }
}

void Soldier::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(soldier_body_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(soldier_turret_model_index);
}

void Soldier::Update() {
  SoldierMove(2.0f, glm::radians(90.0f));
  TurretRotate();
  Fire();
}

void Soldier::SoldierMove(float move_speed, float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f};
    bool key_pressed = false;

    if (input_data.key_down[GLFW_KEY_W]) {
      offset.y += 1.0f;
      key_pressed = true;
    }
    if (input_data.key_down[GLFW_KEY_S]) {
      offset.y -= 1.0f;
      key_pressed = true;
    }

    if (key_pressed) {
      steps_moved_++;
    }

    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};

    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
      position_ = new_position;
    }

    float rotation_offset = 0.0f;
    if (input_data.key_down[GLFW_KEY_A]) {
      rotation_offset += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      rotation_offset -= 1.0f;
    }
    rotation_offset *= kSecondPerTick * rotate_angular_speed * GetSpeedScale();
    game_core_->PushEventRotateUnit(id_, rotation_ + rotation_offset);
  }
}

void Soldier::TurretRotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    if (glm::length(diff) < 1e-4) {
      turret_rotation_ = rotation_;
    } else {
      turret_rotation_ = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
  }
}

void Soldier::Fire() {
  if (steps_moved_ >= 100 && fire_count_down_ == 0) {  // Require 100 key presses
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_LEFT]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, turret_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = kTickPerSecond;  // Fire interval 1 second
        steps_moved_ = 0;  // Reset key count after shooting
      }
    }
  }
  if (fire_count_down_) {
    fire_count_down_--;
  }
}

bool Soldier::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.5f && position.x < 0.5f && position.y > -0.5f &&
         position.y < 0.5f;
}

const char *Soldier::UnitName() const {
  return "Soldier";
}

const char *Soldier::Author() const {
  return "Dasha";
}

}  // namespace battle_game::unit