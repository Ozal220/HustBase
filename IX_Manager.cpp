#include "stdafx.h"
#include "IX_Manager.h"


int threshold;

//�����ļ��Ĵ���
/*
* fileName: �����ļ���
* attrType: ���������Ե�����
* attrLength: ���������Եĳ���
*/
//RC CreateIndex(const char * fileName, AttrType attrType, int attrLength)
RC CreateIndex(char * fileName, AttrType attrType, int attrLength)
{
	CreateFile(fileName);  //���������ļ�

	PF_FileHandle *fileHandle = NULL;
	fileHandle = (PF_FileHandle *)malloc(sizeof(PF_FileHandle));
	OpenFile(fileName, fileHandle);	//�������ļ�

	PF_PageHandle *firstPageHandle = NULL;
	firstPageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	AllocatePage(fileHandle, firstPageHandle);		//���������ļ��ĵ�һ��ҳ��
	PageNum pageNum;
	GetPageNum(firstPageHandle, &pageNum);

	//��������������Ϣ
	IX_FileHeader index_FileHeader;
	index_FileHeader.attrLength = attrLength;
	index_FileHeader.keyLength = attrLength + sizeof(RID);
	index_FileHeader.attrType = attrType;
	index_FileHeader.rootPage = pageNum;
	index_FileHeader.first_leaf = pageNum;
	int order = (PF_PAGE_SIZE - (sizeof(IX_FileHeader) + sizeof(IX_Node))) / (2 * sizeof(RID) + attrLength);
	index_FileHeader.order = order;
	threshold = order >> 1;

	//��ȡ��һҳ��������
	char *pData;
	GetData(firstPageHandle, &pData);
	memcpy(pData, &index_FileHeader, sizeof(IX_FileHeader));	//������������Ϣ���Ƶ���һҳ

																//��ʼ���ڵ������Ϣ�������ڵ����ΪҶ�ӽڵ㣬�ؼ�����Ϊ0��
	IX_Node index_NodeControl;
	index_NodeControl.brother = 0;
	index_NodeControl.is_leaf = 1;
	index_NodeControl.keynum = 0;
	index_NodeControl.parent = 0;
	memcpy(pData + sizeof(IX_FileHeader), &index_NodeControl, sizeof(IX_Node));

	MarkDirty(firstPageHandle);

	UnpinPage(firstPageHandle);

	//�ر������ļ�
	CloseFile(fileHandle);
	free(firstPageHandle);
	free(fileHandle);
	return SUCCESS;
}

//�����ļ��Ĵ�
RC OpenIndex(char *fileName, IX_IndexHandle *indexHandle)
{
	//�������ļ�
	PF_FileHandle fileHandle;
	RC rc;
	if ((rc = OpenFile(fileName, &fileHandle)) != SUCCESS) {
		return rc;
	}

	PF_PageHandle *pageHandle = NULL;
	pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&fileHandle, 1, pageHandle);   //��ȡ��һҳ

	char *pData;
	GetData(pageHandle, &pData);    //��ȡ��һҳ��������

	IX_FileHeader fileHeader;
	memcpy(&fileHeader, pData, sizeof(IX_FileHeader));  //���Ƶ�һҳ����������Ϣ

	indexHandle->bOpen = true;
	indexHandle->fileHandle = fileHandle;
	indexHandle->fileHeader = fileHeader;
	free(pageHandle);
	return SUCCESS;
}

//�ر������ļ�
RC CloseIndex(IX_IndexHandle *indexHandle)
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	CloseFile(&fileHandle);
	return SUCCESS;
}

