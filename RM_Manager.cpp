#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
{
	//��ʼ���ļ�ɨ��ṹ
	rmFileScan->bOpen = true;
	rmFileScan->pRMFileHandle = fileHandle;
	rmFileScan->conNum = conNum;
	rmFileScan->conditions = conditions;
	rmFileScan->pageHandle = NULL; //�����е�ҳ����

	//ɨ��ҳ�����λͼ��˳��Ѱ�ҵ�һ������������ҳ
	char *pBitMap = fileHandle->pf_fileHandle->pBitmap;
	
	if (fileHandle->pf_fileHandle->pFileSubHeader->nAllocatedPages <= 2)
	{ //������ҳ�����ļ���û�д����κμ�¼
		rmFileScan->pn = 0;
		rmFileScan->sn = 0; //pn��sn����0����ʾ�ļ���û�м�¼
	}
	else { //������ҳ��Ѱ�ҵ�һ�������������ҳҳ��
		rmFileScan->pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		int i = 2;
		while (true)
		{
			char x = 1 << (i % 8);
			if ((*(pBitMap+i/8) & x) != 0)
			{ //�ҵ�һ���ѷ���ҳ
				rmFileScan->pn = i;
				GetThisPage(fileHandle->pf_fileHandle, rmFileScan->pn, rmFileScan->pageHandle);
				break;
			}
			else
			{
				i++;
			}
		}
	
		//ɨ������ҳλͼ��Ѱ�ҵ�һ����Ч��¼λ��
		char *rBitMap;
		GetData(rmFileScan->pageHandle, &rBitMap);
		i = 0;
		while (true)
		{
			char x = 1 << (i % 8);
			if ((*(rBitMap+i/8) & x) != 0)
			{
				rmFileScan->sn = i;
				break;
			}
			else
			{
				i++;
			}

		}
	}

	return SUCCESS;
}

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
	//һ��ҳ��ɨ����ϣ�Ҫ����ҳ�������������פ��
	if (rmFileScan->pn == 0)
	{ //ɨ���ҳ���Ϊ�㣬��ʾ�ļ����޼�¼
		return RM_EOF;
	}

	RM_Record *localRec = rec;

	while (true)
	{

		//��ü�¼
		localRec->bValid = true;
		localRec->rid.bValid = true;
		localRec->rid.pageNum = rmFileScan->pn;
		localRec->rid.slotNum = rmFileScan->sn;

		char *pData;
		GetData(rmFileScan->pageHandle, &pData);

		localRec->pData = pData + rmFileScan->pRMFileHandle->rm_fileSubHeader->firstRecordOffset
			+ localRec->rid.slotNum*rmFileScan->pRMFileHandle->rm_fileSubHeader->recordSize;

		int i;
		//���бȽ�
		if (rmFileScan->conNum == 0)
		{ //ɨ���漰����������Ϊ0��ֱ�ӷ���ɨ�赽�ļ�¼
			i = rmFileScan->conNum;
		}
		else {
			//ɨ�赽��������������0������������жԱ�
			Con *con;
			//���������һ�����������㣬����ǰ�˳�forѭ��
			i = 0;
			for (; i < rmFileScan->conNum; i++)
			{
				con = rmFileScan->conditions + i;
				bool flag = true; //��ǰ�����ıȽϽ��������Ҫ��ʱ��flag����false
				//�ж����ڱȽϵ���������
				switch (con->attrType)
				{
				case chars:
					char *Lattr, *Rattr;
					if (con->bLhsIsAttr == 1)
					{ //���������������
						Lattr = (char *)malloc(sizeof(char)*con->LattrLength);
						strncpy(Lattr, localRec->pData + con->LattrOffset, con->LattrLength);
						//strncpy_s(Lattr, localRec->pData + con->LattrOffset, con->LattrLength);
					}
					else { //�����������ֵ
						Lattr = (char *)con->Lvalue;
					}

					if (con->bRhsIsAttr == 1)
					{ //�������ұ�������
						Rattr = (char *)malloc(sizeof(char)*con->LattrLength);
						strncpy(Lattr, localRec->pData + con->RattrOffset, con->RattrLength);
					}
					else { //�������ұ���ֵ
						Rattr = (char *)con->Rvalue;
					}

					flag = Compare(con->compOp, con->attrType, Lattr, Rattr);
					break;
				case ints:
					int *IntLattr, *IntRattr;
					if (con->bLhsIsAttr == 1)
					{ //���������������
						//IntLattr = (int *)malloc(sizeof(int));
						IntLattr = (int *)(localRec->pData + con->LattrOffset);
					}
					else { //�����������ֵ
						IntLattr = (int *)con->Lvalue;
					}

					if (con->bRhsIsAttr == 1)
					{ //�������ұ�������
						//Rattr = (char *)malloc(sizeof(char)*con->LattrLength);
						IntRattr = (int *)(localRec->pData + con->RattrOffset);
					}
					else { //�������ұ���ֵ
						IntRattr = (int *)con->Rvalue;
					}

					flag = Compare(con->compOp, con->attrType, IntLattr, IntRattr);
					break;
				case floats:
					float *FloatLattr, *FloatRattr;
					if (con->bLhsIsAttr == 1)
					{ //���������������
					  //IntLattr = (int *)malloc(sizeof(int));
						FloatLattr = (float *)(localRec->pData + con->LattrOffset);
					}
					else { //�����������ֵ
						FloatLattr = (float *)con->Lvalue;
					}

					if (con->bRhsIsAttr == 1)
					{ //�������ұ�������
					  //Rattr = (char *)malloc(sizeof(char)*con->LattrLength);
						FloatRattr = (float *)(localRec->pData + con->RattrOffset);
					}
					else { //�������ұ���ֵ
						FloatRattr = (float *)con->Rvalue;
					}

					flag = Compare(con->compOp, con->attrType, FloatLattr, FloatRattr);

					break;
				default:
					break;
				}

				if (!flag)
				{
					break;
				}
			}
		
		}
		//���ü�¼ɨ�����һ����¼��ҳ��źͲ�λ��
		while (true)
		{
			//����˳��ɨ�赱ǰҳ�����һ����¼��
			int j = rmFileScan->sn + 1;
			char *rBitMap, x;
			GetData(rmFileScan->pageHandle, &rBitMap);
			for (; j < rmFileScan->pRMFileHandle->rm_fileSubHeader->nRecords; j++)
			{
				x = 1 << (j % 8);
				if ((*(rBitMap + j / 8) & x) != 0)
				{
					rmFileScan->sn = j;
					break;
				}

			}

			if (j < rmFileScan->pRMFileHandle->rm_fileSubHeader->nRecords)
			{ //�ڵ�ǰҳ���ҵ���һ����Ч��¼
				break;
			}
			else { //�ڵ�ǰҳ��δ�ҵ���һ����Ч��¼��������һ����¼ҳ��
				unsigned int k = rmFileScan->pn+1;
				char y;
				char *pBitMap = rmFileScan->pRMFileHandle->pf_fileHandle->pBitmap;
				while (k <= rmFileScan->pRMFileHandle->pf_fileHandle->pFileSubHeader->pageCount)
				{ //PageNum
					y = 1 << (k % 8);
					if ((*(pBitMap + k / 8) & y) != 0)
					{ //�ҵ�һ���ѷ���ҳ
						rmFileScan->pn = k;
						UnpinPage(rmFileScan->pageHandle);  //�����ҳ��פ��������
						//��ҳ��פ������
						GetThisPage(rmFileScan->pRMFileHandle->pf_fileHandle, rmFileScan->pn, rmFileScan->pageHandle);
						break;
					}
					else
					{
						k++;
					}
				}

				if (k > rmFileScan->pRMFileHandle->pf_fileHandle->pFileSubHeader->pageCount)
				{
					rmFileScan->pn = 0;
					rmFileScan->sn = 0;
					break;
				}

				//ɨ������ҳλͼ��Ѱ�ҵ�һ����Ч��¼λ��
				GetData(rmFileScan->pageHandle, &rBitMap);
				k = 0;
				while (true)
				{
					y = 1 << (k % 8);
					if ((*(rBitMap + k / 8) & y) != 0)
					{
						rmFileScan->sn = k;
						break;
					}
					else
					{
						k++;
					}
				}

			}
		}

		if (i < rmFileScan->conNum)
		{ //��ǰ��¼�����㣬������һ����¼
		 	RC re = GetNextRec(rmFileScan, localRec);
			if (re == SUCCESS)
			{
				break;
			}
			else {
				return re;
			}
		} //��ǰ��¼����������������������
		else
		{
			break;
		}

	}

	rec = localRec;

	return SUCCESS;
}

