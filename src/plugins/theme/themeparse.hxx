#ifndef THEMEPARSE_HXX
#define THEMEPARSE_HXX

#include <string>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/escape_char.hpp>
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <sigc++/sigc++.h>

#include "themetypes.hxx"
#include "guistyle.hxx"

typedef boost::shared_array<char>              pchar_shared;
typedef boost::spirit::phrase_scanner_t        scanner_t;
typedef boost::spirit::rule<scanner_t>         rule_t;

class GuiRC
{
    typedef GuiRC   self_t;
protected:
    GuiStyle*       style;


    bool            parse_rc_buffer(pchar_shared& buf);
    bool            parse_rc_file(std::string& conf_name);

    virtual void    new_style(const char* first, const char* last);
    virtual void    set_parent_style(const char* first, const char* last);
    virtual void    add_pixmap_path(const char* first, const char* last);
    virtual void    add_widget_pattern(GuiClassType t, const char* first, const char* last);
    virtual void    bind_class_to_style(const char* first, const char* last);
    virtual void    set_engine(const char* first, const char* last);
    virtual void    parse_engine_body(const char* first, const char* last);

public:
    GuiRC(std::string conf_name);
    virtual ~GuiRC() {};

    SigC::Signal1<void, GuiStyle*>   new_style_done;
    SigC::Signal1<void, GuiStyle*>   new_style_created;
};

using namespace boost;

class StylePtr
{
protected:
    GuiStyle          *style;
public:
    virtual ~StylePtr() {};
    virtual void       set_style(GuiStyle*);
};

class ColorsGramma: public spirit::grammar<ColorsGramma>, 
		    public SigC::Object,
		    public StylePtr
{
    typedef ColorsGramma   self_t;
protected:
    virtual void    set_color_array(GuiColorArray t);
    virtual void    set_color_type(GuiColorType t);
    virtual void    set_color_uint(unsigned int color);
    
public:
    virtual ~ColorsGramma() {};

    template <typename scanner_t>
    struct definition
    {
        definition(ColorsGramma const& self)
        {
	    self_t*  self_ptr=const_cast<self_t*>(&self);
	    
	    //fg/bg/base [NORMAL/.../]="#color" construction
	    rcolor_name=
		(str_p("NORMAL")[bind(&self_t::set_color_type, self_ptr, normal_color)] 
		 | str_p("ACTIVE")[bind(&self_t::set_color_type, self_ptr, active_color)] 
		 | str_p("PRELIGHT")[bind(&self_t::set_color_type, self_ptr, prelight_color)] 
		 | str_p("SELECTED")[bind(&self_t::set_color_type, self_ptr, selected_color)] 
		 | str_p("INSENSITIVE")[bind(&self_t::set_color_type, self_ptr, insensitive_color)]);

	    rcolor_def=confix_p('"', 
				lexeme_d[ch_p('#') >> hex_p[bind(&self_t::set_color_uint, self_ptr, _1)]] 
				,'"');
	    rcolor=confix_p('[',rcolor_name,']') >> '=' >> rcolor_def;

	    r_fg= str_p("fg")[bind(&self_t::set_color_array, self_ptr, fg_array)] >> rcolor;
	    r_bg= str_p("bg")[bind(&self_t::set_color_array, self_ptr, bg_array)] >> rcolor;
	    r_base= str_p("base")[bind(&self_t::set_color_array, self_ptr, base_array)] >> rcolor;
	    r_light= str_p("light")[bind(&self_t::set_color_array, self_ptr, light_array)] >> rcolor;
	    r_dark= str_p("dark")[bind(&self_t::set_color_array, self_ptr, dark_array)] >> rcolor;
	    r_mid= str_p("mid")[bind(&self_t::set_color_array, self_ptr, mid_array)] >> rcolor;
	    r_text= str_p("text")[bind(&self_t::set_color_array, self_ptr, text_array)] >> rcolor;

	    rmain=
		(
		 r_fg  
		 | r_bg 
		 | r_base 
		 | r_light 
		 | r_dark 
		 | r_mid 
		 | r_text 
		 );
        };

        rule_t r_fg, r_bg, r_light, r_dark, r_mid, r_text, r_base;
	rule_t rcolor, rcolor_def, rcolor_name;
	rule_t rmain;

        rule_t const&
        start() const { return rmain; }
    };

};

