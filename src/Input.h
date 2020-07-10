#pragma once
#include <stddef.h>
#include <stdint.h>

//Constants
#define CONTROLLERS	4
#define MAX_BINDS 2

//Input binding enumerations

//Keyboard mappings - https://github.com/SDL-mirror/SDL/blob/master/include/SDL_scancode.h
enum INPUTBINDKEY
{
	IBK_UNKNOWN,
	
	//Letters
	IBK_A,
	IBK_B,
	IBK_C,
	IBK_D,
	IBK_E,
	IBK_F,
	IBK_G,
	IBK_H,
	IBK_I,
	IBK_J,
	IBK_K,
	IBK_L,
	IBK_M,
	IBK_N,
	IBK_O,
	IBK_P,
	IBK_Q,
	IBK_R,
	IBK_S,
	IBK_T,
	IBK_U,
	IBK_V,
	IBK_W,
	IBK_X,
	IBK_Y,
	IBK_Z,
	
	//Numbers
	IBK_1,
	IBK_2,
	IBK_3,
	IBK_4,
	IBK_5,
	IBK_6,
	IBK_7,
	IBK_8,
	IBK_9,
	IBK_0,
	
	//Whitespace and escape
	IBK_RETURN,
	IBK_ESCAPE,
	IBK_BACKSPACE,
	IBK_TAB,
	IBK_SPACE,

	//Symbols
	IBK_MINUS,
	IBK_EQUALS,
	IBK_LEFTBRACKET,
	IBK_RIGHTBRACKET,
	IBK_BACKSLASH,
	IBK_NONUSHASH,
	IBK_SEMICOLON,
	IBK_APOSTROPHE,
	IBK_GRAVE,
	IBK_COMMA,
	IBK_PERIOD,
	IBK_SLASH,

	//Caps lock
	IBK_CAPSLOCK,

	//Function keys
	IBK_F1,
	IBK_F2,
	IBK_F3,
	IBK_F4,
	IBK_F5,
	IBK_F6,
	IBK_F7,
	IBK_F8,
	IBK_F9,
	IBK_F10,
	IBK_F11,
	IBK_F12,
	
	//Control keys
	IBK_PRINTSCREEN,
	IBK_SCROLLLOCK,
	IBK_PAUSE,
	IBK_INSERT,
	IBK_HOME,
	IBK_PAGEUP,
	IBK_DELETE,
	IBK_END,
	IBK_PAGEDOWN,
	
	//Arrow keys
	IBK_RIGHT,
	IBK_LEFT,
	IBK_DOWN,
	IBK_UP,

	//Keypad
	IBK_NUMLOCKCLEAR,
	IBK_KP_DIVIDE,
	IBK_KP_MULTIPLY,
	IBK_KP_MINUS,
	IBK_KP_PLUS,
	IBK_KP_ENTER,
	IBK_KP_1,
	IBK_KP_2,
	IBK_KP_3,
	IBK_KP_4,
	IBK_KP_5,
	IBK_KP_6,
	IBK_KP_7,
	IBK_KP_8,
	IBK_KP_9,
	IBK_KP_0,
	IBK_KP_PERIOD,

	//Some USB standard stuff
	IBK_NONUSBACKSLASH,
	IBK_APPLICATION,
	IBK_POWER,
	IBK_KP_EQUALS,
	IBK_F13,
	IBK_F14,
	IBK_F15,
	IBK_F16,
	IBK_F17,
	IBK_F18,
	IBK_F19,
	IBK_F20,
	IBK_F21,
	IBK_F22,
	IBK_F23,
	IBK_F24,
	IBK_EXECUTE,
	IBK_HELP,
	IBK_MENU,
	IBK_SELECT,
	IBK_STOP,
	IBK_AGAIN,
	IBK_UNDO,
	IBK_CUT,
	IBK_COPY,
	IBK_PASTE,
	IBK_FIND,
	IBK_MUTE,
	IBK_VOLUMEUP,
	IBK_VOLUMEDOWN,
	IBK_KP_COMMA,
	IBK_KP_EQUALSAS400,

	IBK_INTERNATIONAL1,
	IBK_INTERNATIONAL2,
	IBK_INTERNATIONAL3,
	IBK_INTERNATIONAL4,
	IBK_INTERNATIONAL5,
	IBK_INTERNATIONAL6,
	IBK_INTERNATIONAL7,
	IBK_INTERNATIONAL8,
	IBK_INTERNATIONAL9,
	IBK_LANG1,
	IBK_LANG2,
	IBK_LANG3,
	IBK_LANG4,
	IBK_LANG5,
	IBK_LANG6,
	IBK_LANG7,
	IBK_LANG8,
	IBK_LANG9,

	IBK_ALTERASE,
	IBK_SYSREQ,
	IBK_CANCEL,
	IBK_CLEAR,
	IBK_PRIOR,
	IBK_RETURN2,
	IBK_SEPARATOR,
	IBK_OUT,
	IBK_OPER,
	IBK_CLEARAGAIN,
	IBK_CRSEL,
	IBK_EXSEL,