//���ݱȽϲ��������Ƚϴ������������
//���ȽϽ������Ƚϲ�������Ҫ���򷵻�true�����򣬷���false
bool Compare(CompOp compOp, AttrType attrType, void *Lvalue, void *Rvalue) {

	bool result = false;

	char *CharLv;
	char *CharRv;
	int IntLv;
	int IntRv;
	float FloatLv;
	float FloatRv;

	switch (attrType)
	{
	case chars:
		CharLv = (char *)Lvalue;
		CharRv = (char *)Rvalue;
		int re;
		re = strcmp(CharLv, CharRv);
		switch (compOp)
		{
		case EQual:		        //"="			0
			if (re == 0)
				result = true;
			break;
		case LEqual:			//"<="          1 
			if (re <= 0)
				result = true;
			break;
		case NEqual:			//"<>"			2 
			if (re != 0)
				result = true;
			break;
		case LessT:			    //"<"			3
			if (re < 0)
				result = true;
			break;
		case GEqual:			//">="			4 
			if (re >= 0)
				result = true;
			break;
		case GreatT:			//">"           5 
			if (re > 0)
				result = true;
			break;
		case NO_OP:
			result = true;
			break;
		default:
			break;
		}

		break;
	case ints:
		IntLv = *(int *)Lvalue;
		IntRv = *(int *)Rvalue;
		switch (compOp)
		{
		case EQual:		        //"="			0
			if (IntLv == IntRv)
				result = true;
			break;
		case LEqual:			//"<="          1
			if (IntLv <= IntRv)
				result = true;
			break;
		case NEqual:			//"<>"			2 
			if (IntLv != IntRv)
				result = true;
			break;
		case LessT:			    //"<"			3
			if (IntLv < IntRv)
				result = true;
			break;
		case GEqual:			//">="			4 
			if (IntLv >= IntRv)
				result = true;
			break;
		case GreatT:			//">"           5 
			if (IntLv > IntRv)
				result = true;
			break;
		case NO_OP:
			result = true;
			break;
		default:
			break;
		}

		break;
	case floats:
		FloatLv = *(float *)Lvalue;
		FloatRv = *(float *)Rvalue;
		switch (compOp)
		{
		case EQual:		        //"="			0
			if (FloatLv == FloatRv)
				result = true;
			break;
		case LEqual:			//"<="          1
			if (FloatLv <= FloatRv)
				result = true;
			break;
		case NEqual:			//"<>"			2 
			if (FloatLv != FloatRv)
				result = true;
			break;
		case LessT:			    //"<"			3
			if (FloatLv < FloatRv)
				result = true;
			break;
		case GEqual:			//">="			4 
			if (FloatLv >= FloatRv)
				result = true;
			break;
		case GreatT:			//">"           5 
			if (FloatLv > FloatRv)
				result = true;
			break;
		case NO_OP:
			result = true;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	return result;

}

RC CloseScan(RM_FileScan *rmFileScan) 
{
	rmFileScan->bOpen = false;
	rmFileScan->pRMFileHandle = NULL;
	rmFileScan->conNum = 0;
	rmFileScan->conditions = NULL;
	if (rmFileScan->pageHandle != NULL)
	{
		UnpinPage(rmFileScan->pageHandle);
		free(rmFileScan->pageHandle);
		rmFileScan->pageHandle = NULL;
	}

	return SUCCESS;
}

RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	PF_PageHandle* pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	RM_Record *localRec = rec;
	char *pData;
	char *pRec;

	GetThisPage(fileHandle->pf_fileHandle, rid->pageNum, pageHandle);
	GetData(pageHandle, &pData);

	pRec = pData + fileHandle->rm_fileSubHeader->firstRecordOffset + rid->slotNum*fileHandle->rm_fileSubHeader->recordSize;
	memcpy(localRec, pRec, fileHandle->rm_fileSubHeader->recordSize);
	/*if (!localRec->bValid)      //�жϼ�¼�Ƿ���Ч
	{
		return 
	}*/

	rec = localRec;
	UnpinPage(pageHandle);
	free(pageHandle);

	return SUCCESS;
}

RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	PF_PageHandle* pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	RID *localRid = rid;
	int pageNum;      //�ѷ���ҳ����Ŀ
	char *pf_bitMap;
	char *rm_bitMap;
	char *pageData;

	pageNum = fileHandle->pf_fileHandle->pFileSubHeader->nAllocatedPages;
	
	pf_bitMap = fileHandle->pf_fileHandle->pBitmap;
	rm_bitMap = fileHandle->rBitMap;
	localRid->bValid = false;
	for (unsigned int i = 2; i <= fileHandle->pf_fileHandle->pFileSubHeader->pageCount; i++)
	{

		if ((*(pf_bitMap+i/8) & (1<<(i%8))) != 0)
		{ //�ҵ�һ���ѷ���ҳ
			if ((*(rm_bitMap+i/8) & (1<<(i%8))) != 0) //��ҳ������
			{
				continue;
			}

			GetThisPage(fileHandle->pf_fileHandle, i, pageHandle);
			GetData(pageHandle, &pageData);

			//��ҳ��δ�����ڸ�ҳ�����ҵ�һ�����м�¼��
			for (int j = 0; j < fileHandle->rm_fileSubHeader->recordsPerPage; j++)
			{
				if ((*(pageData+j/8) & (1<<j%8)) == 0) //�ҵ�һ�����м�¼��
				{
					localRid->bValid = true;
					localRid->pageNum = i;
					localRid->slotNum = j;
					break;
				}
			}

			if (localRid->bValid)
			{  //�ҵ�һ�����м�¼��
				break;
			}
		}
	}

	
	if (!localRid->bValid)
	{ //���û�ҵ����м�¼�ۣ����·���һ������ҳ
		RC tmp;
		if ((tmp = AllocatePage(fileHandle->pf_fileHandle, pageHandle)) != SUCCESS) {
			return tmp;
		}

		GetData(pageHandle, &pageData);
		GetPageNum(pageHandle, &localRid->pageNum);
		localRid->bValid = true;
		localRid->slotNum = 0;

		char x = ~(1 << (localRid->pageNum % 8));
		*(rm_bitMap + localRid->pageNum / 8) = *(rm_bitMap + localRid->pageNum / 8) & x;

	}
	
	//�ҵ�һ�����м�¼��,����¼����,�޸Ŀ���ҳ��¼����,�޸�����ҳλͼ���ж��Ƿ��޸Ŀ���ҳλͼ
	
	memcpy(pageData + fileHandle->rm_fileSubHeader->firstRecordOffset + localRid->slotNum*fileHandle->rm_fileSubHeader->recordSize,
		pData, fileHandle->rm_fileSubHeader->recordSize);

	//�޸�λͼ�����ü�¼�۱��Ϊ��Ч
	*(pageData + localRid->slotNum / 8) = *(pageData + localRid->slotNum / 8) | (1 << localRid->slotNum % 8);
	//�жϸ�����ҳ�Ƿ��������������Ƿ��޸Ŀ���ҳλͼ
	int i = localRid->slotNum + 1;
	for ( ; i < fileHandle->rm_fileSubHeader->recordsPerPage; i++)
	{
		if ((*(pageData + i / 8) & (1 << i % 8)) == 0)
		{ //�����µļ�¼�����ҵ�һ�����в�
			break;
		}
	}
	if (i >= fileHandle->rm_fileSubHeader->recordsPerPage)
	{
		*(rm_bitMap + localRid->pageNum / 8) = *(rm_bitMap + localRid->pageNum / 8) | (1 << (localRid->pageNum % 8));
	}

	fileHandle->rm_fileSubHeader->nRecords++;

	MarkDirty(pageHandle);
	if (!fileHandle->pageHandle->pFrame->bDirty)
	{
		MarkDirty(fileHandle->pageHandle);
	}

	
	UnpinPage(pageHandle);
	free(pageHandle);

	return SUCCESS;
}

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	PF_PageHandle* pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	char *pageData;

	GetThisPage(fileHandle->pf_fileHandle, rid->pageNum, pageHandle);
	GetData(pageHandle, &pageData);
	//�޸���������ҳ��λͼ����Ӧλ����Ϊ��Ч
	char x = ~(1 << (rid->slotNum % 8));
	*(pageData + rid->slotNum / 8) = *(pageData + rid->slotNum / 8) & x;
	
	//��¼��Чλ����Ч
	//char * record = pageData+fileHandle->rm_fileSubHeader->firstRecordOffset+rid->slotNum*fileHandle->rm_fileSubHeader->recordSize;
	//bool f = false;
	//memcmp(record, &f, sizeof(bool));
	//fileHandle->rm_fileSubHeader->nRecords--;

	//�޸ļ�¼����ҳ��λͼ��Ϣ
	char * rm_bitMap = fileHandle->rBitMap;
	x = ~(1 << (rid->pageNum % 8));
	*(rm_bitMap + rid->pageNum / 8) = *(rm_bitMap + rid->pageNum / 8) & x;

	//char * pf_bitMap = fileHandle->pf_fileHandle->pBitmap;
	int i = 0;
	for (; i < fileHandle->rm_fileSubHeader->recordsPerPage; i++)
	{
		if ((*(pageData + i / 8) & (1 << i % 8)) != 0)
		{ //�����µļ�¼�����ҵ�һ���ǿ��в۾��˳�
			break;
		}
	}

	if (i >= fileHandle->rm_fileSubHeader->recordsPerPage)
	{ //û�зǿ��вۣ�ɾ����ҳ��
		DisposePage(fileHandle->pf_fileHandle, rid->pageNum);
		//*(rm_bitMap + localRid->pageNum / 8) = *(rm_bitMap + localRid->pageNum / 8) | (1 << (localRid->pageNum % 8));
	}
	//else
	//{
		MarkDirty(pageHandle);
	//}

	if (!fileHandle->pageHandle->pFrame->bDirty)
	{
		MarkDirty(fileHandle->pageHandle);
	}

	UnpinPage(pageHandle);
	free(pageHandle);

	return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	PF_PageHandle* pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	char *pageData;

	GetThisPage(fileHandle->pf_fileHandle, rec->rid.pageNum, pageHandle);
	GetData(pageHandle, &pageData);

	//���¼�¼����
	memcpy(pageData + fileHandle->rm_fileSubHeader->firstRecordOffset + rec->rid.slotNum*fileHandle->rm_fileSubHeader->recordSize,
		rec->pData, fileHandle->rm_fileSubHeader->recordSize);

	MarkDirty(pageHandle);

	UnpinPage(pageHandle);
	free(pageHandle);
	return SUCCESS;
}

