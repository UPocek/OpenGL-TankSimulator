# Tank Simulator Game from Scratch - OpenGL Graphics

![OpenGL Uros Pocek](https://github.com/UPocek/OpenGL-TankSimulator/blob/master/docs/tank1.PNG)

## Idea
The goal for this project was to create a small, but functional game from scratch in c/c++ programming language using the OpenGL graphics library. 

OpenGL is "The Industry Standard for High-Performance Graphics", as they say on their website, but in all seriousness, it is a pretty cool API that enables us to code our own rendering logic and game engine from scratch so 👍 for that.

I chose the M-84 tank simulator for two reasons. Firstly this topic enabled me to try out all the different things OpenGL can offer, including textures, shaders, uniforms, and user controls, second is that I wanted to have this game when I was a kid, you can't blame me for that. What is more rewarding than being able to use your new learning and skills to make your past self proud? So with everything in place, I jumped into the code.

![Uros Pocek](https://github.com/UPocek/OpenGL-TankSimulator/blob/master/docs/tank2.PNG)

## Game
If you want to play my game just clone this repo and double-click on the .sln file to open it in your editor (I have used VisualStudio 2022 for development purposes) then just run the source code and a full-screen tank simulation will pop up ready to kick some serious ass. 

Commands are pretty straightforward, use arrow keys `↑ ↓ → ←` for moving the M-84 cockpit and aiming for targets when you are on target press the `Space` key for firing is you are accurate target will disappear from the screen and the indicator light from the top left corner of the screen will turn off for 7.5 seconds indicationing CoolDown period, after which you will be able to fire again. If the tank's cupola is moving too slowly for your liking try pomping in more power to it by pressing the `p` and `o` keys to increase and decrease the voltage in the motors respectively. If have just 6 ammos use them wisely. Clicking the `c` key will transfer your view inside the cockpit where you will be able to see tank statistics and plan your next move, clicking the `v` key will bring back sight and aiming view. Practice your shooting and write your best time to hit all 3 targets.

![Tank Simulator Poster](https://github.com/UPocek/OpenGL-TankSimulator/blob/master/docs/tank4.PNG)

## 3D
So one extra thing that I added to this project is a complete 3D world where thank can shoot targets, rotate, and much more. Rendering graphics in 3D is much more complex than rendering in 2D and only when you dive into 3D graphics do you start to appreciate how all this mathematics comes together. Everything is about matrices and vectors. How something is positioned on the scene, matrices, how light bounces off objects, matrices again, but if you like that kind of stuff this is a phenomenal part of software development diapason you can be in. I used different culling algorithms, depth testing, Phong lighting with different kinds of lights, generation 3D models in Blender and loading them in my OpenGL project and so much more. Same as the 2D version of this little game of mine, if you are interested in trying it for yourself just go to the 3D branch start the .sln, and enjoy.  

![OpenGL 3D Tank Simulator](https://github.com/UPocek/OpenGL-TankSimulator/blob/master/docs/tenk_3d.PNG)

## Conclusion
This project is very different from things that I usually do, but I tried to make it as much fun as possible. So I hope you enjoyed reading this project summary as much as I have enjoyed programming this big boy. Cheers.

![Uros Projects](https://github.com/UPocek/OpenGL-TankSimulator/blob/master/docs/tank3.PNG)

