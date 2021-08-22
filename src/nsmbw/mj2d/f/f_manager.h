#include "../../types.h"

class fLiNdBa_c {
    public:
    u32 prev; //fLiNdBa_c *
    u32 next; //fLiNdBa_c *
    u32 thisobj; // void *
};

struct PTMF {
    public:
    s32 classOffset;
    s32 vtOffset;
    u32 ptr; // void *
};

class PTMFList {
    public:
    u32 start; //fLiNdBa_c *
    u32 end; //fLiNdBa_c *
    PTMF ptmf;
};

class fManager_c {
    public:
    fManager_c();
    static PTMFList m_createManage;
    static PTMFList m_executeManage;
    static PTMFList m_drawManage;
    static PTMFList m_deleteManage;
};
