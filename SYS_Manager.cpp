#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include "HustBaseDoc.h"
#include <iostream>

/*ִ��sql��䲢��Ӧ������ʾ��Ϣ��δ��*/
void ExecuteAndMessage(char *sql, CEditArea* editArea, CHustBaseDoc* pDoc)
{//����ִ�е���������ڽ�������ʾִ�н�����˺������޸�
	std::string s_sql = sql;
	if (s_sql.find("select") == 0) {
		SelResult res;
		SelResult *temp = &res;//�����ۼӲ�ѯ����������ı���ָ��

		Init_Result(&res);
		Query(sql, &res);//ִ��Query��ѯ

		pDoc->selColNum = res.col_num;//�����ǲ�ѯ���������
		pDoc->selRowNum = 0;//�����ǲ�ѯ�����������ע���ͷͬ����Ҫһ��;
							//����ʾ���ս��ʱ�����Բ�ѯ������˴���������0����1��ִ��ʱ�������
		temp = &res;
		while (temp)
		{
			pDoc->selRowNum += res.row_num;
			temp = temp->next_res;
		}
		//Ȼ����˳�򣬽���ѯ������ַ�������ʽ������pDoc->selResult[i][j]����ͷ��Ϣͬ����Ҫ����������
		pDoc->isEdit = 1;//����ò�ѯ����󽫸ñ�־λ��Ϊ1�������϶����ʱ�Բ�ѯ��������ػ档

						 //���б�ͷ
		char **fields = new char*[20];
		for (int i = 0; i < pDoc->selColNum; i++)
		{
			fields[i] = new char[20];
			memset(fields[i], '\0', 20);//�ÿ�
			memcpy(fields[i], res.fields[i], 20);
		}

		//��ѯ��õĽ��
		temp = &res;
		char ***Result = new char**[pDoc->selRowNum];
		for (int i = 0; i < pDoc->selRowNum; i++)
		{
			Result[i] = new char*[pDoc->selColNum];//���һ����¼

			int x;
			memcpy(&x, &(**temp->res[i]) + 18, sizeof(int));
			memcpy(&x, &(**temp->res[i]) + 22, sizeof(int));

			for (int j = 0; j < pDoc->selColNum; j++)
			{
				Result[i][j] = new char[20];
				memset(Result[i][j], '\0', 20);

				memcpy(Result[i][j], (**(temp->res + i)) + temp->offset[j], temp->length[j]);
				if (temp->attrType[j] == ints) {
					int x;
					memcpy(&x, Result[i][j], 4);
					sprintf(Result[i][j], "%d", x);
				}
				else if (temp->attrType[j] == floats) {
					float x;
					memcpy(&x, Result[i][j], 4);
					sprintf(Result[i][j], "%.3f", x);
				}
				//memcpy(Result[i][j], temp->res + temp->offset[j], sizeof(temp->attrType[j]));
			}//Ҫ���ݽṹ�о����ֶε����ͺ�ƫ�������������ֶδ�һ����¼�в�ֳ���
			if (i == 99)temp = temp->next_res;//һ���ṹ�������100����¼
		}

		//��ʾ
		editArea->ShowSelResult(pDoc->selColNum, pDoc->selRowNum, fields, Result);

		//�ͷ��ڴ�ռ�
		for (int i = 0; i<pDoc->selColNum; i++) {
			delete[] fields[i];
		}
		delete[] fields;

		for (int i = 0; i<pDoc->selRowNum; i++) {
			for (int j = 0; j < pDoc->selColNum; j++)
				delete[] Result[i][j];
		}
		delete[] Result;
		Destory_Result(&res);
		return;
	}

	RC rc = execute(sql,pDoc);
	int row_num = 0;
	char **messages;
	switch (rc) {
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "Successful Execution!";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "Syntax Error!";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "Function Not Implemented!";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	}

}

/**/
bool CanButtonClick()
{//��Ҫ����ʵ��
 //�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;

}

/*
��·��dbPath�´���һ����ΪdbName�Ŀտ⣬������Ӧ��ϵͳ�ļ�
�о���������е�dbname����û����
*/
RC CreateDB(char *dbpath, char *dbname)
{
	RC rc;
	//����ϵͳ���ļ���ϵͳ���ļ�
	SetCurrentDirectory(dbpath);//�������ļ���·����λ�����ݿ��·��
	//rc = RM_CreateFile("SYSTABLES", sizeof(SysTable));
	rc = RM_CreateFile("SYSTABLES", 25);
	if (rc != SUCCESS)
	{
		return rc;
	}
	//rc = RM_CreateFile("SYSCOLUMNS", sizeof(SysColumns));//ʹ�ü�¼�ļ�
	rc = RM_CreateFile("SYSCOLUMNS", 76);//ʹ�ü�¼�ļ�
	if (rc != SUCCESS)
	{
		return rc;
	}
	return SUCCESS;
}