//�����Ĳ���
RC InsertEntry(IX_IndexHandle *indexHandle, void *pData, RID * rid)
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	IX_FileHeader fileHeader = indexHandle->fileHeader;
	PF_PageHandle *pageHandle = NULL;

	//�����ļ�ҳ�������
	int order = fileHeader.order;

	//�����ؼ��ֵĳ���
	int attrLength = fileHeader.attrLength;

	//��ȡ���ڵ�ҳ��
	pageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	pageHandle->bOpen = false;
	GetThisPage(&fileHandle, fileHeader.rootPage, pageHandle);

	//��ȡ���ڵ�ҳ���������
	char *pageData;
	GetData(pageHandle, &pageData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));

	//�жϽڵ������Ҷ�ӽڵ�
	while (index_NodeControlInfo->is_leaf != 1)
	{
		RID tempRid; 
		insertKeyAndRidToPage(pageHandle, order, fileHeader.attrType, fileHeader.attrLength, pData, &tempRid, false);    //���ҽ�Ҫ����ؼ��ֵ�ҳ��
		GetThisPage(&fileHandle, tempRid.pageNum, pageHandle);
		GetData(pageHandle, &pageData);
		index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
	}

	insertKeyAndRidToPage(pageHandle, order, fileHeader.attrType, fileHeader.attrLength, pData, rid, true);    //��ҳ�����ؼ���	

	while (index_NodeControlInfo->keynum == order)
	{   //���з���

		int keynum = index_NodeControlInfo->keynum;
		//��ȡ�ؼ�����
		char *keys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
		//��ȡָ����
		char *rids = keys + order*attrLength;

		PageNum nodePage;
		GetPageNum(pageHandle, &nodePage);

		//��Ҷ�ӽڵ�ҳ��
		PF_PageHandle *newLeafPageHandle = NULL;
		newLeafPageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		newLeafPageHandle->bOpen = false;
		AllocatePage(&fileHandle, newLeafPageHandle);
		PageNum newLeafPage;
		GetPageNum(newLeafPageHandle, &newLeafPage);

		int divide1 = keynum >> 1;
		int divide2 = keynum - divide1;

		if (index_NodeControlInfo->parent == 0)   //˵����ǰ���ѵĽڵ�Ϊ���ڵ�
		{
			//�����µĸ�ҳ��
			PF_PageHandle *newRootPageHandle = NULL;
			newRootPageHandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			newRootPageHandle->bOpen = false;
			AllocatePage(&fileHandle, newRootPageHandle);
			PageNum newRootPage;
			GetPageNum(newRootPageHandle, &newRootPage);
			copyNewNodeInfoToPage(newRootPageHandle, 0, 0, 0, 0);	//�����¸��ڵ�Ľڵ������Ϣ

			copyNewNodeInfoToPage(newLeafPageHandle, index_NodeControlInfo->brother, newRootPage, index_NodeControlInfo->is_leaf, divide2);  //�����·��ѽڵ�Ľڵ������Ϣ			
			copyNewNodeInfoToPage(pageHandle, newLeafPage, newRootPage, index_NodeControlInfo->is_leaf, divide1);	//����ԭ�ڵ������Ϣ

			copyKeysAndRIDsToPage(newLeafPageHandle, keys + divide1*attrLength, attrLength, divide2, order, rids + divide1 * sizeof(RID));   //���ƹؼ��ֺ�ָ�뵽���Ѻ��µ�ҳ����

			char *tempData;
			GetData(pageHandle, &tempData);
			RID tempRid;
			tempRid.bValid = false;
			tempRid.pageNum = nodePage;
			insertKeyAndRidToPage(newRootPageHandle, order, fileHeader.attrType, fileHeader.attrLength, tempData, &tempRid, true);   //���µĸ��ڵ�����ӽڵ�Ĺؼ��ֺ�ָ��

			GetData(newLeafPageHandle, &tempData);
			tempRid.pageNum = newLeafPage;
			insertKeyAndRidToPage(newRootPageHandle, order, fileHeader.attrType, fileHeader.attrLength, tempData, &tempRid, true);  //���µĸ��ڵ�����ӽڵ�Ĺؼ��ֺ�ָ��

			indexHandle->fileHeader.rootPage = newRootPage;		//�޸�����������Ϣ�еĸ��ڵ�ҳ��
			free(newRootPageHandle);
		}
		else		//˵����ǰ���ѵĽڵ㲻�Ǹ��ڵ�
		{
			PageNum parentPage = index_NodeControlInfo->parent;
			copyNewNodeInfoToPage(newLeafPageHandle, nodePage, parentPage, index_NodeControlInfo->is_leaf, divide2);  //�����·��ѽڵ�Ľڵ������Ϣ
			copyNewNodeInfoToPage(pageHandle, newLeafPage, parentPage, index_NodeControlInfo->is_leaf, divide1);	//����ԭ�ڵ������Ϣ
			copyKeysAndRIDsToPage(newLeafPageHandle, keys + divide1*attrLength, attrLength, divide2, order, rids + divide1 * sizeof(RID));   //���ƹؼ��ֺ�ָ�뵽���Ѻ��µ�ҳ����

			char *tempData;
			GetData(newLeafPageHandle, &tempData);

			RID tempRid;
			tempRid.bValid = false;
			tempRid.pageNum = newLeafPage;

			GetThisPage(&fileHandle, parentPage, pageHandle);   //��pageHandleָ���丸�ڵ�ҳ��
			insertKeyAndRidToPage(pageHandle, order, fileHeader.attrType, fileHeader.attrLength, tempData, &tempRid, true);  //�򸸽ڵ�������ӽڵ�Ĺؼ��ֺ�ָ��

			GetData(pageHandle, &pageData);   //��pageDataָ�򸸽ڵ��������
			index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));   //���ڵ�Ľڵ������Ϣ
		}
		free(newLeafPageHandle);
	}

	free(pageHandle);

	return SUCCESS;
}

