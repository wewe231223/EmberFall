#include "pch.h"
#include "ThreadUtils.h"

void InitTls()
{
    lThreadId = gThreadId.fetch_add(1);
}

void ClearTls() 
{ 
}
