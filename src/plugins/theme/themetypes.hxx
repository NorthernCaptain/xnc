#ifndef THEMETYPES_HXX
#define THEMETYPES_HXX

enum GuiClassType 
    { 
	t_gui_class=1, 
	t_gui_widget=2, 
	t_gui_widget_class=3
    };
enum GuiColorType 
    { 
	normal_color=0, 
	active_color=1, 
	prelight_color=2, 
	selected_color=3,
	insensitive_color=4,
	max_color=5
    };

enum GuiColorArray
    {
	fg_array=0,
	bg_array=1,
	base_array=2,
	light_array=3,
	dark_array=4,
	mid_array=5,
	text_array=6,
	max_array=7
    };

#endif