	IBK_KP_00,
	IBK_KP_000,
	IBK_THOUSANDSSEPARATOR,
	IBK_DECIMALSEPARATOR,
	IBK_CURRENCYUNIT,
	IBK_CURRENCYSUBUNIT,
	IBK_KP_LEFTPAREN,
	IBK_KP_RIGHTPAREN,
	IBK_KP_LEFTBRACE,
	IBK_KP_RIGHTBRACE,
	IBK_KP_TAB,
	IBK_KP_BACKSPACE,
	IBK_KP_A,
	IBK_KP_B,
	IBK_KP_C,
	IBK_KP_D,
	IBK_KP_E,
	IBK_KP_F,
	IBK_KP_XOR,
	IBK_KP_POWER,
	IBK_KP_PERCENT,
	IBK_KP_LESS,
	IBK_KP_GREATER,
	IBK_KP_AMPERSAND,
	IBK_KP_DBLAMPERSAND,
	IBK_KP_VERTICALBAR,
	IBK_KP_DBLVERTICALBAR,
	IBK_KP_COLON,
	IBK_KP_HASH,
	IBK_KP_SPACE,
	IBK_KP_AT,
	IBK_KP_EXCLAM,
	IBK_KP_MEMSTORE,
	IBK_KP_MEMRECALL,
	IBK_KP_MEMCLEAR,
	IBK_KP_MEMADD,
	IBK_KP_MEMSUBTRACT,
	IBK_KP_MEMMULTIPLY,
	IBK_KP_MEMDIVIDE,
	IBK_KP_PLUSMINUS,
	IBK_KP_CLEAR,
	IBK_KP_CLEARENTRY,
	IBK_KP_BINARY,
	IBK_KP_OCTAL,
	IBK_KP_DECIMAL,
	IBK_KP_HEXADECIMAL,

	IBK_LCTRL,
	IBK_LSHIFT,
	IBK_LALT,
	IBK_LGUI,
	IBK_RCTRL,
	IBK_RSHIFT,
	IBK_RALT,
	IBK_RGUI,

	IBK_MODE,

	IBK_AUDIONEXT,
	IBK_AUDIOPREV,
	IBK_AUDIOSTOP,
	IBK_AUDIOPLAY,
	IBK_AUDIOMUTE,
	IBK_MEDIASELECT,
	IBK_WWW,
	IBK_MAIL,
	IBK_CALCULATOR,
	IBK_COMPUTER,
	IBK_AC_SEARCH,
	IBK_AC_HOME,
	IBK_AC_BACK,
	IBK_AC_FORWARD,
	IBK_AC_STOP,
	IBK_AC_REFRESH,
	IBK_AC_BOOKMARKS,

	IBK_BRIGHTNESSDOWN,
	IBK_BRIGHTNESSUP,
	IBK_DISPLAYSWITCH,
	
	IBK_KBDILLUMTOGGLE,
	IBK_KBDILLUMDOWN,
	IBK_KBDILLUMUP,
	IBK_EJECT,
	IBK_SLEEP,

	IBK_APP1,
	IBK_APP2,
	IBK_AUDIOREWIND,
	IBK_AUDIOFASTFORWARD,
	
	IBK_MAX,
};

//Gamepad mappings - https://github.com/SDL-mirror/SDL/blob/master/include/SDL_gamecontroller.h
enum INPUTBINDBUTTON
{
	IBB_UNKNOWN,
	
	//Face buttons
	IBB_A,
	IBB_B,
	IBB_X,
	IBB_Y,
	
	//Middle buttons
	IBB_BACK,
	IBB_GUIDE,
	IBB_START,
	
	//Left and right analogue sticks
	IBB_LEFTSTICK,
	IBB_RIGHTSTICK,
	
	//Shoulder buttons
	IBB_LEFTSHOULDER,
	IBB_RIGHTSHOULDER,
	
	//DPad
	IBB_DPAD_UP,
	IBB_DPAD_DOWN,
	IBB_DPAD_LEFT,
	IBB_DPAD_RIGHT,
	
	IBB_MAX,
};

//Structures
struct CONTROLMASK
{
	bool start = false;
	
	bool a = false;
	bool b = false;
	bool c = false;
	
	bool right = false;
	bool left = false;
	bool down = false;
	bool up = false;
};

struct BUTTONBIND
{
	//Keyboard and gamepad bindings
	INPUTBINDKEY key = IBK_UNKNOWN;
	INPUTBINDBUTTON button = IBB_UNKNOWN;
};

struct BUTTONBINDS
{
	//Keyboard and gamepad bindings for each controller button
	BUTTONBIND start[MAX_BINDS];
	
	BUTTONBIND a[MAX_BINDS];
	BUTTONBIND b[MAX_BINDS];
	BUTTONBIND c[MAX_BINDS];
	
	BUTTONBIND right[MAX_BINDS];
	BUTTONBIND left[MAX_BINDS];
	BUTTONBIND down[MAX_BINDS];
	BUTTONBIND up[MAX_BINDS];
};

class CONTROLLER
{
	public:
		//Current control masks
		CONTROLMASK held;
		CONTROLMASK lastHeld;
		CONTROLMASK press;
		
		//Button binds
		BUTTONBINDS binds;
		
		//Gamepad axis
		int16_t axisX = 0; //Left < Right
		int16_t axisY = 0; // Up  <  Down
		
	public:
		CONTROLMASK GetAxisState(int16_t chkAxisX, int16_t chkAxisY);
		void Update(size_t controllerIndex);
};

//Controller state global
extern CONTROLLER gController[CONTROLLERS];

//Subsystem functions
void ClearControllerInput();
void UpdateInput();

bool InitializeInput();
void QuitInput();