//ɾ��һ���ǿ��ļ�������������
void myDeleteDirectory(CString dir_path)
{
	CFileFind tmpFind;
	CString path;

	path.Format("%s/*.*", dir_path);
	BOOL bWorking = tmpFind.FindFile(path);
	while (bWorking)
	{
		bWorking = tmpFind.FindNextFile();
		if (tmpFind.IsDirectory() && !tmpFind.IsDots()) {
			myDeleteDirectory(tmpFind.GetFilePath());//�ݹ�ɾ��
			RemoveDirectory(tmpFind.GetFilePath());
		}//�������ļ���
		else {
			DeleteFile(tmpFind.GetFilePath());
		}//�����ļ�
	}

}

/*ɾ��һ�����ݿ⣬dbNameΪҪɾ�������ݿ��Ӧ�ļ��е�·����*/
RC DropDB(char *dbname)
{
	myDeleteDirectory(dbname);
	if (RemoveDirectory(dbname))
		AfxMessageBox("Delete Database Successfully��");//ɾ�������ݿ��ļ���,���ú���ֻ��ɾ�����ļ���
	return SUCCESS;
}

/*�ı�ϵͳ�ĵ�ǰ���ݿ�ΪdbName��Ӧ���ļ����е����ݿ�*/
RC OpenDB(char *dbname)
{
	return SUCCESS;
}

/*�رյ�ǰ���ݿ⡣�ò������رյ�ǰ���ݿ��д򿪵������ļ����ر��ļ��������Զ�ʹ������صĻ�����ҳ����µ�����
δʵ��
*/
RC CloseDB()
{
	return SUCCESS;
}

/*ִ��һ����SELECT֮���SQL��䣬���ִ�гɹ�������SUCCESS�����򷵻ش�����*/
RC execute(char * sql, CHustBaseDoc* pDoc) {
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX

	if (rc == SUCCESS)
	{
		//int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////�ж�SQL���Ϊselect���
			//break;
		case 2:
			//�ж�SQL���Ϊinsert���
			Insert(sql_str->sstr.ins.relName, sql_str->sstr.ins.nValues, sql_str->sstr.ins.values);
			pDoc->m_pTreeView->PopulateTree();
			break;
		case 3:
			//�ж�SQL���Ϊupdate���
			updates up = sql_str->sstr.upd;
			Update(up.relName, up.attrName, &up.value, up.nConditions, up.conditions);
			break;

		case 4:
			//�ж�SQL���Ϊdelete���
			Delete(sql_str->sstr.del.relName, sql_str->sstr.del.nConditions, sql_str->sstr.del.conditions);
			break;
			
		case 5:
			//�ж�SQL���ΪcreateTable���
			CreateTable(sql_str->sstr.cret.relName, sql_str->sstr.cret.attrCount, sql_str->sstr.cret.attributes);
			pDoc->m_pTreeView->PopulateTree(); //������ͼ
			//pDoc->m_pListView->displayTabInfo(sql_str->sstr.cret.relName);//�Ҳ�ˢ�±���

			break;

		case 6:
			//�ж�SQL���ΪdropTable���
			DropTable(sql_str->sstr.cret.relName);
			pDoc->m_pTreeView->PopulateTree(); //������ͼ
			//pDoc->m_pListView->displayTabInfo(sql_str->sstr.cret.relName);//�Ҳ�ˢ�±���

			break;

		case 7:
			//�ж�SQL���ΪcreateIndex���
			//CreateIndex(sql_str->sstr.crei.indexName, AttrType attrType, int attrLength);
			break;

		case 8:
			//�ж�SQL���ΪdropIndex���
			break;

		case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ
			AfxMessageBox("������������");
			break;

		case 10:
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
			AfxMessageBox("��ӭ�´�ʹ�ã�");
			AfxGetMainWnd()->SendMessage(WM_CLOSE);//�رմ���
			//DestroyWindow();
			break;
		}

		return SUCCESS;
	}
	else {
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
}

