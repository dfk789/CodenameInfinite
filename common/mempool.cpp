#include "mempool.h"

#include <common.h>
#include <platform.h>

size_t CMemPool::s_iMemoryAllocated = 0;
eastl::vector<CMemPool*> CMemPool::s_apMemPools;
size_t CMemPool::s_iLastMemPoolHandle = 0;

CMemPool::CMemPool()
	: m_pMemPool(NULL), m_iMemPoolSize(0), m_pAllocMap(NULL), m_pAllocMapBack(NULL)
{
}

CMemPool::~CMemPool()
{
	if (m_pMemPool)
		free(m_pMemPool);
}

CMemPool* CMemPool::AddPool(size_t iSize, size_t iHandle)
{
	s_apMemPools.push_back(new CMemPool());

	// Pre-allocate our memory
	CMemPool* pPool = s_apMemPools[s_apMemPools.size()-1];
	pPool->m_iMemPoolSize = iSize;
	pPool->m_pMemPool = malloc(pPool->m_iMemPoolSize);
	pPool->m_iMemoryAllocated = 0;
	pPool->m_iHandle = iHandle;

	TAssert(pPool->m_pMemPool);

	return pPool;
}

void CMemPool::ClearPool(size_t iHandle)
{
	for (size_t i = s_apMemPools.size()-1; i < s_apMemPools.size(); i--)
	{
		if (s_apMemPools[i]->m_iHandle != iHandle)
			continue;

		delete s_apMemPools[i];
		s_apMemPools.erase(s_apMemPools.begin()+i);
	}
}

void* CMemPool::Reserve(void* pLocation, size_t iSize, CMemChunk* pAfter)
{
	if (!m_pAllocMap)
	{
		m_pAllocMap = m_pAllocMapBack = (CMemChunk*)pLocation;
		m_pAllocMap->m_pMem = (void*)((size_t)pLocation + sizeof(CMemChunk));
		m_pAllocMap->m_iSize = iSize;
		m_pAllocMap->m_pNext = NULL;
		m_iMemoryAllocated = sizeof(CMemChunk) + iSize;
		return m_pAllocMap->m_pMem;
	}

	CMemChunk* pChunk = (CMemChunk*)pLocation;
	pChunk->m_pMem = (void*)((size_t)pLocation + sizeof(CMemChunk));
	pChunk->m_iSize = iSize;

	if (!pAfter)
	{
		pChunk->m_pNext = m_pAllocMap;
		m_pAllocMap = pChunk;
	}
	else
	{
		pChunk->m_pNext = pAfter->m_pNext;
		pAfter->m_pNext = pChunk;

		if (m_pAllocMapBack == pAfter)
			m_pAllocMapBack = pChunk;
	}

	m_iMemoryAllocated += sizeof(CMemChunk) + iSize;

	return pChunk->m_pMem;
}

void* CMemPool::Alloc(size_t iSize, size_t iHandle)
{
	if (!s_apMemPools.size())
	{
		CMemPool::AddPool(256 * sizeof(float), 0);	// 1kb
		s_iMemoryAllocated = 0;
	}

	void* pReturn = NULL;
	for (unsigned int i = 0; i < s_apMemPools.size(); i++)
	{
		CMemPool* pPool = s_apMemPools[i];

		if (pPool->m_iHandle != iHandle)
			continue;

		// Easy out, is there not enough room?
		if (pPool->m_iMemoryAllocated + sizeof(CMemChunk) + iSize > pPool->m_iMemPoolSize)
			continue;

		// Easy out, are there no elements yet?
		else if (!pPool->m_pAllocMap)
			pReturn = pPool->Reserve(pPool->m_pMemPool, iSize);

		// Easy out, is there room at the beginning?
		else if ((size_t)pPool->m_pAllocMap - (size_t)pPool->m_pMemPool >= iSize + sizeof(CMemChunk))
			pReturn = pPool->Reserve(pPool->m_pMemPool, iSize, NULL);

		// Easy out, is there room at the end?
		else if ((size_t)pPool->m_pAllocMapBack->m_pMem + pPool->m_pAllocMapBack->m_iSize + iSize + sizeof(CMemChunk) <= (size_t)pPool->m_pMemPool + pPool->m_iMemPoolSize)
			pReturn = pPool->Reserve((void*)((size_t)pPool->m_pAllocMapBack->m_pMem + pPool->m_pAllocMapBack->m_iSize), iSize, pPool->m_pAllocMapBack);

		else
		{
			// Find a free spot large enough.
			for (CMemChunk* pChunk = pPool->m_pAllocMap; pChunk; pChunk = pChunk->m_pNext)
			{
				if ((size_t)pChunk->m_pNext - ((size_t)pChunk->m_pMem + pChunk->m_iSize) >= iSize + sizeof(CMemChunk))
				{
					// If there is enough space here to insert a new piece of memory, then we'll use it.
					pReturn = pPool->Reserve((void*)((size_t)pChunk->m_pMem + pChunk->m_iSize), iSize, pChunk);
					break;
				}
			}
		}

		// If a proper space was located in the above code, don't search any more pools.
		if (pReturn)
			break;
	}

	if (!pReturn)
	{
		// Create a new one same size as the old plus a little more, so effectively we have a little more than 2x as much memory now.
		CMemPool* pPool = CMemPool::AddPool(s_iMemoryAllocated+iSize+sizeof(CMemChunk), iHandle);
		pReturn = pPool->Reserve(pPool->m_pMemPool, iSize);
	}

	TAssert(pReturn);

	s_iMemoryAllocated += sizeof(CMemChunk) + iSize;
	return pReturn;
}

void CMemPool::Free(void* p, size_t iHandle)
{
	if (!p)
		return;

	bool bFound = false;
	for (size_t i = 0; i < s_apMemPools.size(); i++)
	{
		CMemPool* pPool = s_apMemPools[i];

		if (pPool->m_iHandle != iHandle)
			continue;

		CMemChunk* pLast = NULL;
		for (CMemChunk* pChunk = pPool->m_pAllocMap; pChunk; pLast = pChunk, pChunk = pChunk->m_pNext)
		{
			if (pChunk->m_pMem != p)
				continue;

			bFound = true;
			s_iMemoryAllocated -= sizeof(CMemChunk) + pChunk->m_iSize;
			pPool->m_iMemoryAllocated -= sizeof(CMemChunk) + pChunk->m_iSize;

			if (pLast)
				pLast->m_pNext = pChunk->m_pNext;

			if (pChunk == pPool->m_pAllocMap)
				pPool->m_pAllocMap = pChunk->m_pNext;

			if (pChunk == pPool->m_pAllocMapBack)
				pPool->m_pAllocMapBack = pLast;

			// Don't delete the last pool.
			if (!pPool->m_iMemoryAllocated && s_apMemPools.size() > 1)
			{
				delete pPool;
				s_apMemPools.erase(s_apMemPools.begin()+i);
			}

			break;
		}

		if (bFound)
			break;
	}

	TAssert(bFound);
}

size_t CMemPool::GetMemPoolHandle()
{
	return ++s_iLastMemPoolHandle;
}

void* mempool_alloc(size_t iSize, size_t iHandle)
{
	return CMemPool::Alloc(iSize, iHandle);
}

void mempool_free(void* p, size_t iHandle)
{
	CMemPool::Free(p, iHandle);
}

size_t mempool_gethandle()
{
	return CMemPool::GetMemPoolHandle();
}

void mempool_clearpool(size_t iHandle)
{
	CMemPool::ClearPool(iHandle);
}
