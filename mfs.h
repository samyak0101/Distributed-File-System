#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)
#define BUFFER_SIZE (1000)
#define MFS_BLOCK_SIZE   (4096)

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

enum message_type{
        LOOKUP,
        STAT,
        WRITE,
       SHUTDOWN
};

typedef struct _message{
    enum message_type type;
    int pinum;
    char *name;
    int inum;
    int ttype;
    int offset;
    int nbytes;
    char buffer[BUFFER_SIZE];
    // mfs stat struct info
    MFS_Stat_t statstruct;
    // int mfs_stat_type;
    // int mfs_stat_size;
} messagestruct;


typedef struct {
    int inum;
    char buffer;
    int offset;
    int nbytes;
} _write;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int offset, int nbytes);
int MFS_Read(int inum, char *buffer, int offset, int nbytes);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__