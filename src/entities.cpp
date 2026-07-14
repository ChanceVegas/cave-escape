// entities.cpp — player: input → physics body, pit respawn, placeholder render.
#include "entities.h"
#include "physics.h"
#include "input.h"
#include "config.h"
#include "board_config.h"

namespace {

struct Player {
  physics::Body body;
  float prevX = 0, prevY = 0;   // position at start of last update tick (render interp)
  float drawX = 0, drawY = 0;   // interpolated position latched by beginRender
};

Player s_player;

void respawn() {
  s_player.body.box = { PLAYER_SPAWN_X, PLAYER_SPAWN_Y, PLAYER_W, PLAYER_H };
  s_player.body.vx = 0;
  s_player.body.vy = 0;
  s_player.body.grounded = false;
  s_player.prevX = PLAYER_SPAWN_X;
  s_player.prevY = PLAYER_SPAWN_Y;
}

// Intersect a world-space rect with the band's rows and fill it.
void fillRectInBand(lgfx::LGFX_Sprite& band, int32_t bandY,
                    int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
  int32_t y0 = y - bandY;
  int32_t y1 = y0 + h;
  if (y1 <= 0 || y0 >= band.height()) return;
  if (y0 < 0) y0 = 0;
  if (y1 > band.height()) y1 = band.height();
  band.fillRect(x, y0, w, y1 - y0, color);
}

} // namespace

namespace entities {

bool init() {
  respawn();
  return true;
}

void update(float dt) {
  const input::State& in = input::state();

  // Horizontal: direct velocity from stick. Same authority in air (full air
  // control) — classic platformer feel; revisit at M5 tuning if too floaty.
  s_player.body.vx = in.moveX * PLAYER_SPEED_PX_S;

  // Jump: edge-triggered by input module, only honored when grounded.
  if (in.jumpPressed && s_player.body.grounded)
    s_player.body.vy = -PLAYER_JUMP_VEL_PX_S;

  // Save pre-step position for render interpolation, then step.
  s_player.prevX = s_player.body.box.x;
  s_player.prevY = s_player.body.box.y;
  physics::step(s_player.body, dt);

  // Screen-edge clamp (M2b only: camera is fixed; M3 world scroll replaces this).
  if (s_player.body.box.x < 0) s_player.body.box.x = 0;
  if (s_player.body.box.x > LCD_WIDTH - PLAYER_W)
    s_player.body.box.x = LCD_WIDTH - PLAYER_W;

  // Fell into a pit: off-screen bottom = death → respawn (lives arrive at M4).
  if (s_player.body.box.y > LCD_HEIGHT + PLAYER_PIT_MARGIN_PX) respawn();
}

void beginRender(float alpha) {
  s_player.drawX = s_player.prevX + (s_player.body.box.x - s_player.prevX) * alpha;
  s_player.drawY = s_player.prevY + (s_player.body.box.y - s_player.prevY) * alpha;
}

void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY) {
  // Placeholder terrain: draw the physics solids directly so what you see IS
  // the collision set. Deleted at M3 when level chunks own terrain art.
  int n;
  const physics::AABB* s = physics::solids(n);
  for (int i = 0; i < n; ++i)
    fillRectInBand(band, bandY, (int32_t)s[i].x, (int32_t)s[i].y,
                   (int32_t)s[i].w, (int32_t)s[i].h, COLOR_TERRAIN_DEBUG);

  // Player: plain rect for now (sprites module arrives later in M2).
  fillRectInBand(band, bandY, (int32_t)s_player.drawX, (int32_t)s_player.drawY,
                 PLAYER_W, PLAYER_H, COLOR_PLAYER_DEBUG);
}

} // namespace entities
