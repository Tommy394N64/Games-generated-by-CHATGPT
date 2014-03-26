/*
 *
 * Designed by Emile Belanger
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>

#include "SDL_scancode.h"

#include "TouchControlsContainer.h"
extern "C"
{

//This is a new function I put into SDL2, file SDL_androidgl.c
extern void SDL_SetSwapBufferCallBack(void (*pt2Func)(void));

#include "in_android.h"

#include "../GL/gl.h"
#include "../GL/nano_gl.h"

#ifndef LOGI
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,"DUKE", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "DUKE", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,"DUKE", __VA_ARGS__))
#endif

#define REND_SOFT  0
#define REND_GL    1

static int curRenderer;

int android_screen_width;
int android_screen_height;


#define KEY_QUICK_CMD    0x1005
#define KEY_SHOW_KBRD    0x1008
#define KEY_SHOW_INVEN    0x1009
#define KEY_QUICK_SAVE    0x100A
#define KEY_QUICK_LOAD    0x100B
#define KEY_SNIPER        0x100C

#define KEY_QUICK_KEY1    0x1011
#define KEY_QUICK_KEY2    0x1012
#define KEY_QUICK_KEY3    0x1013
#define KEY_QUICK_KEY4    0x1014


float gameControlsAlpha = 0.5;
bool showWeaponCycle = false;
bool turnMouseMode = true;
bool invertLook = false;
bool precisionShoot = false;
bool showSticks = true;
bool hideTouchControls = true;

bool shooting = false;
bool sniperMode = false;

static int controlsCreated = 0;
touchcontrols::TouchControlsContainer controlsContainer;

touchcontrols::TouchControls *tcMenuMain=0;
touchcontrols::TouchControls *tcGameMain=0;
touchcontrols::TouchControls *tcGameWeapons=0;
touchcontrols::TouchControls *tcAutomap=0;
touchcontrols::TouchControls *tcInventory=0;
touchcontrols::TouchControls *tcGameLook=0; //Just used for whole screen look mode


touchcontrols::TouchJoy *touchJoyLeft;
touchcontrols::TouchJoy *touchJoyRight;
touchcontrols::WheelSelect *weaponWheel;

extern JNIEnv* env_;

void openGLStart()
{

	if (curRenderer == REND_GL)
	{
		glPushMatrix();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrthof (0, android_screen_width, android_screen_height, 0, 0, 1);

		//glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		//LOGI("openGLStart");
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
		glEnable(GL_TEXTURE_2D);
		glEnable (GL_BLEND);

		glColor4f(1,1,1,1);

		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY );

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_CULL_FACE);

		glMatrixMode(GL_MODELVIEW);

		nanoPushState();
	}
	else //software mode
	{

		glDisable(GL_ALPHA_TEST);
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY );
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable (GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glMatrixMode(GL_MODELVIEW);

	}

}

void openGLEnd()
{

	if (curRenderer == REND_GL)
	{
		//glPopMatrix();
		nanoPopState();
		glPopMatrix();
	}
	else// Software mode
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glMatrixMode(GL_MODELVIEW);

	}
}

void gameSettingsButton(int state)
{

	//LOGTOUCH("gameSettingsButton %d",state);
	if (state == 1)
	{
		jclass helloWorldClass;
		jmethodID mainMethod;

		helloWorldClass = env_->FindClass("com/beloko/duke/QuakeTouchControlsSettings");
		mainMethod = env_->GetStaticMethodID(helloWorldClass, "showSettings", "()V");
		env_->CallStaticVoidMethod(helloWorldClass, mainMethod);
	}
}


void showCustomCommands()
{

	jclass helloWorldClass;
	jmethodID mainMethod;
	helloWorldClass = env_->FindClass("com/beloko/duke/QuakeCustomCommands");
	mainMethod = env_->GetStaticMethodID(helloWorldClass, "showCommands", "()V");
	env_->CallStaticVoidMethod(helloWorldClass, mainMethod);
}

void toggleKeyboard()
{
	LOGI("toggleKeyboard");
	jclass helloWorldClass;
	jmethodID mainMethod;
	helloWorldClass = env_->FindClass("com/beloko/duke/ShowKeyboard");
	mainMethod = env_->GetStaticMethodID(helloWorldClass, "toggleKeyboard", "()V");
	env_->CallStaticVoidMethod(helloWorldClass, mainMethod);
}

//Because there is no Frame(), we need to check back to java each frame to see if the app hase paused

static jclass NativeLibClass = 0;
static jmethodID checkPauseMethod = 0;
void swapBuffers()
{
	if (NativeLibClass == 0)
	{
		NativeLibClass = env_->FindClass("com/beloko/duke/engine/NativeLib");
		checkPauseMethod = env_->GetStaticMethodID(NativeLibClass, "swapBuffers", "()V");
	}
	env_->CallStaticVoidMethod(NativeLibClass, checkPauseMethod);
}


void gameButton(int state,int code)
{
	if (code == gamefunc_Fire)
	{
		shooting = state;
		PortableAction(state, code);
	}
	if (code == KEY_SNIPER)
	{
		sniperMode = state;
	}
	else if  (code == KEY_SHOW_KBRD)
	{
		if (state)
			toggleKeyboard();

		PortableKeyEvent(state, SDL_SCANCODE_GRAVE,0);
	}
	else if  (code == KEY_QUICK_CMD){
		if (state == 1)
			showCustomCommands();
	}
	else if  (code == KEY_SHOW_INVEN){
		if (state == 1)
		{
			if (!tcInventory->isEnabled())
			{
				tcInventory->setEnabled(true);
				tcInventory->fade(0,5);
				//disable weapon wheel so it does not show, enabled again after inventory cleared
				tcGameWeapons->setEnabled(false);
			}
			else
			{
				tcInventory->setEnabled(false);
				tcGameWeapons->setEnabled(true);
			}
		}
	}
	else if (code == KEY_QUICK_SAVE){
		LOGI("QUICK SAVE");
		PortableKeyEvent(state, SDL_SCANCODE_F6,0);
	}
	else if (code == KEY_QUICK_LOAD){
		LOGI("QUICK LOAD");
		PortableKeyEvent(state, SDL_SCANCODE_F9,0);
	}
	else
		PortableAction(state, code);
}

void automapButton(int state,int code)
{
	PortableAction(state,code);
}

void inventoryButton(int state,int code)
{
	PortableAction(state,code);
	if (state == 0)
	{
		tcGameWeapons->setEnabled(true);
		tcInventory->setEnabled(false);
	}

}

void menuButton(int state,int code)
{
	PortableKeyEvent(state, code,code);
}

static int weaponWheelVisible = false;
void weaponWheelSelected(int enabled)
{
	if (enabled)
	{
		for (int n=0;n<10;n++)
		{
			weaponWheel->setSegmentEnabled(n,(PortableGetWeapons() >> n) & 0x1);
		}

		tcGameWeapons->fade(0,5); //fade in
		weaponWheelVisible = true;
	}
	else
	{
		tcGameWeapons->fade(1,5);
		weaponWheelVisible = false;
	}
}
void weaponWheelChosen(int segment)
{
	LOGI("weaponWheel %d",segment);
	if (segment != -1)
	{
		PortableAction(1, gamefunc_Weapon_1 + segment);
		PortableAction(0, gamefunc_Weapon_1 + segment);
	}
	else //Nothing was selected
	{
		if (getLastWeapon() != -1)
		{
			PortableAction(1, gamefunc_Weapon_1 + getLastWeapon());
			PortableAction(0, gamefunc_Weapon_1 + getLastWeapon());
		}
	}
}


int left_double_action;
int right_double_action;

void left_double_tap(int state)
{
	//LOGTOUCH("L double %d",state);
	if (left_double_action)
		PortableAction(state,left_double_action);
}

void right_double_tap(int state)
{
	//LOGTOUCH("R double %d",state);
	if (right_double_action)
		PortableAction(state,right_double_action);
}


//To be set by android
float strafe_sens,forward_sens;
float pitch_sens,yaw_sens;

void left_stick(float joy_x, float joy_y,float mouse_x, float mouse_y)
{
	joy_x *=10;
	float strafe = joy_x*joy_x;
	//float strafe = joy_x;
	if (joy_x < 0)
		strafe *= -1;

	PortableMove(joy_y * 15 * forward_sens,-strafe * strafe_sens);
}

extern int crouchToggleState; //Set in anroid_in.c

void right_stick(float joy_x, float joy_y,float mouse_x, float mouse_y)
{
	//LOGI(" mouse x = %f",mouse_x);
	int invert = invertLook?-1:1;

	float scale = ((shooting && precisionShoot) || crouchToggleState)?0.3:1;

	PortableLookPitch(LOOK_MODE_MOUSE,-mouse_y  * pitch_sens * invert * scale);

	if (turnMouseMode)
		PortableLookYaw(LOOK_MODE_MOUSE,mouse_x*2*yaw_sens * scale);
	else
		PortableLookYaw(LOOK_MODE_JOYSTICK,joy_x*6*yaw_sens * scale);
}

void mouseMove(int action,float x, float y,float dx, float dy)
{
	LOGI(" mouse dx = %f",dx);

	if (weaponWheelVisible)
		return;
	/*
	if (abs(dx) < 0.001)
	{
		dx * 0.1;
	}
	 */
	int invert = invertLook?-1:1;
	float scale;

	if (sniperMode)
		scale = 0.2;
	else
		scale = ((shooting && precisionShoot) || crouchToggleState)?0.2:1;


	PortableLookPitch(LOOK_MODE_MOUSE,-dy  * pitch_sens * invert * scale);

	PortableLookYaw(LOOK_MODE_MOUSE,dx*2*yaw_sens * scale);
}

