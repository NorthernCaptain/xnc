#include "themeparse.hxx"
#include "xncdebug.h"


using namespace std;
using namespace boost::spirit;

/////////////////////StylePtr////////////////////////////////
void StylePtr::set_style(GuiStyle *st)
{
    xncdprintf(("StylePtr::set_style %x\n", st));
    style=st;
}

/////////////////////ColorsGramma///////////////////////////

void ColorsGramma::set_color_array(GuiColorArray t)
{
    switch(t)
    {
    case fg_array:
	xncdprintf(("set color array: fg\n"));
	break;
    case bg_array:
	xncdprintf(("set color array: bg\n"));
	break;
    case base_array:
	xncdprintf(("set color array: base\n"));
	break;
    }
}

void ColorsGramma::set_color_type(GuiColorType t)
{
    xncdprintf(("set color type: %d\n", t));
}

void ColorsGramma::set_color_uint(unsigned int color)
{
    xncdprintf(("set color: %x\n", color));
}

//////////////////////BgPixmapGramma////////////////////////

void BgPixmapGramma::set_color_type(GuiColorType t)
{
    xncdprintf(("set color type: %d\n", t));
}

void BgPixmapGramma::set_pixmap_descr(const char* first, const char* last)
{
    string pixmap_name(first, last);
    xncdprintf(("load bg_pixmap: %s\n", pixmap_name.c_str()));
}

///////////////////////PropertyGramma/////////////////////////
PropertyGramma::PropertyGramma()
{
    xncdprintf(("PropertyGramma at %x\n", this));
}

void PropertyGramma::set_border(const char* first, const char* last)
{
    string pixmap_name(first, last);
    xncdprintf(("load bg_pixmap: %s\n", pixmap_name.c_str()));
}

/////////////////////////////GuiRC///////////////////////////

GuiRC::GuiRC(string conf_name)
{
    style=0;
    parse_rc_file(conf_name);
}

void GuiRC::new_style(const char* first, const char* last)
{
    string style_name(first, last);
    xncdprintf(("New style: %s\n", style_name.c_str()));
    style=new GuiStyle(style_name);
    new_style_created.emit(style);
}

void GuiRC::set_parent_style(const char* first, const char* last)
{
    string style_name(first, last);
    xncdprintf(("Set parent style to: %s\n", style_name.c_str()));
}

void GuiRC::add_pixmap_path(const char* first, const char* last)
{
    string path_name(first, last);
    xncdprintf(("Add pixmap path: %s\n", path_name.c_str()));
}

void GuiRC::add_widget_pattern(GuiClassType t, const char* first, const char* last)
{
    string class_name(first, last);
    switch(t)
    {
    case t_gui_class:
	xncdprintf(("class for style bind: %s\n", class_name.c_str()));
	break;
    case t_gui_widget_class:
	xncdprintf(("widget_class for style bind: %s\n", class_name.c_str()));
	break;
    case t_gui_widget:
	xncdprintf(("widget for style bind: %s\n", class_name.c_str()));
	break;
    }
}

void GuiRC::bind_class_to_style(const char* first, const char* last)
{
    string style_name(first, last);
    xncdprintf(("Binding style: %s\n", style_name.c_str()));    
}

void GuiRC::set_engine(const char* first, const char* last)
{
    string engine_name(first, last);
    xncdprintf(("set engine: %s\n", engine_name.c_str()));    
}

void GuiRC::parse_engine_body(const char* first, const char* last)
{
    string body(first, last);
    xncdprintf(("parse engine body: %s\n", body.c_str()));    
}

