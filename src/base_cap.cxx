/****************************************************************************
*  Copyright (C) 2000 by Leo Khramov
*  email:     leo@xnc.dubna.su
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
  ****************************************************************************/
/* $Id: base_cap.cxx,v 1.12 2003/11/27 15:36:43 leo Exp $ */
#include "base_cap.h"
#include "ftpfs.h"
#include "ftpcfg.h"
#include "systemfuncs.h"
#include "commonfuncs.h"
#include "c_externs.h"

int    disable_reread = 0;        //If 1 reread disables in ****::expose;
int    dirdelay=1;                //Timeout in seconds for rereading directory contens
int    compare_by_size=YES;       //Use sizecompare for directory comparision
int    compare_by_time=NO;        //Use timecompare for directory comparision



//--------------------------------------------------------------------------
//Search entry in the file list by name and return index from start of list
//--------------------------------------------------------------------------
int    BaseCaptain::findentry(char *nme)
{
	FList *fo = dl.next;
	int    i = 0;
	while (fo != NULL)
		if (strcmp(fo->name, nme) == 0)
			break;
		else
		{
			fo = fo->next;
			i++;
		};
	if (fo != NULL)
		return i;
	return -1;
}

//--------------------------------------------------------------------------
//Adds subdirectory to path (curdir)
//example  was:/usr/local cont=bin will:/usr/local/bin
//--------------------------------------------------------------------------
void   BaseCaptain::addpathcontent(char *cont)
{
	if (curdir[strlen(curdir) - 1] != '/')
		strcat(curdir, "/");
	strcat(curdir, cont);
}

//--------------------------------------------------------------------------
//Make curdir show upper dir (cd ..)
//example was: /usr/local/bin will:/usr/local
//--------------------------------------------------------------------------
void   BaseCaptain::upperpath()
{
	char  *bb = strrchr(curdir, '/');
	*bb = 0;
	if (strlen(curdir) == 0)
		strcpy(curdir, "/");
}

//--------------------------------------------------------------------------
//sets cursor to position dd against current position *cr
//--------------------------------------------------------------------------
int    BaseCaptain::setcur(FList ** cr, int dd)
{
    FList *fo = *cr;
    if (dd > 0)
	while (fo->next != NULL && dd > 0)
	{
	    fo = fo->next;
	    dd--;
	}
    else
	while (fo->prev != &dl && dd < 0)
	{
	    fo = fo->prev;
	    dd++;
	}
    if (dd == 0)
    {
	*cr = fo;
	return 1;
    }
    return 0;
}

//--------------------------------------------------------------------------
//sets current cursor to absolute position or to the nearest if exact one 
//doesn't exist.
//--------------------------------------------------------------------------
void    BaseCaptain::set_cursor_to_nearest(int pos)
{
    int count=pos;
    FList *fo = dl.next;
    while (fo->next != 0 && count > 0)
    {
	fo = fo->next;
	count--;
    }
    cur=fo;
    curn=pos - count;
    base=dl.next;
    setcur(&base, curn - (curn % items_per_page));
    curn%=items_per_page;
}

//--------------------------------------------------------------------------
// Read directory listing using current *vfs
// If vfs now in background mode then pop up previous one
//--------------------------------------------------------------------------
int    BaseCaptain::direc(char *dir_ch)
{
	FList *o,*o2;
	compare = comp;
	int    ret,coun;
	sort_order=rev_order ^ 1;

	xncdprintf(("BaseCap direc with vfs died bit %d\n",vfs->options & VFSDIED_BIT)); 
	if(vfs->options & VFSDIED_BIT)
	{
		vfs=pop_vfs();
		vfs->ch_curdir();
	}

	if(is_branch_view_mode())
	    ret = vfs->branch_direc(dir_ch, &dir_cache_man);
	else
	    ret = vfs->direc(dir_ch);
	if (ret)
	{
		if(vfs->bgbit)        //If VFS became BG then POP previous VFS
		{
			vfs=pop_vfs();
			vfs->ch_curdir();
			ret=vfs->direc(vfs->curdir);
			if(ret==0) 
				return ret;
		}
        /*      lar = vfs->lar;
	    	    larmax = vfs->larmax;
        */
		dl.next = vfs->dl.next;
		fl.next = vfs->fl.next;


		if (strcmp(dl.next->name, ".") == 0)
			dl.next = dl.next->next;
		dl.next->prev = &dl;
		cur = &dl;
		coun=0;
		while (cur->next != NULL)
		{
			cur = cur->next;
			coun++;
		}
		cur->next = fl.next;
		if (fl.next != NULL)
			fl.next->prev = cur;
		cur=fl.next;
		while(cur)
		{
			cur=cur->next;
			coun++;
		}
		base = cur = dl.next;
		lcurn = curn = 0;
		selfiles = selsize = 0;
		time(&lastscan);
        //    scr->setpages(coun/(th*col));
        //    scr->setrange(0, coun);
        //    scr->expose();
	}
	return ret;
}

//--------------------------------------------------------------------------
//Select given file for future packet operations
//--------------------------------------------------------------------------
void   BaseCaptain::selected(FList * fo)
{
	if(strcmp(fo->name,"..")==0)
		return;
	fo->options ^= S_SELECT;
	if (fo->options & S_SELECT)
	{
		selfiles++;
		selsize += fo->size;
	}
	else
	{
		selfiles--;
		selsize -= fo->size;
	}
}

//--------------------------------------------------------------------------
//Set attibutes to selected files
//--------------------------------------------------------------------------
void   BaseCaptain::setattrs(int scf,int atrib)
{
	FList *ko;
	ko = dl.next;
	vfs->ch_curdir();
	while (ko != NULL)
	{
		if (ko->options & S_SELECT)
		{
			if (scf)
				ko->mode |= atrib;
			else
				ko->mode &= atrib;
			if(ko->get_dir())
			    vfs->chdir(ko->get_dir());
			if (vfs->chmod(ko->name, ko->mode) >= 0)
			{
				ko->options ^= S_SELECT;
				selfiles--;
				selsize -= ko->size;
			}
		};
		ko = ko->next;
	}
	vfs->ch_curdir();
}


//--------------------------------------------------------------------------
//Parse given string, detect vfs type and break down path+file to parts
//--------------------------------------------------------------------------
struct VFS_Type_Name
{
	int  vfs_type;
	char vfs_name[16];
};

const VFS_Type_Name vfs_names[]=
{
	{ FTP_TYPE, "ftp://" },
	{ FTP_TYPE, "FTP:" },
	{ FTP_TYPE, "file:/#ftp:" },
	{ AFS_TYPE, "tar:" },
	{ DFS_TYPE, "file:" }
};

