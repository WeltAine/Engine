#pragma once

//说实话，引入AYIN_KEY_CODE，对GLFW这一实现侧进行一定程度解耦后，我才注意到GLFW是一个实现侧
//之前它只是游走在那些具体的实现侧中，比如这里的WindowsWindow（讲真，这个文件应该叫GLFWXXX之类的而不是windows，因为这里其实没有和具体windows相关的东西，只有一个基于GLFW的实现类，而非Windows，当然，如果用Window2API我想现在的命名很合适）
//但是在我们有了Input轮询系统之后，GLFW就不在只是游离在实现侧中
//为了询问按钮状态我们需要在查询时指定目标code，而我们之前是使用的GLFW的code宏，这就导致了我们要查询就必须和GLFW直接联系
//而这是耦合的，如果我们之后不打算使用GLFW，而是别的，更换就很麻烦
//所以我们要有一个自己的输入映射宏，将这些底层的实现困在核心中，而不是暴露到客户端


//来源于glfw3.h
//我们直接使用GLFW的按键设置作为我们Ayin引擎的按键设置，至少在当下这个阶段，我们就不需要准备各种各样的转换辅助方法了
//如果我们执意要自己来设置这些常量，那么我们就必须要实现一些辅助转换方法了 

//按键流向抽象侧时变为AyinKeyCode，按键流向实现侧是转换为对应的KeyCode

/* Printable keys */
#define AYIN_KEY_SPACE              32
#define AYIN_KEY_APOSTROPHE         39  /* ' */
#define AYIN_KEY_COMMA              44  /* , */
#define AYIN_KEY_MINUS              45  /* - */
#define AYIN_KEY_PERIOD             46  /* . */
#define AYIN_KEY_SLASH              47  /* / */
#define AYIN_KEY_0                  48
#define AYIN_KEY_1                  49
#define AYIN_KEY_2                  50
#define AYIN_KEY_3                  51
#define AYIN_KEY_4                  52
#define AYIN_KEY_5                  53
#define AYIN_KEY_6                  54
#define AYIN_KEY_7                  55
#define AYIN_KEY_8                  56
#define AYIN_KEY_9                  57
#define AYIN_KEY_SEMICOLON          59  /* ; */
#define AYIN_KEY_EQUAL              61  /* = */
#define AYIN_KEY_A                  65
#define AYIN_KEY_B                  66
#define AYIN_KEY_C                  67
#define AYIN_KEY_D                  68
#define AYIN_KEY_E                  69
#define AYIN_KEY_F                  70
#define AYIN_KEY_G                  71
#define AYIN_KEY_H                  72
#define AYIN_KEY_I                  73
#define AYIN_KEY_J                  74
#define AYIN_KEY_K                  75
#define AYIN_KEY_L                  76
#define AYIN_KEY_M                  77
#define AYIN_KEY_N                  78
#define AYIN_KEY_O                  79
#define AYIN_KEY_P                  80
#define AYIN_KEY_Q                  81
#define AYIN_KEY_R                  82
#define AYIN_KEY_S                  83
#define AYIN_KEY_T                  84
#define AYIN_KEY_U                  85
#define AYIN_KEY_V                  86
#define AYIN_KEY_W                  87
#define AYIN_KEY_X                  88
#define AYIN_KEY_Y                  89
#define AYIN_KEY_Z                  90
#define AYIN_KEY_LEFT_BRACKET       91  /* [ */
#define AYIN_KEY_BACKSLASH          92  /* \ */
#define AYIN_KEY_RIGHT_BRACKET      93  /* ] */
#define AYIN_KEY_GRAVE_ACCENT       96  /* ` */
#define AYIN_KEY_WORLD_1            161 /* non-US #1 */
#define AYIN_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define AYIN_KEY_ESCAPE             256
#define AYIN_KEY_ENTER              257
#define AYIN_KEY_TAB                258
#define AYIN_KEY_BACKSPACE          259
#define AYIN_KEY_INSERT             260
#define AYIN_KEY_DELETE             261
#define AYIN_KEY_RIGHT              262
#define AYIN_KEY_LEFT               263
#define AYIN_KEY_DOWN               264
#define AYIN_KEY_UP                 265
#define AYIN_KEY_PAGE_UP            266
#define AYIN_KEY_PAGE_DOWN          267
#define AYIN_KEY_HOME               268
#define AYIN_KEY_END                269
#define AYIN_KEY_CAPS_LOCK          280
#define AYIN_KEY_SCROLL_LOCK        281
#define AYIN_KEY_NUM_LOCK           282
#define AYIN_KEY_PRINT_SCREEN       283
#define AYIN_KEY_PAUSE              284
#define AYIN_KEY_F1                 290
#define AYIN_KEY_F2                 291
#define AYIN_KEY_F3                 292
#define AYIN_KEY_F4                 293
#define AYIN_KEY_F5                 294
#define AYIN_KEY_F6                 295
#define AYIN_KEY_F7                 296
#define AYIN_KEY_F8                 297
#define AYIN_KEY_F9                 298
#define AYIN_KEY_F10                299
#define AYIN_KEY_F11                300
#define AYIN_KEY_F12                301
#define AYIN_KEY_F13                302
#define AYIN_KEY_F14                303
#define AYIN_KEY_F15                304
#define AYIN_KEY_F16                305
#define AYIN_KEY_F17                306
#define AYIN_KEY_F18                307
#define AYIN_KEY_F19                308
#define AYIN_KEY_F20                309
#define AYIN_KEY_F21                310
#define AYIN_KEY_F22                311
#define AYIN_KEY_F23                312
#define AYIN_KEY_F24                313
#define AYIN_KEY_F25                314
#define AYIN_KEY_KP_0               320
#define AYIN_KEY_KP_1               321
#define AYIN_KEY_KP_2               322
#define AYIN_KEY_KP_3               323
#define AYIN_KEY_KP_4               324
#define AYIN_KEY_KP_5               325
#define AYIN_KEY_KP_6               326
#define AYIN_KEY_KP_7               327
#define AYIN_KEY_KP_8               328
#define AYIN_KEY_KP_9               329
#define AYIN_KEY_KP_DECIMAL         330
#define AYIN_KEY_KP_DIVIDE          331
#define AYIN_KEY_KP_MULTIPLY        332
#define AYIN_KEY_KP_SUBTRACT        333
#define AYIN_KEY_KP_ADD             334
#define AYIN_KEY_KP_ENTER           335
#define AYIN_KEY_KP_EQUAL           336
#define AYIN_KEY_LEFT_SHIFT         340
#define AYIN_KEY_LEFT_CONTROL       341
#define AYIN_KEY_LEFT_ALT           342
#define AYIN_KEY_LEFT_SUPER         343
#define AYIN_KEY_RIGHT_SHIFT        344
#define AYIN_KEY_RIGHT_CONTROL      345
#define AYIN_KEY_RIGHT_ALT          346
#define AYIN_KEY_RIGHT_SUPER        347
#define AYIN_KEY_MENU               348