//������ɾ��
RC DeleteEntry(IX_IndexHandle *indexHandle, void *pData, RID * rid)
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	IX_FileHeader fileHeader = indexHandle->fileHeader;
	PF_PageHandle *pageHandle = NULL;

	//�����ļ�ҳ�������
	int order = fileHeader.order;

	//�����ؼ��ֵĳ���
	int attrLength = fileHeader.attrLength;

	//��ȡ���ڵ�ҳ��
	GetThisPage(&fileHandle, fileHeader.rootPage, pageHandle);

	//��ȡ���ڵ�ҳ���������
	char *pageData;
	GetData(pageHandle, &pageData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));

	//�жϽڵ��������Ҷ�ӽڵ�
	while (index_NodeControlInfo->is_leaf != 1)
	{
		RID tempRid;
		bool existence = false;
		findKeyAndRidForDelete(pageHandle, order, fileHeader.attrType, fileHeader.attrLength, pData, &tempRid, &existence);   //���ҹؼ��ֶ�Ӧ��ҳ��
		if (existence)
		{
			GetThisPage(&fileHandle, tempRid.pageNum, pageHandle);
			GetData(pageHandle, &pageData);
			index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
		}
		else
			return IX_EOF;
	}

	int keynum = index_NodeControlInfo->keynum;

	char *parentKeys;
	char *parentRids;
	int flag = 0;

	//��ȡ�ؼ�����
	parentKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	parentRids = parentKeys + order*attrLength;

	int position = 0;
	switch (fileHeader.attrType)
	{
	case chars:
		for (; position < keynum; position++) {
			if (strcmp(parentKeys + position*attrLength, (char*)pData) > 0)
				break;
			else if (strcmp((char*)pData, parentKeys + position*attrLength) == 0 && compareRid(rid, (RID*)(parentRids + position * sizeof(RID))))
			{
				flag = 1;
				break;
			}
		}
		break;
	case ints:
		int data1;
		data1 = *((int *)pData);
		for (; position < keynum; position++) {
			int data2 = *((int *)parentKeys + position*attrLength);
			if (data2 > data1)
				break;
			if (data1 == data2 && compareRid(rid, (RID*)(parentRids + position * sizeof(RID))))
			{
				flag = 1;
				break;
			}
		}
		break;
	case floats:
		float data_floats;
		data_floats = *((float *)pData);
		for (; position < keynum; position++) {
			float data2 = *((float *)parentKeys + position*attrLength);
			if (data2 > data_floats)
				break;
			if (data_floats == data2 && compareRid(rid, (RID*)(parentRids + position * sizeof(RID))))
			{
				flag = 1;
				break;
			}
		}
		break;
	}

	if (flag == 1)   //˵���ҵ���Ӧ�Ĺؼ��ֺͼ�¼
	{

		memcpy(parentKeys + position*attrLength, parentKeys + (position + 1)*attrLength, (keynum - position - 1)*attrLength);   //���ؼ�����ǰ�ƶ�һ��λ��
		memcpy(parentRids + position * sizeof(RID), parentRids + (position + 1) * sizeof(RID), (keynum - position - 1) * sizeof(RID));   //����¼ָ����ǰ�ƶ�һ����λ
		keynum--;   //�ؼ��ָ�����1
		index_NodeControlInfo->keynum = keynum;

		while (index_NodeControlInfo->parent != 0)   //˵��Ϊ�Ǹ��ڵ�
		{
			int status = 0;
			if (keynum < threshold)   //B+���ķǸ��ڵ�ķ�֧���������threshold����Ҫ���ֵܽڵ��һ���ڵ㣬�������ֵܽڵ���кϲ�
			{
				getFromBrother(pageHandle, &fileHandle, order, fileHeader.attrType, attrLength, &status);   //���ֵܽڵ���д���
			}

			PF_PageHandle *parentPageHandle = NULL;
			PageNum nodePageNum;

			GetThisPage(&fileHandle, index_NodeControlInfo->parent, parentPageHandle);
			GetPageNum(pageHandle, &nodePageNum);


			if (status == 2)   //˵������ڵ�����˺ϲ�
			{
				deleteChildNode(parentPageHandle, &fileHandle, order, fileHeader.attrType, attrLength, nodePageNum, true, NULL);   //����ɾ��
				pageHandle = parentPageHandle;    //ָ�򸸽ڵ�
				GetData(pageHandle, &pageData);
				index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
				//��ȡ�ؼ�����
				parentKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);   //���ڵ�Ĺؼ�����
				keynum = index_NodeControlInfo->keynum;
			}
			else if (status == 4)   //˵���ҽڵ㱻���кϲ���
			{
				if (position == 0)   //ͬʱ˵���ýڵ�ĵ�һ��Ԫ�ر�ɾ�����ˣ���Ҫ�޸�
				{
					deleteChildNode(parentPageHandle, &fileHandle, order, fileHeader.attrType, attrLength, nodePageNum, false, parentKeys);   //�ݹ�����޸�
					position = (position == 0 ? 1 : position);   //��ֹ�ٴ��ظ��ö���
				}

				GetThisPage(&fileHandle, index_NodeControlInfo->brother, pageHandle);   //��pageHandleָ���ҽڵ�
				GetData(pageHandle, &pageData);    //��ȡ�ҽڵ��������
				GetPageNum(pageHandle, &nodePageNum);   //��ȡ�ҽڵ��ҳ���

				index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
				GetThisPage(&fileHandle, index_NodeControlInfo->parent, parentPageHandle);    //��ȡ�ҽڵ�ĸ��ڵ�

				deleteChildNode(parentPageHandle, &fileHandle, order, fileHeader.attrType, attrLength, nodePageNum, true, NULL);    //�Ӹ��ڵ���ɾ���ҽڵ��Ӧ�Ĺؼ���

				pageHandle = parentPageHandle;    //ָ�򸸽ڵ�
				GetData(pageHandle, &pageData);
				index_NodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
				//��ȡ�ؼ�����
				parentKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);   //���ڵ�Ĺؼ�����
				keynum = index_NodeControlInfo->keynum;
			}
			else if (status == 1 || position == 0)   //����ڵ�裬����position=0��
			{
				deleteChildNode(parentPageHandle, &fileHandle, order, fileHeader.attrType, attrLength, nodePageNum, false, parentKeys);   //�ݹ�����޸�
				break;   //��ֹѭ��
			}

		}
		return FAIL;
	}
	else
		return FAIL;

}


