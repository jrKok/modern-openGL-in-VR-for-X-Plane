#include "testvr.h"

static XPLMWindowID	g_window;

void                WriteDebug(string message);
void				draw(XPLMWindowID in_window_id, void * in_refcon);
int					handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);

int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }
float               myFlightLoop(float lastCall, float lastFL, int counter, void* refcon);
void                OpenGLInit();
void                checkCompileErrors(unsigned int shader, string type);
void                MakeVBO();
void                ComputeVertices();

static XPLMDataRef g_vr_dref;
static bool g_in_vr = false;
static int windowWidth(0),windowHeight(0);
static string mouseButton("");
static float vertices[8];
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
    XPLMDebugString("Begin testVR Plugin");

    g_vr_dref = XPLMFindDataRef("sim/graphics/VR/enabled");

    mouseButton="Up";
    return g_vr_dref != nullptr;

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
        params.drawWindowFunc = draw;
        params.handleMouseClickFunc = handle_mouse;
        params.handleRightClickFunc = dummy_mouse_handler;
        params.handleMouseWheelFunc = dummy_wheel_handler;
        params.handleKeyFunc = dummy_key_handler;
        params.handleCursorFunc = dummy_cursor_status_handler;
        params.refcon = nullptr;
        params.layer = xplm_WindowLayerFloatingWindows;
        params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

        g_window = XPLMCreateWindowEx(&params);

        const int vr_is_enabled = XPLMGetDatai(g_vr_dref);
        XPLMSetWindowPositioningMode(g_window, vr_is_enabled ? xplm_WindowVR : xplm_WindowPositionFree, -1);
        g_in_vr = vr_is_enabled;

        XPLMSetWindowResizingLimits(g_window, windowWidth-50, windowHeight-100, windowWidth+200, windowHeight+200);
        XPLMSetWindowTitle(g_window, "VR Window in modern openGL");
        mouseButton="mouse up";
        OpenGLInit();
    }
}

void OpenGLInit(){
   glewInit();

 //shader sources
    const char *vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);\n"
        "}\0";

    const char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor =vec4(1.0f,0.6f,0.0f,1.0f);\n"
        "}\n\0";

    // vertex shader compile
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

    //fragment shader compile
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
        checkCompileErrors(shaderProgram, "FRAGMENT");

   // merge shaders in Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glLinkProgram(shaderProgram);

    WriteDebug("OpenGL Init : vertex and fragment shaders compiled");//Let us know that 2 more shaders are now on the GPU

 // Make space
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int indices[] = {
                   0, 1, 3,  // first Triangle
                   1, 2, 3   // second Triangle
     };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    ComputeVertices();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    //hopefully this it not needed but don't want anybody to mess with my VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void MakeVBO(){

    ComputeVertices();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER,0, sizeof(vertices), &vertices);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(nullptr));


}

void ComputeVertices(){
    if (XPLMWindowIsInVR(g_window)){
            int margin(12);
            int upperMargin(40);
            float wRatio=1.0f-float(2*margin)/float(2*margin+windowWidth);
            float huRatio=1.0f-float(2*upperMargin)/float(windowHeight+margin+upperMargin);
            float hlRatio=-1.0f+float(2*margin)/float(windowHeight+margin+upperMargin);
            WriteDebug("next hlRatio huRatio wRatio is "+std::to_string(hlRatio)+" "+std::to_string(huRatio)+" "+std::to_string(wRatio));

            vertices[0]=wRatio;vertices[1]=huRatio;  // right top
            vertices[2]=wRatio;vertices[3]=hlRatio; // right bottom
            vertices[4]=-wRatio;vertices[5]=hlRatio;// left bottom
            vertices[6]=-wRatio;vertices[7]=huRatio; // left top
       }
            else{
            vertices[0]=1;vertices[1]=1;  // right top
            vertices[2]=1;vertices[3]=-1; // right bottom
            vertices[4]=-1;vertices[5]=-1;// left bottom
            vertices[6]=-1;vertices[7]=1; // left top
}

}

void checkCompileErrors(unsigned int shader, string type)
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
            WriteDebug("ERROR::SHADER_COMPILATION_ERROR of type: "+ilog+" "+type+" "+std::to_string(success));        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            string ilog=infoLog;
            WriteDebug("ERROR::PROGRAM_LINKING_ERROR of type: "+ilog+" "+ type+" "+std::to_string(success));
        }
    }
}

void draw(XPLMWindowID, void *){

    int wW(0),wH(0),screenL(0),screenR(0),screenT(0),screenB(0);
     XPLMGetWindowGeometry(g_window,&screenL,&screenT,&screenR,&screenB);

        if (XPLMWindowIsInVR(g_window)){
            XPLMGetWindowGeometryVR(g_window,&wW,&wH);
            if (wW!=windowWidth||wH!=windowHeight){
                windowWidth=wW;
                windowHeight=wH;
                MakeVBO();
            }
        }
        else {       
           wW=screenR-screenL;
           wH=screenT-screenB;
           glViewport(screenL,screenB,wW,wH);
        }


        glUseProgram(shaderProgram);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        XPLMSetGraphicsState(
            0,   // No fog, equivalent to glDisable(GL_FOG);
            0,   // One texture, equivalent to glEnable(GL_TEXTURE_2D);
            0,   // No lighting, equivalent to glDisable(GL_LIGHT0);
            0,   // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
            1,   // Use alpha blending, e.g. glEnable(GL_BLEND);
            1,   // No depth read, e.g. glDisable(GL_DEPTH_TEST);
            0);
        int wth,hth;
        XPLMGetScreenSize(&wth,&hth);
        if (!XPLMWindowIsInVR(g_window)) glViewport(0,0,wth,hth);
        float color[]={0.15f,0.1f,0.9f};
        XPLMDrawString(color,screenL+100,screenT-50,(char*)(mouseButton.c_str()),nullptr,xplmFont_Proportional);
}

int	handle_mouse(XPLMWindowID in_window_id, int, int, XPLMMouseStatus mouse_status, void * )
{
    if(mouse_status == xplm_MouseDown)

    {

        if(!XPLMIsWindowInFront(in_window_id))
        {
            XPLMBringWindowToFront(in_window_id);
        }
        else
        {
            int wW(0),wH(0),screenL(0),screenR(0),screenT(0),screenB(0);
            XPLMGetWindowGeometryVR(g_window,&wW,&wH);
            XPLMGetWindowGeometry(g_window,&screenL,&screenT,&screenR,&screenB);

            mouseButton="Click";

        }
    }
    if (mouse_status == xplm_MouseDrag){}

    if (mouse_status == xplm_MouseUp){ mouseButton="Up";}

    return 1;
}
