#pragma once

//说实话，引入AYIN_KEY_CODE，对GLFW这一实现侧进行一定程度解耦后，我才注意到GLFW是一个实现侧
//之前它只是游走在那些具体的实现侧中，比如这里的WindowsWindow（讲真，这个文件应该叫GLFWXXX之类的而不是windows，因为这里其实没有和具体windows相关的东西，只有一个基于GLFW的实现类，而非Windows，当然，如果用Window2API我想现在的命名很合适）
//但是在我们有了Input轮询系统之后，GLFW就不在只是游离在实现侧中
//为了询问按钮状态我们需要在查询时指定目标code，而我们之前是使用的GLFW的code宏，这就导致了我们要查询就必须和GLFW直接联系
//而这是耦合的，如果我们之后不打算使用GLFW，而是别的，更换就很麻烦
//所以我们要有一个自己的输入映射宏，将这些底层的实现困在核心中，而不是暴露到客户端


namespace Ayin {


	enum class KeyCode : uint16_t {

		//来源于glfw3.h
		//我们直接使用GLFW的按键设置作为我们Ayin引擎的按键设置，至少在当下这个阶段，我们就不需要准备各种各样的转换辅助方法了
		//如果我们执意要自己来设置这些常量，那么我们就必须要实现一些辅助转换方法了 

		//按键流向抽象侧时变为AyinKeyCode，按键流向实现侧是转换为对应的KeyCode


		Space		        = 32 ,
		Apostrophe			= 39 , /* ' */
		Comma				= 44 , /* , */
		Minus				= 45 , /* - */
		Period				= 46 , /* . */
		Slash				= 47 , /* / */

		D0                  = 48 , /* 0 */
		D1                  = 49 , /* 1 */
		D2                  = 50 , /* 2 */
		D3                  = 51 , /* 3 */
		D4                  = 52 , /* 4 */
		D5                  = 53 , /* 5 */
		D6                  = 54 , /* 6 */
		D7                  = 55 , /* 7 */
		D8                  = 56 , /* 8 */
		D9                  = 57 , /* 9 */

		Semicolon			= 59 , /* ; */
		Equal				= 61 , /* = */

		A					= 65 ,
		B					= 66 ,
		C					= 67 ,
		D					= 68 ,
		E					= 69 ,
		F					= 70 ,
		G					= 71 ,
		H					= 72 ,
		I					= 73 ,
		J					= 74 ,
		K					= 75 ,
		L					= 76 ,
		M					= 77 ,
		N					= 78 ,
		O					= 79 ,
		P					= 80 ,
		Q					= 81 ,
		R					= 82 ,
		S					= 83 ,
		T					= 84 ,
		U					= 85 ,
		V					= 86 ,
		W					= 87 ,
		X					= 88 ,
		Y					= 89 ,
		Z					= 90 ,

		Left_bracket		= 91 , /* [ */
		Backslash			= 92 , /* \ */
		Right_bracket		= 93 , /* ] */
		Grave_accent		= 96 , /* ` */

		World_1				= 161, /* non-US #1 */
		World_2				= 162, /* non-US #2 */
		
		/* Function keys */
		Escape				= 256,
		Enter				= 257,
		Tab					= 258,
		Backspace			= 259,
		Insert				= 260,
		Delete				= 261,
		Right				= 262,
		Left				= 263,
		Down				= 264,
		Up					= 265,
		Page_up				= 266,
		Page_down			= 267,
		Home				= 268,
		End					= 269,
		Caps_lock			= 280,
		Scroll_lock			= 281,
		Num_lock			= 282,
		Print_screen		= 283,
		Pause				= 284,
		F1					= 290,
		F2					= 291,
		F3					= 292,
		F4					= 293,
		F5					= 294,
		F6					= 295,
		F7					= 296,
		F8					= 297,
		F9					= 298,
		F10					= 299,
		F11					= 300,
		F12					= 301,
		F13					= 302,
		F14					= 303,
		F15					= 304,
		F16					= 305,
		F17					= 306,
		F18					= 307,
		F19					= 308,
		F20					= 309,
		F21					= 310,
		F22					= 311,
		F23					= 312,
		F24					= 313,
		F25					= 314,