//�򿪻���������ɨ��
RC OpenIndexScan(IX_IndexScan *indexScan, IX_IndexHandle *indexHandle, CompOp compOp, char *value)
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	IX_FileHeader fileHeader = indexHandle->fileHeader;


	indexScan->compOp = compOp;
	indexScan->value = value;
	indexScan->bOpen = true;
	indexScan->pIXIndexHandle = indexHandle;


	int ridIx = 0;
	PageNum startPage = 0;
	bool existence;

	switch (compOp)
	{
	case EQual:
		theFirstEqualScan(indexHandle, value, &startPage, &ridIx, &existence);
		if (existence)
			indexScan->pnNext = startPage;
		else
			indexScan->pnNext = 0;
		indexScan->ridIx = ridIx;
		break;
	case LEqual:
		indexScan->pnNext = fileHeader.first_leaf;
		indexScan->ridIx = ridIx;
		break;
	case NEqual:
		indexScan->pnNext = fileHeader.first_leaf;
		indexScan->ridIx = ridIx;
		break;
	case LessT:
		indexScan->pnNext = fileHeader.first_leaf;
		indexScan->ridIx = ridIx;
		break;
	case GEqual:
		theFirstEqualScan(indexHandle, value, &startPage, &ridIx, &existence);
		indexScan->pnNext = startPage;
		indexScan->ridIx = ridIx;
		break;
	case GreatT:
		theLEqualScan(indexHandle, value, &startPage, &ridIx);
		indexScan->pnNext = startPage;
		indexScan->ridIx = ridIx;
		break;
	case NO_OP:
		indexScan->pnNext = fileHeader.first_leaf;
		indexScan->ridIx = ridIx;
		break;
	default:
		break;
	}

	return SUCCESS;
}

//��ȡ��¼
RC IX_GetNextEntry(IX_IndexScan *indexScan, RID * rid)
{
	if (indexScan->bOpen)   //�ж�����ɨ���Ƿ��Ѿ���
	{
		PageNum pageNum = indexScan->pnNext;    //��ȡ���������ҳ��
		CompOp compOp = indexScan->compOp;   //�ȽϷ�
		IX_IndexHandle *indexHandle = indexScan->pIXIndexHandle;    //�����ļ�������
		char *value = indexScan->value;   //����ֵ
		int  ridIx = indexScan->ridIx;    //�����������������

		if (pageNum == 0)   //˵�������ڻ����Ѿ�ɨ�����
			rid = NULL;
		else
		{
			PF_FileHandle fileHandle = indexHandle->fileHandle;
			IX_FileHeader fileHeader = indexHandle->fileHeader;

			//�����ļ�ҳ�������
			int order = fileHeader.order;

			//�����ؼ��ֵĳ���
			int attrLength = fileHeader.attrLength;

			PF_PageHandle *pageHandle = NULL;
			GetThisPage(&fileHandle, pageNum, pageHandle);   //��ȡҳ��

			char *pageData;
			char *pageKeys;
			char *pageRids;
			GetData(pageHandle, &pageData);
			IX_Node* pageNodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
			int pageKeynum = pageNodeControlInfo->keynum;
			//��ȡ�ؼ�����
			pageKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
			//��ȡָ����
			pageRids = pageKeys + order*attrLength;

			if (ridIx == pageKeynum)   //˵���Ѿ�ɨ���굱ǰҳ��
			{
				if (pageNodeControlInfo->brother == 0)   //˵���Ѿ�ɨ�赽�ļ���ĩβ
				{
					rid = NULL;
					return IX_EOF;
				}
				else
				{
					pageNum = pageNodeControlInfo->brother;
					indexScan->pnNext = pageNum;   //�޸�ɨ��ṹ�������Ӧ��ҳ��

					GetThisPage(&fileHandle, pageNum, pageHandle);   //��ȡ��һ��Ҷ��ҳ��
					GetData(pageHandle, &pageData);
					pageNodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
					pageKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
					pageRids = pageKeys + order*attrLength;
					ridIx = 0;
				}
			}

			int temp = keyCompare(pageKeys + ridIx*attrLength, value, fileHeader.attrType);   //�ؼ��ֱȽ�
			switch (compOp)
			{
			case EQual:
				if (temp == 0)   //˵����Ȼ��������
				{
					memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
					return SUCCESS;
				}
				else
				{
					rid = NULL;
					return SUCCESS;
				}

				break;
			case LEqual:
				if (temp <= 0)   //˵����Ȼ��������
				{
					memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
					return SUCCESS;
				}
				else
				{
					rid = NULL;
					return SUCCESS;
				}
				break;
			case NEqual:
				if (temp != 0)   //˵����Ȼ��������
				{
					memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
					return SUCCESS;
				}
				else
				{
					rid = NULL;
					return SUCCESS;
				}
				break;
			case LessT:
				if (temp < 0)   //˵����Ȼ��������
				{
					memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
					return SUCCESS;
				}
				else
				{
					rid = NULL;
					return SUCCESS;
				}
				break;
			case GEqual:    //ֱ��ɨ�赽��󼴿�
				memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
				break;
			case GreatT:   //ֱ��ɨ��ֱ�����
				memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
				break;
			case NO_OP:    //ֱ��ɨ�赽��󼴿�
				memcpy(rid, pageRids + ridIx * sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;   //�޸�ɨ��ṹ������������
				break;
			}
		}
		return SUCCESS;
	}
	else {
		rid = NULL;
		return SUCCESS;
	}
}

//�ر�����ɨ��
RC CloseIndexScan(IX_IndexScan *indexScan)
{
	free(indexScan);
	indexScan = NULL;
	return SUCCESS;
}


void theFirstEqualScan(IX_IndexHandle *indexHandle, void *pData, PageNum *startPage, int *ridIx, bool *existence)     //�ҵ���һ���ؼ���������ڵ�ҳ��startPage��λ��ridIx
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	IX_FileHeader fileHeader = indexHandle->fileHeader;

	PF_PageHandle *pageHandle = NULL;

	//�����ļ�ҳ�������
	int order = fileHeader.order;
	//�����ؼ��ֵĳ���
	int attrLength = fileHeader.attrLength;
	//�ؼ�������
	AttrType attrType = fileHeader.attrType;

	char *pageData;
	char *pKeys;
	char *pRids;
	//��ȡ���ڵ�ҳ��
	GetThisPage(&fileHandle, fileHeader.rootPage, pageHandle);

	//��ȡ���ڵ�ҳ���������
	GetData(pageHandle, &pageData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* nodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));

	int keynum = nodeControlInfo->keynum;

	int position;
	int flag;
	while (true)   //���±���ֱ��Ҷ�ӽڵ�
	{
		//��ȡ�ؼ�����
		pKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
		//��ȡָ����
		pRids = pKeys + order*attrLength;

		for (position = 0, flag = 0; position < keynum; position++)
		{
			int temp = keyCompare(pKeys + position*attrLength, pData, attrType);
			if (temp == 0)
			{
				flag = 1;
				break;
			}
			else if (temp > 0)
				break;
		}
		if (flag == 0 && nodeControlInfo->is_leaf == 0)     //˵��������Ҷ�ӽڵ�
		{
			if (position > 0)
				position--;
		}
		else if (nodeControlInfo->is_leaf == 1)   //˵����ǰ�ڵ��Ѿ���Ҷ�ӽڵ�
		{
			if (flag == 1)
				*existence = true;
			else
				*existence = false;

			GetPageNum(pageHandle, startPage);  //˵���ҵ���һ���ؼ�����Ȼ��ߵ�һ�����ڵĹؼ���
			*ridIx = position;
			return;
		}
		RID *tempRid = (RID*)(pRids + position * sizeof(RID));
		GetThisPage(&fileHandle, tempRid->pageNum, pageHandle);   //��ȡ���ӽڵ�ҳ��
		GetData(pageHandle, &pageData);   //��ȡ�ӽڵ�������
		nodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
		keynum = nodeControlInfo->keynum;
	}

}

