## TERRAIN GENERATOR
In game industry, AAA game companies prepares their own tools for generating terrains. For example game called “No Man’s Sky” generates procedural planets. Procedural generation meaning by the way is a method of creating data algorithmically as opposed to manually, typically through a combination of human-generated assets and algorithms coupled with computer-generated randomness and processing power. In computer graphics, it is commonly used to create textures and 3D models. Example video about procedural generation from “No Man’s Sky”: https://www.youtube.com/watch?v=PM7ArMxUFR0&feature=emb_title

In this project I made my own procedural terrain generator in Unity3D and explain how it works below. Let’s get start!

### Create a mesh from scratch
Apply perlin noise algorithm to generate procedural mountains/hills.
Assign material controlled by a specific shader to the mesh. (In this part I coded my own shader)
#### Result:
<img src="pics/doc_t_0.png" width = "920" height = "843">

As you can see here we get procedural mountain like shape with specific material. In our shader, it chooses different texture for each vertex depending on steepness of terrain with vertex coloring method. But result is not something we want. We want to have much realistic looking terrain. In this case we use different method.

#### Octaves:
Simple perlin noise algorithm looks like this in one dimension (With different height and scale values)
<img src="pics/octave01.png" width = "768" height = "400">

As you can see, we can change height and scale values of signal. If we add them all, we get something like this:
<img src="pics/terrain_.png" width = "256" height = "200">
If we apply this in our terrain:
<img src="pics/terrain_.png" width = "1612" height = "680">
Yes..! It looks much better than before. So perlin noise is a simple and powerful algorithm but not enough if we use it alone. We need to make some changes like adding, scaling… on it.  

### Adding Sea Threshold Value:
<img src="pics/doc_t_1.png" width = "907" height = "512">

If we want to add sea to our terrain, we know that we should make our seas bigger to get realistic nature. When we give threshold value, heights that are lower than threshold will be calculated different than values above threshold.

Algorithm:
if (height < treshold)
       return treshold - ((treshold - height) / seaDeep);
else
       return height;

Then we get a curve like this:




