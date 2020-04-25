#include "testvr.h"



void   WriteDebug         (string message);
void   Draw               (XPLMWindowID in_window_id, void * in_refcon);
void   DrawDirect         ();
int	   Handle_mouse       (XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);
void   OpenGLInit         ();
void   CheckCompileErrors (unsigned int shader, string type); //this function wont be used for release, only for debugging
void   MakeRectangle      (unsigned long long number, int x, int y, int width, int height, const float color[3]);
vertex MakeVertex         (int x,int y, const float color[3]);
void   MakeVBO            ();
float  VertexXToNDCFloat  (int in_x);
float  VertexYToNDCFloat  (int in_y);


int					Dummy_mouse_handler(XPLMWindowID, int, int, int, void*) { return 0; }
XPLMCursorStatus	Dummy_cursor_status_handler(XPLMWindowID, int , int , void * ) { return xplm_CursorDefault; }
int					Dummy_wheel_handler(XPLMWindowID, int, int, int, int, void * ) { return 0; }
void				Dummy_key_handler(XPLMWindowID, char, XPLMKeyFlags, char, void *,int ) { }

const float margin        = 12.0f;
const float upperMargin   = 40.0f;
const float margin2D      = 0.0f;
const float upperMargin2D = 0.0f;
static XPLMWindowID	g_window;

static bool directRendering (false);
static bool firstPass(true);
static XPLMDataRef g_in_vrDref;
static int windowWidth(0),windowHeight(0);//will be computed later and updated after each resize
static float totalWidth(0),totalHeight(0);//idem
static string mouseButton("Up"),renderType("modern OGL");
static std::vector<rectangleData> rectangles;//a std::map here would be even better !
static std::vector<float> vertices;//for applications which build on this, this can be enhanced with structs
static std::vector<vertex> ivertices;//holds all attributes for a vertex aimed at direct rendering (useless if only modern rendering)
static float  bck[3]={0.15f,0.15f,0.20f};
static float  green[3]={0.1f,0.90f,0.2f};
static float  cyan[]={0.15f,0.9f,0.9f};
static unsigned int shaderProgram;
static unsigned int VAO;
static unsigned int VBO;
static unsigned int EBO;


void WriteDebug(std::string message){

    string in_String="VR Test : " +message+"\n";
    unsigned long long sz=in_String.size();sz++;//all this to only cast a std::string to a c-string without compiler warning
    char * fromString=new char [sz];            //otherwise just do XPLMDebugString((char*)in_String.c_str());
    if (in_String!="")
    {
        #if IBM
        strncpy_s(fromString,sz,in_String.c_str(),sz);
        #else
        strcpy(fromString,in_String.c_str());
        #endif
    }
    *fromString=*fromString+'\0';

    XPLMDebugString(fromString);

}

PLUGIN_API int XPluginStart(
                        char *		outName,
                        char *		outSig,
                        char *		outDesc)
{
    strcpy(outName, "VRSamplePlugin");
    strcpy(outSig, "xpsdk.examples.vrsampleplugin");
    strcpy(outDesc, "A test plug-in for showcasing using modern openGL");

    g_in_vrDref    = XPLMFindDataRef("sim/graphics/VR/enabled");
    return 1;

}

PLUGIN_API void	XPluginStop(void)
{

    XPLMDestroyWindow(g_window);
    g_window = nullptr;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int msg, void *)
{

    if(!g_window && msg == XPLM_MSG_SCENERY_LOADED)
    {
        bool g_in_vr(true);
        g_in_vr=XPLMGetDatai(g_in_vrDref);
        int global_desktop_bounds[4]; // left, bottom, right, top
        XPLMGetScreenBoundsGlobal(&global_desktop_bounds[0], &global_desktop_bounds[3], &global_desktop_bounds[2], &global_desktop_bounds[1]);
        int left=global_desktop_bounds[0] + 50;
        int bottom=global_desktop_bounds[1] + 150;
        int right=global_desktop_bounds[0] + 350;
        int top=global_desktop_bounds[1] + 450;
        windowWidth=right-left;
        windowHeight=top-bottom;

        XPLMCreateWindow_t params;
        params.structSize = sizeof(params);
        params.left = left;
        params.bottom = bottom;
        params.right = right;
        params.top = top;
        params.visible = 1;
        params.drawWindowFunc = Draw;
        params.handleMouseClickFunc = Handle_mouse;
        params.handleRightClickFunc = Dummy_mouse_handler;
        params.handleMouseWheelFunc = Dummy_wheel_handler;
        params.handleKeyFunc = Dummy_key_handler;
        params.handleCursorFunc = Dummy_cursor_status_handler;
        params.refcon = nullptr;
        params.layer = xplm_WindowLayerFloatingWindows;
        params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

        g_window = XPLMCreateWindowEx(&params);

        XPLMSetWindowPositioningMode(g_window, g_in_vr ? xplm_WindowVR : xplm_WindowPositionFree, -1);

        XPLMSetWindowResizingLimits(g_window, windowWidth-50, windowHeight-100, windowWidth+200, windowHeight+200);
        XPLMSetWindowTitle(g_window, "VR Window in modern openGL");
        mouseButton="mouse up";
        OpenGLInit();
    }
}