const int max_vfs_names=5;

VFS_Info* BaseCaptain::get_vfs_info_for_file(char* fbuf)
{
	char copybuf[L_MAXPATH];
	char *s,*s1=copybuf;
	int i;
	VFS_Info* vfsi=new VFS_Info;
	strncpy(copybuf,fbuf,L_MAXPATH);
    //First of all check for GMC #utar and others
	if((s=strstr(copybuf,"#u"))!=0)
	{
		vfsi->vfstype=AFS_TYPE;
		*s=0;
		strcpy(vfsi->full_path,s1+5); //skipping 'file:' prefix
		s++;
		while(*s!='/' && *s!=0) s++;
		get_last_and_rest_content(s,vfsi->filename,vfsi->full_path+strlen(vfsi->full_path));
	} else
	{

		for(i=0;i<max_vfs_names;i++)
		{
			if(strncmp(vfs_names[i].vfs_name,fbuf,
						strlen(vfs_names[i].vfs_name))==0)
				break;
		}
		if(i>=max_vfs_names) //Can't find any type so set to VFS_TYPE
		{
			vfsi->vfstype=VFS_TYPE;
			get_last_and_rest_content(fbuf,vfsi->filename,vfsi->full_path);
		} else
		{
			vfsi->vfstype=vfs_names[i].vfs_type;
			get_last_and_rest_content(fbuf+strlen(vfs_names[i].vfs_name),
				vfsi->filename,vfsi->full_path);    
		}
	}
	xncdprintf(("vfsi: [%d][%s]->[%s]\n",
			vfsi->vfstype,
			vfsi->full_path,
			vfsi->filename));
	return vfsi;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//                            VFS stack operations
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//--------------------------------------------------------------------------
//Pushes current VFS *v on top of stack
//--------------------------------------------------------------------------
void   BaseCaptain::push_vfs(VFS * v)
{
	VFS   *o;
	DFS   *df,*df2=(DFS*)v;
	FTP   *f;
	switch (v->fstype)
	{
		case DFS_TYPE:
			df = new DFS;
			memcpy(df, v, sizeof(DFS));
			df->curdir=new char[df2->dlen];
			strcpy(df->curdir,v->curdir);
			o=df;
			break;
		case AFS_TYPE:
			o = new AFS;
			memcpy(o, v, sizeof(AFS));
			o->set_dir_pointer(NULL,0);
			strcpy(o->curdir,v->curdir);
			break;
		case FTP_TYPE:
			o = new FTP;
			memcpy(o, v, sizeof(FTP));
			f=(FTP*)o;
			f->ftp_to_bg((FTP*)v);
			break;
		default:
			return;
	};
	o->next = vfstack;
	vfstack = o;
}

//--------------------------------------------------------------------------
// Pop up vfs from stack
//--------------------------------------------------------------------------
VFS*   BaseCaptain::pop_vfs()
{
	char   *dir;
	VFS   *o = vfstack;
	if (o == NULL)
		return &dfs;
	vfstack = o->next;
	o->need_reread = 1;
	switch (o->fstype)
	{
		case DFS_TYPE:
			dir=dfs.curdir;
			memcpy(&dfs, o, sizeof(DFS));
			dfs.curdir=dir;
			strcpy(dfs.curdir,o->curdir);
			delete o->curdir;
			delete o;
			return &dfs;
		case AFS_TYPE:
			dir=afs.curdir;
			memcpy(&afs, o, sizeof(AFS));
			afs.curdir=dir;
			if(dir==NULL)
				afs.set_dir_pointer(NULL,0);
			strcpy(afs.curdir,o->curdir);
			delete o->curdir;
			delete o;
			return &afs;
		case FTP_TYPE:
			if(is_this_ftp((FTP*)o)==0)
			{
				xncdprintf(("Lister::pop_vfs() FTP Object not in list\n"));
				return pop_vfs();
			}
			memcpy(&ftp, o, sizeof(FTP));
			ftp.ftp_to_bg((FTP*)o);
			delete o;
			return &ftp;

	};
	delete o;
	return &dfs;
}

//--------------------------------------------------------------------------
// Pop up previous vfs, reread and show directory
//--------------------------------------------------------------------------
void BaseCaptain::switch_to_prev_vfs()
{
	delete_vfs_list();
	vfs=pop_vfs();
    // *INDENT-OFF*        
	::chdir(curdir);
    // *INDENT-ON*        
	vfs->ch_curdir();
	if (direc(curdir) == 0)
	{
		vfs_error("VFS", "Can't change to dir");
		show_vfs_error();
		vfs->chdir("..");
		vfs->getcwd(curdir, 1020);
		direc(curdir);
	}
	showdir();
}

//--------------------------------------------------------------------------
// Pushing current vfs, then switch to given
// If we switching to DFS, then don't push prev vfs -> clean all stack.
//--------------------------------------------------------------------------
void   BaseCaptain::push_n_switch(int vfs_type, char *dir)
{
	char realdir[L_MAXPATH],realfile[L_MAXPATH],rest[L_MAXPATH];
	char  *name, *b;
	delete_vfs_list();
	switch (vfs_type)
	{
		case DFS_TYPE:
			while (vfstack != NULL)
				pop_vfs();
			vfs = &dfs;
			vfs->init_support(0);
			reset_cur();
			strcpy(curdir, dir);
			vfs->chdir(curdir);
			senddir(curdir);
			reread();
			expose_panel();
			break;
		case AFS_TYPE:
			push_vfs(vfs);
			reset_cur();
			vfs = &afs;
			break_to_real_dirfile(dir,realdir, realfile, rest);
			xncdprintf(("[%s] broken to [%s][%s][%s]\n",dir,realdir,realfile,rest));
			::chdir(realdir);
        // *INDENT-ON*        
			if (afs.init_support(realfile,rest) == 0)        //If error we return to prev VFS
			{
				show_vfs_error();
				vfs = pop_vfs();
			}
        // *INDENT-OFF*        
			::chdir(curdir);
        // *INDENT-ON*        
			vfs->ch_curdir();
			direc(".");
			expose_panel();
			break;
		case FTP_TYPE:
			push_vfs(vfs);
			if(ftprec.init(dir))
				do_ftp_link();
			else
				do_simple_dialog("Enter your password",FFTPCONPWD,0);
			break;
	};
}

//--------------------------------------------------------------------------
// Pop up vfses from stack, cleanup all stack, then switch
//--------------------------------------------------------------------------
void   BaseCaptain::switch_to_vfs(int vfs_type, char *dir)
{
	char  *name, *b;
	delete_vfs_list();
	if (vfs->fstype != DFS_TYPE)
		while (vfstack != NULL)
			pop_vfs();
	vfs = &dfs;
	switch (vfs_type)
	{
		case DFS_TYPE:
			vfs = &dfs;
			vfs->init_support(0);
			reset_cur();
			strcpy(curdir, dir);
			vfs->chdir(curdir);
			senddir(curdir);
			reread();
			expose_panel();
			break;
		case AFS_TYPE:
			delete_vfs_list();
			reset_cur();
			push_vfs(vfs);
			vfs = &afs;
			b = strrchr(dir, '/');
			*b = 0;
        // *INDENT-OFF*        
			if(*dir==0) 
				::chdir("/");
			else 
				::chdir(dir);
        // *INDENT-ON*        
			*b = '/';
			name = b + 1;
			if (afs.init_support(name) == 0)        //If error we return to prev VFS
			{
				show_vfs_error();
				vfs = pop_vfs();
			}
        // *INDENT-OFF*        
			::chdir(curdir);
        // *INDENT-ON*        
			vfs->ch_curdir();
			direc(".");
			expose_panel();
			break;
		case FTP_TYPE:
			if(ftprec.init(dir))
				do_ftp_link();
			else
				do_simple_dialog(_("Enter your password"),FFTPCONPWD,0);
			break;
	};
}

//--------------------------------------------------------------------------
//Push current VFS then switch to given FTP with its init.
//--------------------------------------------------------------------------
VFS* BaseCaptain::push_n_pop(VFS *p)
{
	delete_vfs_list();
	push_vfs(vfs);
	switch(p->fstype)
	{
		case FTP_TYPE:
			vfs = &ftp;
			ftp=*(FTP*)p;
			ftp.init_internals();
			vfs->ch_curdir();
			if (direc(curdir) == 0)
			{
				vfs_error("FTP", _("Can't change to dir"));
				show_vfs_error();
				vfs->chdir("..");
				vfs->getcwd(curdir, 1020);
				direc(curdir);
			}
			break;
		default:
			xncdprintf(("WARNING: push_n_pop unsupported VFS type!!!"));
			vfs=pop_vfs();
	}
	total_reread();
	return vfs;
}

//--------------------------------------------------------------------------
// Check current vfs and given one then if equal pop another one and show it
//--------------------------------------------------------------------------
void BaseCaptain::check_and_pop(VFS *vf)
{
	int res;
	switch(vf->fstype)
	{
		case DFS_TYPE:
			res=memcmp(vf,vfs,sizeof(DFS));
			break;
		case AFS_TYPE:
			res=memcmp(vf,vfs,sizeof(AFS));
			break;
		case FTP_TYPE:
			res=memcmp(vf,vfs,sizeof(FTP));
			break;
		default:
			res=1;
	}
	if(res==0)
		pop_and_show();
}

//--------------------------------------------------------------------------
//Pop vfs from vfs_stack and then show directory listing.
//--------------------------------------------------------------------------
void BaseCaptain::pop_and_show()
{
	vfs=pop_vfs();
    // *INDENT-OFF*        
	::chdir(curdir);
    // *INDENT-ON*        
	vfs->ch_curdir();
	if (direc(curdir) == 0)
	{
		vfs_error(vfs->vfs_prefix, _("Can't change to dir"));
		show_vfs_error();
		vfs->chdir("..");
		vfs->getcwd(curdir, 1020);
		direc(curdir);
	}
	showdir();
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//Cleaning directory listing
void BaseCaptain::delete_vfs_list()
{
	vfs->delete_vfs_list();
	dl.next=fl.next=0;
	lar=0;
	larmax=0;
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Sorting functions
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void   BaseCaptain::sortbyext()
{
	comp = extcmp;
	reread();
}

void   BaseCaptain::sortbysize()
{
	comp = sizecmp;
	reread();
}

void   BaseCaptain::sortbytime()
{
	comp = timecmp;
	reread();
}

void   BaseCaptain::sortbyname()
{
	comp = mystrcmp;
	reread();
}

void   BaseCaptain::sortbyunsort()
{
	comp = unsortcmp;
	reread();
}


//Clear fast find buffer
void   BaseCaptain::clear_ff()
{
	ff_buf[0] = '\0';
	last_ff = NULL;
}



//--------------------------------------------------------------------------
//Reread current directory, then calls virtual method for showing list of files
//--------------------------------------------------------------------------
void   BaseCaptain::reread()
{
	int    kn, kk, sel_fl = 0;
	if (selfiles)
	{
		sel_fl = 1;
		save_select();
	};
	if (cur)
		strcpy(refindchr, cur->name);
	else
		refindchr[0] = 0;
	if (vfs->ch_curdir() == -1)
		vfs->ch_to_rootdir();
	dl.next = fl.next = NULL;
	direc(curdir);
	if (refindchr[0] != 0)
	{
		if (!am_i_visible())
			curn = lcurn;
		kn = findentry(refindchr);
		if (kn != -1)
		{
			curn = kn % items_per_page;
			kk = kn - curn;
			setcur(&base, kk);
			setcur(&cur, kn);
		}
	}
	if (sel_fl)
		restore_select();
	if (!am_i_visible())
	{
		lcurn = curn;
		curn = -1;
		if (lay == 0)
			showdir();
	}
	else
		showdir();
	xncdprintf(("Reread dir: %s\n",curdir));
}


//--------------------------------------------------------------------------
// Save selected files to special buffer
//--------------------------------------------------------------------------
void   BaseCaptain::save_select()
{
	FList *o=dl.next;
	int coun=0;
	xncdprintf(("BaseCaptain::save_select() %x->save_select1(%d)=%x\n",this, selfiles, asel));
	if(asel)
		delete asel;
	asel=NULL;
	if(selfiles==0)
		return;
	asel = new FList[selfiles];
	aselmax = 0;
	while(o)
	{
		if (o->options & S_SELECT)
			asel[aselmax++].init(o->name, o->mode, 0, 0, 0, 0);
		o=o->next;
		coun++;
	}
	xncdprintf(("BaseCaptain::save_select() %x->save_select2(%d/%d)=%x\n",this, selfiles, coun, asel));
}

//--------------------------------------------------------------------------
// Restore saved selections to current listing
//--------------------------------------------------------------------------
void   BaseCaptain::restore_select()
{
	FList *o;
	int coun=0;
	selsize = selfiles = 0;
	if(asel==NULL)
		return;
	for (int i = 0; i < aselmax; i++)
	{
		o=dl.next;
		while(o)
		{
			if (strcmp(o->name, asel[i].name) == 0)
			{
				o->options |= S_SELECT;
				selsize += o->size;
				selfiles++;
				break;
			}
			o=o->next;
			coun++;
		}
	}
	xncdprintf(("BaseCaptain::restore_select() %x->restore_select(%d/%d/%d)=%x...",this, selfiles, coun, aselmax, asel));
	delete asel;
	asel=NULL;
	xncdprintf(("...done\n"));
}


//--------------------------------------------------------------------------
// Clean selections of current listing
//--------------------------------------------------------------------------
void   BaseCaptain::clean_selection()
{
	FList *o=dl.next;
	int coun=0;
	selsize = selfiles = 0;
	while(o)
	{
		o->options&= ~S_SELECT;
		o=o->next;
	}
}

//--------------------------------------------------------------------------
// Find item in list by filename, sets it to current
//--------------------------------------------------------------------------
void   BaseCaptain::set_by_name(char *findchr)
{
	int    kn, kk;
	kn = findentry(findchr);
	if (kn != -1)
	{
		curn = kn % items_per_page;
		kk = kn - curn;
		setcur(&base, kk);
		setcur(&cur, kn);
	}
}

//--------------------------------------------------------------------------
// Change current directory to newdir
//--------------------------------------------------------------------------
void   BaseCaptain::change_dir(char *newdir)
{
	char   findchr[250];
	int    findfl = 0, kn, kk;
	masks|=PANEL_CD_DONE;
	vfs->ch_curdir();
	if (vfs->chdir(newdir) == 0)
	{
		if (strcmp(newdir, "..") == 0)
		{
			findfl = 1;
			strcpy(findchr, strrchr(curdir, '/') + 1);
		};
		vfs->getcwd(curdir, 1020);
		dl.next = fl.next = NULL;
		direc(curdir);
		if (findfl != 0 && findchr[0] != 0)
		{
			kn = findentry(findchr);
			if (kn != -1)
			{
				curn = kn % items_per_page;
				kk = kn - curn;
				setcur(&base, kk);
				setcur(&cur, kn);
			}
		}
		showdir();
	};
}

//--------------------------------------------------------------------------
// Select files from dir-list by mask
//--------------------------------------------------------------------------
void   BaseCaptain::select_by_mask(int sel)
{
	strcpy(maskchr, dbuf);
	FList *o = dl.next->next;
	int coun=0;
	char *fname=NULL;
	while (o != NULL)
	{
		if (is_filtered(o->name, maskchr))
		{
			if (sel)
			{
				if ((o->options & S_SELECT) == 0)
				{
					o->options |= S_SELECT;
					selfiles++;
					coun++;
					selsize += o->size;
					if(!fname)
						fname=o->name;
				};
			}
			else if (o->options & S_SELECT)
			{
				o->options ^= S_SELECT;
				selfiles--;
				coun++;
				selsize -= o->size;
			}
		}
		o = o->next;
	}
	if (selfiles < 0)
	{
		selfiles = 0;
		selsize = 0;
	}
	if(coun==0)
		simple_mes(_("Information"),_("No files match given criteria."));
	else
	{
		showdir();
		if(fname)
			step_to_name(fname);
	}
}


//--------------------------------------------------------------------------
// Select given file
//--------------------------------------------------------------------------
void BaseCaptain::select_file(FList* o)
{
	o->options|=S_SELECT;
	selfiles++;
	selsize+=o->size;
}

//--------------------------------------------------------------------------
// Find file entry by name
//--------------------------------------------------------------------------
FList *BaseCaptain::find_file_entry(char *name)
{
	FList *o=dl.next;
	while(o)
	{
		if(strcmp(o->name,name)==0)
			return o;
		o=o->next;
	}
	return NULL;
}

//--------------------------------------------------------------------------
// Extract extension from file and set it to mask
//--------------------------------------------------------------------------
void   BaseCaptain::select_file_mask(int sel)
{
	char *name=cur->name;
	int l=strlen(name);
	if(name[l-1]=='.')
		return;
	char *b=strrchr(name,'.');
	if(b==NULL)
	{
		simple_mes(_("Sorry!"), _("Current FileName has no extension!"));
		return;
	}
	sprintf(dbuf,"*%s",b);
	select_by_mask(sel);
}

//--------------------------------------------------------------------------
//Search item in list by name and make it current
//--------------------------------------------------------------------------
void   BaseCaptain::step_to_name(char *name)
{
	FList *o = dl.next;
	int    kn = 0, kk=0;
	while (o)
		if (strcmp(o->name, name) == 0)
			break;
		else
		{
			o = o->next;
			kn++;
		};
	if (o != NULL)
	{
		xncdprintf(("Stepping - ok (to 0x%x)\n",o));
		base = dl.next;
		cur = o;
		lcurn = curn = kn % items_per_page;
		kk = kn - curn;
		setcur(&base, kk);
		showfilelist();
		last_ff = o;
	} else
	{
		xncdprintf(("Stepping - FAIL (was %s)\n",name));
	}
}

//--------------------------------------------------------------------------
//
//               Global FS operations (copy/move/delete)...
//
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Delete file of selected files from current VFS, show progress bar
// and error messages if any
//--------------------------------------------------------------------------
void   BaseCaptain::fdelete()
{
    FList *pcur, *ko;
    VFS *pvfs=vfs;
    int    ferr = 0;
    disable_reread = 1;
    total_expose();
    set_recycle_state(0);
    if (vfs->is_del_supported() == 0)
    {
	simple_mes(_("Error"), _("Can't DELETE on this VFS"));
	return;
    }
    create_infowin(_("Deleting..."));
    if (selfiles == 0 || vfs->can_we_optimize_del())
    {
	if(selfiles)
	    vfs->use_optimization();
	to_infowin(cur->name);
	if (vfs->remove(cur))
	{
	    del_infowin();
	    fserror(_("Delete file"), cur);
	}
	else
	    if(selfiles==0 && pvfs->bgbit==0) {
		if (cur->next)
		    cur->next->prev = cur->prev;
		cur->prev->next = cur->next;
		pcur = cur->next;
		ko = cur->prev;
		if (pcur)
		    cur = pcur;
		else
		{
		    cur = ko;
		    curn--;
		};
		del_infowin();
		showdir();
	    };
	if(pvfs->bgbit)
	    reread();
    }
    else
    {
	ko = dl.next;
	infowin_set_coun(selfiles);
	while (ko != NULL)
	{
	    if (ko->options & S_SELECT)
	    {
		to_infowin(ko->name);
		if (vfs->remove(ko))
		{
		    ko = ko->next;
		    ferr = 1;
		}
		else                // fserror("Delete Files");
		{
		    pcur = ko->next;
		    if (ko->next)
			ko->next->prev = ko->prev;
		    ko->prev->next = ko->next;
		    selfiles--;
		    selsize -= ko->size;
		    ko = pcur;
		    infowin_update_coun(1);
		}
	    }
	    else
		ko = ko->next;
	}
	del_infowin();
	if (ferr)
	{
	    ko = dl.next;
	    while (ko != NULL)
		if (ko->options & S_SELECT)
		    break;
		else
		    ko = ko->next;
	    fserror(_("Delete files"), ko);
	}
	set_cursor_to_nearest(curn);
	showdir();
    }
    disable_reread = 0;
}

//--------------------------------------------------------------------------
// Copy file of selected files from current VFS, show progress bar
// and error messages if any
// Return 1 on succes, and 0 on any error
//--------------------------------------------------------------------------
int   BaseCaptain::fcopy()
{
  FList *ko,*so;
  int    rcode = 0;
  VFS   *vfstmp,*pvfs;
  disable_reread = 1;
  total_expose();
  if (vfs->curdir[0] != '/')
    vfs->make_fullpath();
  if (selfiles != 0 && vfs->fstype == AFS_TYPE && vfs->is_full_extract_allowed())
  {
    if (vfs->check_for_full_extract(selfiles))
    {
      vfstmp = define_vfs(dbuf);
      afstmp_defined = 0;
      if (vfstmp == NULL)
      {
	show_vfs_error();
	return 0;
      }
      create_infowin(_("Copying..."));
      to_infowin(_("Full Unpack"));

      rcode = vfs->full_extract_to_vfs(vfstmp);

      del_infowin();
      if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
      {
	vfstmp->delete_vfs_list();
	vfstmp->close_fs();
      }
      if (rcode != 0 && ow_cancel == 0)
	show_vfs_error();
      else
      {
	selfiles = 0;
	selsize = 0;
      };
      flush_screen();
      disable_reread = 0;
      total_reread();
      vfs->ch_curdir();
      ow_all = 0;
      ow_cancel = ow_no = 0;
      return rcode==0;
    }
  }

  vfstmp = define_vfs(dbuf);
  afstmp_defined = 0;
  if (vfstmp == NULL)
  {
    show_vfs_error();
    return 0;
  }
  if (vfstmp->is_copy_supported() == 0)
  {
    simple_mes(_("Error"), _("Can't COPY on this VFS"));
    return 0;
  }
  create_infowin(_("Copying..."));

  if (selfiles == 0 || vfs->can_we_optimize() ||
      (vfstmp->fstype == FTP_TYPE && vfstmp->can_we_optimize()) ||
      (vfstmp->fstype == AFS_TYPE && vfstmp->is_pocket_addition_allowed()))
  {
    if (selfiles == 0)
      to_infowin(cur->name);
    else
      to_infowin(_("Pocket operation"));
    if (selfiles)
      vfs->use_optimization();
    pvfs=vfs;
    rcode = vfs->copy(cur, vfstmp);
    so=NULL;
    if(rcode==0 && pvfs->bgbit)        //Are we in background?
    {
      selfiles=selsize=0;                //Yes! So some cleanups.
    } else        
      if (selfiles)
      {
	ko = dl.next;
	selfiles = 0;
	selsize = 0;
	while (ko != NULL)
	{
	  if (ko->options & S_SELECT)
	  {
	    selfiles++;
	    selsize += ko->size;
	    so=ko;
	  }
	  ko = ko->next;
	}
      }
    del_infowin();
    if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
    {
      vfstmp->delete_vfs_list();
      vfstmp->close_fs();
    }
    if (rcode != 0 && ow_cancel == 0)
      fserror(_("Copy file"), so ? so : cur);
    flush_screen();
  }
  else
  {
    ko = dl.next;
    infowin_set_coun(selsize);
    while (ko != NULL)
    {
      if (ko->options & S_SELECT)
      {
	to_infowin(ko->name);
	ow_no = 0;
	rcode = vfs->copy(ko, vfstmp) ? 1 : 0;
	if (rcode && ow_cancel)
	  break;
	if (rcode == 0 && ow_no == 0)
	{
	  ko->options ^= S_SELECT;
	  selfiles--;
	  selsize -= ko->size;
	  infowin_update_coun(ko->size);
	  show_item(ko);
	  flush_screen();
	}
	showfinfo(ko,selfiles);
      };
      ko = ko->next;
    }
    del_infowin();
    if (rcode && ow_cancel == 0)
    {
      ko = dl.next;
      while (ko != NULL)
	if (ko->options & S_SELECT)
	  break;
	else
	  ko = ko->next;
      fserror(_("Copy files"), ko);
    }
    if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
    {
      vfstmp->delete_vfs_list();
      vfstmp->close_fs();
    }
  }
  disable_reread = 0;
  total_reread();
  vfs->ch_curdir();
  ow_all = 0;
  ow_cancel = ow_no = 0;
  return rcode==0;
}


//--------------------------------------------------------------------------
// Copy file of selected files from current VFS, show progress bar
// and error messages if any
//--------------------------------------------------------------------------
void   BaseCaptain::fmove()
{
	FList *ko;
	int    ferr = 0;
	int    rcode;
	int    old_curn=curn;
	VFS   *vfstmp;
	disable_reread = 1;
	total_expose();
	if (vfs->curdir[0] != '/')
		vfs->make_fullpath();
	create_infowin(_("Moving..."));
	if (selfiles == 0)
	{
		to_infowin(cur->name);
		vfstmp = define_vfs(dbuf);
		afstmp_defined = 0;
		if (vfstmp == NULL)
		{
			del_infowin();
			show_vfs_error();
			return;
		}
		if ((vfstmp->is_move_supported()) == 0)
		{
			del_infowin();
			simple_mes(_("Error"), _("Can't MOVE on this VFS"));
			return;
		}
		rcode = vfs->move(cur, vfstmp);
		if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
		{
			vfstmp->delete_vfs_list();
			vfstmp->close_fs();
		}
		if (rcode && ow_cancel == 0)
		{
			del_infowin();
			fserror(_("Move file"), cur);
		}
		else
			del_infowin();
	}
	else
	{
		ko = dl.next;
		vfstmp = define_vfs(dbuf);
		afstmp_defined = 0;
		if (vfstmp == NULL)
		{
			del_infowin();
			show_vfs_error();
			return;
		}
		if ((vfstmp->is_move_supported()) == 0)
		{
			del_infowin();
			simple_mes(_("Error"), _("Can't MOVE on this VFS"));
			return;
		}
		infowin_set_coun(selsize);
		while (ko != NULL)
		{
			if (ko->options & S_SELECT)
			{
				to_infowin(ko->name);
				ow_no = 0;
				ferr = vfs->move(ko, vfstmp) ? 1 : 0;
				if (ferr && ow_cancel)
					break;
				if (ferr == 0 && ow_no == 0)
				{
					ko->options ^= S_SELECT;
					selfiles--;
					selsize -= ko->size;
					show_item(ko);
					showfinfo(ko, selfiles);
					infowin_update_coun(ko->size);
					flush_screen();
				};
			};
			ko = ko->next;
		}
		del_infowin();
		if (ferr && ow_cancel == 0)
		{
			ko = dl.next;
			while (ko != NULL)
				if (ko->options & S_SELECT)
					break;
				else
					ko = ko->next;
			fserror(_("Move files"), ko);
		}
		if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
		{
			vfstmp->delete_vfs_list();
			vfstmp->close_fs();
		}
	}
	flush_screen();
	disable_reread = 0;
	total_reread();
	set_cursor_to_nearest(old_curn);
	showdir();
	vfs->ch_curdir();
	ow_cancel = ow_no = ow_all = 0;

}

//--------------------------------------------------------------------------
// Making directory on current VFS
//--------------------------------------------------------------------------
void   BaseCaptain::fmkdir()
{
	VFS *pvfs=vfs;
	FList *o = new FList(dbuf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	if (vfs->mkdir(dbuf, o->mode) == 0)
	{
		if(pvfs->bgbit==0)
		{
			strcpy(cur->name, dbuf);
			vfs->need_reread=1;
			vfs->options|=REALREAD_BIT;
			disable_reread=0;
		}
		reread();
	}
	else
		fserror("Make directory", o);
	delete o;
}


//--------------------------------------------------------------------------
// Link file or selected files from current VFS, show progress bar
// and error messages if any
// Return 1 on succes, and 0 on any error
//--------------------------------------------------------------------------
int   BaseCaptain::flink()
{
  FList *ko,*so;
  int    rcode = 0;
  VFS   *vfstmp,*pvfs;
  disable_reread = 1;
  total_expose();
  if (vfs->curdir[0] != '/')
    vfs->make_fullpath();

  vfstmp = define_vfs(dbuf);
  afstmp_defined = 0;
  if (vfstmp == NULL)
  {
    show_vfs_error();
    return 0;
  }
  if (vfstmp->is_symlink_supported() == 0)
  {
    simple_mes(_("Error"), _("Can't LINK on this VFS"));
    return 0;
  }
  create_infowin(_("Linking..."));

  if (selfiles == 0 || vfs->can_we_optimize())
  {
    if (selfiles == 0)
      to_infowin(cur->name);
    else
      to_infowin(_("Pocket operation"));
    if (selfiles)
      vfs->use_optimization();
    pvfs=vfs;
    rcode = vfs->symlink(cur, vfstmp);
    so=NULL;
    if(rcode==0 && pvfs->bgbit)        //Are we in background?
    {
      selfiles=selsize=0;                //Yes! So some cleanups.
    } else        
      if (selfiles)
      {
	ko = dl.next;
	selfiles = 0;
	selsize = 0;
	while (ko != NULL)
	{
	  if (ko->options & S_SELECT)
	  {
	    selfiles++;
	    selsize += ko->size;
	    so=ko;
	  }
	  ko = ko->next;
	}
      }
    del_infowin();
    if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
    {
      vfstmp->delete_vfs_list();
      vfstmp->close_fs();
    }
    if (rcode != 0 && ow_cancel == 0)
      fserror(_("Link file"), so ? so : cur);
    flush_screen();
  }
  else
  {
    ko = dl.next;
    infowin_set_coun(selsize);
    while (ko != NULL)
    {
      if (ko->options & S_SELECT)
      {
	to_infowin(ko->name);
	ow_no = 0;
	rcode = vfs->symlink(ko, vfstmp) ? 1 : 0;
	if (rcode && ow_cancel)
	  break;
	if (rcode == 0 && ow_no == 0)
	{
	  ko->options ^= S_SELECT;
	  selfiles--;
	  selsize -= ko->size;
	  infowin_update_coun(ko->size);
	  show_item(ko);
	  flush_screen();
	}
	showfinfo(ko,selfiles);
      };
      ko = ko->next;
    }
    del_infowin();
    if (rcode && ow_cancel == 0)
    {
      ko = dl.next;
      while (ko != NULL)
	if (ko->options & S_SELECT)
	  break;
	else
	  ko = ko->next;
      fserror(_("Link files"), ko);
    }
    if (!is_panel_vfs(vfstmp) && vfstmp->bgbit==0)
    {
      vfstmp->delete_vfs_list();
      vfstmp->close_fs();
    }
  }
  disable_reread = 0;
  total_reread();
  vfs->ch_curdir();
  ow_all = 0;
  ow_cancel = ow_no = 0;
  return rcode==0;
}




//--------------------------------------------------------------------------
// Packing files with archiver
// return 1 on success
//--------------------------------------------------------------------------
int   BaseCaptain::pack_files(const char* arc_name)
{
  char tmpstr[L_MAXPATH];
  char only_name[L_MAXPATH];

  AFS  *to_vfs;
  SUP  *sp; 
  int  ret;
  int  files=selfiles;

  strcpy(tmpstr,arc_name);
  get_last_content(tmpstr, only_name);
  sp = find_support(only_name);
  if(!sp)
  {
      simple_mes(_("Error"), _("No archiver association with your file!"));
      return 0;
  }

  if(files==0 && !sp->is_create_supported())
  {
      simple_mes(_("Error"), _("Archiver don't support archive creation!"));
      return 0;
  }

  if(files!=0 && !sp->is_many_create_supported())
  {
      simple_mes(_("Error"), _("Archiver don't support archive creation!"));
      return 0;
  }

  if(files==0) //Only one current file need to be packed
  {
      create_infowin(_("Packing..."));
      to_infowin(_("Creating archive"));

      to_vfs=(AFS*)define_vfs("afs",tmpstr);
      afstmp_defined=0;
      ret=to_vfs->create_archive_from_files(cur->name, 1);
      to_vfs->delete_vfs_list();
      vfs->ch_curdir();
      
      del_infowin();
      if(!ret)
	  show_vfs_error();
      return ret;
  }

  //Packing many files at ones

  create_xnc_tmp_dir(dbuf);

  xncdprintf(("PACK_FILES: Copy to tmp: %s\n", dbuf));

  if(!fcopy())
  {
    delete_xnc_tmp_dir(dbuf);
    return 0;  //There was error during copy to tmpdir
  }

  to_vfs=(AFS*)define_vfs("afs",tmpstr);
  afstmp_defined=0;

  if(::chdir(dbuf)==-1)
  {
    vfs_error("AFS", _("Can't access TMP dir"));
    return 0;
  }

  create_infowin(_("Packing..."));
  to_infowin(_("Creating archive"));

  ret=to_vfs->create_archive_from_files(".", 0);
  to_vfs->delete_vfs_list();
  vfs->ch_curdir();

  del_infowin();
  if(!ret)
    show_vfs_error();
  return ret;
}

//--------------------------------------------------------------------------
// Trying reread current directory and show listing
//--------------------------------------------------------------------------
void   BaseCaptain::try_reread_dir()
{
	time_t tt;
	if (disable_reread == 0)
	{
		xncdprintf(("Basecap try to reread dir with scan %d\n",scanfl));
		if (scanfl)
		{
			if (vfs->ch_curdir() == -1)
				vfs->ch_to_rootdir();
			direc(curdir);
			scanfl = 0;
			if (!am_i_visible())
				curn = -1;
			showdir();
		}
		else
		{
			if(vfs->need_reread)
			{
				reread();
				if(lay == 0 && other_vfs()->need_reread)
					other()->reread();
			} else
			{
				time(&tt);
				if (tt < lastscan + dirdelay)
					showdir();
				else
				{
					total_reread();
				}
			}
		}
	}
	else
		showdir();
}

//--------------------------------------------------------------------------
// Change owners on current or selected files
//--------------------------------------------------------------------------
void   BaseCaptain::chowns()
{
	int    i = 0;
	char  *gr = NULL;
	int    r=0, ui, gi;
	FList *o, *oo=NULL;
	while (dbuf[i] != 0 && dbuf[i] != '.')
		i++;
	if (dbuf[i] == '.')
	{
		dbuf[i] = 0;
		gr = dbuf + i + 1;
	};
	ui = finduserid_by_name(dbuf);
	if (gr != NULL)
		gi = findgroupid_by_name(gr);
	else
		gi = -1;
	if (selfiles == 0)
	{
	    if(cur->get_dir())
		vfs->chdir(cur->get_dir());
	    r = vfs->chown(cur->name, ui, gi);
	    oo = cur;
	}
	else
	{
	    o = dl.next;
	    while (o != NULL)
	    {
		if (o->options & S_SELECT)
		{
		    if(o->get_dir())
			vfs->chdir(o->get_dir());
		    else
			vfs->ch_curdir();
		    if (vfs->chown(o->name, ui, gi) == -1)
		    {
			r |= -1;
			oo = o;
		    }
		    else
		    {
			o->options ^= S_SELECT;
			selfiles--;
			selsize -= o->size;
		    }
		}
		o = o->next;
	    }
	}
	if (r == -1)
	{
		errno2mes();
		fserror(_("Change Owner"), oo);
	}
}

//--------------------------------------------------------------------------
// Parse file though MAGIC database (man file :)
//--------------------------------------------------------------------------
void   BaseCaptain::do_quick_file()
{
	if(!cur)
	    return;
	if(cur->get_dir())
	    vfs->chdir(cur->get_dir());
	if(cur->magic[0]==0)
		vfs->magic_file_process(cur);
	if(cur->magic[0])
		show_string_info(cur->magic);
	vfs->ch_curdir();
}


//--------------------------------------------------------------------------
// Show file listing
//--------------------------------------------------------------------------
void   BaseCaptain::showfilelist()
{
    FList *fo = base;
    int    i = 0;
    while (fo != NULL && i < items_per_page)
    {
	showitem(fo, i);
	fo = fo->next;
	i++;
    }
    while (i < items_per_page)
    {
	showempty(i);
	i++;
    };
}

//--------------------------------------------------------------------------
// Move cursor to one line down
//--------------------------------------------------------------------------
void   BaseCaptain::go_downline()
{
	clear_ff();
	if (cur->next)
	{
		curn++;
		cur = cur->next;
		if (curn >= items_per_page)
		{
			curn--;
			base = base->next;
			showfilelist();
		}
		else
		{
			showitem(cur->prev, curn - 1);
			showitem(cur, curn);
		}
	}
}

//--------------------------------------------------------------------------
// Move cursor to one line up
//--------------------------------------------------------------------------
void   BaseCaptain::go_upline()
{
	clear_ff();
	if (cur->prev != &dl)
	{
		curn--;
		cur = cur->prev;
		if (curn < 0)
		{
			curn = 0;
			base = base->prev;
			showfilelist();
		}
		else
		{
			showitem(cur->next, curn + 1);
			showitem(cur, curn);
		}
	}
}

//--------------------------------------------------------------------------
// Move cursor to home
//--------------------------------------------------------------------------
void   BaseCaptain::go_home()
{
	if(cur!=dl.next)
	{
		base = dl.next;
		cur = base;
		curn = 0;
		showdir();
	}
}

//--------------------------------------------------------------------------
// Move cursor to end
//--------------------------------------------------------------------------
void    BaseCaptain::go_end()
{
	FList *o=dl.next;
	int n=0;
	while(o->next)
	{
		o=o->next;
		n++;
	}
	if(cur==o)
		return;
	base=dl.next;
	cur=o;
	curn=n % items_per_page;
	setcur(&base, n-curn);
	showdir();
}

//--------------------------------------------------------------------------
// Move cursor to left column
//--------------------------------------------------------------------------
void   BaseCaptain::left()
{
    int    i = 0;
    FList *pcur;
    pcur = cur;
    if (setcur(&cur, -items_per_column))
    {
	curn -= items_per_column;
	if (curn < 0)
	{
	    curn += items_per_column;
	    if (setcur(&base, -items_per_column) == 0)
	    {
		base = dl.next;
		pcur = base;
		curn = 0;
		while (pcur != cur)
		{
		    pcur = pcur->next;
		    curn++;
		};
	    };
	    showdir();
	}
	else
	{
	    showitem(pcur, curn + items_per_column);
	    showitem(cur, curn);
	}
    }
    else if (cur != dl.next)
    {
	i = curn;
	curn = 0;
	showitem(pcur, i);
	if (base != dl.next)
	{
	    base = cur = dl.next;
	    showdir();
	}
	else
	{
	    cur = dl.next;
	    showitem(cur, curn);
	};
    }
}

//--------------------------------------------------------------------------
// Move cursor to right column
//--------------------------------------------------------------------------
void   BaseCaptain::right()
{
    FList *pcur, *ko;
    int    i = 0;
    pcur = cur;
    if (setcur(&cur, items_per_column))
    {
	curn += items_per_column;
	if (curn >= items_per_page)
	{
	    curn -= items_per_column;
	    setcur(&base, items_per_column);
	    showfilelist();
	}
	else
	{
	    showitem(pcur, curn - items_per_column);
	    showitem(cur, curn);
	}
    }
    else
    {
	i = 0;
	ko = cur;
	while (cur->next != NULL)
	{
	    i++;
	    cur = cur->next;
	};
	if (i > 0)
	{
	    curn += i;
	    showitem(ko, curn - i);
	    if (curn >= items_per_page)
	    {
		curn -= items_per_column;
		setcur(&base, items_per_column);
		showdir();
	    }
	    else
		showitem(cur, curn);
	};
    }
}

//--------------------------------------------------------------------------
// Move cursor one page up
//--------------------------------------------------------------------------
void   BaseCaptain::page_up()
{
    int    i = 0;
    FList *pcur;
    pcur = cur;
    if (setcur(&cur, -items_per_page))
    {
	if (setcur(&base, -items_per_page) == 0)
	{
	    base = dl.next;
	    pcur = base;
	    curn = 0;
	    while (pcur != cur)
	    {
		pcur = pcur->next;
		curn++;
	    };
	};
	showdir();
    }
    else if (cur != dl.next)
    {
	i = curn;
	curn = 0;
	showitem(pcur, i);
	if (base != dl.next)
	{
	    base = cur = dl.next;
	    showdir();
	}
	else
	{
	    cur = dl.next;
	    showitem(cur, curn);
	};
    }
}

//--------------------------------------------------------------------------
// Move cursor one page down
//--------------------------------------------------------------------------
void  BaseCaptain::page_down()
{
    FList *pcur, *ko;
    int    i = 0;
    pcur = cur;
    if (setcur(&cur, items_per_page))
    {
	setcur(&base, items_per_page);
	showfilelist();
    }
    else
    {
	i = 0;
	ko = cur;
	while (cur->next != NULL)
	{
	    i++;
	    cur = cur->next;
	};
	if (i > 0)
	{
	    curn += i;
	    showitem(ko, curn - i);
	    if (curn >= items_per_page)
	    {
		curn -= items_per_page;
		setcur(&base, items_per_page);
		showdir();
	    }
	    else
		showitem(cur, curn);
	};
    }
}

//--------------------------------------------------------------------------
// Compare two panels and highlight difference
//--------------------------------------------------------------------------
void BaseCaptain::compare_panels()
{
    FList *o=dl.next->next, *oo;
    selfiles=selsize=0;
    while(o)
    {
	oo=other()->find_file_entry(o->name);
	if(oo==NULL)
	    select_file(o);
	else
	    if(compare_by_size && o->size!=oo->size)

		select_file(o);
	    else
		if(compare_by_time && o->time!=oo->time)
		    select_file(o);
	o=o->next;
    }
    if(am_i_visible() || lay==0)
	showdir();
}

//--------------------------------------------------------------------------
// Set attributes to current file
//--------------------------------------------------------------------------
void   BaseCaptain::setattr()
{
    if(cur->get_dir())
	chdir(cur->get_dir());
    else
	vfs->ch_curdir();
    if (vfs->chmod(cur->name, cur->mode) == -1)
    {
	errno2mes();
	fserror(_("Set attributes"), cur);
    }
    total_reread();
}

//--------------------------------------------------------------------------
// Push current vfs and try to switch to new ftp
//--------------------------------------------------------------------------
void   BaseCaptain::do_ftp_link()
{
	delete_vfs_list();
	push_vfs(vfs);
	vfs = &ftp;
	if (ftp.init_support((char*)&ftprec) == 0)        //If error we return to panels DFS
	{
		show_vfs_error();
		vfs = pop_vfs();
	} else
		if(ftp.bgbit)
			vfs=pop_vfs();
    // *INDENT-OFF*        
	::chdir(curdir);
    // *INDENT-ON*        


	vfs->ch_curdir();
	if (direc(curdir) == 0)
	{
		vfs_error(vfs->vfs_prefix, _("Can't change to dir"));
		show_vfs_error();
		vfs->chdir("..");
		vfs->getcwd(curdir, 1020);
		direc(curdir);
	}
	showdir();
	masks|=PANEL_CD_DONE;
}


void   BaseCaptain::do_ftp_link_passwd(char *passwd)
{
	strcpy(ftprec.ftppasswd,passwd);
	do_ftp_link();
}



//----------------------------Other stuff-----------------------
extern char syspath[];


//Create and get tmp directory path, generated
//Always without ending '/'
//Return 1 if directory created
int create_xnc_tmp_dir(char* to)
{
  time_t t;
  time(&t);
  sprintf(to, "%s/.%d", syspath, getpid());
  ::mkdir(to, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  sprintf(to, "%s/.%d/%d", syspath, getpid(), t);
  if(::mkdir(to, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)!=-1)
    return 1;
  return 0;
}

//Delete tmp directory created by create_xnc_tmp_dir
// Return 1 if success
int delete_xnc_tmp_dir(char* dir)
{
  char buf[L_MAXPATH];
  char buf2[L_MAXPATH];
  quote_path(buf, dir);
  sprintf(buf2, "rm -rf %s", buf);
  return psystem(buf2)==0;
}



//******************************** Virtual Methods ***********************************

BaseVirtual(void,showdir())
BaseVirtual(void,fserror(char*,FList*))
BaseVirtual(void,total_expose())
BaseVirtual(void,set_recycle_state(int))
BaseVirtual(void,create_infowin(char *head))
BaseVirtual(void,del_infowin())
BaseVirtual(void,to_infowin(char *str))
BaseVirtual(void,infowin_set_coun(int))
BaseVirtual(void,infowin_update_coun(int))
BaseVirtual(void,total_reread())
BaseVirtual(void,flush_screen())
BaseVirtual(void,show_item(FList*))
BaseVirtual(void,showfinfo(FList*, int))
BaseVirtual(void,expose_panel())
BaseVirtual(void,showdirname())
BaseVirtual(void,showitem(FList*,int))
BaseVirtual(void,showempty(int))
BaseVirtual(void,show_ff_item(int,int))
BaseVirtual(void,show_string_info(char*))
BaseVirtual(void,do_simple_dialog(char*,ManFuncs,int))

rBaseVirtual(int,am_i_visible(),0)
rBaseVirtual(int,is_panel_vfs(VFS*),0)
rBaseVirtual(BaseCaptain*,other(),0)
rBaseVirtual(VFS*,other_vfs(),0)