//����ɨ��
void theLEqualScan(IX_IndexHandle *indexHandle, void *pData, PageNum *startPage, int *ridIx)    //�ҵ���һ���ؼ��ִ������ڵ�ҳ��startPage��λ��ridIx
{
	PF_FileHandle fileHandle = indexHandle->fileHandle;
	IX_FileHeader fileHeader = indexHandle->fileHeader;

	PF_PageHandle *pageHandle = NULL;

	//�����ļ�ҳ�������
	int order = fileHeader.order;
	//�����ؼ��ֵĳ���
	int attrLength = fileHeader.attrLength;
	//�ؼ�������
	AttrType attrType = fileHeader.attrType;

	char *pageData;
	char *pKeys;
	char *pRids;
	//��ȡ���ڵ�ҳ��
	GetThisPage(&fileHandle, fileHeader.rootPage, pageHandle);

	//��ȡ���ڵ�ҳ���������
	GetData(pageHandle, &pageData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* nodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));

	int keynum = nodeControlInfo->keynum;

	int position;
	int flag;
	while (true)   //���±���ֱ��Ҷ�ӽڵ�
	{
		//��ȡ�ؼ�����
		pKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
		//��ȡָ����
		pRids = pKeys + order*attrLength;

		for (position = keynum - 1, flag = 0; position >= 0; position--)   //�Ӻ���ǰ����
		{
			int temp = keyCompare(pKeys + position*attrLength, pData, attrType);
			if (temp <= 0)
				break;
		}

		if (nodeControlInfo->is_leaf == 1)   //˵����ǰ�ڵ��Ѿ���Ҷ�ӽڵ�
		{
			position++;
			*ridIx = position;
			GetPageNum(pageHandle, startPage);
			return;
		}
		RID *tempRid = (RID*)(pRids + position * sizeof(RID));
		GetThisPage(&fileHandle, tempRid->pageNum, pageHandle);   //��ȡ���ӽڵ�ҳ��
		GetData(pageHandle, &pageData);   //��ȡ�ӽڵ�������
		nodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
		keynum = nodeControlInfo->keynum;
	}

}


//�ؼ��ֵıȽ�
int keyCompare(void *data1, void *data2, AttrType attrType)
{
	int temp;
	switch (attrType)
	{
	case chars:
		temp = strcmp((char*)data1, (char*)data2);
		break;
	case ints:
		int tempData1;
		tempData1 = *((int *)data1);
		int tempData2;
		tempData2 = *((int *)data2);
		if (tempData1 > tempData2)
			temp = 1;
		else if (tempData1 < tempData2)
			temp = -1;
		else
			temp = 0;
		break;
	case floats:
		float tempData3;
		tempData3 = *((float *)data1);
		float tempData4;
		tempData4 = *((float *)data2);
		if (tempData3 > tempData4)
			temp = 1;
		else if (tempData3 < tempData4)
			temp = -1;
		else
			temp = 0;
		break;
	}
	return temp;
}