/*����һ����ΪrelName�ı�����attrCount��ʾ��ϵ�����Ե�������ȡֵΪ1��MAXATTRS֮�䣩������attributes��һ������ΪattrCount�����顣
�����¹�ϵ�е�i�����ԣ�attributes�����еĵ�i��Ԫ�ذ������ơ����ͺ����Եĳ��ȣ���AttrInfo�ṹ���壩*/
RC CreateTable(char *relName, int attrCount, AttrInfo *attributes)
{
	/*
	typedef struct _ AttrInfo AttrInfo;
	struct _AttrInfo {
	char			*attrName;			// ���Ե�����
	AttrType	attrType;			// ���Ե�����
	int			attrLength;			// ���Եĳ���
	};

	*/

	RC rc;
	//char *pData;
	RM_FileHandle *tab_Handle,*col_Handle;
	RID *rid;
	int recordsize=0;//���ݱ��ÿ����¼�Ĵ�С

	//��ϵͳ���ļ���ϵͳ���ļ�
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_Handle->bOpen = false;
	col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	col_Handle->bOpen = false;
	rid = (RID *)malloc(sizeof(RID));
	rid->bValid = false;

	rc = RM_OpenFile("SYSTABLES", tab_Handle);
	if (rc != SUCCESS)
		return rc;
	rc = RM_OpenFile("SYSCOLUMNS", col_Handle);
	if (rc != SUCCESS)
		return rc;

	//��ϵͳ���в����¼
	char *pData = (char *)malloc(sizeof(SysTable));
	//SysTable sysTable;
	//strncpy(sysTable.tablename, relName, 21);
	//sysTable.attrcount = attrCount;
	
	//memcpy(pData, &sysTable, sizeof(SysTable));
	memcpy(pData, relName, 21);//������
	memcpy(pData + 21, &attrCount, sizeof(int));//�����������
	rc = InsertRec(tab_Handle, pData, rid);
	if (rc != SUCCESS)return rc;
	rc = RM_CloseFile(tab_Handle);
	if (rc != SUCCESS)return rc;
	free(tab_Handle);
	//free(pData);
	free(rid);

	//��ϵͳ����ѭ�������Ϣ һ�����а�����������У�����Ҫѭ��
	for (int i = 0, offset = 0; i < attrCount; i++){
		pData = (char *)malloc(sizeof(SysColumns));
		memcpy(pData, relName, 21);
		memcpy(pData + 21, (attributes+i)->attrName, 21);
		memcpy(pData + 42, &((attributes + i)->attrType), sizeof(int));
		memcpy(pData + 42 + sizeof(int), &((attributes + i)->attrLength), sizeof(int));
		memcpy(pData + 42 + 2*sizeof(int), &offset, sizeof(int));
		memcpy(pData + 42 + 3 * sizeof(int), "0", sizeof(char));
		rid = (RID *)malloc(sizeof(RID));
		rid->bValid = false;
		rc = InsertRec(col_Handle, pData, rid);
		if (rc != SUCCESS){
			return rc;
		}
		free(pData);
		free(rid);
		offset += (attributes + i)->attrLength;
	}
	rc = RM_CloseFile(col_Handle);
	if (rc != SUCCESS)return rc;
	free(col_Handle);

	//�������ݱ�
	for (int i = 0; i < attrCount; i++){
		recordsize += (attributes + i)->attrLength;
	}
	rc = RM_CreateFile(relName, recordsize);
	if (rc != SUCCESS)return rc;
	return SUCCESS;
}

/*������ΪrelName�ı��Լ��ڸñ��Ͻ�������������*/
RC DropTable(char *relName)
{
	CFile tmp;
	RM_FileHandle *tab_Handle, *col_Handle;
	RC rc;
	RM_FileScan *FileScan;
	RM_Record *tab_rec, *col_rec;

	//��ϵͳ���ϵͳ���ж�Ӧ�����ؼ�¼ɾ��
	//��ϵͳ��ϵͳ���ļ�
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_Handle->bOpen = false;
	col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	col_Handle->bOpen = false;
	tab_rec = (RM_Record*)malloc(sizeof(RM_Record));
	tab_rec->bValid = false;
	col_rec = (RM_Record*)malloc(sizeof(RM_Record));
	col_rec->bValid = false;

	rc = RM_OpenFile("SYSTABLES", tab_Handle);
	if (rc != SUCCESS) return rc;
	rc = RM_OpenFile("SYSCOLUMNS", col_Handle);
	if (rc != SUCCESS) return rc;

	FileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	FileScan->bOpen = false;
	rc = OpenScan(FileScan, tab_Handle, 0, NULL);
	if (rc != SUCCESS) return rc;
	//ѭ�����ұ���ΪrelName��Ӧ��ϵͳ���еļ�¼,������¼��Ϣ������tab_rec��
	while(GetNextRec(FileScan, tab_rec) == SUCCESS){
		if (strcmp(relName, tab_rec->pData) == 0){
			DeleteRec(tab_Handle, &(tab_rec->rid));
			break;
		}
	}
	CloseScan(FileScan);

	FileScan->bOpen = false;
	rc = OpenScan(FileScan, col_Handle, 0, NULL);
	if (rc != SUCCESS)
		return rc;
	//ѭ�����ұ���ΪrelName��Ӧ��ϵͳ���еļ�¼,ɾ��
	while(GetNextRec(FileScan, col_rec) == SUCCESS){
		if (strcmp(relName, col_rec->pData) == 0){
			DeleteRec(col_Handle, &(col_rec->rid));
			//break;  
		}
	}
	CloseScan(FileScan);
	free(FileScan);


	//�ر��ļ����
	rc = RM_CloseFile(tab_Handle);if (rc != SUCCESS)return rc;
	free(tab_Handle);
	rc = RM_CloseFile(col_Handle);if (rc != SUCCESS)return rc;
	free(col_Handle);

	free(tab_rec);
	free(col_rec);
	
	DeleteFile((LPCTSTR)relName);//ɾ�����ݱ��ļ�
	return SUCCESS;
}

