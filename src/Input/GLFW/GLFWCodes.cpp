#include "GLFWCodes.hpp"
#include <memory>

namespace ehj {

IBCodes::InputAction GLFWCodes::getIA(int32_t code) {
	switch (code) {
	case GLFW_RELEASE: return IA_RELEASE; break;
	case GLFW_PRESS: return IA_PRESS; break;
	case GLFW_REPEAT: return IA_REPEAT; break;
	default: return IA_INVALID; break;
	}
};

IBCodes::KeyboardKey GLFWCodes::getKK(int32_t code) {
	switch (code) {
	case GLFW_KEY_UNKNOWN :return KK_KEY_UNKNOWN; break;

	// printable keys
	case GLFW_KEY_SPACE :        return KK_KEY_SPACE;         break;
	case GLFW_KEY_APOSTROPHE :   return KK_KEY_APOSTROPHE;    break;
	case GLFW_KEY_COMMA :        return KK_KEY_COMMA;         break;
	case GLFW_KEY_MINUS :        return KK_KEY_MINUS;         break;
	case GLFW_KEY_PERIOD :       return KK_KEY_PERIOD;        break;
	case GLFW_KEY_SLASH :        return KK_KEY_SLASH;         break;
	case GLFW_KEY_0 :            return KK_KEY_0;             break;
	case GLFW_KEY_1 :            return KK_KEY_1;             break;
	case GLFW_KEY_2 :            return KK_KEY_2;             break;
	case GLFW_KEY_3 :            return KK_KEY_3;             break;
	case GLFW_KEY_4 :            return KK_KEY_4;             break;
	case GLFW_KEY_5 :            return KK_KEY_5;             break;
	case GLFW_KEY_6 :            return KK_KEY_6;             break;
	case GLFW_KEY_7 :            return KK_KEY_7;             break;
	case GLFW_KEY_8 :            return KK_KEY_8;             break;
	case GLFW_KEY_9 :            return KK_KEY_9;             break;
	case GLFW_KEY_SEMICOLON :    return KK_KEY_SEMICOLON;     break;
	case GLFW_KEY_EQUAL :        return KK_KEY_EQUAL;         break;
	case GLFW_KEY_A :            return KK_KEY_A;             break;
	case GLFW_KEY_B :            return KK_KEY_B;             break;
	case GLFW_KEY_C :            return KK_KEY_C;             break;
	case GLFW_KEY_D :            return KK_KEY_D;             break;
	case GLFW_KEY_E :            return KK_KEY_E;             break;
	case GLFW_KEY_F :            return KK_KEY_F;             break;
	case GLFW_KEY_G :            return KK_KEY_G;             break;
	case GLFW_KEY_H :            return KK_KEY_H;             break;
	case GLFW_KEY_I :            return KK_KEY_I;             break;
	case GLFW_KEY_J :            return KK_KEY_J;             break;
	case GLFW_KEY_K :            return KK_KEY_K;             break;
	case GLFW_KEY_L :            return KK_KEY_L;             break;
	case GLFW_KEY_M :            return KK_KEY_M;             break;
	case GLFW_KEY_N :            return KK_KEY_N;             break;
	case GLFW_KEY_O :            return KK_KEY_O;             break;
	case GLFW_KEY_P :            return KK_KEY_P;             break;
	case GLFW_KEY_Q :            return KK_KEY_Q;             break;
	case GLFW_KEY_R :            return KK_KEY_R;             break;
	case GLFW_KEY_S :            return KK_KEY_S;             break;
	case GLFW_KEY_T :            return KK_KEY_T;             break;
	case GLFW_KEY_U :            return KK_KEY_U;             break;
	case GLFW_KEY_V :            return KK_KEY_V;             break;
	case GLFW_KEY_W :            return KK_KEY_W;             break;
	case GLFW_KEY_X :            return KK_KEY_X;             break;
	case GLFW_KEY_Y :            return KK_KEY_Y;             break;
	case GLFW_KEY_Z :            return KK_KEY_Z;             break;
	case GLFW_KEY_LEFT_BRACKET : return KK_KEY_LEFT_BRACKET;  break;
	case GLFW_KEY_BACKSLASH :    return KK_KEY_BACKSLASH;     break;
	case GLFW_KEY_RIGHT_BRACKET :return KK_KEY_RIGHT_BRACKET; break;
	case GLFW_KEY_GRAVE_ACCENT : return KK_KEY_GRAVE_ACCENT;  break;
	case GLFW_KEY_WORLD_1 :      return KK_KEY_WORLD_1;       break;
	case GLFW_KEY_WORLD_2 :      return KK_KEY_WORLD_2;       break;

	//Funktion Keys
	case GLFW_KEY_ESCAPE :       return KK_KEY_ESCAPE;        break;
	case GLFW_KEY_ENTER :        return KK_KEY_ENTER;         break;
	case GLFW_KEY_TAB :          return KK_KEY_TAB;           break;
	case GLFW_KEY_BACKSPACE :    return KK_KEY_BACKSPACE;     break;
	case GLFW_KEY_INSERT :       return KK_KEY_INSERT;        break;
	case GLFW_KEY_DELETE :       return KK_KEY_DELETE;        break;
	case GLFW_KEY_RIGHT :        return KK_KEY_RIGHT;         break;
	case GLFW_KEY_LEFT :         return KK_KEY_LEFT;          break;
	case GLFW_KEY_DOWN :         return KK_KEY_DOWN;          break;
	case GLFW_KEY_UP :           return KK_KEY_UP;            break;
	case GLFW_KEY_PAGE_UP :      return KK_KEY_PAGE_UP;       break;
	case GLFW_KEY_PAGE_DOWN :    return KK_KEY_PAGE_DOWN;     break;
	case GLFW_KEY_HOME :         return KK_KEY_HOME;          break;
	case GLFW_KEY_END :          return KK_KEY_END;           break;
	case GLFW_KEY_CAPS_LOCK :    return KK_KEY_CAPS_LOCK;     break;
	case GLFW_KEY_SCROLL_LOCK :  return KK_KEY_SCROLL_LOCK;   break;
	case GLFW_KEY_NUM_LOCK :     return KK_KEY_NUM_LOCK;      break;
	case GLFW_KEY_PRINT_SCREEN : return KK_KEY_PRINT_SCREEN;  break;
	case GLFW_KEY_PAUSE :        return KK_KEY_PAUSE;         break;
	case GLFW_KEY_F1 :           return KK_KEY_F1;            break;
	case GLFW_KEY_F2 :           return KK_KEY_F2;            break;
	case GLFW_KEY_F3 :           return KK_KEY_F3;            break;
	case GLFW_KEY_F4 :           return KK_KEY_F4;            break;
	case GLFW_KEY_F5 :           return KK_KEY_F5;            break;
	case GLFW_KEY_F6 :           return KK_KEY_F6;            break;
	case GLFW_KEY_F7 :           return KK_KEY_F7;            break;
	case GLFW_KEY_F8 :           return KK_KEY_F8;            break;
	case GLFW_KEY_F9 :           return KK_KEY_F9;            break;
	case GLFW_KEY_F10 :          return KK_KEY_F10;           break;
	case GLFW_KEY_F11 :          return KK_KEY_F11;           break;
	case GLFW_KEY_F12 :          return KK_KEY_F12;           break;
	case GLFW_KEY_F13 :          return KK_KEY_F13;           break;
	case GLFW_KEY_F14 :          return KK_KEY_F14;           break;
	case GLFW_KEY_F15 :          return KK_KEY_F15;           break;
	case GLFW_KEY_F16 :          return KK_KEY_F16;           break;
	case GLFW_KEY_F17 :          return KK_KEY_F17;           break;
	case GLFW_KEY_F18 :          return KK_KEY_F18;           break;
	case GLFW_KEY_F19 :          return KK_KEY_F19;           break;
	case GLFW_KEY_F20 :          return KK_KEY_F20;           break;
	case GLFW_KEY_F21 :          return KK_KEY_F21;           break;
	case GLFW_KEY_F22 :          return KK_KEY_F22;           break;
	case GLFW_KEY_F23 :          return KK_KEY_F23;           break;
	case GLFW_KEY_F24 :          return KK_KEY_F24;           break;
	case GLFW_KEY_F25 :          return KK_KEY_F25;           break;
	case GLFW_KEY_KP_0 :         return KK_KEY_KP_0;          break;
	case GLFW_KEY_KP_1 :         return KK_KEY_KP_1;          break;
	case GLFW_KEY_KP_2 :         return KK_KEY_KP_2;          break;
	case GLFW_KEY_KP_3 :         return KK_KEY_KP_3;          break;
	case GLFW_KEY_KP_4 :         return KK_KEY_KP_4;          break;
	case GLFW_KEY_KP_5 :         return KK_KEY_KP_5;          break;
	case GLFW_KEY_KP_6 :         return KK_KEY_KP_6;          break;
	case GLFW_KEY_KP_7 :         return KK_KEY_KP_7;          break;
	case GLFW_KEY_KP_8 :         return KK_KEY_KP_8;          break;
	case GLFW_KEY_KP_9 :         return KK_KEY_KP_9;          break;
	case GLFW_KEY_KP_DECIMAL :   return KK_KEY_KP_DECIMAL;    break;
	case GLFW_KEY_KP_DIVIDE :    return KK_KEY_KP_DIVIDE;     break;
	case GLFW_KEY_KP_MULTIPLY :  return KK_KEY_KP_MULTIPLY;   break;
	case GLFW_KEY_KP_SUBTRACT :  return KK_KEY_KP_SUBTRACT;   break;
	case GLFW_KEY_KP_ADD :       return KK_KEY_KP_ADD;        break;
	case GLFW_KEY_KP_ENTER :     return KK_KEY_KP_ENTER;      break;
	case GLFW_KEY_KP_EQUAL :     return KK_KEY_KP_EQUAL;      break;
	case GLFW_KEY_LEFT_SHIFT :   return KK_KEY_LEFT_SHIFT;    break;
	case GLFW_KEY_LEFT_CONTROL : return KK_KEY_LEFT_CONTROL;  break;
	case GLFW_KEY_LEFT_ALT :     return KK_KEY_LEFT_ALT;      break;
	case GLFW_KEY_LEFT_SUPER :   return KK_KEY_LEFT_SUPER;    break;
	case GLFW_KEY_RIGHT_SHIFT :  return KK_KEY_RIGHT_SHIFT;   break;
	case GLFW_KEY_RIGHT_CONTROL :return KK_KEY_RIGHT_CONTROL; break;
	case GLFW_KEY_RIGHT_ALT :    return KK_KEY_RIGHT_ALT;     break;
	case GLFW_KEY_RIGHT_SUPER :  return KK_KEY_RIGHT_SUPER;   break;
	case GLFW_KEY_MENU :         return KK_KEY_MENU;          break;

	default: return KK_KEY_UNKNOWN; break;
	}
}

IBCodes::MouseButton GLFWCodes::getMB(int32_t code) {
	switch(code) {
	case GLFW_MOUSE_BUTTON_4: return MB_BUTTON_4; break;
	case GLFW_MOUSE_BUTTON_5: return MB_BUTTON_5; break;
	case GLFW_MOUSE_BUTTON_6: return MB_BUTTON_6; break;
	case GLFW_MOUSE_BUTTON_7: return MB_BUTTON_7; break;
	case GLFW_MOUSE_BUTTON_LAST: return MB_BUTTON_LAST; break;
	case GLFW_MOUSE_BUTTON_LEFT: return MB_BUTTON_LEFT; break;
	case GLFW_MOUSE_BUTTON_RIGHT: return MB_BUTTON_RIGHT; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: return MB_BUTTON_MIDDLE; break;
	default: return MB_BUTTON_NONE; break;
	}
}

}//ehj