void setHideSticks(bool v)
{
	if (touchJoyLeft) touchJoyLeft->setHideGraphics(v);
	if (touchJoyRight) touchJoyRight->setHideGraphics(v);
}

void touchSettingsButton(int state)
{
	//We wanna pause the game when doign settings
	if (state && !isPaused())
	{
		PortableKeyEvent(1,SDL_SCANCODE_PAUSE,0);
		PortableKeyEvent(0,SDL_SCANCODE_PAUSE,0);
	}
	else if (!state && isPaused())
	{
		PortableKeyEvent(1,SDL_SCANCODE_PAUSE,0);
		PortableKeyEvent(0,SDL_SCANCODE_PAUSE,0);
	}
}

void initControls(int width, int height,const char * graphics_path,const char *settings_file)
{
	touchcontrols::GLScaleWidth = (float)width;
	touchcontrols::GLScaleHeight = (float)height;

	LOGI("initControls %d x %d,x path = %s, settings = %s",width,height,graphics_path,settings_file);

	if (!controlsCreated)
	{
		LOGI("creating controls");

		touchcontrols::setGraphicsBasePath(graphics_path);

		controlsContainer.openGL_start.connect( sigc::ptr_fun(&openGLStart));
		controlsContainer.openGL_end.connect( sigc::ptr_fun(&openGLEnd));
		controlsContainer.signal_settings.connect( sigc::ptr_fun(&touchSettingsButton));


		tcMenuMain = new touchcontrols::TouchControls("menu",true,false);
		tcGameMain = new touchcontrols::TouchControls("game",false,true,1,true);
		tcGameWeapons = new touchcontrols::TouchControls("weapons",false,true,1,false);
		tcAutomap  = new touchcontrols::TouchControls("automap",false,true,1,false);
		tcInventory = new touchcontrols::TouchControls("inventory",false,true,1,false);
		tcGameLook = new touchcontrols::TouchControls("mouse",true,false);
		controlsContainer.dukeHack = 1;

		tcGameMain->signal_settingsButton.connect(  sigc::ptr_fun(&gameSettingsButton) );

		//Menu
		tcMenuMain->addControl(new touchcontrols::Button("down_arrow",touchcontrols::RectF(20,13,23,16),"arrow_down",SDL_SCANCODE_DOWN,true)); //Repeating buttons
		tcMenuMain->addControl(new touchcontrols::Button("up_arrow",touchcontrols::RectF(20,10,23,13),"arrow_up",SDL_SCANCODE_UP,true));
		tcMenuMain->addControl(new touchcontrols::Button("left_arrow",touchcontrols::RectF(17,13,20,16),"arrow_left",SDL_SCANCODE_LEFT,true));
		tcMenuMain->addControl(new touchcontrols::Button("right_arrow",touchcontrols::RectF(23,13,26,16),"arrow_right",SDL_SCANCODE_RIGHT,true));
		tcMenuMain->addControl(new touchcontrols::Button("enter",touchcontrols::RectF(0,12,4,16),"enter",SDL_SCANCODE_RETURN));
		tcMenuMain->addControl(new touchcontrols::Button("esc",touchcontrols::RectF(0,9,4,12),"esc",SDL_SCANCODE_ESCAPE));


		tcMenuMain->signal_button.connect(  sigc::ptr_fun(&menuButton) );
		tcMenuMain->setAlpha(0.5);


		tcInventory->addControl(new touchcontrols::Button("jetpack", touchcontrols::RectF(4,3,7,6),"jetpack",gamefunc_Jetpack));
		tcInventory->addControl(new touchcontrols::Button("medkit",  touchcontrols::RectF(7,3,10,6),"medkit",gamefunc_MedKit));
		tcInventory->addControl(new touchcontrols::Button("nightv",  touchcontrols::RectF(4,6,7,9),"nightvision",gamefunc_NightVision));
		tcInventory->addControl(new touchcontrols::Button("holoduke",touchcontrols::RectF(7,6,10,9),"holoduke",gamefunc_Holo_Duke));
		tcInventory->addControl(new touchcontrols::Button("steroids",touchcontrols::RectF(4,9,7,12),"steroids",gamefunc_Steroids));
		tcMenuMain->setAlpha(1);


		tcInventory->signal_button.connect(  sigc::ptr_fun(&inventoryButton));

		/*
		//Automap
		tcAutomap->addControl(new touchcontrols::Button("down_arrow",touchcontrols::RectF(2,14,4,16),"arrow_down",PORT_ACT_MAP_DOWN));
		tcAutomap->addControl(new touchcontrols::Button("up_arrow",touchcontrols::RectF(2,10,4,12),"arrow_up",PORT_ACT_MAP_UP));
		tcAutomap->addControl(new touchcontrols::Button("left_arrow",touchcontrols::RectF(0,12,2,14),"arrow_left",PORT_ACT_MAP_LEFT));
		tcAutomap->addControl(new touchcontrols::Button("right_arrow",touchcontrols::RectF(4,12,6,14),"arrow_right",PORT_ACT_MAP_RIGHT));
		tcAutomap->addControl(new touchcontrols::Button("zoom_in",touchcontrols::RectF(4,10,6,12),"zoom_in",PORT_ACT_MAP_ZOOM_IN));
		tcAutomap->addControl(new touchcontrols::Button("zoom_out",touchcontrols::RectF(4,14,6,16),"zoom_out",PORT_ACT_MAP_ZOOM_OUT));

		tcAutomap->signal_button.connect(  sigc::ptr_fun(&automapButton) );
		tcAutomap->setAlpha(0.5);
		 */

		//Game
		tcGameMain->setAlpha(gameControlsAlpha);
		controlsContainer.editButtonAlpha = gameControlsAlpha;
		tcGameMain->addControl(new touchcontrols::Button("use",touchcontrols::RectF(23,4,26,7),"use",gamefunc_Open));
		tcGameMain->addControl(new touchcontrols::Button("attack",touchcontrols::RectF(20,7,23,10),"fire2",gamefunc_Fire));
		tcGameMain->addControl(new touchcontrols::Button("jump",touchcontrols::RectF(23,7,26,10),"jump",gamefunc_Jump));
		tcGameMain->addControl(new touchcontrols::Button("crouch",touchcontrols::RectF(24,14,26,16),"crouch",gamefunc_Crouch));
		tcGameMain->addControl(new touchcontrols::Button("kick",touchcontrols::RectF(20,4,23,7),"boot",gamefunc_Quick_Kick));

		tcGameMain->addControl(new touchcontrols::Button("quick_save",touchcontrols::RectF(24,0,26,2),"save",KEY_QUICK_SAVE));
		tcGameMain->addControl(new touchcontrols::Button("quick_load",touchcontrols::RectF(20,0,22,2),"load",KEY_QUICK_LOAD));
		tcGameMain->addControl(new touchcontrols::Button("map",touchcontrols::RectF(4,0,6,2),"map",gamefunc_Map));
		tcGameMain->addControl(new touchcontrols::Button("keyboard",touchcontrols::RectF(8,0,10,2),"keyboard",KEY_SHOW_KBRD));

		tcGameMain->addControl(new touchcontrols::Button("show_inventory",touchcontrols::RectF(0,0,2,3),"inv",KEY_SHOW_INVEN));

		tcGameMain->addControl(new touchcontrols::Button("next_weapon",touchcontrols::RectF(0,3,3,5),"next_weap",gamefunc_Next_Weapon));
		tcGameMain->addControl(new touchcontrols::Button("prev_weapon",touchcontrols::RectF(0,7,3,9),"prev_weap",gamefunc_Previous_Weapon));
		//tcGameMain->addControl(new touchcontrols::Button("sniper",touchcontrols::RectF(0,5,3,7),"sniper",KEY_SNIPER));

		//quick actions binds
		tcGameMain->addControl(new touchcontrols::Button("quick_key_1",touchcontrols::RectF(4,3,6,5),"quick_key_1",KEY_QUICK_KEY1,false,true));
		tcGameMain->addControl(new touchcontrols::Button("quick_key_2",touchcontrols::RectF(6,3,8,5),"quick_key_2",KEY_QUICK_KEY2,false,true));
		tcGameMain->addControl(new touchcontrols::Button("quick_key_3",touchcontrols::RectF(8,3,10,5),"quick_key_3",KEY_QUICK_KEY3,false,true));
		tcGameMain->addControl(new touchcontrols::Button("quick_key_4",touchcontrols::RectF(10,3,12,5),"quick_key_4",KEY_QUICK_KEY4,false,true));

		//Left stick
		touchJoyLeft = new touchcontrols::TouchJoy("stick",touchcontrols::RectF(0,7,8,16),"strafe_arrow");
		tcGameMain->addControl(touchJoyLeft);
		touchJoyLeft->signal_move.connect(sigc::ptr_fun(&left_stick) );
		touchJoyLeft->signal_double_tap.connect(sigc::ptr_fun(&left_double_tap) );

		//Right stick
		touchJoyRight = new touchcontrols::TouchJoy("touch",touchcontrols::RectF(17,7,26,16),"look_arrow");
		tcGameMain->addControl(touchJoyRight);
		touchJoyRight->signal_move.connect(sigc::ptr_fun(&right_stick) );
		touchJoyRight->signal_double_tap.connect(sigc::ptr_fun(&right_double_tap) );
		touchJoyRight->setEnabled(false);

		tcGameMain->signal_button.connect(  sigc::ptr_fun(&gameButton) );


		//Mouse look for whole screen
		touchcontrols::Mouse *mouse = new touchcontrols::Mouse("mouse",touchcontrols::RectF(3,0,26,16),"");
		mouse->signal_action.connect(sigc::ptr_fun(&mouseMove));
		mouse->signal_double_tap.connect(sigc::ptr_fun(&right_double_tap) );

		mouse->setHideGraphics(true);
		tcGameLook->addControl(mouse);



		//Weapons
		weaponWheel = new touchcontrols::WheelSelect("weapon_wheel",touchcontrols::RectF(7,2,19,14),"weapon_wheel",10);
		weaponWheel->signal_selected.connect(sigc::ptr_fun(&weaponWheelChosen) );
		weaponWheel->signal_enabled.connect(sigc::ptr_fun(&weaponWheelSelected));
		tcGameWeapons->addControl(weaponWheel);


		tcGameWeapons->setAlpha(0.9);

		controlsContainer.addControlGroup(tcMenuMain);

		controlsContainer.addControlGroup(tcInventory);//Need to be above tcGameMain incase buttons over stick
		controlsContainer.addControlGroup(tcGameMain);
		controlsContainer.addControlGroup(tcGameWeapons);
		//controlsContainer.addControlGroup(tcAutomap);
		controlsContainer.addControlGroup(tcGameLook);
		controlsCreated = 1;

		tcGameMain->setXMLFile((std::string)graphics_path +  "/game.xml");
		tcGameWeapons->setXMLFile((std::string)graphics_path +  "/weapons.xml");
		tcAutomap->setXMLFile((std::string)graphics_path +  "/automap.xml");
		tcInventory->setXMLFile((std::string)graphics_path +  "/inventory.xml");

	}
	else
		LOGI("NOT creating controls");

	//controlsContainer.initGL();
}