/*�ú����ڹ�ϵrelName������attrName�ϴ�����ΪindexName���������������ȼ���ڱ���������Ƿ��Ѿ�����һ��������������ڣ�
�򷵻�һ������Ĵ����롣���򣬴�����������
���������Ĺ����������ٴ������������ļ��������ɨ�豻�����ļ�¼�����������ļ��в���������۹ر�����*/
RC CreateIndex(char *indexName, char *relName, char *attrName)
{
	return SUCCESS;
}

/*�ú�������ɾ����ΪindexName���������������ȼ�������Ƿ���ڣ���������ڣ��򷵻�һ������Ĵ����롣�������ٸ�����*/
RC DropIndex(char *indexName)
{
	return SUCCESS;
}

/*�ú���������relName���в������ָ������ֵ����Ԫ�飬nValuesΪ����ֵ������valuesΪ��Ӧ������ֵ���顣
�������ݸ���������ֵ����Ԫ�飬���ü�¼����ģ��ĺ��������Ԫ�飬Ȼ���ڸñ��ÿ��������Ϊ��Ԫ�鴴�����ʵ�������*/
RC Insert(char *relName, int nValues, Value * values)
{
	RC rc;
	RID *rid;
	RM_FileScan *FileScan;
	RM_Record *tab_rec, *col_rec;
	RM_FileHandle *tab_Handle, *col_Handle, *data_Handle;


	//ɨ��ϵͳ���ļ��������������Ƿ���ڸñ���֪���Բ���
	char *tableName = relName;
	Con *conditions = (Con*)malloc(sizeof(Con));
	conditions->bLhsIsAttr = 1;//�������Ǳ�ʾ�Ƿ����������ԣ�1��
	conditions->bRhsIsAttr = 0;//�������Ǳ�ʾֵ0������
	conditions->attrType = chars;//AttrTypeö�������е�
	conditions->LattrLength = 21;//tableName����Ϊ21���ֽ�
	conditions->RattrLength = 0;
	conditions->LattrOffset = 0;//
	conditions->RattrOffset = 0;//���ʣ�ֵ��ƫ���ǲ���0��
	conditions->compOp = EQual;
	conditions->Rvalue = (void*)tableName;//����ֵ�Ļ���ָ���Ӧ��ֵ

										  //��ϵͳ���ļ�
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_Handle->bOpen = false;
	tab_rec = (RM_Record*)malloc(sizeof(RM_Record));
	tab_rec->bValid = false;

	rc = RM_OpenFile("SYSTABLES", tab_Handle);
	if (rc != SUCCESS)
	{
		AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
		return rc;
	}

	FileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	FileScan->bOpen = false;
	rc = OpenScan(FileScan, tab_Handle, 1, conditions);
	if (rc != SUCCESS) {
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
		return rc;
	}
	rc = GetNextRec(FileScan, tab_rec);
	if (rc == SUCCESS)
	{
		int attrCount;
		//memset(&attrCount, '\0', 4);
		memcpy(&attrCount, tab_rec->pData + 21, sizeof(int));
		CloseScan(FileScan);
		free(FileScan);
		if (attrCount == nValues)
		{
			int recordSize = 0;
			SysColumns *tmp, *column;
			//��ϵͳ���ļ�
			col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
			col_Handle->bOpen = false;

			rc = RM_OpenFile("SYSCOLUMNS", col_Handle);
			if (rc != SUCCESS)
			{
				AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
				return rc;
			}
			FileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
			FileScan->bOpen = false;
			col_rec = (RM_Record*)malloc(sizeof(RM_Record));
			col_rec->bValid = false;
			rc = OpenScan(FileScan, col_Handle, 0, NULL);
			if (rc != SUCCESS) {
				AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
				return rc;
			}
			int i = 0;//��¼ѭ������
			column = (SysColumns*)malloc(sizeof(SysColumns)*nValues);
			tmp = column;
			while (GetNextRec(FileScan, col_rec) == SUCCESS)
			{
				//char *attrLength = (char*)malloc(4 * sizeof(char));
				int attrLength;
				if (strcmp(relName, col_rec->pData) == 0)
				{
					++i;
					memcpy(&attrLength, col_rec->pData + 46, sizeof(int));
					recordSize += attrLength;//�������ݱ��¼��С
					memcpy(tmp->tablename, col_rec->pData, 21);
					memcpy(tmp->attrname, col_rec->pData + 21, 21);
					memcpy(&(tmp->attrtype), col_rec->pData + 42, sizeof(int));
					memcpy(&(tmp->attrlength), col_rec->pData + 42 + sizeof(int), sizeof(int));
					memcpy(&(tmp->attroffeset), col_rec->pData + 42 + 2 * sizeof(int), sizeof(int));
					++tmp;//���ñ������������Ϣ���Ƴ���
				}
				if (i == nValues)
					break;//����ѭ������
				//free(attrLength);
			}
			tmp = column;
			CloseScan(FileScan);
			free(FileScan);


			//�����ݱ��ļ��в����¼
			data_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
			data_Handle->bOpen = false;

			rc = RM_OpenFile(relName, data_Handle);
			if (rc != SUCCESS)
			{
				AfxMessageBox("���ݱ��ļ���ʧ�ܣ�");
				return rc;
			}
			char *pData = (char*)malloc(recordSize);
			for (int i = 0; i < nValues; i++)
			{
				Value *localValue = values + nValues - i - 1;
				AttrType atype = localValue->type;
				if (atype != (tmp+i)->attrtype)
				{
					AfxMessageBox("��������ʽ����!");
					return SQL_SYNTAX;//���ش�����
				}
				memcpy(pData + (tmp + i)->attroffeset, localValue->data, (tmp + i)->attrlength);//������ļ�¼���ݲ�Ϊ����Ҫ��ļ�¼
				//memcpy(pData + (tmp + i)->attroffeset, (values + i)->data, (tmp + i)->attrlength);//������ļ�¼���ݲ�Ϊ����Ҫ��ļ�¼
			}
			rid = (RID*)malloc(sizeof(RID));
			rid->bValid = false;
			InsertRec(data_Handle, pData, rid);//�����¼

			free(rid); free(tab_rec); free(col_rec);
			//free(pData); 
			//free(column);

			//�ر��ļ����
			rc = RM_CloseFile(tab_Handle); if (rc != SUCCESS)return rc;
			free(tab_Handle);
			rc = RM_CloseFile(data_Handle); if (rc != SUCCESS)return rc;
			free(data_Handle);
		}
		else {
			AfxMessageBox("��������ʽ����!");
			return SQL_SYNTAX;//���ش�����
		}
	}//�����
	else
	{
		AfxMessageBox("����ı�����!");
		return rc;
	}

	return SUCCESS;
}

