#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include	<iostream>
#include<ctime>
#include	<wchar.h>
#include <sstream>
#include <time.h>
#include<fstream>

using namespace std;

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);
DWORD WINAPI WatchChanges(LPVOID lpParameter);


//ThreadParameter�ṹ��Ķ���
/*
*���ϲ���Ϊһ���ṹ�崫�����̵߳�ԭ�����ڴ����߳�ʱ
*ָ�����߳���ں���ֻ����һ��LPVOID���͵�ָ�룬�������ݿ��Բο�msdn��
*ʵ���������̴߳��ݲ�������һ�ַ�����ȫ�ֱ�����
*�����������е�WriteLog����һ���ļ������������Ҿ����ڳ����ײ������ȫ�ֱ�����
*/
typedef struct ThreadParameter
{
	LPTSTR in_directory;//��ص�·��
	FILE_NOTIFY_INFORMATION *in_out_notification;//�洢��غ���������Ϣ��ַ
	DWORD in_MemorySize;//���ݴ洢������Ϣ���ڴ���ֽ���
	DWORD *in_out_BytesReturned;//�洢��غ���������Ϣ���ֽ���
	DWORD *in_out_version;//���ذ汾��Ϣ
	FILE_NOTIFY_INFORMATION *temp_notification;//���õ�һ������
}ThreadParameter;



int main(int argc, TCHAR *argv[])
{
    if(argc != 2)
    {
        _tprintf(TEXT("Usage: %s <dir>\n"), argv[0]);
        return 0;
    }
    LPTSTR Directory = argv[1];
    //���ݸ�WatchChanges�����Ĳ���,�ⲿ�ִ������������
            char notify[1024];
            memset(notify,'\0',1024);
            FILE_NOTIFY_INFORMATION *Notification=(FILE_NOTIFY_INFORMATION *)notify;
            FILE_NOTIFY_INFORMATION *TempNotification=NULL;
            DWORD BytesReturned=0;
            DWORD version=0;



            //���ϴ������̵߳Ĳ������ýṹ��Ķ���ο�����Ķ���
            ThreadParameter ParameterToThread={Directory,Notification,sizeof(notify),&BytesReturned,&version,TempNotification};

            //����һ���߳�ר�����ڼ���ļ��仯
            HANDLE TrheadWatch=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WatchChanges,&ParameterToThread,0,NULL);
            WaitForSingleObject(TrheadWatch, INFINITE);
            CloseHandle(TrheadWatch);
            //WatchChanges(&ParameterToThread);

}

void WatchDirectory(LPTSTR lpDir)
{
   DWORD dwWaitStatus;
   HANDLE dwChangeHandles[2];
   TCHAR lpDrive[4];
   TCHAR lpFile[_MAX_FNAME];
   TCHAR lpExt[_MAX_EXT];

   _splitpath(lpDir, lpDrive, NULL,  lpFile , lpExt);

   lpDrive[2] = (TCHAR)'\\';
   lpDrive[3] = (TCHAR)'\0';



// Watch the directory for file creation and deletion.

   dwChangeHandles[0] = FindFirstChangeNotification(
      lpDir,                         // directory to watch
      FALSE,                         // do not watch subtree
      FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes

   if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
   {
     printf("\n ERROR: FindFirstChangeNotification function failed.\n");
     ExitProcess(GetLastError());
   }

// Watch the subtree for directory creation and deletion.

   dwChangeHandles[1] = FindFirstChangeNotification(
      lpDrive,                       // directory to watch
      TRUE,                          // watch the subtree
      FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes

   if (dwChangeHandles[1] == INVALID_HANDLE_VALUE)
   {
     printf("\n ERROR: FindFirstChangeNotification function failed.\n");
     ExitProcess(GetLastError());
   }


// Make a final validation check on our handles.

   if ((dwChangeHandles[0] == NULL) || (dwChangeHandles[1] == NULL))
   {
     printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
     ExitProcess(GetLastError());
   }

// Change notification is set. Now wait on both notification
// handles and refresh accordingly.

   while (TRUE)
   {
   // Wait for notification.

      printf("\nWaiting for notification...\n");

      dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles,
         FALSE, INFINITE);

      switch (dwWaitStatus)
      {
         case WAIT_OBJECT_0:

         // A file was created, renamed, or deleted in the directory.
         // Refresh this directory and restart the notification.

             RefreshDirectory(lpDir);
             if ( FindNextChangeNotification(dwChangeHandles[0]) == FALSE )
             {
               printf("\n ERROR: FindNextChangeNotification function failed.\n");
               ExitProcess(GetLastError());
             }
             break;

         case WAIT_OBJECT_0 + 1:

         // A directory was created, renamed, or deleted.
         // Refresh the tree and restart the notification.

             RefreshTree(lpDrive);
             if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE )
             {
               printf("\n ERROR: FindNextChangeNotification function failed.\n");
               ExitProcess(GetLastError());
             }
             break;

         case WAIT_TIMEOUT:

         // A timeout occurred, this would happen if some value other
         // than INFINITE is used in the Wait call and no changes occur.
         // In a single-threaded environment you might not want an
         // INFINITE wait.

            printf("\nNo changes in the timeout period.\n");
            break;

         default:
            printf("\n ERROR: Unhandled dwWaitStatus.\n");
            ExitProcess(GetLastError());
            break;
      }
   }
}

void RefreshDirectory(LPTSTR lpDir)
{
   // This is where you might place code to refresh your
   // directory listing, but not the subtree because it
   // would not be necessary.

   _tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}

