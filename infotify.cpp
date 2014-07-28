// MonitorTest.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fstream>
#include<iomanip>
#include<io.h>
#include<time.h>
#include <sstream>

using namespace std;


DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	LPCTSTR pDirtory = (char*)lpParam;
    BOOL bRet = FALSE;
    BYTE Buffer[1024] = { 0 };

    FILE_NOTIFY_INFORMATION *pBuffer = (FILE_NOTIFY_INFORMATION *)Buffer;
    DWORD BytesReturned = 0;
    HANDLE hFile = CreateFile(pDirtory,
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        return 1;
    }

    printf("monitor... \r\n");
    fflush(stdout);

    while ( TRUE )
    {
        ZeroMemory(Buffer, 1024);
        //���߳�һֱ�ڶ�ȡĿ¼�ĸı�
        //����ϵͳReadDirectoryChangesW API
        bRet = ReadDirectoryChangesW(hFile,
                        &Buffer,
                        sizeof(Buffer),
                        TRUE,
                        FILE_NOTIFY_CHANGE_FILE_NAME |  // �޸��ļ���
                        FILE_NOTIFY_CHANGE_ATTRIBUTES | // �޸��ļ�����
                        FILE_NOTIFY_CHANGE_LAST_WRITE |
                        FILE_NOTIFY_CHANGE_DIR_NAME, // ���һ��д��
                        &BytesReturned,
                        NULL, NULL);

        if ( bRet == TRUE )
        {
            char szFileName[MAX_PATH] = { 0 };

            // ���ַ�ת�����ֽ�
            WideCharToMultiByte(CP_ACP,
                                0,
                                pBuffer->FileName,
                                pBuffer->FileNameLength / 2,
                                szFileName,
                                MAX_PATH,
                                NULL,
                                NULL);

            switch(pBuffer->Action)
            {
                // ���
            case FILE_ACTION_ADDED:
                {
                    printf("��� : %s\r\n", szFileName);

                    break;
                }
                // ɾ��
            case FILE_ACTION_REMOVED:
                {
                    printf("ɾ�� : %s\r\n", szFileName);

                    break;
                }
                // �޸�
            case FILE_ACTION_MODIFIED:
                {
                	//�޸�Ϊ�ļ���
                	_finddata_t file;
                	string sdirfile;
                	sdirfile = string(pDirtory)+"\\"+szFileName;
                	long lf;
                	if((lf = _findfirst(sdirfile.c_str(), &file))== -1l){
                		cout<<"Ŀ¼������:"<<sdirfile<<flush;

                		return 1;
                	}
                	if((file.attrib & _A_SUBDIR) == 0){

                		printf("�޸� : %s\r\n", szFileName);
                	}

                    break;
                }
                // ������
            case FILE_ACTION_RENAMED_OLD_NAME:
                {
                    printf("������ : %s", szFileName);
                    if ( pBuffer->NextEntryOffset != 0 )
                    {
                        FILE_NOTIFY_INFORMATION *tmpBuffer = (FILE_NOTIFY_INFORMATION *)((DWORD)pBuffer + pBuffer->NextEntryOffset);
                        switch ( tmpBuffer->Action )
                        {
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            {
                                ZeroMemory(szFileName, MAX_PATH);
                                WideCharToMultiByte(CP_ACP,
                                    0,
                                    tmpBuffer->FileName,
                                    tmpBuffer->FileNameLength / 2,
                                    szFileName,
                                    MAX_PATH,
                                    NULL,
                                    NULL);
                                printf(" ->  : %s \r\n", szFileName);
                                break;
                            }
                        }
                    }
                    break;
                }
            case FILE_ACTION_RENAMED_NEW_NAME:
                {
                    printf("������(new) : %s\r\n", szFileName);
                }
            }
        }
        fflush(stdout);
    }

    CloseHandle(hFile);

    return 0;
}

int main(int argc, char* argv[])
{
	if(argc == 1){
		return 0;
	}

    HANDLE hThread = CreateThread(NULL, 0, ThreadProc, argv[1], 0, NULL);
    if ( hThread == NULL )
    {
        return -1;
    }
    //�ȴ��߳̽���
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return 0;
}