bool GuiRC::parse_rc_buffer(pchar_shared& buf)
{
    ColorsGramma g_colors;
    BgPixmapGramma g_bgpix;
    PropertyGramma g_prop;

    new_style_created.clear();
    new_style_created.connect(SigC::slot(g_colors, &ColorsGramma::set_style));
    new_style_created.connect(SigC::slot(g_bgpix, &BgPixmapGramma::set_style));
    new_style_created.connect(SigC::slot(g_prop, &PropertyGramma::set_style));

    //rule for comments '#'
    rule_t rcomment= *comment_p("#");

    rule_t rname= lexeme_d[+(c_escape_ch_p - '"')];
    rule_t rpattern= lexeme_d[+(anychar_p - '"')];
    rule_t rpath= lexeme_d[+(c_escape_ch_p - '"' - ':')];
    rule_t rbody= lexeme_d[+(anychar_p - '}')];

    //style name like "my_style" -> create new style
    rule_t r_style_name=confix_p('"',
				 rname[bind(&self_t::new_style, this, _1, _2)],
				 '"');

    //style parent name "my_parent" -> inherit style from parent
    rule_t r_style_parent_name=confix_p('"',
					rname[bind(&self_t::set_parent_style, this, _1, _2)],
					'"');

    //engine section inside style definition
    rule_t r_engine= str_p("engine") >> confix_p('"',
						rname[bind(&self_t::set_engine, this, _1, _2)],
						'"') >>
	confix_p('{', rbody[bind(&self_t::parse_engine_body, this, _1, _2)], '}');

    //style boby - everything inside { ... }
    rule_t rstylebody= +(
			 g_colors
			 | g_bgpix
			 | g_prop
			 | r_engine
			 );

    //all style structure: style "my_style" [= "my_parent"] { stylebody }
    rule_t rstyle= str_p("style") >> r_style_name >>  
	!(ch_p('=') >> r_style_parent_name) >> 
	 confix_p('{', !rstylebody, '}');

    //module_path - ignore it, we don't need it
    rule_t rmodule_path= str_p("module_path") >> confix_p('"', *c_escape_ch_p, '"');

    //pixmap_path - list of dirs separated by colon(:) -> add_pixmap_path
    rule_t rpixmap_path_body= 
	rpath[bind(&self_t::add_pixmap_path, this, _1, _2)] || ':';
    rule_t rpixmap_path= str_p("pixmap_path") >> 
	confix_p('"', +rpixmap_path_body, '"');

    //"class" type binding -> bind_class_to_style
    // class pattern [ style | binding [ : priority ]] name
    rule_t rclass_pattern= confix_p('"', 
				    rname
				    [bind(&self_t::add_widget_pattern, this, t_gui_class, _1, _2)], '"');
    
    rule_t rstyle_bind= confix_p('"', 
				 rname
				 [bind(&self_t::bind_class_to_style, this, _1, _2)], '"');

    rule_t rclass_bind= str_p("class") >> rclass_pattern >> 
	( (str_p("style") >> rstyle_bind) | 
	  //we ignore any "binding" keys here
	  ( (str_p("binding") || (':' >> int_p)) >> confix_p('"', rname, '"'))
	  );

    //"widget" type binding -> bind_class_to_style
    // widget pattern [ style | binding [ : priority ]] name
    rule_t rwidget_pattern= confix_p('"', 
				     rname
				     [bind(&self_t::add_widget_pattern, this, t_gui_widget, _1, _2)], '"');
    
    rule_t rwidget_bind= str_p("widget") >> rwidget_pattern >> 
	( (str_p("style") >> rstyle_bind) | 
	  //we ignore any "binding" keys here
	  ( (str_p("binding") || (':' >> int_p)) >> confix_p('"', rname, '"'))
	  );


    //"widget_class" type binding -> bind_class_to_style
    // widget_class pattern [ style | binding [ : priority ]] name
    rule_t rwidget_class_pattern= confix_p('"', 
				     rname
				     [bind(&self_t::add_widget_pattern, this, t_gui_widget_class, _1, _2)], '"');
    
    rule_t rwidget_class_bind= str_p("widget_class") >> rwidget_class_pattern >> 
	( (str_p("style") >> rstyle_bind) | 
	  //we ignore any "binding" keys here
	  ( (str_p("binding") || (':' >> int_p)) >> confix_p('"', rname, '"'))
	  );



    //main parsing rule
    rule_t rmain= +(rcomment >> (rstyle | 
				 rmodule_path | 
				 rpixmap_path | 
				 rclass_bind  |
				 rwidget_bind |
				 rwidget_class_bind
				 ));

    //Parsing process and results
    parse_info<> info= 
	boost::spirit::parse(
			     buf.get(),
			     rmain,
			     space_p
			     );
    if(!info.full)
    {
	xncdprintf(("ERROR loading RC: Parsing failed at: %s\n", info.stop));
	return false;
    }
    return true;
}

bool GuiRC::parse_rc_file(string& conf_name)
{
    int fsiz;
    FILE* fp=fopen(conf_name.c_str(), "rb");
    if(!fp)
    {
	xncdprintf(("Can't open RC file : %s\n", conf_name.c_str()));
	return false;
    }

    fseek(fp, 0, SEEK_END);
    fsiz = ftell(fp)+1;
    fseek(fp, 0, SEEK_SET);
    
    pchar_shared   buf(new char[fsiz]);
    
    fread(buf.get(), 1, fsiz, fp);
    fclose(fp);
    buf[fsiz-1]=0;
    for(int i=0;i<fsiz-3;i++)
	if(buf[i]=='G' && buf[i+1]=='t' && buf[i+2]=='k')
	{
	    buf[i]='N';
	    buf[i+1]='G';
	    buf[i+2]='I';
	}

    return parse_rc_buffer(buf);
}


int main(int argc, char** argv)
{
    GuiRC   rc(argv[1]);
    return 0;
}
