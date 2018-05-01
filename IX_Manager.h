#ifndef IX_MANAGER_H_H
#define IX_MANAGER_H_H

#include "RM_Manager.h"
#include "PF_Manager.h"

typedef struct{
	int attrLength;
	int keyLength;
	AttrType attrType;
	PageNum rootPage;
	PageNum first_leaf;
	int order;
}IX_FileHeader;

typedef struct{
	bool bOpen;
	PF_FileHandle fileHandle;
	IX_FileHeader fileHeader;
}IX_IndexHandle;

typedef struct{
	int is_leaf;
	int keynum;
	PageNum parent;
	PageNum brother;
	char *keys;
	RID *rids;
}IX_Node;

typedef struct{
	bool bOpen;		/*ɨ���Ƿ�� */
	IX_IndexHandle *pIXIndexHandle;	//ָ�������ļ�������ָ��
	CompOp compOp;  /* ���ڱȽϵĲ�����*/
	char *value;		 /* �������бȽϵ�ֵ */
    PF_PageHandle pfPageHandles[PF_BUFFER_SIZE]; // �̶��ڻ�����ҳ������Ӧ��ҳ������б�
	PageNum pnNext; 	//��һ����Ҫ�������ҳ���
	int  ridIx;		//ɨ�輴���������������
}IX_IndexScan;

RC CreateIndex(char * fileName,AttrType attrType,int attrLength);
RC OpenIndex(char *fileName,IX_IndexHandle *indexHandle);
RC CloseIndex(IX_IndexHandle *indexHandle);

RC InsertEntry(IX_IndexHandle *indexHandle, void *pData, RID * rid);
RC DeleteEntry(IX_IndexHandle *indexHandle,void *pData, RID * rid);
RC OpenIndexScan(IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value);
RC IX_GetNextEntry(IX_IndexScan *indexScan,RID * rid);
RC CloseIndexScan(IX_IndexScan *indexScan);

//�������Ĺؼ��ֺ�ָ��д��ָ����ҳ��
void insertKeyAndRidToPage(PF_PageHandle *pageHandle, int order, AttrType attrType, int attrLength, void *pData, RID *rid, bool insertIfTrue);
//������ҳ��ڵ������Ϣ
void copyNewNodeInfoToPage(PF_PageHandle *pageHandle, PageNum brother, PageNum parent, int is_leaf, int keynum);
//���Ʒ��Ѻ�Ĺؼ��ֺ�ָ�뵽��ҳ��
void  copyKeysAndRIDsToPage(PF_PageHandle *pageHandle, void *keySrc, int attrLength, int num, int order, void *ridSrc);
//������ҳ��ڵ������Ϣ
void copyNewNodeInfoToPage(PF_PageHandle *pageHandle, PageNum brother, PageNum parent, int is_leaf, int keynum);
//ɾ��ǰ���ҽڵ��еĹؼ���
void findKeyAndRidForDelete(PF_PageHandle *pageHandle, int order, AttrType attrType, int attrLength, void *pData, RID *rid, bool *existence);
//RID�ıȽ�
bool compareRid(RID *src, RID *des);
//���ֵܽڵ��н�ڵ���ߺϲ�
void getFromBrother(PF_PageHandle *pageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, int *status);
//�򵥵ش�ĳ�����ڵ�ҳ����ɾ��ĳ����֧�����޸Ĺؼ���
void deleteChildNode(PF_PageHandle *parentPageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, PageNum nodePageNum, bool deleteIfTrue, void *pData);
void theFirstEqualScan(IX_IndexHandle *indexHandle, void *pData, PageNum *startPage, int *ridIx, bool *existence);     //�ҵ���һ���ؼ���������ڵ�ҳ��startPage��λ��ridIx
																													   //����ɨ��
void theLEqualScan(IX_IndexHandle *indexHandle, void *pData, PageNum *startPage, int *ridIx);    //�ҵ���һ���ؼ��ִ������ڵ�ҳ��startPage��λ��ridIx
																								 //�ؼ��ֵıȽ�
int keyCompare(void *data1, void *data2, AttrType attrType);
//�ҳ���ǰ�ڵ�����ֵܽڵ�
void findLeftBrother(PF_PageHandle *pageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, PageNum *leftBrother);
//�����ֵܽڵ���д���
void getFromLeft(PF_PageHandle *pageHandle, PF_PageHandle *leftHandle, int order, AttrType attrType, int attrLength, int *status);
//�����ֵܽڵ���д���
void getFromRight(PF_PageHandle *pageHandle, PF_PageHandle *rightHandle, int order, AttrType attrType, int attrLength, int *status);

#endif