int loadedGLImages=0;

int inMenuLast = 1;
int inAutomapLast = 0;
void frameControls()
{
	//LOGI("frameControls");
	//We need to do this here now because duke loads a new gl context
	if (!loadedGLImages)
	{
		touchJoyRight->setEnabled(false);

		controlsContainer.initGL();
		loadedGLImages = 1;

	}

	//LOGI("frameControls");
	if (PortableIsSoftwareMode())
		curRenderer = REND_SOFT;
	else
		curRenderer = REND_GL;

	int inMenuNew = PortableInMenu();
	if (inMenuLast != inMenuNew)
	{
		inMenuLast = inMenuNew;
		if (!inMenuNew)
		{
			tcGameMain->setEnabled(true);
			tcGameWeapons->setEnabled(true);
			tcGameWeapons->fade(1,5);
			tcMenuMain->setEnabled(false);
		}
		else
		{
			tcGameMain->setEnabled(false);
			tcGameWeapons->setEnabled(false);
			tcMenuMain->setEnabled(true);
		}
	}

	int inAutomapNew =  PortableInAutomap() && !inMenuLast; //Dont show if menu comes up
	if (inAutomapLast != inAutomapNew)
	{
		inAutomapLast = inAutomapNew;
		if (inAutomapNew)
		{
			tcAutomap->animateIn(5);
		}
		else
		{
			tcAutomap->animateOut(5);
		}
	}

	setHideSticks(!showSticks);
	controlsContainer.draw();
}