/*�ú�������ɾ��relName������������ָ��������Ԫ���Լ���Ԫ���Ӧ�������
���û��ָ����������˷���ɾ��relName��ϵ������Ԫ�顣��������������������Щ����֮��Ϊ���ϵ*/
RC Delete(char *relName, int nConditions, Condition *conditions)
{
	RC rc;
	//RID *rid = NULL;
	RM_FileHandle *tab_Handle, *col_Handle, *data_Handle;
	RM_FileScan *fileScan;
	RM_Record *tab_rec, *col_rec, *data_rec;

	//ɨ��ϵͳ���ļ�������
	Con *con = (Con*)malloc(sizeof(Con));
	con->bLhsIsAttr = 1;
	con->bRhsIsAttr = 0;
	con->attrType = chars;
	con->LattrLength = 21;
	con->RattrLength = 0;
	con->LattrOffset = 0;
	con->RattrOffset = 0;
	con->compOp = EQual;
	con->Rvalue = (void *)relName;


	//�����ж��Ƿ���ڸ���Ԫ�����ڵı�
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_rec = (RM_Record*)malloc(sizeof(RM_Record));

	rc = RM_OpenFile("SYSTABLES", tab_Handle);
	if (rc != SUCCESS)
	{
		AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
		return rc;
	}

	fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	rc = OpenScan(fileScan, tab_Handle, 1, con);
	if (rc != SUCCESS) {
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
		return rc;
	}

	rc = GetNextRec(fileScan, tab_rec);
	int nAttrCount;               //������Ը���
	if (rc == SUCCESS)
	{
		memcpy(&nAttrCount, tab_rec->pData + 21, 4);//���ñ�����Ը������Ƴ���

		CloseScan(fileScan);
		free(fileScan);

		//��ϵͳ���ļ����õ���Ӧ���Եĳ��Ⱥ�ƫ��
		//SysColumns *tmp, *column;
		col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));

		rc = RM_OpenFile("SYSCOLUMNS", col_Handle);
		if (rc != SUCCESS)
		{
			AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
			return rc;
		}
		fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
		col_rec = (RM_Record*)malloc(sizeof(RM_Record));

		//ɨ��ϵͳ���ļ�������
		con = (Con*)realloc(con, 2 * sizeof(Con));
		(con + 1)->bLhsIsAttr = 1;
		(con + 1)->bRhsIsAttr = 0;
		(con + 1)->attrType = chars;
		(con + 1)->LattrLength = 21;
		(con + 1)->RattrLength = 0;
		(con + 1)->LattrOffset = 21;
		(con + 1)->RattrOffset = 0;
		(con + 1)->compOp = EQual;
		//(con + 1)->Rvalue = (void*)attrName;

		/*rc = OpenScan(fileScan, col_Handle, 1, con);
		if (rc != SUCCESS) {
			AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
			return rc;
		}
		rc = GetNextRec(fileScan, col_rec);
		if (rc == SUCCESS)
		{*/

			////��������
			//AttrType attrType;
			//memcpy(&attrType, col_rec->pData + 42, sizeof(AttrType));
			////���Գ��Ⱥ�ƫ����
			//int attrlength, attrOffset;
			//memcpy(&attrlength, col_rec->pData + 46, sizeof(int));
			//memcpy(&attrOffset, col_rec->pData + 50, sizeof(int));

			///*tmp = column;*/
			//CloseScan(fileScan);

			Con *cons = (Con *)malloc(sizeof(Con) * nConditions);
			for (int i = 0; i < nConditions; i++) {
				//ֻ������con[1]->rValue,����������������óɲ�ѯʱ��ֵ
				if (conditions[i].bLhsIsAttr == 0 && conditions[i].bRhsIsAttr == 1)
				{  //�����ֵ���ұ�������
					(con + 1)->Rvalue = conditions[i].rhsAttr.attrName;
				}
				else if (conditions[i].bLhsIsAttr == 1 && conditions[i].bRhsIsAttr == 0)
				{   //��������ԣ��ұ���ֵ
					(con + 1)->Rvalue = conditions[i].lhsAttr.attrName;
				}
				else
				{  //���߶������Ի����߶���ֵ���ݲ�����

				}

				OpenScan(fileScan, col_Handle, 2, con);
				rc = GetNextRec(fileScan, col_rec);
				if (rc != SUCCESS) return rc;

				//cons[i].attrType = conditions[i].attrType;
				cons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
				cons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
				cons[i].compOp = conditions[i].op;
				if (conditions[i].bLhsIsAttr == 1) //�������
				{ //�������Գ��Ⱥ�ƫ����
					memcpy(&cons[i].LattrLength, col_rec->pData + 46, 4);
					memcpy(&cons[i].LattrOffset, col_rec->pData + 50, 4);
				}
				else {
					cons[i].attrType = conditions[i].lhsValue.type;
					cons[i].Lvalue = conditions[i].lhsValue.data;
				}

				if (conditions[i].bRhsIsAttr == 1) {
					memcpy(&cons[i].RattrLength, col_rec->pData + 46, 4);
					memcpy(&cons[i].RattrOffset, col_rec->pData + 50, 4);
				}
				else {
					cons[i].attrType = conditions[i].rhsValue.type;
					cons[i].Rvalue = conditions[i].rhsValue.data;
				}

				CloseScan(fileScan);

			}
			free(fileScan);


			//�򿪶�Ӧ�����ݱ��ļ�
			data_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
			data_rec = (RM_Record*)malloc(sizeof(RM_Record));

			rc = RM_OpenFile(relName, data_Handle);
			if (rc != SUCCESS)
			{
				AfxMessageBox("���ݱ��ļ���ʧ�ܣ�");
				return rc;
			}
			fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
			rc = OpenScan(fileScan, data_Handle, nConditions, cons);
			if (rc != SUCCESS) {
				AfxMessageBox("���ݱ��ļ�ɨ��ʧ�ܣ�");
				return rc;
			}

			while (GetNextRec(fileScan, data_rec) == SUCCESS)
			{
				/*memcpy(data_rec->pData + attrOffset, value->data, attrlength);*/

				DeleteRec(data_Handle, &data_rec->rid);   //������������㣬��ɾ��
				//UpdateRec(fileScan->pRMFileHandle, data_rec);
			}


			CloseScan(fileScan);
			free(fileScan);
			free(cons);
		

		rc = RM_CloseFile(tab_Handle);
		rc = RM_CloseFile(data_Handle);
		//�ͷŹ���������������ڴ�ռ�
		free(con);
		free(tab_Handle);
		free(data_Handle);
		free(tab_rec);
		free(data_rec);
		return SUCCESS;
	}//ɾ���ı����
	else
	{
		AfxMessageBox("�����������ı�");

		RM_CloseFile(tab_Handle);
		free(tab_Handle);
		free(tab_rec);
		CloseScan(fileScan);
		free(fileScan);
		return rc;
	}
	return SUCCESS;
}

