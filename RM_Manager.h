#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"

typedef int SlotNum;

typedef struct {	
	PageNum pageNum;	//��¼����ҳ��ҳ��
	SlotNum slotNum;		//��¼�Ĳ�ۺ�
	bool bValid; 			//true��ʾΪһ����Ч��¼�ı�ʶ��
}RID;

typedef struct{
	bool bValid;		 // False��ʾ��δ�������¼
	RID  rid; 		 // ��¼�ı�ʶ�� 
	char *pData; 		 //��¼���洢������ 
}RM_Record;


typedef struct
{
	int bLhsIsAttr,bRhsIsAttr;//���������ԣ�1������ֵ��0��
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
}Con;

typedef struct {
	int nRecords;			//��ǰ�ļ��а����ļ�¼��
	int recordSize;			//ÿ����¼�Ĵ�С
	int recordsPerPage;		//ÿ��ҳ�����װ�صļ�¼����
	int firstRecordOffset;	//ÿҳ��һ����¼���������еĿ�ʼλ��
} RM_FileSubHeader;

typedef struct {
	char *rBitMap; //����ҳλͼ
	RM_Record *rRecord; //��һ����¼����ʼλ��
}RM_Page;

typedef struct {//��¼�ļ����
	bool bOpen;//����Ƿ�򿪣��Ƿ����ڱ�ʹ�ã�
	PF_FileHandle *pf_fileHandle; //��þ��������ҳ���ļ����

	char *rBitMap;  //ָ�����ҳ��λͼ��ָ��
	PF_PageHandle *pageHandle; //��¼����ҳ����
	RM_FileSubHeader *rm_fileSubHeader; //���ļ���RM_FileSubHeader�ṹ
										//��Ҫ�Զ������ڲ��ṹ
}RM_FileHandle;

typedef struct {
	bool  bOpen;		//ɨ���Ƿ�� 
	RM_FileHandle *pRMFileHandle;		//ɨ��ļ�¼�ļ����
	int  conNum;		//ɨ���漰���������� 
	Con  *conditions;	//ɨ���漰����������ָ��
	PF_PageHandle * pageHandle; //�����е�ҳ����
	PageNum  pn; 	//ɨ�輴�������ҳ���
	SlotNum  sn;		//ɨ�輴������Ĳ�ۺ�
}RM_FileScan;



RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);
bool Compare(CompOp compOp, AttrType attrType, void *Lvalue, void *Rvalue);

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);

RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);

//RC CharsRead(char *pData, char* data, int length);         //��pData��ȡlength�����ַ����ݵ�data

#endif