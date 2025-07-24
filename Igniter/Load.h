#pragma once

char MoveDriver(char* pDriverName, char* pExistingPath, char* pNewPath);
char LoadDriver(char* pDriverName, char* pServiceName, char FailOnFailedStart);
char UnloadDriver(char* pDriverName, char* pServiceName);