/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
*  email:     leo@xnc.dubna.su
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
// $Id: config_window.cxx,v 1.13 2003/12/25 12:43:18 leo Exp $
#include "query_windows.h"
#include "panel.h"
#include "pluginman.h"
#include "internals.h"
#include "pager.h"
#include "iconsets.h"
#include "config_file.h"
#include "c_externs.h"


/******************Cofiguration windows*********************/
static Win *confw;
static Pager *confwp;
static KEY *confok,*confcan;
static Text *conft1,*conft2, *conft3, *conft4, *conft_mag,
    *mstate_text,*maction_text, *mbetween_text, 
    *mode_fmt_text, *mode_len_text, 
    *mode_head_t1, *mode_head_t2, 
    *mode_t1, *mode_t2, *mode_t3, 
    *mode_t4, *mode_t5, *mode_t6, *mode_t7,
    *prompt_text, *status_text, *plugin_text,
    *plugin_warn, *branch_text;
static Input *confin1, *confin_mag, *inmode_fmt, *inmode_len,
    *conf_indepth;
static Switch *confsw,*confsw1,*confsw2,*confsw3, *confsw4, 
       *confsw5, *confsw6, *confsw7, *confsw8, *confsw10,
       *confsw11, *confsw12, *confsw13, *confsw14, *confsw15, *confsw16,
       *confsw17, *confsw18, *confsw_pack, *confsw_lynx;
static Separator *confsep, *confsep1, *confsep2, *confsep3, *confsep4, *confsep5;
static Panel *mstate_panel, *maction_panel, *plugin_panel;
static Sw_panel *mode_sw, *iconset_sw;

static char **mstate_list=0, **maction_list=0;
static int  mstate_num, maction_num;

static char dirdelay_chr[12];
static char wm_cor_chr[12];
static char branch_depth_buf[15];

static char mode_len_buf[3][10]={"","",""};
static char mode_format_buf[3][32]={"F","F","F|O"};

void conf_mode_set();
void icon_mode_set();

static MenuItem modes_lst[] =
    {
	menu_none, _noop("Full mode"), 1, conf_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("Brief mode"), 1, conf_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("Custom mode"), 1, conf_mode_set, 0, 0, no_icon, AEmpty
    };


static MenuItem iconset_lst[] =
    {
	menu_none, _noop("XNC style icons"), 1, icon_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("Windows style icons"), 1, icon_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("KDE style icons"), 1, icon_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("AquaFusion icons"), 1, icon_mode_set, 0, 0, no_icon, AEmpty,
	menu_none, _noop("GNOME style icons"), 1, icon_mode_set, 0, 0, no_icon, AEmpty
    };
const int max_icon_styles=5;

const int max_plugins=3;
static char* plugin_titles[max_plugins]=
    { _noop("Original Look"),
      _noop("Five Look"),
      _noop("Aqua Look")
    };

static char* plugin_names[max_plugins]=
    { "none",
      "five",
      "aqua"
    };

void   show_hidden_files()
{
  panel->reread();
  if (panel->lay == 0)
    panel->panel2->reread();
}

void icon_mode_set()
{
  int i;
  for(i=0;i<max_icon_styles;i++)
    if(iconset_sw->get(i))
    {
      xncdprintf(("icon_mode_set[%d]\n",i));
    }
}

void icon_mode_apply()
{
    int i, val;
    for(i=0;i<max_icon_styles;i++)
    {
	if(iconset_sw->get(i))
	{
	    iconset_number=i;
	}
    }
}

void conf_mode_set()
{
  int i;
  for(i=0;i<3;i++)
    if(mode_sw->get(i))
    {
      xncdprintf(("conf_mode_set[%d]\n",i));
      inmode_fmt->setbuf(mode_format_buf[i], 20);
      inmode_fmt->expose();
      inmode_len->setbuf(mode_len_buf[i], 5);
      inmode_len->expose();
    }
}

