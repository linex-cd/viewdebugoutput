#include <Windows.h>
#include <process.h>
#include <stdio.h>

#pragma pack(push)
#pragma pack(1)
typedef struct __DEBUGBUFFER
{
	DWORD sz;      //进程id
	char szString[4096 - sizeof(DWORD)];
} DEBUGBUFFER, * PDEBUGBUFFER;
#pragma pack(pop)
;
#define MAX_DEBUG_BUF_LEN (4096)
#define _Try __try
#define _Finally __finally


int main(int argc, char* argv[])
{

	HANDLE m_hReadyEvent = NULL;
	DWORD m_dwResult;
	HANDLE hMapping = NULL;
	HANDLE hAckEvent = NULL;
	PDEBUGBUFFER pdbBuffer = NULL;
	//TCHAR tzBuffer[MAX_DEBUG_BUF_LEN];
	_Try
	{
#define _LeaveIf(expr) if(expr == TRUE) \
		return FALSE;
		// 设置初始结果
		m_dwResult = ERROR_INVALID_HANDLE;
	// 打开事件句柄
	hAckEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("DBWIN_BUFFER_READY"));
	_LeaveIf(hAckEvent == NULL);
	m_hReadyEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("DBWIN_DATA_READY"));
	_LeaveIf(m_hReadyEvent == NULL);
	// 创建文件映射
	hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_DEBUG_BUF_LEN, TEXT("DBWIN_BUFFER"));
	_LeaveIf(hMapping == NULL);
	// 映射调试缓冲区
	pdbBuffer = (PDEBUGBUFFER)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	_LeaveIf(pdbBuffer == NULL);
	// 循环
	for (m_dwResult = ERROR_SIGNAL_PENDING; (m_dwResult == ERROR_SIGNAL_PENDING); )
	{
		// 等待缓冲区数据
		SetEvent(hAckEvent);
		if (WaitForSingleObject(m_hReadyEvent, INFINITE) == WAIT_OBJECT_0)
		{
			// 如果是继续等待，否则表示主线程发出了停止信号，退出当前线程
			if (m_dwResult == ERROR_SIGNAL_PENDING)
			{
				// 添加新项
				printf(pdbBuffer->szString);
			}
		}
		else
		{
			// 等待失败
			m_dwResult = WAIT_ABANDONED;
		}
	}
	}
		_Finally
	{
#define _SafeCloseHandle(handle) if(handle != NULL) \
		CloseHandle(handle);
		// 释放
		if (pdbBuffer)
		{
			UnmapViewOfFile(pdbBuffer);
		}
		_SafeCloseHandle(hMapping);
		_SafeCloseHandle(m_hReadyEvent);
		_SafeCloseHandle(hAckEvent);

		// 返回结果
		return m_dwResult;
	}
}