//���ֵܽڵ��н�ڵ���ߺϲ�
void getFromBrother(PF_PageHandle *pageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, int *status)
{
	PageNum leftPageNum;
	//int status;
	findLeftBrother(pageHandle, fileHandle, order, attrType, attrLength, &leftPageNum);    //���ȴ����ֵܽڵ㴦��
	if (leftPageNum != 0)   //������ֵܽڵ���ڣ������ֵܽ��д���
	{
		PF_PageHandle *leftHandle = NULL;
		GetThisPage(fileHandle, leftPageNum, leftHandle);
		getFromLeft(pageHandle, leftHandle, order, attrType, attrLength, status);   //�����ֵܽ��д���
	}
	else   //˵���ڵ�����ֵܽڵ㲻���ڣ������ֵܽ��д���
	{
		PF_PageHandle *rightHandle = NULL;
		char *tempData;
		GetData(pageHandle, &tempData);
		IX_Node* tempNodeControlInfo = (IX_Node*)(tempData + sizeof(IX_FileHeader));
		GetThisPage(fileHandle, tempNodeControlInfo->brother, rightHandle);
		getFromRight(pageHandle, rightHandle, order, attrType, attrLength, status);  //�����ֵܽ��д���


		PF_PageHandle *parentPageHandle = NULL;
		GetData(rightHandle, &tempData);
		tempNodeControlInfo = (IX_Node*)(tempData + sizeof(IX_FileHeader));
		GetThisPage(fileHandle, tempNodeControlInfo->parent, parentPageHandle);
		PageNum nodePageNum;
		GetPageNum(rightHandle, &nodePageNum);
		char *tempKeys = tempData + sizeof(IX_FileHeader) + sizeof(IX_Node);

		if (*status == 3)   //˵�������ֵܽ�һ���ؼ���
		{
			deleteChildNode(parentPageHandle, fileHandle, order, attrType, attrLength, nodePageNum, false, tempKeys);  //�ݹ��޸����ֵܽڵ�
		}
	}
}


//�����ֵܽڵ���д���
void getFromLeft(PF_PageHandle *pageHandle, PF_PageHandle *leftHandle, int order, AttrType attrType, int attrLength, int *status)
{

	char *pageData;
	char *pageKeys;
	char *pageRids;

	char *leftData;
	char *leftKeys;
	char *leftRids;

	GetData(leftHandle, &leftData);
	//��ȡ��ڵ�ҳ��ýڵ������Ϣ
	IX_Node* leftNodeControlInfo = (IX_Node*)(leftData + sizeof(IX_FileHeader));
	//��ȡ�ؼ�����
	leftKeys = leftData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	leftRids = leftKeys + order*attrLength;

	GetData(pageHandle, &pageData);
	IX_Node* pageNodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
	int pageKeynum = pageNodeControlInfo->keynum;
	//��ȡ�ؼ�����
	pageKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	pageRids = pageKeys + order*attrLength;

	int leftKeynum = leftNodeControlInfo->keynum;
	if (leftKeynum > threshold)   //˵�����Խ��ȥ
	{

		memcpy(pageKeys + attrLength, pageKeys, pageKeynum*attrLength);   //�ؼ����������
		memcpy(pageRids + sizeof(RID), pageRids, pageKeynum * sizeof(RID));   //�ؼ���ָ���������

		memcpy(pageKeys, leftKeys + (leftKeynum - 1)*attrLength, attrLength);  //������ڵ�����һ���ؼ���
		memcpy(pageRids, leftRids + (leftKeynum - 1) * sizeof(RID), sizeof(RID));  //������ڵ����һ���ؼ���ָ��

		leftKeynum--;
		pageKeynum++;
		leftNodeControlInfo->keynum = leftKeynum;    //�޸Ĺؼ��ָ���
		pageNodeControlInfo->keynum = pageKeynum;   //�޸Ĺؼ��ָ���
		*status = 1;

	}
	else   //˵�����ܽ裬ֻ�ܽ��кϲ�
	{
		memcpy(leftKeys + leftKeynum*attrLength, pageKeys, pageKeynum*attrLength);   //�ؼ������帴�Ƶ���ڵ���
		memcpy(leftRids + leftKeynum * sizeof(RID), pageRids, pageKeynum * sizeof(RID));   //�ؼ���ָ�����帴�Ƶ���ڵ���
		leftKeynum = leftKeynum + pageKeynum;
		pageKeynum = 0;
		leftNodeControlInfo->keynum = leftKeynum;    //�޸Ĺؼ��ָ���
		pageNodeControlInfo->keynum = pageKeynum;   //�޸Ĺؼ��ָ���
		leftNodeControlInfo->brother = pageNodeControlInfo->brother;    //�޸�Ҷ��ҳ������ָ��
		*status = 2;
	}

}


