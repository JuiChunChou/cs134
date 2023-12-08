SJSU - CS 134 – Game Design Programming – Final Project
3D Lander – Putting it all Together

The LEM must be able to maneuver (left, right, forward, back, up/down and rotate along it’s UP axis)
using thrust force using a full physics simulation (which includes thrust, gravity and turbulence forces.)
This was covered completely in the midterm for the 2D case. You need to add another dimension. A
non-physics way of moving or a hacked physics implementation will not receive credit. Students can
choose to write a custom simulator as we did on the midterm or use the particle simulator example.
Since the particle simulator supports external forces, this may be an easier starting point.

• The craft will be equipped with a telemetry sensor to detect altitude (AGL) above ground level and
display it on the screen. You must use ray-based collision detection to accomplish this task. The ray must
intersect the 3D terrain. The feature should be able to be turned on/off with a hotkey.
Done by: Rishi

• The ship must use a particle emitter for the rocket exhaust. A particle emitter must be used for the
explosion.
Done by: Rishi

• Particles must be rendered using a shader otherwise it will be too slow. An example will be given in
class.

• The ship must be able to do full collision detection with the terrain (including mountains and landing
area). If the ship contacts the terrain, the collision must be resolved with a suitable impulse force that
can be demonstrated. You are only required to have one (1) bounding box on your vehicle, but your
terrain should be spatially partitioned using an octree. Your demo video must show collision with the
landing area AND mountainous terrain to prove that it is reliable.

• Use lighting techniques learned in class to light the landing area scene using OpenGL lighting (ofLight)
using at least three (3) lights. Adding an additional light on the spacecraft that can be turned on/off is an
interesting effect.
Done by Rishi:

• The player should be able to switch between multiple camera views which include a minimum of: 1) A
“tracking” camera that stays aimed at the spacecraft from a fixed location. 2) One onboard. Can be
aimed in different directions, depending on your scenario.
o The player will also have to have access to the EasyCam camera to be able to navigate anywhere
they like over the surface.
Done by: Rishi

o The EasyCam camera should have an option so that you can retarget the view to a point or the
spacecraft.

• Sound is required for effects (spacecraft thrust) and any background sounds you would like.

• For diagnostic purposes, you should have a feature (such as in the) to be able to manually move the
spacecraft around the scene using the mouse directly and then start your game from that position.

• The scene should have a simple 2D image background (such a starfield)

Teams are required to model at least one of the models in the game. You can choose the ship or
terrain. If you choose terrain it must meet the criteria below (substituting content). Models will be
judged based on completeness and how they contribute to the compelling nature of the game. If you
turn in an oversimplified model, you may not receive that much credit.
Done by Rishi:

