#ifndef C_EXTERNS_H
#define C_EXTERNS_H

extern "C" {

    void     clean_exit(); 
    void     map_vt_windows(Pixmap);
    int      magic_init(char*, int);
    void     process_x_event(XEvent *);
    int      is_show_intro();
    int      is_look_five();
    int      psystem(char *);
    int      is_fast_load();
    void     tt_printf(char *fmt,...);
    void     tt_write(unsigned char *buf, int count);
    int      psystem2(char *str,...);
    char*    magic_process(char*, struct stat*);
    void     process_X();
    void     process_whole_sequence_event();
    void     reset_fast_loading();
    void     unset_back_load();
    int      is_back_load();
    int      is_shared_load();
    int      is_term_on();
    void     resize_window();
}

extern   const char * rs_plugin_name;
extern void (*xnc_run_once)();
extern void (*xnc_switch_term)();
extern   int xnc_now_on_top;
extern   int vt_show;
#endif

/* ------------ End of file -------------- */

