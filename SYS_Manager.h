#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
//#include "PF_Manager.h"
//#include "RM_Manager.h"
#include "EditArea.h"
#include "str.h"

typedef struct {
	char tablename[21];//��ű���
	int attrcount;//�������Ե�����
}SysTable;//ϵͳ���ļ��ṹ
typedef struct {
	char tablename[21];//����       0
	char attrname[21];//������      21
	//int attrtype;//��������
	AttrType attrtype;
	int attrlength;//���Գ���
	int attroffeset;//�����ڼ�¼�е�ƫ����
	char ix_flag;//�����������Ƿ���������ı�ʶ,1��ʾ���ڣ�0��ʾ������
	char indexname[21];//��������
}SysColumns;//ϵͳ���ļ�

void ExecuteAndMessage(char *, CEditArea*, CHustBaseDoc*);
bool CanButtonClick();

RC CreateDB(char *dbpath,char *dbname);
RC DropDB(char *dbname);
RC OpenDB(char *dbname);
RC CloseDB();

RC execute(char * sql,CHustBaseDoc *pDoc);

RC CreateTable(char *relName,int attrCount,AttrInfo *attributes);
RC DropTable(char *relName);
RC CreateIndex(char *indexName, char *relName, char *attrName);
RC DropIndex(char *indexName);
RC Insert(char *relName,int nValues,Value * values);
RC Delete(char *relName,int nConditions,Condition *conditions);
RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions);

bool hasTable(char * tableName);
bool hasColumn(char * tableName,char * columnName);
bool hasIndex(char * tableName,char * columnName);


#endif