class BgPixmapGramma: public spirit::grammar<BgPixmapGramma>, 
		      public SigC::Object,
		      public StylePtr
{
    typedef BgPixmapGramma   self_t;
protected:
    virtual void    set_color_type(GuiColorType t);
    virtual void    set_pixmap_descr(const char* first, const char* last);
    
public:
    virtual ~BgPixmapGramma() {};

    template <typename scanner_t>
    struct definition
    {
        definition(self_t const& self)
        {
	    self_t*  self_ptr=const_cast<self_t*>(&self);
	    
	    //bg_pixmap [NORMAL/.../]="imagefile" construction
	    rname=
		(str_p("NORMAL")[bind(&self_t::set_color_type, self_ptr, normal_color)] 
		 | str_p("ACTIVE")[bind(&self_t::set_color_type, self_ptr, active_color)] 
		 | str_p("PRELIGHT")[bind(&self_t::set_color_type, self_ptr, prelight_color)] 
		 | str_p("SELECTED")[bind(&self_t::set_color_type, self_ptr, selected_color)] 
		 | str_p("INSENSITIVE")[bind(&self_t::set_color_type, self_ptr, insensitive_color)]);

	    rpixmap_def=confix_p('"', 
				lexeme_d[+(anychar_p - '"')][bind(&self_t::set_pixmap_descr, self_ptr, _1, _2)]
				,'"');
	    rpixmap=confix_p('[',rname,']') >> '=' >> rpixmap_def;

	    rmain= str_p("bg_pixmap") >> rpixmap;

        };

        rule_t rname;
	rule_t rpixmap, rpixmap_def;
	rule_t rmain;

        rule_t const&
        start() const { return rmain; };
    };
};

class PropertyGramma: public spirit::grammar<PropertyGramma>, 
		      public SigC::Object,
		      public StylePtr
{
    typedef PropertyGramma   self_t;
protected:
    virtual void    set_border(const char* first, const char* last);
    
public:
    PropertyGramma();
    virtual ~PropertyGramma() {};

    template <typename scanner_t>
    struct definition
    {
        definition(self_t const& self)
        {
	    self_t*  self_ptr=const_cast<self_t*>(&self);
	    
	    //xthickness = int_value
	    r_xthickness= str_p("xthickness") 
		>> '=' 
		>> uint_p[bind(&GuiStyle::set_xthickness, self_ptr->style, _1)];

	    //ythickness = int_value
	    r_ythickness= str_p("ythickness") 
		>> '=' 
	    	>> uint_p[bind(&GuiStyle::set_ythickness, self_ptr->style, _1)];

	    //any widget class like: NGI*:: (NGIButton:: , NGIWidget:: ...)
	    r_widclass=lexeme_d[str_p("NGI") >> +alnum_p] >> "::";

	    //NGI*::interior_focus = int_value
	    r_interior_focus= r_widclass >> str_p("interior_focus")
		>> '=' 
	    	>> uint_p[bind(&GuiStyle::set_interior_focus, self_ptr->style, _1)];

	    //NGI*::trough_border = int_value
	    r_trough_border= r_widclass >> str_p("trough_border")
		>> '=' 
	    	>> uint_p[bind(&GuiStyle::set_trough_border, self_ptr->style, _1)];

	    rmain= r_xthickness 
		| r_ythickness 
		| r_interior_focus
		| r_trough_border;

        };

	rule_t r_xthickness, r_ythickness, r_interior_focus;
	rule_t r_trough_border;
	rule_t rmain, r_widclass;

        rule_t const&
        start() const { return rmain; };
    };
};



#endif