/*�ú������ڸ���relName������������ָ��������Ԫ�飬��ÿһ�����µ�Ԫ���н�����attrName��ֵ����Ϊһ���µ�ֵ��
���û��ָ����������˷�������relName������Ԫ�顣���Ҫ����һ�������������ԣ�Ӧ����ɾ��ÿ��������Ԫ���Ӧ��������Ŀ��
Ȼ�����һ���µ�������Ŀ*/
RC Update(char *relName, char *attrName, Value *value, int nConditions, Condition *conditions)
{
	RC rc;
	//RID *rid = NULL;
	RM_FileHandle *tab_Handle, *col_Handle, *data_Handle;
	RM_FileScan *fileScan;
	RM_Record *tab_rec, *col_rec, *data_rec;

	//ɨ��ϵͳ���ļ�������
	Con *con = (Con*)malloc(sizeof(Con));
	con->bLhsIsAttr = 1;
	con->bRhsIsAttr = 0;
	con->attrType = chars;
	con->LattrLength = 21;
	con->RattrLength = 0;
	con->LattrOffset = 0;
	con->RattrOffset = 0;
	con->compOp = EQual;
	con->Rvalue = (void *)relName;


	//�����ж��Ƿ���ڸ���Ԫ�����ڵı�
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_rec = (RM_Record*)malloc(sizeof(RM_Record));

	rc = RM_OpenFile("SYSTABLES", tab_Handle);
	if (rc != SUCCESS)
	{
		AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
		return rc;
	}

	fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	rc = OpenScan(fileScan, tab_Handle, 1, con);
	if (rc != SUCCESS) {
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
		return rc;
	}

	rc = GetNextRec(fileScan, tab_rec);
	int nAttrCount;               //������Ը���
	if (rc == SUCCESS)
	{
		memcpy(&nAttrCount, tab_rec->pData + 21, 4);//���ñ�����Ը������Ƴ���

		CloseScan(fileScan);
		free(fileScan);

		//��ϵͳ���ļ����õ���Ӧ���Եĳ��Ⱥ�ƫ��
//		SysColumns *tmp, *column;
		col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));

		rc = RM_OpenFile("SYSCOLUMNS", col_Handle);
		if (rc != SUCCESS)
		{
			AfxMessageBox("ϵͳ���ļ���ʧ�ܣ�");
			return rc;
		}
		fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
		col_rec = (RM_Record*)malloc(sizeof(RM_Record));

		//ɨ��ϵͳ���ļ�������
		con = (Con*)realloc(con, 2 * sizeof(Con));
		(con + 1)->bLhsIsAttr = 1;
		(con + 1)->bRhsIsAttr = 0;
		(con + 1)->attrType = chars;
		(con + 1)->LattrLength = 21;
		(con + 1)->RattrLength = 0;
		(con + 1)->LattrOffset = 21;
		(con + 1)->RattrOffset = 0;
		(con + 1)->compOp = EQual;
		(con + 1)->Rvalue = (void*)attrName;

		rc = OpenScan(fileScan, col_Handle, 2, con);
		if (rc != SUCCESS) {
			AfxMessageBox("ϵͳ���ļ�ɨ��ʧ�ܣ�");
			return rc;
		}
		rc = GetNextRec(fileScan, col_rec);
		if (rc == SUCCESS)
		{

			//��������
			AttrType attrType;
			memcpy(&attrType, col_rec->pData + 42, sizeof(AttrType));
			//���Գ��Ⱥ�ƫ����
			int attrlength, attrOffset;
			memcpy(&attrlength, col_rec->pData + 46, sizeof(int));
			memcpy(&attrOffset, col_rec->pData + 50, sizeof(int));

			/*tmp = column;*/
			CloseScan(fileScan);

			Con *cons = (Con *)malloc(sizeof(Con) * nConditions);
			for (int i = 0; i < nConditions; i++) {
				//ֻ������con[1]->rValue,����������������óɲ�ѯʱ��ֵ
				if (conditions[i].bLhsIsAttr==0 && conditions[i].bRhsIsAttr==1)
				{  //�����ֵ���ұ�������
					(con + 1)->Rvalue = conditions[i].rhsAttr.attrName;
				}
				else if (conditions[i].bLhsIsAttr == 1 && conditions[i].bRhsIsAttr == 0)
				{   //��������ԣ��ұ���ֵ
					(con + 1)->Rvalue = conditions[i].lhsAttr.attrName;
				}
				else
				{  //���߶������Ի����߶���ֵ���ݲ�����

				}

				OpenScan(fileScan, col_Handle, 2, con);
				rc = GetNextRec(fileScan, col_rec);
				if (rc != SUCCESS) return rc;

				//cons[i].attrType = conditions[i].attrType;
				cons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
				cons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
				cons[i].compOp = conditions[i].op;
				if (conditions[i].bLhsIsAttr == 1) //�������
				{ //�������Գ��Ⱥ�ƫ����
					memcpy(&cons[i].LattrLength, col_rec->pData + 46, 4);
					memcpy(&cons[i].LattrOffset, col_rec->pData + 50, 4);
				}
				else {
					cons[i].attrType = conditions[i].lhsValue.type;
					cons[i].Lvalue = conditions[i].lhsValue.data;
				}
				
				if (conditions[i].bRhsIsAttr == 1) {
					memcpy(&cons[i].RattrLength, col_rec->pData + 46, 4);
					memcpy(&cons[i].RattrOffset, col_rec->pData + 50, 4);
				}
				else {
					cons[i].attrType = conditions[i].rhsValue.type;
					cons[i].Rvalue = conditions[i].rhsValue.data;
				}

				CloseScan(fileScan);

			}
			free(fileScan);


			//�򿪶�Ӧ�����ݱ��ļ�
			data_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
			data_rec = (RM_Record*)malloc(sizeof(RM_Record));

			rc = RM_OpenFile(relName, data_Handle);
			if (rc != SUCCESS)
			{
				AfxMessageBox("���ݱ��ļ���ʧ�ܣ�");
				return rc;
			}
			fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
			rc = OpenScan(fileScan, data_Handle, nConditions, cons);
			if (rc != SUCCESS) {
				AfxMessageBox("���ݱ��ļ�ɨ��ʧ�ܣ�");
				return rc;
			}

			while (GetNextRec(fileScan, data_rec) == SUCCESS)
			{
				memcpy(data_rec->pData+attrOffset, value->data, attrlength);

				UpdateRec(fileScan->pRMFileHandle, data_rec);
			}
			

			CloseScan(fileScan); 
			free(fileScan);
			free(cons);
		}
		else
		{
			AfxMessageBox("�ñ������������ԣ�");
			return rc;
		}

		rc = RM_CloseFile(tab_Handle);
		rc = RM_CloseFile(data_Handle);
		//�ͷŹ���������������ڴ�ռ�
		free(con); 
		free(tab_Handle); 
		free(data_Handle);
		free(tab_rec); 
		free(data_rec);
		return SUCCESS;
	}//ɾ���ı����
	else
	{
		AfxMessageBox("�����������ı�");
		
		RM_CloseFile(tab_Handle);
		free(tab_Handle);
		free(tab_rec);
		CloseScan(fileScan);
		free(fileScan);
		return rc;
	}
