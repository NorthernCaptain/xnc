#ifndef MOUSE_ACTIONS
#define MOUSE_ACTIONS

#include "xheaders.h"
#include "x_actions.h"

enum MouseState //define combination of mouse buttons and Control/Shift mods
    {
	mButtonNone,

	mButton1,
	mButton2,
	mButton3,
	mButton4,
	mButton5,
	mButton1Double,
	mButton1Ctrl,
	mButton2Ctrl,
	mButton3Ctrl,
	mButton4Ctrl,
	mButton5Ctrl,
	mButton1Shift,
	mButton2Shift,
	mButton3Shift,
	mButton4Shift,
	mButton5Shift,
	mButton1Motion,
	mButton2Motion,
	mButton3Motion,
	mButton1CtrlMotion,
	mButton2CtrlMotion,
	mButton3CtrlMotion,
	mButton1ShiftMotion,
	mButton2ShiftMotion,
	mButton3ShiftMotion
    };

enum MouseClickType
    {
	OneClick,
	DoubleClick,
	TrippleClick
    };

const int max_mouse_active_states=24;

const int L_MAX_MOUSE_STATE_NAME=32;	

//Linking MouseState/Name <-> XEvent
struct MouseItem
{
    MouseState        mstate;
    char              mname[L_MAX_MOUSE_STATE_NAME];
    unsigned int      x_state;
    unsigned int      x_button;
    MouseClickType    click;
};

//Linking Action <-> MouseState
struct MouseActions
{
    MouseState        mstate;
    int               action;
};

const int L_MAX_ACTION_NAME=50;
const int MAX_TOTAL_ACTIONS=256;

//Action name <-> action
struct ActionNames
{
    int               action;
    char              action_name[L_MAX_ACTION_NAME];
};

//structure that transfers data between methods
struct MouseData
{
    int            action;
    XEvent*        xev;
    MouseClickType click_type;
    MouseData(XEvent& ev, MouseClickType iclick=OneClick)
    {
	xev= &ev;
	click_type=iclick;
	action=AEmpty;
    }
};



//MouseActionManager is a class that will do all convertions between
//states, x11 events and names, load/save actions to file etc...
class MouseActionManager
{
 protected:
    static MouseItem     mouse_motion_items[];
    static MouseItem     mouse_press_items[];
    static MouseActions  default_actions[];
    static ActionNames   action_names[];

    MouseActions         real_actions[max_mouse_active_states];
    char*                mstate_name_list[max_mouse_active_states];
    char*                maction_name_list[MAX_TOTAL_ACTIONS];
    int                  max_actions;

    void                 set_default_actions();
    void                 clear_real_actions();
    MouseState           get_mouse_press_state_by_x_state(unsigned int button,
							  unsigned int state,
							  MouseClickType click);
    MouseState           get_mouse_motion_state_by_x_state(unsigned int state);
    int                  get_action_by_mstate(MouseState mstate);
    void                 get_actions_file_name(char* name_to_return);
    MouseState           get_mstate_by_name(char* name);
    char*                get_mstate_name_by_state(MouseState mstate);
    int                  get_action_by_name(char* name);
    char*                get_action_name_by_action(int action);

 public:
    MouseActionManager();
    int                  get_action_by_mouse_data(MouseData& data);
    int                  load_actions_from_file();
    int                  save_actions_to_file();

    int                  get_action_names_idx_by_mstate_name(char*);
    int                  update_action_by_mstate_name(char* mstate_name, char* action_name);
    char**               get_mstate_name_list(int& max_num) 
	{ 
	    max_num=max_mouse_active_states;
	    return mstate_name_list;
	};
    char**               get_action_name_list(int& max_num) 
	{ 
	    max_num=max_actions;
	    return maction_name_list;
	};
};

extern MouseActionManager *default_mouse_action_man;

#endif
