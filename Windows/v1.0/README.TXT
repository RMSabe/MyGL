MyGL: Win32 OpenGL auxiliary resource
Version 1.0

This resource uses my custom WinLib version 4.0
GitHub Repository: https://github.com/RMSabe/WinLib/v4.0

Description:
I was working on an application for plotting graphics, more specifically math functions.
I realized there was a limitation on relying on OpenGL's buffer. I wanted to be able to redraw the entire graph plotting at any moment during runtime.
Even if the GL buffer is cleared, if the device and rendering context are redefined, I should still be able to replicate the drawing in my previous window.

I decided to create a dynamically allocated buffer, like a list or a C++ vector, to store, step by step, all changes made to the GL buffer. And I created this resource.

The way how MyGL works is, every supported GL function is written on a heap buffer as an entry. Entries are structs that define the type of function, the type of data, the size and parameters used.

This resource so far supports basic resources:
set GL color
set GL point size
set GL line width
draw point
draw line
draw triangle
draw quad
draw polygon

Those are the basic resources I needed for my work, and those are the resources I've implemented. It is still possible to implement more if needed.

Disclaimer:
I'm not a graphics developer or a graphics designer.
I'm not totally familiarized with the OpenGL resources. I only use OpenGL to draw math functions, smiley faces and stick men.
There are probably easier and more efficient ways to implement this resource, ways that I'm not aware of.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
