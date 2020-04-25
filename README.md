# modern-openGL-in-VR-for-X-Plane
Derived from tyler young's VR Window sample, showcase the use of modern openGL for rendering to VR custom windows from X-Plane's plugins

To compile successfully, Glew has to be added. I've done it by adding glew.h and the .lib files to my plugins code folder.If another library for required openGL functions is used, change the #includes and LIBS accordingly, same if you have a glew elsewhere.

This code has been built with the Qt IDE and the make file is processed through QMake.The compiler was MSVC2017.

When compiled and run it shows an amber rectangle with a text showing the state of the left mouse button in a custom decorated window. This is to show how the codeis able to track the dimensions of the rectangle with regard to the window's zone which is able to receive clicks. In VR the viewport must not be set (while in 2D it must be defined with glViewport) but the viewport covers the whole of the VR window when it is decorated as xplm_WindowDecorationRoundRectangle. In all other cases the vertices can extend normally between -1 and 1 to receive clicks and the VBO doesn't need to be modified. 

In 2D the Viewport can be set with the left/bottom coordinates returned by X Plane and the width and height of the custom window computed with (right-left) and (top-bottom) respectively.

This code can be easily adapted to render a texture or drawing a set of primitives on the viewport. It doesn't alter XPLMDrawString in either case (2D or VR).  

*** Disclaimers

Developed in the Qt IDE, under the LGPL licence(https://www.gnu.org/licenses/lgpl-3.0.en.html), no Qt libraries are used.