void setTouchSettings(float alpha,float strafe,float fwd,float pitch,float yaw,int other)
{

	gameControlsAlpha = alpha;
	if (tcGameMain)
	{
		tcGameMain->setAlpha(gameControlsAlpha);
		controlsContainer.editButtonAlpha = gameControlsAlpha;
	}

	showWeaponCycle = other & 0x1?true:false;
	turnMouseMode   = other & 0x2?true:false;
	invertLook      = other & 0x4?true:false;
	precisionShoot  = other & 0x8?true:false;
	showSticks      = other & 0x1000?true:false;

	hideTouchControls = other & 0x80000000?true:false;


	switch ((other>>4) & 0xF)
	{
	case 1:
		left_double_action = gamefunc_Fire;
		break;
	case 2:
		left_double_action = gamefunc_Jump;
		break;
	default:
		left_double_action = 0;
	}

	switch ((other>>8) & 0xF)
	{
	case 1:
		right_double_action = gamefunc_Fire;
		break;
	case 2:
		right_double_action = gamefunc_Jump;
		break;
	default:
		right_double_action = 0;
	}

	strafe_sens = strafe;
	forward_sens = fwd;
	pitch_sens = pitch;
	yaw_sens = yaw;

}

int quit_now = 0;

#define EXPORT_ME __attribute__ ((visibility("default")))

