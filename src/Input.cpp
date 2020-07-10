#include <string.h>
#include "Backend/Input.h"
#include "Input.h"
#include "Filesystem.h"
#include "MathUtil.h"
#include "Log.h"
#include "Error.h"

//Binding save constants
#define BINDSAVE_NAME	"InputBind.ibs"	//The name of the file
const char *bindsaveSign = "IBV01";		//The signature, change this if you change the bindings structure or the binding enums in any way

//Controller state and default bindings
CONTROLLER gController[CONTROLLERS];

const BUTTONBINDS defaultBinds[CONTROLLERS] = {
	{ //Controller 1
		{{IBK_RETURN,	IBB_START},			{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Start
		{{IBK_A,		IBB_A},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//A
		{{IBK_S,		IBB_B},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//B
		{{IBK_D,		IBB_X},				{IBK_UNKNOWN,	IBB_Y}},		//C
		{{IBK_RIGHT,	IBB_DPAD_RIGHT},	{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Right
		{{IBK_LEFT,		IBB_DPAD_LEFT},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Left
		{{IBK_DOWN,		IBB_DPAD_DOWN},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Down
		{{IBK_UP,		IBB_DPAD_UP},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Up
	},
	
	{ //Controller 2
		{{IBK_UNKNOWN,	IBB_START},			{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Start
		{{IBK_UNKNOWN,	IBB_A},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//A
		{{IBK_UNKNOWN,	IBB_B},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//B
		{{IBK_UNKNOWN,	IBB_X},				{IBK_UNKNOWN,	IBB_Y}},		//C
		{{IBK_UNKNOWN,	IBB_DPAD_RIGHT},	{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Right
		{{IBK_UNKNOWN,	IBB_DPAD_LEFT},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Left
		{{IBK_UNKNOWN,	IBB_DPAD_DOWN},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Down
		{{IBK_UNKNOWN,	IBB_DPAD_UP},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Up
	},
	
	{ //Controller 3
		{{IBK_UNKNOWN,	IBB_START},			{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Start
		{{IBK_UNKNOWN,	IBB_A},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//A
		{{IBK_UNKNOWN,	IBB_B},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//B
		{{IBK_UNKNOWN,	IBB_X},				{IBK_UNKNOWN,	IBB_Y}},		//C
		{{IBK_UNKNOWN,	IBB_DPAD_RIGHT},	{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Right
		{{IBK_UNKNOWN,	IBB_DPAD_LEFT},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Left
		{{IBK_UNKNOWN,	IBB_DPAD_DOWN},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Down
		{{IBK_UNKNOWN,	IBB_DPAD_UP},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Up
	},
	
	{ //Controller 4
		{{IBK_UNKNOWN,	IBB_START},			{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Start
		{{IBK_UNKNOWN,	IBB_A},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//A
		{{IBK_UNKNOWN,	IBB_B},				{IBK_UNKNOWN,	IBB_UNKNOWN}},	//B
		{{IBK_UNKNOWN,	IBB_X},				{IBK_UNKNOWN,	IBB_Y}},		//C
		{{IBK_UNKNOWN,	IBB_DPAD_RIGHT},	{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Right
		{{IBK_UNKNOWN,	IBB_DPAD_LEFT},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Left
		{{IBK_UNKNOWN,	IBB_DPAD_DOWN},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Down
		{{IBK_UNKNOWN,	IBB_DPAD_UP},		{IBK_UNKNOWN,	IBB_UNKNOWN}},	//Up
	},
};

//Controller update functions
#include <math.h>

#define AXIS_DEADZONE	0x180
#define DIRRAD			1.01229 //About 58 degrees

//Get the directions the given axis position is towards
CONTROLMASK CONTROLLER::GetAxisState(int16_t chkAxisX, int16_t chkAxisY)
{
	CONTROLMASK state;
	
	//Deadzone check
	if ((chkAxisX * chkAxisX + chkAxisY * chkAxisY) < (AXIS_DEADZONE * AXIS_DEADZONE))
		return state;
	
	//Get our direction (converted to fixed point) and primary directions
	double angle = std::atan2(chkAxisY, chkAxisX);
	
	//Right
	if (angle >= -DIRRAD && angle <= DIRRAD)
		state.right = true;
	//Down
	if (angle >= M_PI_2 - DIRRAD && angle <= M_PI_2 + DIRRAD)
		state.down = true;
	//Left
	if (angle >= M_PI - DIRRAD || angle <= -M_PI + DIRRAD)
		state.left = true;
	//Up
	if (angle >= -M_PI_2 - DIRRAD && angle <= -M_PI_2 + DIRRAD)
		state.up = true;
	
	return state;
}

//Controller update per frame
#define DO_HELD_CHECK(buttonName)	for (size_t i = 0; i < MAX_BINDS; i++) \
									{	\
										if ((held.buttonName = Backend_IsKeyDown(binds.buttonName[i].key) || Backend_IsButtonDown(controllerIndex, binds.buttonName[i].button)) == true)	\
											break;	\
									}
#define DO_PRESS_CHECK(buttonName) press.buttonName = (held.buttonName ? !lastHeld.buttonName : false);

void CONTROLLER::Update(size_t controllerIndex)
{
	//Get our held buttons
	held = {};
	DO_HELD_CHECK(start);
	DO_HELD_CHECK(a);
	DO_HELD_CHECK(b);
	DO_HELD_CHECK(c);
	DO_HELD_CHECK(right);
	DO_HELD_CHECK(left);
	DO_HELD_CHECK(down);
	DO_HELD_CHECK(up);
	
	//Apply analogue stick control
	Backend_GetAnalogueStick(controllerIndex, &axisX, &axisY);
	
	CONTROLMASK axisState = GetAxisState(axisX, axisY);
	if (axisState.right)
		held.right = true;
	if (axisState.left)
		held.left = true;
	if (axisState.down)
		held.down = true;
	if (axisState.up)
		held.up = true;
	
	//Get our pressed buttons
	DO_PRESS_CHECK(start);
	DO_PRESS_CHECK(a);
	DO_PRESS_CHECK(b);
	DO_PRESS_CHECK(c);
	DO_PRESS_CHECK(right);
	DO_PRESS_CHECK(left);
	DO_PRESS_CHECK(down);
	DO_PRESS_CHECK(up);
	
	//Copy our last held for next update
	lastHeld = held;
}

//Accessible input functions
void ClearControllerInput()
{
	//Clear each controller's current input state
	for (size_t i = 0; i < CONTROLLERS; i++)
	{
		gController[i].held = {};
		gController[i].lastHeld = {};
		gController[i].press = {};
	}
}

void UpdateInput()
{
	//Update each controller
	for (size_t i = 0; i < CONTROLLERS; i++)
		gController[i].Update(i);
}

//Subsystem initialization and quitting
bool InitializeInput()
{
	LOG(("Initializing input... "));
	
	//Attempt to load our saved input bindings
	FS_FILE fp(gPrefPath + BINDSAVE_NAME, "rb");
	enum
	{
		FPFAIL_NONE,
		FPFAIL_FILEOPEN,
		FPFAIL_INVALID_SIGNATURE,
	} fpFail = FPFAIL_NONE;
	
	if (!fp.fail)
	{
		//Check our signature
		char *signature = new char[strlen(bindsaveSign)];
		fp.Read(signature, 1, strlen(bindsaveSign));
		
		if (strncmp(signature, bindsaveSign, strlen(bindsaveSign)))
		{
			//Invalid signature fail
			fpFail = FPFAIL_INVALID_SIGNATURE;
		}
		else
		{
			//Read bindings from file now that the signature's been confirmed
			for (size_t i = 0; i < CONTROLLERS; i++)
				fp.Read(&gController[i].binds, 1, sizeof(BUTTONBINDS));
		}
	}
	else
	{
		//File open failure
		fpFail = FPFAIL_FILEOPEN;
	}
	
	//Check if we've failed and use default bindings
	if (fpFail != FPFAIL_NONE)
	{
		//Get our fail string
		const char *failstr;
		switch (fpFail)
		{
			case FPFAIL_FILEOPEN:
				failstr = fp.fail;
				break;
			case FPFAIL_INVALID_SIGNATURE:
				failstr = "Invalid signature";
				break;
			default:
				failstr = "Unknown error";
				break;
		}
		
		//Print failure and use default bindings
		LOG(("NOTE: Using default input bindings - %s\n", failstr));
		for (size_t i = 0; i < CONTROLLERS; i++)
			gController[i].binds = defaultBinds[i];
	}
	
	LOG(("Success!\n"));
	return false;
}

void QuitInput()
{
	LOG(("Ending input... "));
	
	//Save our input bindings
	FS_FILE fp(gPrefPath + BINDSAVE_NAME, "wb");
	if (fp.fail != nullptr)
	{
		//Failed to open file
		LOG(("Failed to save input bindings: %s\n", fp.fail));
		return;
	}
	else
	{
		//Save our bindings
		fp.Write(bindsaveSign, 1, strlen(bindsaveSign));
		for (size_t i = 0; i < CONTROLLERS; i++)
			fp.Write(&gController[i].binds, 1, sizeof(BUTTONBINDS));
	}
	
	LOG(("Success!\n"));
}