void OpenGLInit(){
   glewInit();

 //shaders
    const char *vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 vCoordA;\n"
        "layout (location = 1) in vec3 inColor;\n"
        "out vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(vCoordA, 0.0f,1.0f);\n"
        "   vColor = inColor;\n"
        "}\0";

    const char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in  vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor =vec4(vColor,1.0f);\n"
        "}\n\0";

    // shader compilation & shader program creation, error checks are out commented

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
        //if you modify the shaders you can uncomment the following to check for compiling errors
        //CheckCompileErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
        //CheckCompileErrors(shaderProgram, "FRAGMENT");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
        //CheckCompileErrors(shaderProgram, "PROGRAM");

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

  // Build VAO (with VBO and EBO)

    unsigned int indices[] = {
        0,1,3,
        1,2,3,
        4,5,7,
        5,6,7

     };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    MakeVBO(); //Not really, here for the first pass, only build vertices

    firstPass=false;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(),GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof (float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof (float),reinterpret_cast<void*>(2*sizeof (float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MakeVBO(){

    if (XPLMWindowIsInVR(g_window)){
       totalWidth=2*margin+windowWidth;
       totalHeight=margin+upperMargin+windowHeight;
    }
    else {
        totalWidth=2*margin2D+windowWidth;
        totalHeight=margin2D+upperMargin2D+windowHeight;
        WriteDebug("2D margins");
    }
    ivertices.clear();
    rectangles.clear();
    MakeRectangle(0,0,0,windowWidth,windowHeight,bck);//the "background rectangle"
    MakeRectangle(1,30,windowHeight/2,90,20,green);//the "button" rectangle;

    for (auto vt:ivertices)
    {
        vertices.push_back(VertexXToNDCFloat(vt.vx));
        vertices.push_back(VertexYToNDCFloat(vt.vy));
        vertices.push_back(vt.red  );
        vertices.push_back(vt.green);
        vertices.push_back(vt.blue );
    }

    if (firstPass) return;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER,0, vertices.size()*sizeof (float), vertices.data());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof (float), static_cast<void*>(nullptr));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof (float), reinterpret_cast<void*>(2*sizeof (float)));


}

void MakeRectangle(unsigned long long number, int x, int y, int width, int height, const float color[3]){

    rectangleData rect;
    rect.number=number;
    rect.left=x;
    rect.bottom=y;
    rect.width=width;
    rect.height=height;
    rect.red=color[0];
    rect.green=color[1];
    rect.blue=color[2];
    rectangles.push_back(rect);

    vertex rightTop   =MakeVertex(x+width,y+height,color);
    vertex rightBottom=MakeVertex(x+width,y,color);
    vertex leftBottom =MakeVertex(x,y,color);
    vertex leftTop    =MakeVertex(x,y+height,color);

    ivertices.push_back(rightTop); //0 and 4
    ivertices.push_back(rightBottom);//1 and 5
    ivertices.push_back(leftBottom);//2 and 6
    ivertices.push_back(leftTop);//3 and 7
}

vertex MakeVertex (int x, int y, const float color[]){
    vertex point;
    point.vx=x;
    point.vy=y;
    point.red=  color[0];
    point.green=color[1];
    point.blue= color[2];
    return point;
}

float VertexXToNDCFloat(int in_x){
    float corr=(XPLMWindowIsInVR(g_window)?margin:margin2D);
    float xf=-1+2*(in_x+corr)/totalWidth;
    return xf;
}

float VertexYToNDCFloat(int in_y){
    float corr=(XPLMWindowIsInVR(g_window)?margin:margin2D);
    float yf=-1+2*(in_y+corr)/totalHeight;
    return yf;
}

void CheckCompileErrors(unsigned int shader, string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            string ilog=infoLog;
            WriteDebug("SHADER COMPILATION ERROR of type: "+ilog+" "+type);        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            string ilog=infoLog;
            WriteDebug("PROGRAM LINKING ERROR of type: "+ilog+" "+ type);
        }
    }
}