return SUCCESS;
}

/*�жϵ�ǰ���ݿ����Ƿ����ָ�������ı�*/
bool hasTable(char * tableName)
{
	CFile tmp;
	RM_FileHandle *tab_Handle;
	RC rc;
	RM_FileScan *FileScan = NULL;
	RM_Record *tab_rec = NULL;

	//��ϵͳ���ϵͳ���ж�Ӧ�����ؼ�¼ɾ��
	tab_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tab_Handle->bOpen = false;

	rc = RM_OpenFile("SYSTABLES", tab_Handle);

	FileScan->bOpen = false;
	rc = OpenScan(FileScan, tab_Handle, 0, NULL);
	while (GetNextRec(FileScan, tab_rec) == SUCCESS){
		if (strcmp(tableName, tab_rec->pData) == 0){
			return TRUE;//���ڸñ�ļ�¼
		}
	}
	CloseScan(FileScan);

	//�ر��ļ����
	rc = RM_CloseFile(tab_Handle);
	free(tab_Handle);

	return FALSE;
}

/*�ж�ָ�������Ƿ����ָ����*/
bool hasColumn(char * tableName, char * columnName)
{
	CFile tmp;
	RM_FileHandle *col_Handle;
	RC rc;
	RM_FileScan *FileScan = NULL;
	RM_Record *col_rec = NULL;

	//��ϵͳ��ϵͳ���ļ�
	col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	col_Handle->bOpen = false;

	rc = RM_OpenFile("SYSCOLUMNS", col_Handle);

	FileScan->bOpen = false;
	rc = OpenScan(FileScan, col_Handle, 0, NULL);
	while (GetNextRec(FileScan, col_rec) == SUCCESS){
		char *data1 = (char *)malloc(25 * sizeof(char));
		char *data2 = (char *)malloc(25 * sizeof(char));
		memset(data1, '\0', 25); memset(data2, '\0', 25);
		memcpy(data1, col_rec->pData, 21); memcpy(data2, col_rec->pData + 21, 21);
		if (!strcmp(tableName, data1) && !strcmp(columnName, data2))
		{
			return TRUE;
		}
		free(data1); free(data2);
	}
	CloseScan(FileScan);

	//�ر��ļ����
	rc = RM_CloseFile(col_Handle); 
	free(col_Handle);

	return FALSE;
}

