#ifndef METACALLER_H
#define METACALLER_H

#include "globals.h"

#define L_MAXDATA  L_MAXPATH

class Gui;

struct MetaCallData
{
    Gui  *caller;
    MetaCallData(Gui* icaller=0)
    {
	caller=icaller;
    };
};

class MetaCaller
{
 protected:
    Gui      *object;
    int      (Gui::*method)(MetaCallData);
 public:
    MetaCaller(Gui *iobj, int (Gui::*imeth)(MetaCallData))
	{
	    object=iobj;
	    method=imeth;
	};

    virtual int call(MetaCallData dat) 
	{
	    if(!object)
		return 0;
	    return (object->*method)(dat);
	};
};

class Lister;

class ListerPtrCaller: public MetaCaller
{
 protected:
    Lister     **lister_object;
    int        (Lister::*lister_method)(MetaCallData);

 public:
    ListerPtrCaller(Lister **iobj, int (Lister::*imeth)(MetaCallData)) :
	MetaCaller(0,0)
	{
	    lister_object=iobj;
	    lister_method=imeth;
	};

    virtual int call(MetaCallData dat) 
	{
	    if(!lister_object || !(*lister_object))
		return 0;
	    return ((*lister_object)->*lister_method)(dat);
	};
    
};

class ListerVoidCaller: public MetaCaller
{
 protected:
    Lister     **lister_object;
    void        (Lister::*lister_method)();

 public:
    ListerVoidCaller(Lister **iobj, void (Lister::*imeth)()) :
	MetaCaller(0,0)
	{
	    lister_object=iobj;
	    lister_method=imeth;
	};

    virtual int call(MetaCallData dat) 
	{
	    if(!lister_object || !(*lister_object))
		return 0;
	    ((*lister_object)->*lister_method)();
	    return 1;
	};
    
};

class ListerPtrCallerChar: public MetaCaller
{
 protected:
    Lister     **lister_object;
    int        (Lister::*lister_method)(const char*);
    char       data[L_MAXDATA];
 public:
    ListerPtrCallerChar(Lister **iobj, int (Lister::*imeth)(const char*), 
			const char* pdata) :
	MetaCaller(0,0)
	{
	    lister_object=iobj;
	    lister_method=imeth;
	    strcpy(data, pdata);
	};

    virtual int call(MetaCallData dat) 
	{
	    if(!lister_object || !(*lister_object))
		return 0;
	    return ((*lister_object)->*lister_method)(data);
	};

    void set_data(const char* pdata) { strcpy(data, pdata); };
    char* get_data() { return data; };
    
};

class Cmdline;

class CmdlineCaller: public MetaCaller
{
 protected:
    Cmdline     *cmdobject;
    int         (Cmdline::*cmdmethod)(int, MetaCallData);
    int         value;
 public:
    CmdlineCaller():MetaCaller(0,0) { cmdobject=0; };
    CmdlineCaller(Cmdline* ptrobj, int (Cmdline::*cmdmeth)(int, MetaCallData), int val=0):
	MetaCaller(0,0)
	{
	    init(ptrobj,cmdmeth,val);
	};

    void init(Cmdline* ptrobj, int (Cmdline::*cmdmeth)(int, MetaCallData), int val=0)
	{
	    cmdobject=ptrobj;
	    cmdmethod=cmdmeth;
	    value=val;
	};

    virtual int call(MetaCallData dat) 
	{
	    if(!cmdobject)
		return 0;
	    return (cmdobject->*cmdmethod)(value,dat);
	};
};

#endif
