// entities.cpp — player: input → physics body, pit respawn, placeholder render.
#include "entities.h"
#include "physics.h"
#include "input.h"
#include "config.h"
#include "board_config.h"
#include "camera.h"
#include "level.h"

namespace {

struct Player {
  physics::Body body;
  float prevX = 0, prevY = 0;   // position at start of last update tick (render interp)
  float drawX = 0, drawY = 0;   // interpolated position latched by beginRender
};

Player s_player;

void respawn() {
  // M3b: respawn at last chunk boundary crossed (level::checkpointX). Chunk
  // authoring guarantees >=96 px flat floor at every chunk start, so
  // checkpoint + PLAYER_SPAWN_X (40) always lands on ground. Camera snaps
  // back with the player (respawn-only ratchet exception). Retires M3A-1.
  float rx = level::checkpointX() + PLAYER_SPAWN_X;
  camera::snapTo(level::checkpointX());
  s_player.body.box = { rx, PLAYER_SPAWN_Y, PLAYER_W, PLAYER_H };
  s_player.body.vx = 0;
  s_player.body.vy = 0;
  s_player.body.grounded = false;
  s_player.prevX = rx;
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

  // Back-edge clamp (M3a): camera ratchets forward only, so its left edge is
  // the world's trailing wall. No right clamp — walking right drags the camera.
  if (s_player.body.box.x < camera::x())
    s_player.body.box.x = camera::x();

  // Fell into a pit: below screen bottom = death → respawn (lives arrive at M4).
  // World Y == screen Y (camera scrolls horizontally only).
  if (s_player.body.box.y > LCD_HEIGHT + PLAYER_PIT_MARGIN_PX) { respawn(); return; }

  // Spike hazards (M3c): AABB overlap = death → respawn. Checked after the
  // physics step so the tested position is where the player actually is.
  int nh;
  const physics::AABB* hz = level::hazards(nh);
  const physics::AABB& p = s_player.body.box;
  for (int i = 0; i < nh; ++i) {
    if (p.x < hz[i].x + hz[i].w && p.x + p.w > hz[i].x &&
        p.y < hz[i].y + hz[i].h && p.y + p.h > hz[i].y) { respawn(); return; }
  }
}

void beginRender(float alpha) {
  s_player.drawX = s_player.prevX + (s_player.body.box.x - s_player.prevX) * alpha;
  s_player.drawY = s_player.prevY + (s_player.body.box.y - s_player.prevY) * alpha;
}

void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY) {
  // World→screen: subtract interpolated camera X. Player and camera use the
  // SAME frame alpha, so their difference is beat-free on screen.
  const int32_t camX = (int32_t)camera::drawX();

  // Placeholder terrain: draw the physics solids directly so what you see IS
  // the collision set. Deleted at M3b when level chunks own terrain art.
  int n;
  const physics::AABB* s = physics::solids(n);
  for (int i = 0; i < n; ++i) {
    int32_t sx = (int32_t)s[i].x - camX;
    if (sx + (int32_t)s[i].w <= 0 || sx >= LCD_WIDTH) continue;  // off-screen
    fillRectInBand(band, bandY, sx, (int32_t)s[i].y,
                   (int32_t)s[i].w, (int32_t)s[i].h, COLOR_TERRAIN_DEBUG);
  }

  // Spike hazards (M3c): drawn box == hitbox, in warning yellow.
  int nh;
  const physics::AABB* hz = level::hazards(nh);
  for (int i = 0; i < nh; ++i) {
    int32_t zx = (int32_t)hz[i].x - camX;
    if (zx + (int32_t)hz[i].w <= 0 || zx >= LCD_WIDTH) continue;
    fillRectInBand(band, bandY, zx, (int32_t)hz[i].y,
                   (int32_t)hz[i].w, (int32_t)hz[i].h, COLOR_HAZARD_DEBUG);
  }

  // Player: plain rect for now (sprites module arrives at M3d/M5).
  fillRectInBand(band, bandY, (int32_t)s_player.drawX - camX,
                 (int32_t)s_player.drawY, PLAYER_W, PLAYER_H, COLOR_PLAYER_DEBUG);
}

float playerX() { return s_player.body.box.x; }

} // namespace entities
