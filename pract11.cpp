#include <iostream>
#include <Windows.h>
#include <conio.h>

#define MAX_LOGS 1000
#define MAX_TEXT 128
char logs[][MAX_TEXT] = {"log1","log2","log3","log4","log5"};
bool csUse = false;
DWORD tick = GetTickCount64();

struct LogRecord
{
	DWORD threadId;
	int priority;
	DWORD tick;

	char message[MAX_TEXT];
};

struct LogBuffer 
{
	LogRecord records[MAX_LOGS];
	LONG index;
};

LogBuffer buffer;
CRITICAL_SECTION cs;

DWORD WINAPI Logger(DWORD priority)
{;
	srand(time(NULL));
	while (buffer.index <= MAX_LOGS)
	{
		if (csUse)
		{
			EnterCriticalSection(&cs);
		}

		LogRecord log;
		log.threadId = GetCurrentThreadId();
		log.priority = priority;
		log.tick = GetTickCount64() - tick;

		strcpy_s(log.message, logs[rand() % 5]);

		buffer.records[buffer.index] = log;
		buffer.index++;

		if (csUse)
		{
			LeaveCriticalSection(&cs);
		}

		Sleep(rand() % 100 + 10);
	};
	return 0;
}
DWORD WINAPI Watcher()
{
	do
	{
		if (csUse)
		{
			EnterCriticalSection(&cs);
		}
		int index = buffer.index - 5;
		for (int i = 0; i < 5; i++)
		{
			if (index <= 0)
				break;
			LogRecord log = buffer.records[index + i];
			std::cout << "[" << index + i << "]" <<
				" priority: " << log.priority <<
				" message: " << log.message <<
				" tick: " << log.tick <<
				std::endl;
		}
		std::cout << std::endl;
		if (csUse)
		{
			LeaveCriticalSection(&cs);
		}
		Sleep(100);
	} while (buffer.index <= MAX_LOGS);

	return 0;
}

int main() //логи теряются и выводятся не все, с критической секцией они все работают нормально
{

	InitializeCriticalSection(&cs);

	char selection;
	std::cout << "use critical section 0-false, 1-true" << std::endl;
	selection = _getch();
	if (selection == '1')
	{
		csUse = true;
	}

	DWORD IDThread1, IDThread2, IDThread3, IDThread4;
	HANDLE highPriorityLogger = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Logger, (void*)THREAD_PRIORITY_HIGHEST, 0, &IDThread1);

	if (highPriorityLogger == NULL)
		return GetLastError();
	SetPriorityClass(highPriorityLogger, THREAD_PRIORITY_HIGHEST);

	HANDLE normalPriorityLogger = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Logger, (void*)THREAD_PRIORITY_NORMAL, 0, &IDThread2);
	if (normalPriorityLogger == NULL)
		return GetLastError();
	SetPriorityClass(normalPriorityLogger, THREAD_PRIORITY_NORMAL);

	HANDLE lowPriorityLogger = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Logger, (void*)THREAD_PRIORITY_BELOW_NORMAL, 0, &IDThread3);
	if (lowPriorityLogger == NULL)
		return GetLastError();
	SetPriorityClass(lowPriorityLogger, THREAD_PRIORITY_BELOW_NORMAL);

	HANDLE watcher = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Watcher, (void*)THREAD_PRIORITY_LOWEST, 0, &IDThread4);
	if (watcher == NULL)
		return GetLastError();
	SetPriorityClass(watcher, THREAD_PRIORITY_LOWEST);

	WaitForSingleObject(highPriorityLogger, INFINITE);
	WaitForSingleObject(normalPriorityLogger, INFINITE);
	WaitForSingleObject(lowPriorityLogger, INFINITE);
	WaitForSingleObject(watcher, INFINITE);

	CloseHandle(highPriorityLogger);
	CloseHandle(normalPriorityLogger);
	CloseHandle(lowPriorityLogger);
	CloseHandle(watcher);
}