RC RM_CreateFile (char *fileName, int recordSize)
{
	PF_FileHandle *fileHandle = (PF_FileHandle *)malloc(sizeof(PF_FileHandle));
	//PF_FileHandle fileHandle;
	PF_PageHandle *pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	PF_PageHandle *pageHandle1 = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	char *subHeaderChars = (char *)malloc(sizeof(PF_FileSubHeader));
	//PF_FileSubHeader *pf_fileSubHeader;
	RM_FileSubHeader *rm_fileSubHeader = (RM_FileSubHeader *)malloc(sizeof(RM_FileSubHeader));
	int recordsPerPage, bitMapLength;

	char *pData1;

	//�����ļ�
	CreateFile(fileName);
	//���ļ�
	OpenFile(fileName, fileHandle);

	//��ʼ����¼�ļ�

	//���ҳ����Ϣ����ҳ���
	GetThisPage(fileHandle, 0, pageHandle);

	//���������ָ��
//	GetData(pageHandle, &pData);
//	fileSubHeader = (PF_FileSubHeader *)strncpy(subHeaderChars, pData, sizeof(PF_FileSubHeader));
	
	//������ҳ,���䵽����ҳҳ��ӦΪ1
	AllocatePage(fileHandle, pageHandle1);
	MarkDirty(pageHandle);  //��ҳ����Ϣ����ҳ���Ϊdirty

	rm_fileSubHeader->nRecords = 0;
	rm_fileSubHeader->recordSize = recordSize;
	
	//ȷ����¼��ʼλ��firstRecordOffset��ÿ��ҳ�����װ�صļ�¼��recordsPerPage
	//��������4092�ֽ�
	recordsPerPage = 4092 / recordSize;
	bitMapLength = (recordsPerPage + 7) / 8;

	while (recordsPerPage*recordSize+bitMapLength > 4092) {
		recordsPerPage--;
		bitMapLength = (recordsPerPage + 7) / 8;
	}

	rm_fileSubHeader->firstRecordOffset = bitMapLength;
	rm_fileSubHeader->recordsPerPage = recordsPerPage;

	GetData(pageHandle1, &pData1);
	memcpy(pData1, (char *)rm_fileSubHeader, sizeof(RM_FileSubHeader));
	MarkDirty(pageHandle1);

	//��ʼʱ��¼��Ϊ0�����ڼ�¼��ҳ������ҲΪ0���������ü�¼ҳ��λͼ

	//���פ������������
	UnpinPage(pageHandle);
	UnpinPage(pageHandle1);

	//�ر��ļ�
	CloseFile(fileHandle);
	free(subHeaderChars);
	free(fileHandle);
	free(pageHandle);

	return SUCCESS;
}

RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RM_FileHandle *pfileHandle = fileHandle;
	PF_FileHandle *pf_fileHandle = (PF_FileHandle *)malloc(sizeof(PF_FileHandle));
	PF_PageHandle *pageHandle1 = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	char *pData;

	//���ļ������ҳ���ļ����
	RC tmp;
	tmp =  OpenFile(fileName, pf_fileHandle);
	if (tmp == PF_FILEERR)
	{
		printf("PF_FILEERR\n");
		return PF_FILEERR;
	}

	pfileHandle->bOpen = true;
	pfileHandle->pf_fileHandle = pf_fileHandle;

	//��ü�¼����ҳ����
	GetThisPage(pf_fileHandle, 1, pageHandle1);  //��¼����ҳ���ڼ�¼�ļ��ر�ʱ���ļ��رպ������פ����������
	GetData(pageHandle1, &pData);

	pfileHandle->rBitMap = pData + sizeof(RM_FileSubHeader);
	pfileHandle->rm_fileSubHeader = (RM_FileSubHeader *)pData;
	pfileHandle->pageHandle = pageHandle1;

	fileHandle = pfileHandle;

	return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC tmp;

	//���ҳ�����ҳ����פ��
	UnpinPage(fileHandle->pageHandle);

	if ((tmp = CloseFile(fileHandle->pf_fileHandle)) != SUCCESS)
	{ //�رն�Ӧ��ҳ���ļ�
		return tmp;
	}

	return SUCCESS;
}