		/* Keypad */
		KP_0				= 320,
		KP_1				= 321,
		KP_2				= 322,
		KP_3				= 323,
		KP_4				= 324,
		KP_5				= 325,
		KP_6				= 326,
		KP_7				= 327,
		KP_8				= 328,
		KP_9				= 329,
		Kp_Decimal			= 330,
		Kp_Divide			= 331,
		Kp_Multiply			= 332,
		Kp_Subtract			= 333,
		Kp_Add				= 334,
		Kp_Enter			= 335,
		Kp_Equal			= 336,

		Left_Shift			= 340,
		Left_Control		= 341,
		Left_Alt			= 342,
		Left_Super			= 343,
		Right_Shift			= 344,
		Right_Control		= 345,
		Right_Alt			= 346,
		Right_Super			= 347,
		Menu				= 348, 

		MAX					= 999,
	};

	AYIN_API inline std::ostream& operator<< (std::ostream& os, KeyCode keyCode) {

		os << static_cast<uint32_t>(keyCode);
		return os;

	}

}


/* Printable keys */
#define AYIN_KEY_SPACE					::Ayin::KeyCode::Space		    
#define AYIN_KEY_APOSTROPHE         	::Ayin::KeyCode::Apostrophe		
#define AYIN_KEY_COMMA              	::Ayin::KeyCode::Comma			
#define AYIN_KEY_MINUS              	::Ayin::KeyCode::Minus			
#define AYIN_KEY_PERIOD             	::Ayin::KeyCode::Period			
#define AYIN_KEY_SLASH              	::Ayin::KeyCode::Slash			
#define AYIN_KEY_0                  	::Ayin::KeyCode::D0              
#define AYIN_KEY_1                  	::Ayin::KeyCode::D1              
#define AYIN_KEY_2                  	::Ayin::KeyCode::D2              
#define AYIN_KEY_3                  	::Ayin::KeyCode::D3              
#define AYIN_KEY_4                  	::Ayin::KeyCode::D4              
#define AYIN_KEY_5                  	::Ayin::KeyCode::D5              
#define AYIN_KEY_6                  	::Ayin::KeyCode::D6              
#define AYIN_KEY_7                  	::Ayin::KeyCode::D7              
#define AYIN_KEY_8                  	::Ayin::KeyCode::D8              
#define AYIN_KEY_9                  	::Ayin::KeyCode::D9              
#define AYIN_KEY_SEMICOLON          	::Ayin::KeyCode::Semicolon		
#define AYIN_KEY_EQUAL              	::Ayin::KeyCode::Equal			
#define AYIN_KEY_A                  	::Ayin::KeyCode::A				
#define AYIN_KEY_B                  	::Ayin::KeyCode::B				
#define AYIN_KEY_C                  	::Ayin::KeyCode::C				
#define AYIN_KEY_D                  	::Ayin::KeyCode::D				
#define AYIN_KEY_E                  	::Ayin::KeyCode::E				
#define AYIN_KEY_F                  	::Ayin::KeyCode::F				
#define AYIN_KEY_G                  	::Ayin::KeyCode::G				
#define AYIN_KEY_H                  	::Ayin::KeyCode::H				
#define AYIN_KEY_I                  	::Ayin::KeyCode::I				
#define AYIN_KEY_J                  	::Ayin::KeyCode::J				
#define AYIN_KEY_K                  	::Ayin::KeyCode::K				
#define AYIN_KEY_L                  	::Ayin::KeyCode::L				
#define AYIN_KEY_M                  	::Ayin::KeyCode::M				
#define AYIN_KEY_N                  	::Ayin::KeyCode::N				
#define AYIN_KEY_O                  	::Ayin::KeyCode::O				
#define AYIN_KEY_P                  	::Ayin::KeyCode::P				
#define AYIN_KEY_Q                  	::Ayin::KeyCode::Q				
#define AYIN_KEY_R                  	::Ayin::KeyCode::R				
#define AYIN_KEY_S                  	::Ayin::KeyCode::S				
#define AYIN_KEY_T                  	::Ayin::KeyCode::T				
#define AYIN_KEY_U                  	::Ayin::KeyCode::U				
#define AYIN_KEY_V                  	::Ayin::KeyCode::V				
#define AYIN_KEY_W                  	::Ayin::KeyCode::W				
#define AYIN_KEY_X                  	::Ayin::KeyCode::X				
#define AYIN_KEY_Y                  	::Ayin::KeyCode::Y				
#define AYIN_KEY_Z                  	::Ayin::KeyCode::Z				
#define AYIN_KEY_LEFT_BRACKET       	::Ayin::KeyCode::Left_bracket	
#define AYIN_KEY_BACKSLASH          	::Ayin::KeyCode::Backslash		
#define AYIN_KEY_RIGHT_BRACKET      	::Ayin::KeyCode::Right_bracket	
#define AYIN_KEY_GRAVE_ACCENT       	::Ayin::KeyCode::Grave_accent	
#define AYIN_KEY_WORLD_1            	::Ayin::KeyCode::World_1			
#define AYIN_KEY_WORLD_2            	::Ayin::KeyCode::World_2			
										