JNIEnv* env_;

int argc=1;
const char * argv[32];
std::string graphicpath;


std::string doom_path;

const char * getDoomPath()
{
	return doom_path.c_str();
}

jint EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_init( JNIEnv* env,
		jobject thiz,jstring graphics_dir,jint mem_mb,jobjectArray argsArray,jint renderer,jstring doom_path_ )
{
	env_ = env;
	curRenderer = renderer;
	//curRenderer = REND_SOFT;
	curRenderer = REND_GL;

	argv[0] = "eduke32";
	int argCount = (env)->GetArrayLength( argsArray);
	LOGI("argCount = %d",argCount);
	for (int i=0; i<argCount; i++) {
		jstring string = (jstring) (env)->GetObjectArrayElement( argsArray, i);
		argv[argc] = (char *)(env)->GetStringUTFChars( string, 0);
		LOGI("arg = %s",argv[argc]);
		argc++;
	}



	doom_path = (char *)(env)->GetStringUTFChars( doom_path_, 0);

	//Change working dir, save games etc

	chdir(getDoomPath());
	putenv("TIMIDITY_CFG=../timidity.cfg");

	LOGI("doom_path = %s",getDoomPath());
	//argv[0] = "vavoom";
	//argv[1] = "-basedir";
	//argv[2] = "/sdcard/Beloko/Quake/FULL";
	//args[3] = "-doom";


	const char * p = env->GetStringUTFChars(graphics_dir,NULL);
	graphicpath =  std::string(p);



	initControls(android_screen_width,-android_screen_height,graphicpath.c_str(),(graphicpath + "/touch_controls.xml").c_str());
	//initControls(2,-2,graphicpath.c_str(),(graphicpath + "/prdoom.xml").c_str());

	/*
	if (renderer != REND_SOFT)
		SDL_SetSwapBufferCallBack(frameControls);

	if (renderer == REND_SOFT)// In soft mode SDL calls swap buffer, disable so it does not flicker
		SDL_SwapBufferPerformsSwap(false);
	 */


	SDL_SetSwapBufferCallBack(frameControls);

	//Now doen in java to keep context etc
	//SDL_SwapBufferPerformsSwap(false);


	PortableInit(argc,argv);


	return 0;
}