void conf_mode_apply()
{
    int i, val;
    for(i=0;i<3;i++)
    {
	val=atoi(mode_len_buf[i]);
	val= val<=0 ? 16 : val;
	panel->set_display_max_file_len(PanelDisplayMode(i), val);
	panel->set_list_format(PanelDisplayMode(i), mode_format_buf[i]);
	panel->panel2->set_display_max_file_len(PanelDisplayMode(i), val);
	panel->panel2->set_list_format(PanelDisplayMode(i), mode_format_buf[i]);
    }
}


void conf_mode_init()
{
    int i;
    for(i=0;i<3;i++)
    {
	strcpy(mode_format_buf[i], panel->get_list_format(i));
	sprintf(mode_len_buf[i],"%d", panel->get_display_max_file_len(PanelDisplayMode(i)));
    }
}

void conf_can()
{
        confok->hide();
        confcan->hide();
        confwp->hide();
	confin1->hide();
	confin_mag->hide();
        confw->hide();
        
        delete confcan;
        delete confok;
        delete conft1;
        delete conft2;
        delete conft3;
        delete conft4;
        delete conft_mag;
        delete confsw;
        delete confsw1;
        delete confsw2;
        delete confsw3;
        delete confsw4;
        delete confsw5;
        delete confsw6;
        delete confsw7;
        delete confsw8;
        delete confsw10;
        delete confsw11;
        delete confsw12;
        delete confsw13;
        delete confsw14;
        delete confsw15;
        delete confsw16;
        delete confsw17;
        delete confsw18;
	delete confsw_lynx;
        delete confsep1;
        delete confsep2;
        delete confsep3;
        delete confin1;
	delete conf_indepth;
        delete confin_mag;
	delete mstate_text;
	delete mstate_panel;
	delete maction_text;
	delete mbetween_text;
	delete plugin_text;
	delete plugin_warn;
	delete plugin_panel;
	delete maction_panel;
	delete mode_sw;
	delete inmode_fmt;
	delete inmode_len;
	delete mode_fmt_text;
	delete mode_len_text;
	delete mode_head_t1;
	delete mode_head_t2;
	delete mode_t1;
	delete mode_t2;
	delete mode_t3;
	delete mode_t4;
	delete mode_t5;
	delete mode_t6;
	delete mode_t7;
	delete prompt_text;
	delete status_text;
	delete iconset_sw;
        delete confwp;
        delete confw;
	delete confsw_pack;
	delete branch_text;
        
	default_mouse_action_man->load_actions_from_file();
	set_current_iconset_style();
        guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

extern MenuBar *mn;
extern MenuBar *mn2;
void conf_ok()
{
        hide_hidden=!confsw->sw;
        allow_animation=confsw1->sw;
        allow_dir_icon=confsw2->sw;
        allow_file_icon=confsw3->sw;
        allow_t_raising=confsw10->sw;
        shadow=confsw11->sw;
	branch_depth=atoi(branch_depth_buf);

        if(confsw13->sw)
            option_bits|=FILLED_CURSOR;
        else
            option_bits&=~FILLED_CURSOR;

        if(confsw14->sw)
            option_bits|=STATUS_ATTR;
        else
            option_bits&=~STATUS_ATTR;

        if(confsw15->sw)
            option_bits|=STATUS_TIME;
        else
            option_bits&=~STATUS_TIME;

        if(confsw16->sw)
            option_bits|=STATUS_SIZE;
        else
            option_bits&=~STATUS_SIZE;

        if(confsw17->sw)
            option_bits|=STATUS_NAME;
        else
            option_bits&=~STATUS_NAME;

        if(confsw18->sw)
            option_bits|=STATUS_OWNER;
        else
            option_bits&=~STATUS_OWNER;

        if(confsw12->sw)
            option_bits|=IVES_EXIT_TOO;
        else
            option_bits&=~IVES_EXIT_TOO;

        if(confsw_lynx->sw)
            option_bits|=LYNX_NAVIGATION;
        else
            option_bits&=~LYNX_NAVIGATION;

        if(confsw4->sw)
                use_prompt|=PROMPT_COPY;
        else
                use_prompt&= ~PROMPT_COPY;
        if(confsw5->sw)
                use_prompt|=PROMPT_MOVE;
        else
                use_prompt&= ~PROMPT_MOVE;
        if(confsw6->sw)
                use_prompt|=PROMPT_DEL;
        else
                use_prompt&= ~PROMPT_DEL;
        if(confsw7->sw)
        {
                use_prompt|=PROMPT_OVR;
                dont_ask=NO;
         }
        else
        {
                use_prompt&= ~PROMPT_OVR;
                dont_ask=YES;
        }
        if(confsw8->sw)
                use_prompt|=PROMPT_EXIT;
        else
                use_prompt&= ~PROMPT_EXIT;
	if(confsw_pack->sw)
	    use_prompt|=PROMPT_PACK;
	else
	    use_prompt&= ~PROMPT_PACK;
        sscanf(dirdelay_chr,"%d",&dirdelay);
        sscanf(wm_cor_chr,"%d",&wm_y_correction);
	default_mouse_action_man->save_actions_to_file();
	conf_mode_apply();
	icon_mode_apply();
//        magic_init(magic_file_location, 0);
        panel->repanel();
        panel->panel2->repanel();
        conf_can();
        show_hidden_files();
        XClearWindow(disp,bmark->w);
        bmark->expose();
        mn->expose();
        mn2->expose();
        panel->redraw_statusbar();
        if(panel->lay==0)
            panel->panel2->redraw_statusbar();
}


void conf_animload()
{
          int t=allow_animation;
          allow_animation=1;
          dot_bar=1;
          bmark->free_pix();
          create_infowin(_("Wait..."));
          infowin->set_coun(9);
          to_infowin(_("Loading Pixmaps..."));
          bmark->load_flags(7);
          del_infowin();
          dot_bar=0;
          allow_animation=t;
}

void conf_dir_load()
{
          int t=allow_dir_icon;
          int t2=allow_file_icon;
          allow_dir_icon=1;
          allow_file_icon=1;
          dot_bar=1;
          free_panel_pixmaps();
          create_infowin(_("Wait..."));
          infowin->set_coun(6);
          to_infowin(_("Loading Pixmaps..."));
          load_pixmaps();
          del_infowin();
          dot_bar=0;
          confsw2->sw=allow_dir_icon;
          confsw2->expose();
          allow_dir_icon=t;
          allow_file_icon=t2;
}

void conf_file_load()
{
          int t=allow_file_icon;
          int t2=allow_dir_icon;
          allow_file_icon=1;
          allow_dir_icon=1;
          dot_bar=1;
          free_panel_pixmaps();
          create_infowin(_("Wait..."));
          infowin->set_coun(6);
          to_infowin(_("Loading Pixmaps..."));
          load_pixmaps();
          del_infowin();
          confsw3->sw=allow_file_icon;
          confsw3->expose();
          dot_bar=0;
          allow_file_icon=t;
          allow_dir_icon=t2;
}

int conf_mstate_move(int n, char* name)
{
    int action_idx;
    xncdprintf(("Mouse_state moved to %d:%s\n",n,name));
    action_idx=default_mouse_action_man->get_action_names_idx_by_mstate_name(name);
    maction_panel->cursor_to(action_idx);
    maction_panel->expose();
    return 1;
}

int conf_maction_move(int n, char* name)
{
    char* mstate_name=mstate_list[mstate_panel->get_current_index()];
    xncdprintf(("Mouse_action moved to %d:%s/%s\n",n,name,mstate_name));
    default_mouse_action_man->update_action_by_mstate_name(mstate_name,name);
    return 1;
}

int conf_plugin_move(int n, char* name)
{
    strcpy(guiplugin_name, plugin_names[n]);
    return 1;
}

void conf_plugin_init()
{
    int i;
    for(i=0;i<max_plugins;i++)
	if(strcmp(plugin_names[i], guiplugin_name)==0)
	{
	    plugin_panel->cursor_to(i);
	    return;
	}
}

void show_config_win()
{
      confw = guiplugin->new_Win(centerx - 250, centery - 160, 500, 345, _("XNC Configuration"), 5);
      confwp = guiplugin->new_Pager(10, 30, 475, 270, 5);

      conft1 = guiplugin->new_Text(10, 40, _("General:"), 5);
      conft2 = guiplugin->new_Text(260, 40, _("User prompts:"), 5);
      confsw = guiplugin->new_Switch(15, 50, 220, _("Show hidden files"), 1);
      confsep1= guiplugin->new_Separator(20, 80, 200, confwp);
      confsw1 = guiplugin->new_Switch(15, 90, 220, _("Bookmark animation"), 1,conf_animload);
      confsw12 = guiplugin->new_Switch(15, 72, 220, _("IVES exit with XNC"), 1, NULL);
      confsep2= guiplugin->new_Separator(20, 120, 200, confwp);
      confsw2 = guiplugin->new_Switch(15, 130, 220, _("Icons with directories"), 1, conf_dir_load);
      confsw3 = guiplugin->new_Switch(15, 160, 220, _("Icons with files"), 1, conf_file_load);

      status_text=guiplugin->new_Text(250, 50, _("Status bar view:"),5);
      confsw17 = guiplugin->new_Switch(265, 70, 200, _("File name in status"), 1);
      confsw14 = guiplugin->new_Switch(265, 100, 200, _("Attributes in status"), 1);
      confsw15 = guiplugin->new_Switch(265, 130, 200, _("Date/Time in status"), 1);
      confsw16 = guiplugin->new_Switch(265, 160, 200, _("Size in status"), 1);
      confsw18 = guiplugin->new_Switch(265, 190, 200, _("Owner in status"), 1);

      confsw13 = guiplugin->new_Switch(15, 200, 220, _("Filled file cursor"), 1, NULL);
      confsep3= guiplugin->new_Separator(20, 160, 200, confwp);
      confsw10 = guiplugin->new_Switch(15, 94, 220, _("Raise term on exec"), 1);
      confsw_lynx = guiplugin->new_Switch(15, 116, 220, _("Lynx style navigation"), 1);
      confsw11 = guiplugin->new_Switch(15, 50, 200, _("Shadow text"), 1);
      iconset_sw=guiplugin->new_Sw_panel(250, 60, 180, _("Choose iconset:"), 
					 iconset_lst, max_icon_styles);

      prompt_text=guiplugin->new_Text(15, 50, _("Prompts:"),5);
      confsw4 = guiplugin->new_Switch(25, 70, 200, _("Copy prompt"), 1);
      confsw5 = guiplugin->new_Switch(25, 100, 200, _("Move prompt"), 1);
      confsw6 = guiplugin->new_Switch(25, 130, 200, _("Delete prompt"), 1);
      confsw7 = guiplugin->new_Switch(25, 160, 200, _("Overwrite prompt"), 1);
      confsw_pack = guiplugin->new_Switch(25, 190, 200, _("Pack prompt"), 1);
      confsw8 = guiplugin->new_Switch(25, 220, 200, _("Exit prompt"), 1);

      conft3=guiplugin->new_Text(15, 180, _("Dirscan timeout:"),5);
      confin1=guiplugin->new_Input(170, 190, 40, 1, conf_ok);
      branch_text=guiplugin->new_Text(15, 230, _("Branch depth:"),5);
      conf_indepth=guiplugin->new_Input(170, 220, 40, 1, conf_ok);
      conft4=guiplugin->new_Text(220,205, _("sec"), 1);
      
      confok = guiplugin->new_KEY(20, -15, 130, 25, _("OK"), 1, conf_ok);
      confcan = guiplugin->new_KEY(-20, -15, 130, 25, _("Cancel"), 2, conf_can);
      
      
      conft_mag=guiplugin->new_Text(240, 60, _("Magic file:"),5);
      confin_mag=guiplugin->new_Input(250, 70, 200, 1, conf_ok);

      plugin_warn=guiplugin->new_Text(15, 255, _("GUI plugin will work only after restart of XNC."),5);
      plugin_text=guiplugin->new_Text(240, 120, _("GUI plugin (skin):"),5);
      plugin_panel=guiplugin->new_Panel(250,140,200,100,plugin_titles, max_plugins, 1, 0);

      mstate_text=guiplugin->new_Text(15, 50, _("Mouse state:"),5);
      mstate_list=default_mouse_action_man->get_mstate_name_list(mstate_num);
      mstate_panel=guiplugin->new_Panel(15,65,200,180,mstate_list, mstate_num, 1, 0);

      mbetween_text=guiplugin->new_Text(230, 150, "=>",5);

      maction_text=guiplugin->new_Text(260, 50, _("Action for the state:"),5);
      maction_list=default_mouse_action_man->get_action_name_list(maction_num);
      maction_panel=guiplugin->new_Panel(260,65,200,180,maction_list, maction_num, 1, 0);

      mode_sw=guiplugin->new_Sw_panel(15, 70, 135, _("Modes:"), modes_lst, 3);
      inmode_fmt=guiplugin->new_Input(300,190, 130, 1, conf_ok);
      inmode_len=guiplugin->new_Input(300,230, 130, 1, conf_ok);
      mode_fmt_text = guiplugin->new_Text(20, 205, _("Enter mode format string:"), 5);
      mode_len_text = guiplugin->new_Text(20, 245, _("Minimum display file name length:"), 5);
      mode_head_t1  = guiplugin->new_Text(15 , 50, _("Choose mode:"), 1);
      mode_head_t2  = guiplugin->new_Text(200, 50, _("Mode format explanation:"), 1);
      mode_t1       = guiplugin->new_Text(180, 75, _("F - file name"), 5);
      mode_t2       = guiplugin->new_Text(180, 90, _("A - file attributes (-rwxr-x-r-x)"), 5);
      mode_t3       = guiplugin->new_Text(180,105, _("O - file attributes in octal (0755)"), 5);
      mode_t4       = guiplugin->new_Text(180,120, _("S - file size"), 5);
      mode_t5       = guiplugin->new_Text(180,135, _("D - file date/time"), 5);
      mode_t6       = guiplugin->new_Text(180,150, _("| - visual separator"), 5);
      mode_t7       = guiplugin->new_Text(180,165, _(". - small empty space"), 5);

      confw->set_iname("CONFWIN");
      confok->set_iname("CONF_OK");
      confcan->set_iname("CONF_CAN");
      
      confw->init(Main);
      confwp->init(confw->w);
      
      confwp->setpage(0);
      confwp->setpagename(_("Main"));
      confwp->setpage(1);
      confwp->setpagename(_("Prompt/View"));
      confwp->setpage(2);
      confwp->setpagename(_("Look/Feel"));
      confwp->setpage(3);
      confwp->setpagename(_("Mouse"));
      confwp->setpage(4);
      confwp->setpagename(_("Modes"));
      
      confok->init(confw->w);
      confcan->init(confw->w);
      
      confsep1->init(confwp->w);
      confsep2->init(confwp->w);
      confsep3->init(confwp->w);
      confsw->init(confwp->w);
      confsw1->init(confwp->w);
      confsw2->init(confwp->w);
      confsw3->init(confwp->w);
      confsw_lynx->init(confwp->w);
      confsw10->init(confwp->w);
      confsw11->init(confwp->w);
      confsw13->init(confwp->w);
      confsw14->init(confwp->w);
      confsw15->init(confwp->w);
      confsw16->init(confwp->w);
      confsw17->init(confwp->w);
      confsw18->init(confwp->w);
      plugin_text->init(confwp->w);
      plugin_warn->init(confwp->w);
      status_text->init(confwp->w);
      prompt_text->init(confwp->w);
      conf_indepth->init(confwp->w);

      confsw4->init(confwp->w);
      confsw5->init(confwp->w);
      confsw6->init(confwp->w);
      confsw7->init(confwp->w);
      confsw8->init(confwp->w);
      confsw_pack->init(confwp->w);

      confsw12->init(confwp->w);

      conft1->init(confwp->w);
      conft2->init(confwp->w);
      conft3->init(confwp->w);
      conft4->init(confwp->w);
      branch_text->init(confwp->w);
      conft_mag->init(confwp->w);
      confin1->init(confwp->w);
      confin_mag->init(confwp->w);
      
      mstate_text->init(confwp->w);
      mbetween_text->init(confwp->w);
      mstate_panel->init(confwp->w);
      maction_text->init(confwp->w);
      maction_panel->init(confwp->w);
      plugin_panel->init(confwp->w);

      mode_sw->init(confwp->w);
      mode_sw->excl();
      mode_sw->set(0,1);
      iconset_sw->init(confwp->w);
      iconset_sw->excl();
      iconset_sw->set(iconset_number,1);
      inmode_fmt->init(confwp->w);
      inmode_len->init(confwp->w);
      mode_fmt_text->init(confwp->w);
      mode_len_text->init(confwp->w);
      mode_head_t1->init(confwp->w);
      mode_head_t2->init(confwp->w);
      mode_t1->init(confwp->w);
      mode_t2->init(confwp->w);
      mode_t3->init(confwp->w);
      mode_t4->init(confwp->w);
      mode_t5->init(confwp->w);
      mode_t6->init(confwp->w);
      mode_t7->init(confwp->w);

      confwp->setpage(0);
      confwp->pagemaxobj(15);
      confwp->addobj(confsw);
      confwp->addobj(confsw12);
      confwp->addobj(confsw10);
      confwp->addobj(confsw_lynx);
      confwp->addobj(conft3);
      confwp->addobj(conft4);
      confwp->addobj(branch_text);
      confwp->addobj(confin1);
      confwp->addobj(conf_indepth);
      confwp->addobj(conft_mag);
      confwp->addobj(confin_mag);
      confwp->addobj(confsep3);
      confwp->addobj(plugin_text);
      confwp->addobj(plugin_warn);
      confwp->addobj(plugin_panel);

      confsw->link(confsw12);
      confsw12->link(confsw10);
      confsw10->link(confsw_lynx);
      confsw_lynx->link(confin1);
      confin1->link(conf_indepth);
      conf_indepth->link(confin_mag);
      confin_mag->link(plugin_panel);
      plugin_panel->link(confsw);


      confwp->setpage(1);
      confwp->pagemaxobj(13);
      confwp->addobj(prompt_text);
      confwp->addobj(status_text);
      confwp->addobj(confsw4);
      confwp->addobj(confsw5);
      confwp->addobj(confsw6);
      confwp->addobj(confsw7);
      confwp->addobj(confsw_pack);
      confwp->addobj(confsw8);
      confwp->addobj(confsw14);
      confwp->addobj(confsw15);
      confwp->addobj(confsw16);
      confwp->addobj(confsw17);
      confwp->addobj(confsw18);
      
      confsw4->link(confsw5);
      confsw5->link(confsw6);
      confsw6->link(confsw7);
      confsw7->link(confsw_pack);
      confsw_pack->link(confsw8);
      confsw8->link(confsw17);
      confsw17->link(confsw14);
      confsw14->link(confsw15);
      confsw15->link(confsw16);
      confsw16->link(confsw18);

      confwp->setpage(2);
      confwp->pagemaxobj(9);
      confwp->addobj(confsw11);
      confwp->addobj(confsw1);
      confwp->addobj(confsw2);
      confwp->addobj(confsw3);
      confwp->addobj(confsw13);
      confwp->addobj(confsep1);
      confwp->addobj(confsep2);
      confwp->addobj(confsep3);
      confwp->addobj(iconset_sw);

      confsw11->link(confsw1);
      confsw1->link(confsw2);
      confsw2->link(confsw3);
      confsw3->link(confsw13);
      confsw13->link(iconset_sw);
      iconset_sw->link(confsw11);

      confwp->setpage(3);
      confwp->pagemaxobj(5);
      confwp->addobj(mstate_text);
      confwp->addobj(mstate_panel);
      confwp->addobj(maction_text);
      confwp->addobj(mbetween_text);
      confwp->addobj(maction_panel);

      mstate_panel->link(maction_panel);
      maction_panel->link(mstate_panel);
      mstate_panel->set_every_move_to(conf_mstate_move);
      maction_panel->set_every_move_to(conf_maction_move);
      plugin_panel->set_every_move_to(conf_plugin_move);

      confwp->setpage(4);
      confwp->pagemaxobj(14);
      confwp->addobj(mode_sw);
      confwp->addobj(inmode_fmt);
      confwp->addobj(inmode_len);
      confwp->addobj(mode_fmt_text);
      confwp->addobj(mode_len_text);
      confwp->addobj(mode_head_t1);
      confwp->addobj(mode_head_t2);
      confwp->addobj(mode_t1);
      confwp->addobj(mode_t2);
      confwp->addobj(mode_t3);
      confwp->addobj(mode_t4);
      confwp->addobj(mode_t5);
      confwp->addobj(mode_t6);
      confwp->addobj(mode_t7);

      conf_mode_init();
      conf_plugin_init();

      inmode_fmt->setbuf(mode_format_buf[0], 20);
      inmode_len->setbuf(mode_len_buf[0], 5);
      
      confwp->setpage(0);

      confok->link(confcan);
      confcan->link(confok);
      
      confsw->sw=!hide_hidden;
      confsw1->sw=allow_animation;
      confsw2->sw=allow_dir_icon;
      confsw3->sw=allow_file_icon;
      confsw13->sw=(option_bits & FILLED_CURSOR) ? YES : NO;
      confsw12->sw=(option_bits & IVES_EXIT_TOO) ? YES : NO;
      confsw14->sw=(option_bits & STATUS_ATTR) ? YES : NO;
      confsw15->sw=(option_bits & STATUS_TIME) ? YES : NO;
      confsw16->sw=(option_bits & STATUS_SIZE) ? YES : NO;
      confsw17->sw=(option_bits & STATUS_NAME) ? YES : NO;
      confsw18->sw=(option_bits & STATUS_OWNER) ? YES : NO;
      confsw_lynx->sw=(option_bits & LYNX_NAVIGATION) ? YES : NO;
      
      confsw4->sw=(use_prompt & PROMPT_COPY) ? YES : NO;
      confsw5->sw=(use_prompt & PROMPT_MOVE) ? YES : NO;
      confsw6->sw=(use_prompt & PROMPT_DEL) ? YES : NO;
      confsw7->sw=(use_prompt & PROMPT_OVR) ? YES : NO;
      confsw8->sw=(use_prompt & PROMPT_EXIT) ? YES : NO;
      confsw_pack->sw=(use_prompt & PROMPT_PACK) ? YES : NO;
      
      confsw10->sw=allow_t_raising;
      confsw11->sw=shadow;
      
      confw->add_toexpose(confsep);

      confin1->set_escapefunc(conf_can);
      mstate_panel->escfunc(conf_can);
      maction_panel->escfunc(conf_can);
      plugin_panel->escfunc(conf_can);

      confin1->set_histid("DIRTIM");
      sprintf(dirdelay_chr,"%d",dirdelay);
      confin1->setbuf(dirdelay_chr, 10);

      confin_mag->set_histid("MAGICF");
      confin_mag->setbuf(magic_file_location, 255);
      sprintf(branch_depth_buf, "%d", branch_depth);
      conf_indepth->setbuf(branch_depth_buf, 5);

      confw->show();
      confok->show();
      confcan->show();
      confwp->show();
      
      reset_fast_loading();
}     









