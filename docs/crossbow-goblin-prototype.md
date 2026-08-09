# Crossbow Goblin Prototype

The crossbow goblin is a second, explicit enemy type rather than a generic enemy framework.

- It uses a 3x3 character-cell aiming grid, but only the two rows farthest from the goblin are Commit cells. The
  three near cells express minimum range. If the player is not in one of the six valid cells, the goblin moves toward
  the nearest matching stand-off position, including when the player is too close.
- Telegraph holds the goblin in place while continuously updating its aim. The final direction and target position
  lock only on the launch frame; the arrow never retargets after that.
- Its arrow performs fixed-point XY interpolation with a display-only height offset.
- A flying arrow has no Hitbox and deliberately performs no terrain collision checks.
- Only the one-frame landing state owns a small Hitbox.
- An arrow stores launch and target positions only, so it completes after its firing goblin dies or respawns.
- `CrossbowProjectilePool` has a fixed capacity of four and clears only after landing or scene reset.