/* Function keys */						
#define AYIN_KEY_ESCAPE             	::Ayin::KeyCode::Escape			
#define AYIN_KEY_ENTER              	::Ayin::KeyCode::Enter			
#define AYIN_KEY_TAB                	::Ayin::KeyCode::Tab				
#define AYIN_KEY_BACKSPACE          	::Ayin::KeyCode::Backspace		
#define AYIN_KEY_INSERT             	::Ayin::KeyCode::Insert			
#define AYIN_KEY_DELETE             	::Ayin::KeyCode::Delete			
#define AYIN_KEY_RIGHT              	::Ayin::KeyCode::Right			
#define AYIN_KEY_LEFT               	::Ayin::KeyCode::Left			
#define AYIN_KEY_DOWN               	::Ayin::KeyCode::Down			
#define AYIN_KEY_UP                 	::Ayin::KeyCode::Up				
#define AYIN_KEY_PAGE_UP            	::Ayin::KeyCode::Page_up			
#define AYIN_KEY_PAGE_DOWN          	::Ayin::KeyCode::Page_down		
#define AYIN_KEY_HOME               	::Ayin::KeyCode::Home			
#define AYIN_KEY_END                	::Ayin::KeyCode::End				
#define AYIN_KEY_CAPS_LOCK          	::Ayin::KeyCode::Caps_lock		
#define AYIN_KEY_SCROLL_LOCK        	::Ayin::KeyCode::Scroll_lock		
#define AYIN_KEY_NUM_LOCK           	::Ayin::KeyCode::Num_lock		
#define AYIN_KEY_PRINT_SCREEN       	::Ayin::KeyCode::Print_screen	
#define AYIN_KEY_PAUSE              	::Ayin::KeyCode::Pause			
#define AYIN_KEY_F1                 	::Ayin::KeyCode::F1				
#define AYIN_KEY_F2                 	::Ayin::KeyCode::F2				
#define AYIN_KEY_F3                 	::Ayin::KeyCode::F3				
#define AYIN_KEY_F4                 	::Ayin::KeyCode::F4				
#define AYIN_KEY_F5                 	::Ayin::KeyCode::F5				
#define AYIN_KEY_F6                 	::Ayin::KeyCode::F6				
#define AYIN_KEY_F7                 	::Ayin::KeyCode::F7				
#define AYIN_KEY_F8                 	::Ayin::KeyCode::F8				
#define AYIN_KEY_F9                 	::Ayin::KeyCode::F9				
#define AYIN_KEY_F10                	::Ayin::KeyCode::F10				
#define AYIN_KEY_F11                	::Ayin::KeyCode::F11				
#define AYIN_KEY_F12                	::Ayin::KeyCode::F12				
#define AYIN_KEY_F13                	::Ayin::KeyCode::F13				
#define AYIN_KEY_F14                	::Ayin::KeyCode::F14				
#define AYIN_KEY_F15                	::Ayin::KeyCode::F15				
#define AYIN_KEY_F16                	::Ayin::KeyCode::F16				
#define AYIN_KEY_F17                	::Ayin::KeyCode::F17				
#define AYIN_KEY_F18                	::Ayin::KeyCode::F18				
#define AYIN_KEY_F19                	::Ayin::KeyCode::F19				
#define AYIN_KEY_F20                	::Ayin::KeyCode::F20				
#define AYIN_KEY_F21                	::Ayin::KeyCode::F21				
#define AYIN_KEY_F22                	::Ayin::KeyCode::F22				
#define AYIN_KEY_F23                	::Ayin::KeyCode::F23				
#define AYIN_KEY_F24                	::Ayin::KeyCode::F24				
#define AYIN_KEY_F25                	::Ayin::KeyCode::F25		
										
