#include "guistyle.hxx"
#include "xncdebug.h"

GuiStyle::GuiStyle(std::string& iname)
{
    name=iname;
    xncdprintf(("GuiStyle(%s) - %s %x\n", iname.c_str(), 
		name.c_str(),this));
}


void GuiStyle::set_xthickness(unsigned int val)
{
    xthickness=val;
    xncdprintf(("style: '%s' %x set xthickness=%d\n", 
		name.c_str(), this, val));
}

void GuiStyle::set_ythickness(unsigned int val)
{
    ythickness=val;
    xncdprintf(("style: '%s' %x set ythickness=%d\n", 
		name.c_str(), this, val));
}

void GuiStyle::set_interior_focus(unsigned int val)
{
    interior_focus=val;
    xncdprintf(("style: '%s' %x set interior_focus=%d\n", 
		name.c_str(), this, val));
}

void GuiStyle::set_trough_border(unsigned int val)
{
    trough_border=val;
    xncdprintf(("style: '%s' %x set trough_border=%d\n", 
		name.c_str(), this, val));
}

