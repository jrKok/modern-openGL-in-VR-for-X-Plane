# modern-openGL-in-VR-for-X-Plane
Derived from tyler young's VR Window sample, showcase the use of modern openGL for rendering to VR custom windows from X-Plane's plugins

To compile successfully, Glew has to be added. I've done it by adding glew.h and the .lib files to my plugins code folder.If another library for required openGL functions is used, change the #includes and LIBS accordingly, same if you have a glew elsewhere.

This code has been built with the Qt IDE and the make file is processed through QMake.The compiler was MSVC2017.

When compiled and run it shows an green rectangle with dark gray background with a text showing the state of the left mouse button in a custom decorated window and the rendering method (direct or modern openGL). 

In VR the viewport must not be set (while in 2D it can be defined with glViewport) but the viewport covers the whole of the VR window when it is decorated as xplm_WindowDecorationRoundRectangle. In all other cases the vertices can extend normally between -1 and 1 to receive clicks and the VBO doesn't need to be modified. 

In 2D the Viewport can be set with the left/bottom coordinates returned by X Plane and the width and height of the custom window computed with (right-left) and (top-bottom) respectively.

This code can be easily adapted to render a texture or drawing a set of primitives on the viewport. It doesn't alter XPLMDrawString in either case (2D or VR).  

*** Disclaimers and Credits :

1) This Code was developed in the Qt IDE, under the LGPL licence(https://www.gnu.org/licenses/lgpl-3.0.en.html), no Qt libraries are used.
2) openGL implementation is done with the help of The OpenGL Extension Wrangler Library (GLEW Library), authoring and licensing information : https://github.com/nigels-com/glew#authors