/*�ж�ָ�����ָ���ֶ����Ƿ��������*/
bool hasIndex(char * tableName, char * columnName)
{
	CFile tmp;
	RM_FileHandle  *col_Handle;
	RC rc;
	RM_FileScan *FileScan = NULL;
	RM_Record *col_rec = NULL;

	col_Handle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	col_Handle->bOpen = false;

	rc = RM_OpenFile("SYSCOLUMNS", col_Handle);

	FileScan->bOpen = false;
	rc = OpenScan(FileScan, col_Handle, 0, NULL);
	//ѭ�����ұ���ΪrelName��Ӧ��ϵͳ���еļ�¼,ɾ��
	while (GetNextRec(FileScan, col_rec) == SUCCESS){
		char *data1 = (char *)malloc(25 * sizeof(char));
		char *data2 = (char *)malloc(25 * sizeof(char));
		memset(data1, '\0', 25); memset(data2, '\0', 25);
		memcpy(data1, col_rec->pData, 21); memcpy(data2, col_rec->pData + 21, 21);
		if (!strcmp(tableName, data1) && !strcmp(columnName, data2))
		{
			char *data = (char *)malloc(sizeof(char));
			memset(data, '\0', 1); memcpy(data, col_rec->pData + 54,1);//��֤�������Ƿ���ڵ��ֽ�ճ������
			if (data == "1")return TRUE;
			free(data);
		}
		free(data1); free(data2);
	}
	CloseScan(FileScan);

	//�ر��ļ����
	rc = RM_CloseFile(col_Handle);
	free(col_Handle);

	return FALSE;
}

