#ifndef DIRCACHEMAN_H
#define DIRCACHEMAN_H

class DirCacheManager 
{
    struct DirCacheList
    {
	DirCacheList* next;
	char*         data;
	DirCacheList() {data=0;next=0;};
	DirCacheList(char*);
        ~DirCacheList();
	int           equal(const char*);
    };

    DirCacheList      dir_list;

public:
    DirCacheManager();
    ~DirCacheManager();
    void              clean_all();
    char*             add_entry(char*);

};

#endif