#define AYIN_KEY_KP_0               	::Ayin::KeyCode::KP_0
#define AYIN_KEY_KP_1               	::Ayin::KeyCode::KP_1			
#define AYIN_KEY_KP_2               	::Ayin::KeyCode::KP_2			
#define AYIN_KEY_KP_3               	::Ayin::KeyCode::KP_3			
#define AYIN_KEY_KP_4               	::Ayin::KeyCode::KP_4			
#define AYIN_KEY_KP_5               	::Ayin::KeyCode::KP_5			
#define AYIN_KEY_KP_6               	::Ayin::KeyCode::KP_6			
#define AYIN_KEY_KP_7               	::Ayin::KeyCode::KP_7			
#define AYIN_KEY_KP_8               	::Ayin::KeyCode::KP_8			
#define AYIN_KEY_KP_9               	::Ayin::KeyCode::KP_9			
#define AYIN_KEY_KP_DECIMAL         	::Ayin::KeyCode::Kp_Decimal		
#define AYIN_KEY_KP_DIVIDE          	::Ayin::KeyCode::Kp_Divide		
#define AYIN_KEY_KP_MULTIPLY        	::Ayin::KeyCode::Kp_Multiply		
#define AYIN_KEY_KP_SUBTRACT        	::Ayin::KeyCode::Kp_Subtract		
#define AYIN_KEY_KP_ADD             	::Ayin::KeyCode::Kp_Add		
#define AYIN_KEY_KP_ENTER           	::Ayin::KeyCode::Kp_Enter			
#define AYIN_KEY_KP_EQUAL           	::Ayin::KeyCode::Kp_Equal		

#define AYIN_KEY_LEFT_SHIFT         	::Ayin::KeyCode::Left_Shift		
#define AYIN_KEY_LEFT_CONTROL       	::Ayin::KeyCode::Left_Control		
#define AYIN_KEY_LEFT_ALT           	::Ayin::KeyCode::Left_Alt	
#define AYIN_KEY_LEFT_SUPER         	::Ayin::KeyCode::Left_Super		
#define AYIN_KEY_RIGHT_SHIFT        	::Ayin::KeyCode::Right_Shift		
#define AYIN_KEY_RIGHT_CONTROL      	::Ayin::KeyCode::Right_Control		
#define AYIN_KEY_RIGHT_ALT          	::Ayin::KeyCode::Right_Alt	
#define AYIN_KEY_RIGHT_SUPER        	::Ayin::KeyCode::Right_Super		
#define AYIN_KEY_MENU               	::Ayin::KeyCode::Menu		
												

											