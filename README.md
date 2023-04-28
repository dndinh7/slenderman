# SLENDERMAN

Here is a very simple take on Mark J. Hadley's _Slenderman: The Eight Pages_, which is a horror game based around a creepy pasta character called Slenderman. The objective of the game is to collect all 8 pages before Slenderman catches you and you lose. 

You are spawned at the center of a forest and on 8 different trees are pages that you have to collect. Warning: once you collect your first page, Slenderman will start to appear around the forest. AVOID looking at him, or you will lose health. If you look directly at Slenderman, your health will deplete in a couple of seconds. Be careful, as you will not know your current hp. Your hp will recover slowly over time as you avoid Slenderman. 

Controls:
- 'WASD' to move
- 'LEFT-SHIFT' to run
- 'F' to turn flashlight on and off
- 'E' to collect a page when you are near it
- 'Hold RMB and move mouse' to pan camera

## How to build

*Windows*

Open git bash to the directory containing this repository.

```
project-template $ mkdir build
project-template $ cd build
project-template/build $ cmake ..
project-template/build $ start project-template.sln
```

Your solution file should contain four projects.
To run from the git bash command shell, 

```
project-template/build $ ../bin/Debug/game.exe
```


## Demo of basic features

https://user-images.githubusercontent.com/72237791/235048125-5e8b3bcd-6ffe-40a7-a0ed-14a116ecbb9b.mp4

The player can move around using 'WASD' keys and look around by 'right mouse drag'. A flashlight follows the player and lights up the scene using a spotlight shader. The player can also sprint by holding the 'left shift key'.

Player movement is created by rotating the player's x and z axis along the horizontal plane given an azimuth angle, so that it is clamped and does not have a y-displacement. Thus, the player then moves in the corresponding x and z axis directions when they use 'WASD'. LERP is used to create a smoother walking experience. However, the camera is free to move up and down given an elevation angle.

The shader implements spotlight and fog, the former using a variant of the phong-shader model with a direction and FOV angle and the latter using the distance of objects from the viewer to proportionally hide objects.

## Unique features 

There is an entity called Slenderman who is textured, and he will randomly spawn behind the player once they have collected the first page. Slenderman's spawn is calculated using Rodrigues' rotation formula, which computes a point given a vector v and angle of rotation theta. ![image](https://user-images.githubusercontent.com/72237791/235049458-951dd8ed-505d-43aa-a67f-9dd4b72b7828.png). This computes a rotated vector, but I used the vector to find an end point for Slenderman afterwards. k is denoted as a unit vector that vector v rotates around. k is a unit normal vector to v.

As well, there is a mechanic that when the player looks at Slenderman, they will take damage proportional to how direct they look at Slenderman. A smaller angle between the center of the player's camera and Slenderman will result in more severe damage to the player. This calculation is done by using the dot product between the player's forward direction and the vector from the player to Slenderman. The angle is then computed using arccos, and when the angle falls under a certain threshold, the player will take damage.

The trees and pages are billboards which will always face the direction of the user using a heading angle found from taking the arc tangent of the x and z components of the vector from the billboard to the player.

The trees are procedurally placed around the map using Poisson's disk algorithm, which delineates a grid of points that trees can be placed in. A random point is first sampled and placed into the grid. Thereafter, 15 more points are sampled based on that newly inserted point in the grid. If any of the points are not in the same neighborhood (a given minimum distance) as any other point on the grid, then that point will be added to the grid and 15 more points shall be sampled based on that newly inserted point. This process is repeated until there are no more points to be processed within the grid (this will most likely happen when no more points can fit into the grid).

I used a shadertoy mentioned in the references to glitch the scene when the player looks at Slenderman with less than 15 degrees between their camera direction and them to Slenderman. This shadertoy affects the entire scene. This glitch is also used periodically during the losing screen.

I have also added sounds such as ambient cricket noise, flashlight button click, and the classic Slenderman sighting sound.

## References:
- Smooth camera LERPING for movement: https://superhedral.com/2021/10/30/lerping-cameras-in-unity/

- Spawning slenderman behind the player using their forward axis and an angle of rotation: https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula

- Spawning tree billboards using poisson's disk algorithm for 'packing': http://devmag.org.za/2009/05/03/poisson-disk-sampling/

- Glitch shader toy used on losing screen and looking directly (within a 15 degree angle) at Slenderman in game: https://www.shadertoy.com/view/XtK3W3

## Resources:
- Slenderman model: https://free3d.com/3d-model/slenderman-from-slender-the-arrival-79467.html

- Fir tree texture: https://www.pngwing.com/en/free-png-ykplr

- Pine tree texture: https://www.pngwing.com/en/free-png-babso/download

- Flashlight mesh and texture: https://sketchfab.com/3d-models/flashlight-texture-3116044b0cb74ab2a392cbefc53cb723#download

- Grass texture: https://opengameart.org/content/64-billboard-grass-texture-and-mesh