//�����ֵܽڵ���д���
void getFromRight(PF_PageHandle *pageHandle, PF_PageHandle *rightHandle, int order, AttrType attrType, int attrLength, int *status)
{
	char *pageData;
	char *pageKeys;
	char *pageRids;

	char *rightData;
	char *rightKeys;
	char *rightRids;

	GetData(pageHandle, &pageData);
	IX_Node* pageNodeControlInfo = (IX_Node*)(pageData + sizeof(IX_FileHeader));
	int pageKeynum = pageNodeControlInfo->keynum;
	//��ȡ�ؼ�����
	pageKeys = pageData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	pageRids = pageKeys + order*attrLength;



	GetData(rightHandle, &rightData);
	//��ȡy�ڵ�ҳ��ýڵ������Ϣ
	IX_Node* rightNodeControlInfo = (IX_Node*)(rightData + sizeof(IX_FileHeader));
	//��ȡ�ؼ�����
	rightKeys = rightData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	rightRids = rightKeys + order*attrLength;
	PageNum rightPageNum;
	GetPageNum(rightHandle, &rightPageNum);



	int rightKeynum = rightNodeControlInfo->keynum;
	if (rightKeynum > threshold)   //˵�����Խ��ȥ
	{

		memcpy(pageKeys + pageKeynum*attrLength, rightKeys, attrLength);  //�����ҽڵ�ĵ�һ���ؼ���
		memcpy(pageRids + pageKeynum * sizeof(RID), rightRids, sizeof(RID));  //�����ҽڵ�ĵ�һ���ؼ���ָ��


		memcpy(rightKeys, rightKeys + attrLength, (rightKeynum - 1) *attrLength);   //�ؼ�������ǰ��һ��λ��
		memcpy(rightRids, rightRids + sizeof(RID), (rightKeynum - 1) * sizeof(RID));   //�ؼ���ָ������ǰ��һ��λ��

		rightKeynum--;
		pageKeynum++;
		rightNodeControlInfo->keynum = rightKeynum;    //�޸Ĺؼ��ָ���
		pageNodeControlInfo->keynum = pageKeynum;   //�޸Ĺؼ��ָ���
		*status = 3;
	}
	else   //˵�����ܽ裬ֻ�ܽ��кϲ�
	{
		memcpy(pageKeys + pageKeynum*attrLength, rightKeys, rightKeynum*attrLength);  //�����ҽڵ�����йؼ���
		memcpy(pageRids + pageKeynum * sizeof(RID), rightRids, rightKeynum * sizeof(RID));  //�����ҽڵ�����йؼ���ָ��

		pageKeynum = rightKeynum + pageKeynum;
		rightKeynum = 0;
		rightNodeControlInfo->keynum = rightKeynum;    //�޸Ĺؼ��ָ���
		pageNodeControlInfo->keynum = pageKeynum;   //�޸Ĺؼ��ָ���
		*status = 4;

		pageNodeControlInfo->brother = rightNodeControlInfo->brother;   //�޸�ҳ������ָ��
	}
}


//�򵥵ش�ĳ�����ڵ�ҳ����ɾ��ĳ����֧�����޸Ĺؼ���
void deleteChildNode(PF_PageHandle *parentPageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, PageNum nodePageNum, bool deleteIfTrue, void *pData)
{
	char *parentData;
	char *parentKeys;
	char *parentRids;
	while (true)
	{
		GetData(parentPageHandle, &parentData);
		IX_Node* nodeControlInfo = (IX_Node*)(parentData + sizeof(IX_FileHeader));
		int keynum = nodeControlInfo->keynum;
		//��ȡ�ؼ�����
		parentKeys = parentData + sizeof(IX_FileHeader) + sizeof(IX_Node);
		//��ȡָ����
		parentRids = parentKeys + order*attrLength;
		for (int i = 0;; i++)
		{
			RID *tempRid = (RID*)parentRids + i * sizeof(RID);
			if (tempRid->pageNum == nodePageNum)
			{
				if (deleteIfTrue)
				{
					//�Թؼ��ֺ�ָ����и���ɾ��
					memcpy(parentKeys + i*attrLength, parentKeys + (i + 1)*attrLength, (keynum - i - 1)*attrLength);
					memcpy(parentRids + i * sizeof(RID), parentRids + (i + 1) * sizeof(RID), (keynum - i - 1) * sizeof(RID));
					keynum--;
					nodeControlInfo->keynum = keynum;
					return;
				}
				else
				{
					//�޸Ĺؼ���
					memcpy(parentKeys + i*attrLength, pData, attrLength);
					if (i == 0 && nodeControlInfo->parent != 0)   //˵���޸ĵĹؼ���Ϊ��һ������Ҫ�ݹ�ؽ����޸�
					{
						GetPageNum(parentPageHandle, &nodePageNum);
						GetThisPage(fileHandle, nodeControlInfo->parent, parentPageHandle);   //�ݹ�ؽ����޸�
					}
					else
						return;
				}
			}
		}
	}
}


//�ҳ���ǰ�ڵ�����ֵܽڵ�
void findLeftBrother(PF_PageHandle *pageHandle, PF_FileHandle *fileHandle, int order, AttrType attrType, int attrLength, PageNum *leftBrother)
{

	char *data;
	PageNum nowPage;
	GetPageNum(pageHandle, &nowPage);   //��ȡ��ǰҳ���
	GetData(pageHandle, &data);
	IX_Node* nodeControlInfo = (IX_Node*)(data + sizeof(IX_FileHeader));

	PF_PageHandle *parentPageHandle = NULL;
	GetThisPage(fileHandle, nodeControlInfo->parent, parentPageHandle);
	char *parentData;
	char *parentKeys;
	char *parentRids;

	GetData(parentPageHandle, &parentData);
	//��ȡ�ؼ�����
	parentKeys = parentData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	parentRids = parentKeys + order*attrLength;
	for (int i = 0;; i++)
	{
		RID *tempRid = (RID*)parentRids + i * sizeof(RID);
		if (tempRid->pageNum == nowPage)
		{
			if (i != 0)
			{
				i--;
				tempRid = (RID*)parentRids + i * sizeof(RID);
				*leftBrother = tempRid->pageNum;
			}
			else
				*leftBrother = 0;
			return;
		}
	}
}


//RID�ıȽ�
bool compareRid(RID *src, RID *des)
{
	if ((src->bValid == des->bValid) && (src->pageNum == des->pageNum) && (src->slotNum == des->slotNum))
		return true;
	else
		return false;
}


