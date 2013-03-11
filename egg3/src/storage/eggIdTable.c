#include "eggIdTable.h"

HEGGIDTABLE eggIdTable_new(HEGGFILE hEggFile)
{
    
    EGGIDTABLENODE buff[TABLE_INCREMENT_CNT];
    
 
    if(hEggFile==EGG_NULL)
    { 
        return EGG_NULL;
    }
    HEGGIDTABLE hIdTable=(HEGGIDTABLE)malloc(sizeof(EGGIDTABLE));
    HEGGIDTABLEINFO hegginfo=(HEGGIDTABLEINFO)malloc(sizeof(EGGIDTABLEINFO));
    memset(hegginfo,0,sizeof(EGGIDTABLEINFO));
   if(hIdTable==EGG_NULL)
    {
        free(hegginfo);
        return EGG_NULL;
    }
    
    
    hIdTable->hEggFile=hEggFile;
    EggFile_lock_wr_wait(hEggFile, SEEK_SET, 0, 0);
    if(EggFile_size(hEggFile) == 0)
    {
        EggFile_write(hIdTable->hEggFile, hegginfo, sizeof(EGGIDTABLEINFO), 0);
        hIdTable->hIdTableInfo=(HEGGIDTABLE)mmap(NULL,sizeof(EGGIDTABLEINFO),PROT_READ|PROT_WRITE,MAP_SHARED,(EggFile_object(hIdTable->hEggFile))->hFile,0) ;
        hIdTable->hIdTableInfo->docCnt=0;
        hIdTable->hIdTableInfo->tableCnt=TABLE_INCREMENT_CNT;
        memset(buff,0,TABLE_INCREMENT_SIZE);
        EggFile_write(hIdTable->hEggFile,buff,TABLE_INCREMENT_SIZE,sizeof(EGGIDTABLEINFO));
    }
    else
    {
        hIdTable->hIdTableInfo=(HEGGIDTABLE)mmap(NULL,sizeof(EGGIDTABLEINFO),PROT_READ|PROT_WRITE,MAP_SHARED,(EggFile_object(hIdTable->hEggFile))->hFile,0) ;
        
    }
    EggFile_unlock(hEggFile, SEEK_SET, 0, 0);
    
    free(hegginfo);

    return hIdTable;
}


HEGGIDTABLE eggIdTable_delete(HEGGIDTABLE hIdTable)
{
    if(hIdTable==EGG_NULL)
    {
        return EGG_NULL;
    }
    munmap(hIdTable->hIdTableInfo,sizeof(EGGIDTABLEINFO));
    EggFile_close(hIdTable->hEggFile);
    free(hIdTable);
    return EGG_NULL;
}

did_t eggIdTable_map_id(HEGGIDTABLE hIdTable, offset64_t docOff)
{
    EGGIDTABLENODE st_idt_node;
    st_idt_node.docOff = docOff;
    
    did_t docCnt; 
    if(hIdTable==EGG_NULL)
    {
        return 0;
    }
    EggFile_lock_wr_wait(hIdTable->hEggFile,SEEK_SET,0,sizeof(EGGIDTABLEINFO));
    if(hIdTable->hIdTableInfo->docCnt==hIdTable->hIdTableInfo->tableCnt)
    {
        
        EGGIDTABLENODE buff[TABLE_INCREMENT_CNT];
        memset(buff,0,TABLE_INCREMENT_SIZE);
        EggFile_write(hIdTable->hEggFile,buff,TABLE_INCREMENT_SIZE,hIdTable->hIdTableInfo->tableCnt*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO));
      
       hIdTable->hIdTableInfo->tableCnt+=TABLE_INCREMENT_CNT;
    }
    
       EggFile_write(hIdTable->hEggFile, &st_idt_node, sizeof(st_idt_node),(hIdTable->hIdTableInfo->docCnt)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO));
       
       docCnt=++hIdTable->hIdTableInfo->docCnt;
       
       EggFile_unlock(hIdTable->hEggFile, SEEK_SET, 0, sizeof(EGGIDTABLEINFO));
       return docCnt;
}

EBOOL eggIdTable_update_off(HEGGIDTABLE hIdTable, did_t id, offset64_t docOff)
{
    
    EGGIDTABLENODE eggidtnode;
    if(id>hIdTable->hIdTableInfo->docCnt)
    {
        return EGG_FALSE;
    }
    eggidtnode.docOff = docOff;
    
    EggFile_write(hIdTable->hEggFile, &eggidtnode,sizeof(eggidtnode),(id - 1)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO));

     

    return EGG_TRUE;
}

offset64_t eggIdTable_get_off(HEGGIDTABLE hIdTable,did_t id)
{
    
    EGGIDTABLENODE eggidtnode;
    if(id>hIdTable->hIdTableInfo->docCnt)
    {
        return TABLE_ID_OVERFLOW;
    }
    
    EggFile_read(hIdTable->hEggFile,&eggidtnode,sizeof(EGGIDTABLENODE),(id-1)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO));
    
    return eggidtnode.docOff;

}

EBOOL eggIdTable_rdlock_id(HEGGIDTABLE hIdTable, did_t id)
{
    
    EggFile_lock_rd_wait(hIdTable->hEggFile, SEEK_SET , (id-1)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO), sizeof(EGGIDTABLENODE));
    return EGG_TRUE;
}

EBOOL eggIdTable_wrlock_id(HEGGIDTABLE hIdTable, did_t id)
{
    
    EggFile_lock_wr_wait(hIdTable->hEggFile, SEEK_SET , (id-1)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO), sizeof(EGGIDTABLENODE));
    return EGG_TRUE;
}

EBOOL eggIdTable_unlock_id(HEGGIDTABLE hIdTable, did_t id)
{
    EggFile_unlock(hIdTable->hEggFile, SEEK_SET ,(id-1)*sizeof(EGGIDTABLENODE)+sizeof(EGGIDTABLEINFO), sizeof(EGGIDTABLENODE));
    
    return EGG_TRUE;
}
