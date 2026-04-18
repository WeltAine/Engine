#pragma once

//来源于glfw3.h

/*! @defgroup buttons Mouse buttons
 *  @brief Mouse button IDs.
 *
 *  See [mouse button input](@ref input_mouse_button) for how these are used.
 *
 *  @ingroup input
 *  @{ */


namespace Ayin {

	AYIN_API enum class MouseCode : uint16_t {

		Mouse_Button_1         = 0,
		Mouse_Button_2         = 1,
		Mouse_Button_3         = 2,
		Mouse_Button_4         = 3,
		Mouse_Button_5         = 4,
		Mouse_Button_6         = 5,
		Mouse_Button_7         = 6,
		Mouse_Button_8         = 7,
							   
		Mouse_Button_Last      = Mouse_Button_8,
		Mouse_Button_Left      = Mouse_Button_1,
		Mouse_Button_Right     = Mouse_Button_2,
		Mouse_Button_Middle    = Mouse_Button_3

	};

	AYIN_API inline std::ostream& operator<< (std::ostream& os, MouseCode mouseCode) {

		os << static_cast<uint32_t>(mouseCode);
		return os;

	}

}

#define AYIN_MOUSE_BUTTON_1         ::Ayin::MouseCode::Mouse_Button_1       
#define AYIN_MOUSE_BUTTON_2         ::Ayin::MouseCode::Mouse_Button_2       
#define AYIN_MOUSE_BUTTON_3         ::Ayin::MouseCode::Mouse_Button_3       
#define AYIN_MOUSE_BUTTON_4         ::Ayin::MouseCode::Mouse_Button_4       
#define AYIN_MOUSE_BUTTON_5         ::Ayin::MouseCode::Mouse_Button_5       
#define AYIN_MOUSE_BUTTON_6         ::Ayin::MouseCode::Mouse_Button_6       
#define AYIN_MOUSE_BUTTON_7         ::Ayin::MouseCode::Mouse_Button_7       
#define AYIN_MOUSE_BUTTON_8         ::Ayin::MouseCode::Mouse_Button_8       
														 
#define AYIN_MOUSE_BUTTON_LAST      ::Ayin::MouseCode::Mouse_Button_Last    
#define AYIN_MOUSE_BUTTON_LEFT      ::Ayin::MouseCode::Mouse_Button_Left    
#define AYIN_MOUSE_BUTTON_RIGHT     ::Ayin::MouseCode::Mouse_Button_Right   
#define AYIN_MOUSE_BUTTON_MIDDLE    ::Ayin::MouseCode::Mouse_Button_Middle  
