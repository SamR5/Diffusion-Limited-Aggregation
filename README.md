# Diffusion-Limited-Aggregation
Different kind of Brownian trees

This is a repository with multiple kind of Brownian trees


### DiffusionLimitedAggregation.cpp
A seed is in the center and particles aggregate on the seed.  
14/10/2019:
 - The center is now (0, 0) and not (width/2, height/2).  
 - Keeps track of the farthest particle from the center.  
 - The particles are now generated at a distance of farthest+10 from center.  
 - The collisions are now checked only if distance from center <= farthest.  

### Snowflake_1.cpp
A seed is in the center, particles start on the right and only move in a cone between -PI/6 and PI/6.  
The particles are rotated and flipped around the center to form a snowflake.  
A line between the touching particles is drawn.  

### snowflake_2.cpp
Same as version 1 but without the lines.  

### DLA_bottom.cpp
The particles starts from the top and are fixed to the bottom of the screen.  

### DLA_circle.cpp
The particles start from the center and are fixed to a circle.  