void RefreshTree(LPTSTR lpDrive)
{
   // This is where you might place code to refresh your
   // directory listing, including the subtree.

   _tprintf(TEXT("Directory tree (%s) changed.\n"), lpDrive);
}


//  ����: WatchChanges(LPVOID lpParameter)
//
//  Ŀ��: ���Ŀ¼�ĳ���
//
//  ע��:�����������߳�ʱ�ƶ���������������
//		 ��ʱ���ӳ����Զ�����ִ�С�
//  ��ע����Ϊ���벻ȫ��������Ĵ���ʱ����Ҫ�ο���ɫ�����岿��

DWORD WINAPI WatchChanges(LPVOID lpParameter)//���ذ汾��Ϣ
{
	ThreadParameter *parameter=(ThreadParameter*)lpParameter;
	LPCTSTR WatchDirectory=parameter->in_directory;//


	//����һ��Ŀ¼���
	HANDLE handle_directory=CreateFile(WatchDirectory,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	if(handle_directory==INVALID_HANDLE_VALUE)
	{
		DWORD ERROR_DIR=GetLastError();
		MessageBox(NULL,TEXT("��Ŀ¼����!"),TEXT("�ļ����"),0);
	}


	BOOL watch_state;
	time_t ChangeTime;
	int edit_flag = 0;
	ofstream WriteLog("log.txt",ios::app);

	while (TRUE)
	{
		watch_state=ReadDirectoryChangesW(handle_directory,
			(LPVOID)parameter->in_out_notification,
			parameter->in_MemorySize,
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE,
			(LPDWORD)parameter->in_out_BytesReturned,
			NULL,
			NULL);

		time(&ChangeTime);//��¼�޸�ʱ��


		if (GetLastError()==ERROR_INVALID_FUNCTION)
		{
			MessageBox(NULL,TEXT("ϵͳ��֧��!"),TEXT("�ļ����"),0);
			//printf("ϵͳ��֧��!\n");
		}
		else if(watch_state==0)
		{
			MessageBox(NULL,TEXT("���ʧ��!"),TEXT("�ļ����"),0);
			//printf("���ʧ��!\n");
		}
		else if (GetLastError()==ERROR_NOTIFY_ENUM_DIR)
		{
			MessageBox(NULL,TEXT("�ڴ����!"),TEXT("�ļ����"),0);
			//printf("�ڴ����!\n");
		}
		else
		{

			//�����ַ����͵�FileName����ת����string������д��log�ļ�������д����ȥ��ȷ���ļ���
			string file_name;
			DWORD length=WideCharToMultiByte(0,0,parameter->in_out_notification->FileName,-1,NULL,0,NULL,NULL);
			PSTR ps=new CHAR[length];
			if(length>=0)
			{
				WideCharToMultiByte(0,0,parameter->in_out_notification->FileName,-1,ps,length,NULL,NULL);
				file_name=string(ps);
				delete[] ps;
			}

			//������Ҫ���Ǽ�ⷵ�ص���Ϣ����Ҫע��FILE_NOTIFY_INFORMATION�ṹ��Ķ��壬�Ա���ȷ���÷�����Ϣ

			if (parameter->in_out_notification->Action==FILE_ACTION_ADDED)
			{
				WriteLog<<ctime(&ChangeTime)<<"�����ļ� : "<<file_name<<"\n"<<flush;
				cout<<ctime(&ChangeTime)<<"�����ļ� : "<<file_name<<"\n"<<flush;

			}
			if (parameter->in_out_notification->Action==FILE_ACTION_REMOVED)
			{
				WriteLog<<ctime(&ChangeTime)<<"ɾ���ļ� : "<<file_name<<"\n"<<flush;
				cout<<ctime(&ChangeTime)<<"ɾ���ļ� : "<<file_name<<"\n"<<flush;
			}
			if (parameter->in_out_notification->Action==FILE_ACTION_MODIFIED)
			{
				edit_flag++;
				if(edit_flag==1){
					WriteLog<<ctime(&ChangeTime)<<"�޸��ļ� : "<<file_name<<"\n"<<flush;
					cout<<ctime(&ChangeTime)<<"�޸��ļ� : "<<file_name<<"\n"<<flush;
				}else if(edit_flag==2)
				{
					edit_flag=0;
					(*(parameter->in_out_version))--;
				}
				else
					return -1;//break;
			}


			//�����������������Action����Ҳ���ļ�����������old_nameҲ������new_name��
			if (parameter->in_out_notification->Action==FILE_ACTION_RENAMED_OLD_NAME)
			{
				WriteLog<<ctime(&ChangeTime)<<"������\""<<file_name<<"\"�ļ�\n"<<flush;
				cout<<ctime(&ChangeTime)<<"������\""<<file_name<<"\"�ļ�\n"<<flush;
			}
			if (parameter->in_out_notification->Action==FILE_ACTION_RENAMED_NEW_NAME)
			{
				WriteLog<<ctime(&ChangeTime)<<"������\""<<file_name<<"\"�ļ�Ϊ\""<<parameter->in_out_notification->Action<<"\"\n"<<flush;
				cout<<ctime(&ChangeTime)<<"������\""<<file_name<<"\"�ļ�Ϊ\""<<parameter->in_out_notification->Action<<"\"\n"<<flush;
			}
			(*(parameter->in_out_version))++;
			memset(parameter->in_out_notification,'\0',1024);

		}
		//fflush(stdout);
		Sleep(500);
	}
	return 0;
}
