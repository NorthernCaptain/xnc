#ifndef MESQUEUE_H
#define MESQUEUE_H
/****************************************************************************
*  Copyright (C) 1999 by Leo Khramov
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
#include "xh.h"

template <class Tmp>
class MesQue
{
    protected:
        Tmp **arr;
        int maxarr;
        int first, last;
        int total;
    public:
        MesQue(int imax)
        {
            int i;
            maxarr=imax;
            arr=new Tmp* [maxarr];
            for(i=0;i<maxarr;i++)
                arr[i]=NULL;
            first=last=total=0;
        };
        ~MesQue()  { delete arr;};
        int is_not_empty() {return total;};
        int push_mes(Tmp *o)
        {
            if(total<maxarr)
            {
                arr[last]=o;
                roll(last, 1);
                total++;
                return 1;
            }
            return 0;
        }
        Tmp* pop_mes()
        {
            Tmp *o;
            if(total==0)
                return NULL;
            o=arr[first];
            roll(first, 1);
            total--;
            return o;
        }
        void roll(int& v, int delta)
        {
            v+=delta;
            if(v>=maxarr)
                v%=maxarr;
        }
};
        


#endif/* ------------ End of file -------------- */

