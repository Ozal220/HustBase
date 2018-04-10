#ifndef __QUERY_MANAGER_H_
#define __QUERY_MANAGER_H_

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"
#include "str.h"

typedef struct SelResult{
	int col_num;
	int row_num;
	AttrType attrType[20];
	int offset[20];
	int length[20];
	char fields[20][20];//����ʮ���ֶ���������ÿ���ֶεĳ��Ȳ�����20
	char ** res[100];//���һ������¼
	SelResult * next_res;
}SelResult;

void Init_Result(SelResult * res);
void Destory_Result(SelResult * res);

RC Query(char * sql,SelResult * res);

RC Select(int nSelAttrs,RelAttr **selAttrs,int nRelations,char **relations,int nConditions,Condition *conditions,SelResult * res);
RC singleConditionSelect(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);
RC multiSelect(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);
RC recurSelect(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res, int curTable, int *offsets, char *curResult);

RC checkTable(int nRelations, char **relations);
#endif