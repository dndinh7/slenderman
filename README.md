# SLENDERMAN

Here is a very simple take on Mark J. Hadley's _Slenderman: The Eight Pages_, which is a horror game based around a creepy pasta character called Slenderman. The objective of the game is to collect all 8 pages before Slenderman catches you and you lose. 

You are spawned at the center of a forest and on 8 different trees are pages that you have to collect. Warning: once you collect your first page, Slenderman will start to appear around the forest. AVOID looking at him, or you will lose health. The more direct you look at him, the more your health will diminish. Be careful, you will not know your current hp, but it will recover over time slowly as you avoid Slenderman. 

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
project-template/build $ ../bin/Debug/demo.exe
```

*macOS*

Open terminal to the directory containing this repository.

```
project-template $ mkdir build
project-template $ cd build
project-template/build $ cmake ..
project-template/build $ make
```

To run each program from build, you would type

```
project-template/build $ ../bin/demo
```


## Demo of basic features

TODO: Document the main demos your viewer 

## Unique features 

TODO: Show the unique features you made

## References:
- Smooth camera LERPING for movement: https://superhedral.com/2021/10/30/lerping-cameras-in-unity/

- Spawning slenderman behind the player using their forward axis and an angle of rotation: https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula

- Procedurally generating tree billboards using poisson's disk algorithm for 'packing': http://devmag.org.za/2009/05/03/poisson-disk-sampling/

- Glitch shader toy used on losing screen and looking directly at Slenderman in game: https://www.shadertoy.com/view/XtK3W3

## Resources:
- Slenderman model: https://free3d.com/3d-model/slenderman-from-slender-the-arrival-79467.html

- Fir tree texture: https://www.pngwing.com/en/free-png-ykplr

- Pine tree texture: https://www.pngwing.com/en/free-png-babso/download

- Flashlight mesh and texture: https://sketchfab.com/3d-models/flashlight-texture-3116044b0cb74ab2a392cbefc53cb723#download

- Grass texture: https://opengameart.org/content/64-billboard-grass-texture-and-mesh