void Draw(XPLMWindowID, void *){
    if (directRendering){
        DrawDirect();
        return;
    }
    int wW(0),wH(0),screenL(0),screenR(0),screenT(0),screenB(0);
     XPLMGetWindowGeometry(g_window,&screenL,&screenT,&screenR,&screenB);

        if (XPLMWindowIsInVR(g_window)){
            XPLMGetWindowGeometryVR(g_window,&wW,&wH);
            //Detect window's resizing in VR
            if (wW!=windowWidth||wH!=windowHeight){
                windowWidth=wW;
                windowHeight=wH;
                MakeVBO();
            }
        }
        else {
           wW=screenR-screenL;
           wH=screenT-screenB;
           //Detect window's resizing in 2D
           if (wW!=windowWidth||wH!=windowHeight){
               windowWidth=wW;
               windowHeight=wH;
           MakeVBO();
           }
           glPushAttrib(GL_VIEWPORT_BIT);
           glViewport(screenL,screenB,wW,wH);
        }
        XPLMSetGraphicsState(
            0,   // No fog, equivalent to glDisable(GL_FOG);
            0,   // No texture
            0,   // No lighting, equivalent to glDisable(GL_LIGHT0);
            0,   // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
            1,   // Use alpha blending, e.g. glEnable(GL_BLEND);
            1,   // No depth read, e.g. glDisable(GL_DEPTH_TEST);
            0);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);
        if (!XPLMWindowIsInVR(g_window)) glPopAttrib();

        XPLMDrawString(bck,rectangles[1].left+screenL,rectangles[1].bottom+screenB+4,(char*)("modern/direct"),nullptr,xplmFont_Proportional);
        XPLMDrawString(cyan,rectangles[1].left+screenL,rectangles[1].bottom+screenB-70,(char*)(mouseButton.c_str()),nullptr,xplmFont_Proportional);
        XPLMDrawString(cyan,rectangles[1].left+screenL,rectangles[1].bottom+screenB+50,(char*)(renderType.c_str()),nullptr,xplmFont_Proportional);
}

void DrawDirect(){

    int wW(0),wH(0),screenL(0),screenR(0),screenT(0),screenB(0);//wW for window width, wH for window Height
     XPLMGetWindowGeometry(g_window,&screenL,&screenT,&screenR,&screenB);

        if (XPLMWindowIsInVR(g_window)){
            XPLMGetWindowGeometryVR(g_window,&wW,&wH);
            //Detect window's resizing in VR
            if (wW!=windowWidth||wH!=windowHeight){
                windowWidth=wW;
                windowHeight=wH;
                MakeVBO();
            }
        }
        else {
           wW=screenR-screenL;
           wH=screenT-screenB;
           //Detect window's resizing in 2D
           if (wW!=windowWidth||wH!=windowHeight){
               windowWidth=wW;
               windowHeight=wH;
           MakeVBO();
           }
        }

        XPLMSetGraphicsState(
            0,   // No fog, equivalent to glDisable(GL_FOG);
            0,   // No texture
            0,   // No lighting, equivalent to glDisable(GL_LIGHT0);
            0,   // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
            1,   // Use alpha blending, e.g. glEnable(GL_BLEND);
            1,   // No depth read, e.g. glDisable(GL_DEPTH_TEST);
            0);

        for (auto rect:rectangles){
            glColor3f(rect.red,rect.green,rect.blue);
            glBegin(GL_QUADS);
            for (unsigned long long vtx(rect.number*4),count(0) ; count<4 ; vtx++,count++){
                glVertex2i(ivertices[vtx].vx+screenL,ivertices[vtx].vy+screenB);
            }
            glEnd();
        }

        XPLMDrawString(bck,rectangles[1].left+screenL,rectangles[1].bottom+screenB+4,(char*)("modern/direct"),nullptr,xplmFont_Proportional);
        XPLMDrawString(cyan,rectangles[1].left+screenL,rectangles[1].bottom+screenB-70,(char*)(mouseButton.c_str()),nullptr,xplmFont_Proportional);
        XPLMDrawString(cyan,rectangles[1].left+screenL,rectangles[1].bottom+screenB+50,(char*)(renderType.c_str()),nullptr,xplmFont_Proportional);

}

int	Handle_mouse(XPLMWindowID in_window_id, int in_x, int in_y, XPLMMouseStatus mouse_status, void * )
{
    if(mouse_status == xplm_MouseDown)

    {

        if(!XPLMIsWindowInFront(in_window_id))
        {
            XPLMBringWindowToFront(in_window_id);
        }
        else
        {
           int wtop(0),wbot(0),wrght(0),wlft(0),x(in_x),y(in_y);
           XPLMGetWindowGeometry(g_window,&wlft,&wtop,&wrght,&wbot);
           if (!XPLMWindowIsInVR(g_window)){//in 2D click coordinates are relative to viewport
               x=in_x-wlft-margin2D;
               y=in_y-wbot-margin2D;
           }
           else{
               x=in_x-wlft;
               y=in_y-wbot;
           }
           bool buttonIsClicked=(
                       (x>=ivertices[6].vx)&&
                       (x<=ivertices[4].vx)&&
                       (y>=ivertices[6].vy)&&
                       (y<=ivertices[4].vy));
           if (buttonIsClicked){
            directRendering=!directRendering;
            renderType=(directRendering?"Direct Rendering":"modern openGL");
           }

            mouseButton="Click at x,y : "+std::to_string(x)+" , "+std::to_string(y);

        }
    }
    if (mouse_status == xplm_MouseDrag){}

    if (mouse_status == xplm_MouseUp){ mouseButton="Button Up";}

    return 1;
}
