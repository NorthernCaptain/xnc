#ifndef GUISTYLE_HXX
#define GUISTYLE_HXX
#include <string>
#include "themetypes.hxx"

class GuiStyle
{
    std::string     name;
    unsigned int    colors[max_array][max_color];
    unsigned int    xthickness, ythickness;
    unsigned int    interior_focus, trough_border;
    
public:

    GuiStyle() { name="empty";};
    GuiStyle(std::string& iname);
    
    void     set_xthickness(unsigned int val);
    void     set_ythickness(unsigned int val);
    void     set_interior_focus(unsigned int val);
    void     set_trough_border(unsigned int val);
};


#endif
