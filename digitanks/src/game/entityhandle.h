#ifndef DT_ENTITYHANDLE_H
#define DT_ENTITYHANDLE_H

#include <assert.h>

template <class C>
class CEntityHandle
{
public:
	CEntityHandle()
	{
		m_iHandle = ~0;
	}

	CEntityHandle(C* pEntity)
	{
		m_iHandle = pEntity->GetHandle();
	}

public:
	inline const CEntityHandle& operator=(const C* pEntity)
	{
		m_iHandle = pEntity->GetHandle();
		return *this;
	}

	inline bool operator==(C* pEntity)
	{
		return IsEqual(pEntity);
	}

	inline bool operator==(const C* pEntity) const
	{
		return IsEqual(pEntity);
	}

	inline bool operator!=(C* pEntity)
	{
		return !IsEqual(pEntity);
	}

	inline bool operator!=(const C* pEntity) const
	{
		return !IsEqual(pEntity);
	}

	inline operator C*() const
	{
		return GetPointer();
	}

	inline C* operator->() const
	{
		return GetPointer();
	}

	inline bool IsEqual(const C* pOther)
	{
		if (!pOther)
		{
			if (!CBaseEntity::GetEntity(m_iHandle))
				return true;

			return m_iHandle == ~0;
		}

		return m_iHandle == pOther->GetHandle();
	}

	inline C* GetPointer() const
	{
		assert(CBaseEntity::GetEntity(m_iHandle));
		return dynamic_cast<C*>(CBaseEntity::GetEntity(m_iHandle));
	}

	size_t	GetHandle() { return m_iHandle; }

protected:
	size_t	m_iHandle;
};

#endif