jint EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_frame( JNIEnv* env,
		jobject thiz )
{

	LOGI("Java_com_beloko_duke_engine_NativeLib_frame");

	PortableFrame();

	frameControls();

	return 0;
}

__attribute__((visibility("default"))) jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	LOGI("JNI_OnLoad");

	return JNI_VERSION_1_4;
}


void EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_keypress(JNIEnv *env, jobject obj,
		jint down, jint keycode, jint unicode)
{
	LOGI("keypress %d",keycode);
	if (controlsContainer.isEditing())
	{
		if (down && (keycode == SDL_SCANCODE_ESCAPE ))
			controlsContainer.finishEditing();

	}
	else
		PortableKeyEvent(down,keycode,unicode);

}


void EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_touchEvent(JNIEnv *env, jobject obj,
		jint action, jint pid, jfloat x, jfloat y)
{
	//LOGI("TOUCHED");
	controlsContainer.processPointer(action,pid,x,y);
}


void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_doAction(JNIEnv *env, jobject obj,
		jint state, jint action)
{
	//gamepadButtonPressed();
	if (hideTouchControls)
		if (tcGameMain->isEnabled())
			tcGameMain->animateOut(30);
	LOGI("doAction %d %d",state,action);
	PortableAction(state,action);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_analogFwd(JNIEnv *env, jobject obj,
		jfloat v)
{
	PortableMoveFwd(v);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_analogSide(JNIEnv *env, jobject obj,
		jfloat v)
{
	PortableMoveSide(v);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_analogPitch(JNIEnv *env, jobject obj,
		jint mode,jfloat v)
{
	PortableLookPitch(mode, v);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_analogYaw(JNIEnv *env, jobject obj,
		jint mode,jfloat v)
{
	PortableLookYaw(mode, v);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_setTouchSettings(JNIEnv *env, jobject obj,
		jfloat alpha,jfloat strafe,jfloat fwd,jfloat pitch,jfloat yaw,int other)
{
	setTouchSettings(alpha,strafe,fwd,pitch,yaw,other);
}

void EXPORT_ME Java_com_beloko_duke_engine_NativeLib_resetTouchSettings(JNIEnv *env, jobject obj)
{
	controlsContainer.resetDefaults();
}

std::string quickCommandString;
jint EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_quickCommand(JNIEnv *env, jobject obj,
		jstring command)
{
	const char * p = env->GetStringUTFChars(command,NULL);
	quickCommandString =  std::string(p) + "\n";
	env->ReleaseStringUTFChars(command, p);
	PortableCommand(quickCommandString.c_str());
}




void EXPORT_ME
Java_com_beloko_duke_engine_NativeLib_setScreenSize( JNIEnv* env,
		jobject thiz, jint width, jint height)
{
	android_screen_width = width;
	android_screen_height = height;
}


#include "SDL_main.h"
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls)
{
	/* This interface could expand with ABI negotiation, calbacks, etc. */
	SDL_Android_Init(env, cls);
	SDL_SetMainReady();
	// SDL_EventState(SDL_TEXTINPUT,SDL_ENABLE);
}


}