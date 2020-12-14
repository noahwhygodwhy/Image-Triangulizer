# Image-Triangulizer

This is a program to triangularize images. Purely a stylalistic thing, but a fun technical challenge that required the creation of a few tools I needed for a separate project.

![an example](https://i.imgur.com/DIeMtBq.jpg)

I'm super proud of how the fuzzyness of the belly and wiskers is maintained.

This clarity in triangulation is achieved by first using the sobel operator for edge detection. From there, points are distributed randomly favoring the pixels that are deemed edges.These points are then configured into a delaunay triangulation using the Bowyer-Watson algorithm. The color of each triangle is picked from the pixel at the center of the triangle from the original image.The triangles are then rendered in opengl to an internal framebuffer and saved as a file.

I used visual studio 19 to compile, with nuget packages glfw and glm. GLFW for the opengl context, and glm for opengl math stuff.

This program is configured through the top of Image Triangulizer.cpp.

NUMBER_OF_POINTS is, well, the number of points that are added. More points, more detail, but less A E S T H E T I C.

SOBEL_MINIMUM is the minimum value after the sobel operator that a pixel must have to be considered as a point.

RATIO_BRIGHT_TO_DARK is the ratio between how many of the points must be above the SOBEL_MINIMUM, and how many are not. This prevents weird, overly large patches from forming.

The input and output files are just below the constants. The image handling library used is finicky, and as of now the program is only configured to handle PNGs.

This was a blast to make, and I hope you ejoy triangulizing images with it.
