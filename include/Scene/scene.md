## Scene Object 




##### structure 


- each frame has n layers ....


- or does each layer have n images???? (see the disticntion)

-  hmmm so it seems that each layer should just hold an std::vector called images 

- each image has a start point and then duration 


- one thing we CANNOT allow is two images overlapping in the vector (which isnt possible if we make the position based on literal order in the vector)



- i think we should have a dirty list (much like raster tiles ) for any swaps/ moving frames 

- then we 


## Timeline / Animation Data Structure

* **Image storage:** Keep actual `Image` objects in a vector as stable storage.
* **Layer:** Each layer owns an ordered vector of animation frames.
* **Frame:** Store only:

  * `imageIndex` — index of the `Image` in image storage.
  * `duration` — how long the frame plays.
* **Timeline position:** Do not store start time. A frame's start time is determined by the cumulative duration of all previous frames.
* **Timeline UI:** Display as a grid:

  * Each row = one layer.
  * Each frame = a horizontal block.
  * Block position = cumulative start time.
  * Block width = duration.
  * Frame's vector position = its playback order.
* **Playback:** Track the current frame and time within that frame. Image lookup is O(1) through `imageIndex`.
* **Reordering:** Reordering frames only changes the small frame structs; the underlying `Image` objects do not need to move.
* **Potential future change:** If frames need to overlap on the same layer, add an explicit `startTime` instead of relying on cumulative durations.



### what do we need?


- we need an object that can hold Raster Images + 3d backgrounds. for a scene 

- this means handling layers, meshes, backgrounds, positions of objects, ect.. .




### task list 


- create object to hold n "frames" 

- allow each frame to hold layers of images rather than jsut one image 

- alllow each frame to also hold positions of 3d space as welll 

- onion skin

- layer toggling 

- we need to update our canvas renderer to handle mutliple layers 

- we need to create a widget to handle layers as well as frames 

