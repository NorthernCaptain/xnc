#ifndef FTP_COMM
#define FTP_COMM

#include <ftplib.h>

//Ftp Communication Header - Define base structures and functions
//for FTP communication throught Ftp.Xnc agent.

// Ftp commands definition

#define        FTP_NOP             0     //No operation (waiting for command)
#define        FTP_CONNECT   1     //Connect to ftp site
#define        FTP_LS                2     //LS get listing
#define        FTP_CD                3    //Change remote directory
#define        FTP_LCD              4    //Change local directory
#define        FTP_BYE              5    //Close FTP connection
#define        FTP_GET              6   //Get file or dir from remote site
#define        FTP_PUT              7   //Put file or dir to remote site
#define        FTP_MGET           8   //MultiGet (many files) from remote site
#define        FTP_MPUT           9   //MultiPut
#define        FTP_DEL             10  //Delete file from remote
#define        FTP_MDEL          11  //MultiDelete
#define        FTP_MKDIR         12  //Make remote Dir
#define        FTP_DIE              13  //Ftp die ;(
#define        FTP_NOOP             14  //NOOP command


#define        FTP_LS_CACHE_TIME    36000    //Cache timeout in seconds (default 10 hours)
#define        FTP_TRANSFER_CACHE_TIME  3600  //Transfer cache timeout (default 1 hour)


#define        FTP_HOST_UNIX     1       //Remote host is UNIX
#define        FTP_HOST_UNIX_CHR "unix"
#define        FTP_HOST_SUNOS_CHR "sunos"

#define        FTP_HOST_VMS      2       //Remote host is VMS
#define        FTP_HOST_VMS_CHR  "vms"

#define        FTP_HOST_NT       3       //Remote host is WinNT
#define        FTP_HOST_NT_CHR   "windows_nt"
#define        FTP_HOST_WIN32_CHR "win32"

/****************************************************/
struct        Ftp_Base
{
        char           id[7];                   //Identificator  ["XNCFTP"]
        int              apid;                  //FtpXnc agent PID
        netbuf        *netb;                //NetBuf structure for Ftplib functions
        int              owner_pid;        //Process ID which owned this structure (-1 if noone)
        int              command;         //FTPCommnad corrently running (NOP if waiting for one)
        int              last_com;          //Previous FTPCommand
        int              in_use;             //If in_use==1 then we connected to ftp site and use structure
        
        int              response;        //Response from Ftp.Xnc (-1 if operation failed)
        char           reason[256];    //Error reason (Ftp server response)

        int              opcom;                //If 1 then operation complete but response not take by XNC        
        char           hostname[40];           //Ftp hostname
        char           remotepath[1024];     //Remote working path for hostname
        char           path[1024];                //Working path on this machine (path for transmit/receive files)
        char           tmpfile[1024];            //Path and filename for temporary files
        
        char           ftpmes[30][100];        //30 lines Buffer for output Ftp server responses if required

};
#define S_Ftp_Base        sizeof(Ftp_Base)

//     Structure for communication between XNC and FtpXnc agent
struct        Ftp_Com
{
        int       command;        //Command for processing
        int       xncpid;             //PID of caller
        int       response;          
        char      hostname[40];  //ftp hostname
        char      user[40];          //User name for connect
        char      passwd[40];     //Password for connect
        char      remotepath[1024]; //remote ftp path for work
        char      path[1024];            //current local path
        char      tmppath[1024];            //Path for temporary files
        char      reason[256];
        int       file_attr;                //File attributes. Needs for GET operation
        int       options[10];          //Other FTP options.
                                        // [0] - for proxy
                                        // [1] - for host_type
        char      proxyhost[40];
        char      proxyuser[40];
        char      proxypasswd[40];
};
#define S_Ftp_Com        sizeof(Ftp_Com)

#define FTP_MSGLEN        128

struct        Ftp_Xfer
{
        int        type;        //Transfer or Log message from slave
        int        xfered;        //Number of xfered bytes
        int        total;        //Number of total bytes
        int        time;         //Time when xfered
        char       msg[FTP_MSGLEN];        //Message to log if in logmode
};
#define S_Ftp_Xfer        sizeof(Ftp_Xfer)

#define FTP_XBYTES        1        //Pocket with transfering file info i.e number of bytes transfered
#define FTP_LOGMSG        2        //Pocket with log msg.

#define MaxFtps                5        //Maximum open ftp at the same time.  Four will be enough
#define USE_PROXY        23

#endif/* ------------ End of file -------------- */

