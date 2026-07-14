// physics.h — gravity + axis-separated AABB collision against static solids.
// Owns no entities; callers hand it a Body per step. Solids registered once at init.
#pragma once

namespace physics {

struct AABB { float x, y, w, h; };

struct Body {
  AABB  box;
  float vx = 0, vy = 0;
  bool  grounded = false;   // true iff resting on a solid's top face this step
};

// Register the static collision set (borrowed pointer — caller keeps it alive).
// M2b: hardcoded test terrain from main.cpp. M3: level module owns this.
void setSolids(const AABB* solids, int count);

// Integrate gravity + velocity, resolve against solids (X axis, then Y).
void step(Body& b, float dt);

// Solid list accessor (renderer draws the M2b placeholder terrain from it).
const AABB* solids(int& countOut);

}