//ɾ��ǰ���ҽڵ��еĹؼ���
void findKeyAndRidForDelete(PF_PageHandle *pageHandle, int order, AttrType attrType, int attrLength, void *pData, RID *rid, bool *existence)
{
	char *parentData;
	char *parentKeys;
	char *parentRids;
	int flag = 0;

	GetData(pageHandle, &parentData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* nodeControlInfo = (IX_Node*)(parentData + sizeof(IX_FileHeader));
	int keynum = nodeControlInfo->keynum;

	//��ȡ�ؼ�����
	parentKeys = parentData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	parentRids = parentKeys + order*attrLength;

	int position = 0;
	switch (attrType)
	{
	case chars:
		for (; position < keynum; position++) {
			if (strcmp(parentKeys + position*attrLength, (char*)pData) >= 0)
			{
				if (strcmp((char*)pData, parentKeys + position*attrLength) == 0)
					flag = 1;
				break;
			}
		}
		break;
	case ints:
		int data1;
		data1 = *((int *)pData);
		for (; position < keynum; position++) {
			int data2 = *((int *)parentKeys + position*attrLength);
			if (data2 >= data1)
			{
				if (data1 == data2)
					flag = 1;
				break;
			}
		}
		break;
	case floats:
		float data_floats = *((float *)pData);
		for (; position < keynum; position++) {
			float data2 = *((float *)parentKeys + position*attrLength);
			if (data2 >= data_floats)
			{
				if (data_floats == data2)
					flag = 1;
				break;
			}
		}
		break;
	}
	if (flag == 1)
	{
		*existence = true;
		memcpy(rid, parentRids + position * sizeof(RID), sizeof(RID));
	}
	else
	{
		position--;
		if (position < 0)
			*existence = false;
		else
		{
			*existence = true;
			memcpy(rid, parentRids + position * sizeof(RID), sizeof(RID));
		}
	}
}


//������ҳ��ڵ������Ϣ
void copyNewNodeInfoToPage(PF_PageHandle *pageHandle, PageNum brother, PageNum parent, int is_leaf, int keynum)
{
	IX_Node newNodeInfo;
	newNodeInfo.brother = brother;
	newNodeInfo.parent = parent;
	newNodeInfo.is_leaf = is_leaf;
	newNodeInfo.keynum = keynum;
	char *pData;
	GetData(pageHandle, &pData);
	memcpy(pData + sizeof(IX_FileHeader), &newNodeInfo, sizeof(IX_Node));
}


//���Ʒ��Ѻ�Ĺؼ��ֺ�ָ�뵽��ҳ��
void  copyKeysAndRIDsToPage(PF_PageHandle *pageHandle, void *keySrc, int attrLength, int num, int order, void *ridSrc)
{
	char *pData;
	GetData(pageHandle, &pData);
	pData = pData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	memcpy(pData, keySrc, num*attrLength);
	pData = pData + order*attrLength;
	memcpy(pData, ridSrc, num * sizeof(RID));
}


//�������Ĺؼ��ֺ�ָ��д��ָ����ҳ��
void insertKeyAndRidToPage(PF_PageHandle *pageHandle, int order, AttrType attrType, int attrLength, void *pData, RID *rid, bool insertIfTrue)
{

	char *parentData;
	char *parentKeys;
	char *parentRids;

	GetData(pageHandle, &parentData);

	//��ȡ���ڵ�ҳ��ýڵ������Ϣ
	IX_Node* nodeControlInfo = (IX_Node*)(parentData + sizeof(IX_FileHeader));
	int keynum = nodeControlInfo->keynum;

	//��ȡ�ؼ�����
	parentKeys = parentData + sizeof(IX_FileHeader) + sizeof(IX_Node);
	//��ȡָ����
	parentRids = parentKeys + order*attrLength;

	int position = 0;
	switch (attrType)
	{
	case chars:
		for (; position < keynum; position++) {
			if (strcmp(parentKeys + position*attrLength, (char*)pData) > 0)
				break;
		}
		break;
	case ints:
		int data1;
		data1 = *((int *)pData);
		for (; position < keynum; position++) {
			int data2 = *((int *)parentKeys + position*attrLength);
			if (data2 > data1)
				break;
		}
		break;
	case floats:
		float data_floats = *((float *)pData);
		for (; position < keynum; position++) {
			float data2 = *((float *)parentKeys + position*attrLength);
			if (data2 > data_floats)
				break;
		}
		break;
	}
	if (insertIfTrue)
	{
		memcpy(parentKeys + (position + 1)*attrLength, parentKeys + position*attrLength, (keynum - position)*attrLength);
		memcpy(parentKeys + position*attrLength, pData, attrLength);
		//����ؼ��ֵ�ָ��
		memcpy(parentRids + (position + 1) * sizeof(RID), parentRids + position * sizeof(RID), (keynum - position) * sizeof(RID));
		memcpy(parentRids + position * sizeof(RID), rid, sizeof(RID));
		keynum++;
		nodeControlInfo->keynum = keynum;
	}
	else
	{
		position--;
		if (position < 0)    //�ؼ��ֽ�����뵽��һ���ؼ��ִ�
		{
			position = 0;   //���뵽��ǰ���ҳ��
			memcpy(parentKeys, pData, attrLength);   //�޸���ָ��ҳ�����С�ؼ���
		}
		memcpy(rid, parentRids + position * sizeof(RID), sizeof(RID));
	}
}