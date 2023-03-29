#pragma once
#include <memory>
#include <map>

namespace ehj {

class IBCodes {
public:
	enum InputAction : uint32_t {
		IA_INVALID = 0,
		IA_RELEASE,
		IA_PRESS,
		IA_REPEAT,
		IA_COUNT
	};

	enum KeyboardKey : uint32_t {
		KK_KEY_UNKNOWN = 0,

		/* Printable keys */
		KK_KEY_SPACE              ,
		KK_KEY_APOSTROPHE         ,
		KK_KEY_COMMA              ,
		KK_KEY_MINUS              ,
		KK_KEY_PERIOD             ,
		KK_KEY_SLASH              ,
		KK_KEY_0                  ,
		KK_KEY_1                  ,
		KK_KEY_2                  ,
		KK_KEY_3                  ,
		KK_KEY_4                  ,
		KK_KEY_5                  ,
		KK_KEY_6                  ,
		KK_KEY_7                  ,
		KK_KEY_8                  ,
		KK_KEY_9                  ,
		KK_KEY_SEMICOLON          ,
		KK_KEY_EQUAL              ,
		KK_KEY_A                  ,
		KK_KEY_B                  ,
		KK_KEY_C                  ,
		KK_KEY_D                  ,
		KK_KEY_E                  ,
		KK_KEY_F                  ,
		KK_KEY_G                  ,
		KK_KEY_H                  ,
		KK_KEY_I                  ,
		KK_KEY_J                  ,
		KK_KEY_K                  ,
		KK_KEY_L                  ,
		KK_KEY_M                  ,
		KK_KEY_N                  ,
		KK_KEY_O                  ,
		KK_KEY_P                  ,
		KK_KEY_Q                  ,
		KK_KEY_R                  ,
		KK_KEY_S                  ,
		KK_KEY_T                  ,
		KK_KEY_U                  ,
		KK_KEY_V                  ,
		KK_KEY_W                  ,
		KK_KEY_X                  ,
		KK_KEY_Y                  ,
		KK_KEY_Z                  ,
		KK_KEY_LEFT_BRACKET       ,
		KK_KEY_BACKSLASH          ,
		KK_KEY_RIGHT_BRACKET      ,
		KK_KEY_GRAVE_ACCENT       ,
		KK_KEY_WORLD_1            ,
		KK_KEY_WORLD_2            ,

		/* Function keys */
		KK_KEY_ESCAPE             ,
		KK_KEY_ENTER              ,
		KK_KEY_TAB                ,
		KK_KEY_BACKSPACE          ,
		KK_KEY_INSERT             ,
		KK_KEY_DELETE             ,
		KK_KEY_RIGHT              ,
		KK_KEY_LEFT               ,
		KK_KEY_DOWN               ,
		KK_KEY_UP                 ,
		KK_KEY_PAGE_UP            ,
		KK_KEY_PAGE_DOWN          ,
		KK_KEY_HOME               ,
		KK_KEY_END                ,
		KK_KEY_CAPS_LOCK          ,
		KK_KEY_SCROLL_LOCK        ,
		KK_KEY_NUM_LOCK           ,
		KK_KEY_PRINT_SCREEN       ,
		KK_KEY_PAUSE              ,
		KK_KEY_F1                 ,
		KK_KEY_F2                 ,
		KK_KEY_F3                 ,
		KK_KEY_F4                 ,
		KK_KEY_F5                 ,
		KK_KEY_F6                 ,
		KK_KEY_F7                 ,
		KK_KEY_F8                 ,
		KK_KEY_F9                 ,
		KK_KEY_F10                ,
		KK_KEY_F11                ,
		KK_KEY_F12                ,
		KK_KEY_F13                ,
		KK_KEY_F14                ,
		KK_KEY_F15                ,
		KK_KEY_F16                ,
		KK_KEY_F17                ,
		KK_KEY_F18                ,
		KK_KEY_F19                ,
		KK_KEY_F20                ,
		KK_KEY_F21                ,
		KK_KEY_F22                ,
		KK_KEY_F23                ,
		KK_KEY_F24                ,
		KK_KEY_F25                ,
		KK_KEY_KP_0               ,
		KK_KEY_KP_1               ,
		KK_KEY_KP_2               ,
		KK_KEY_KP_3               ,
		KK_KEY_KP_4               ,
		KK_KEY_KP_5               ,
		KK_KEY_KP_6               ,
		KK_KEY_KP_7               ,
		KK_KEY_KP_8               ,
		KK_KEY_KP_9               ,
		KK_KEY_KP_DECIMAL         ,
		KK_KEY_KP_DIVIDE          ,
		KK_KEY_KP_MULTIPLY        ,
		KK_KEY_KP_SUBTRACT        ,
		KK_KEY_KP_ADD             ,
		KK_KEY_KP_ENTER           ,
		KK_KEY_KP_EQUAL           ,
		KK_KEY_LEFT_SHIFT         ,
		KK_KEY_LEFT_CONTROL       ,
		KK_KEY_LEFT_ALT           ,
		KK_KEY_LEFT_SUPER         ,
		KK_KEY_RIGHT_SHIFT        ,
		KK_KEY_RIGHT_CONTROL      ,
		KK_KEY_RIGHT_ALT          ,
		KK_KEY_RIGHT_SUPER        ,
		KK_KEY_MENU               ,

		KK_KEY_LAST = KK_KEY_MENU ,
		KK_COUNT
	};

	enum KeyboardMod {
		//TODO
	};

	enum MouseButton : uint32_t {
		MB_BUTTON_NONE = 0  ,
		MB_BUTTON_1         ,
		MB_BUTTON_2         ,
		MB_BUTTON_3         ,
		MB_BUTTON_4         ,
		MB_BUTTON_5         ,
		MB_BUTTON_6         ,
		MB_BUTTON_7         ,
		MB_BUTTON_8         ,
		MB_BUTTON_LAST = MB_BUTTON_8  ,
		MB_BUTTON_LEFT = MB_BUTTON_1  ,
		MB_BUTTON_RIGHT = MB_BUTTON_2 ,
		MB_BUTTON_MIDDLE = MB_BUTTON_3,
		MB_COUNT
	};

	enum MouseMod {
		//TODO
	};
	virtual InputAction getIA(int32_t code) = 0;
	virtual KeyboardKey getKK(int32_t code) = 0;
	virtual MouseButton getMB(int32_t code) = 0;
};

}//ehj
