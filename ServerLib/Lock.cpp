#include "pch.h"
#include "Lock.h"

Lock::SRWLock::SRWLock() {
    ::InitializeSRWLock(&mLock);
}

Lock::SRWLock::~SRWLock() { }

void Lock::SRWLock::WriteLock() {
    ::AcquireSRWLockExclusive(&mLock);
}

void Lock::SRWLock::WriteUnlock() {
    ::ReleaseSRWLockExclusive(&mLock);
}

void Lock::SRWLock::ReadLock() {
    ::AcquireSRWLockShared(&mLock);
}

void Lock::SRWLock::ReadUnlock() {
    ::ReleaseSRWLockShared(&mLock);
}

bool Lock::SRWLock::TryWriteLock() {
    return ::TryAcquireSRWLockExclusive(&mLock);
}

bool Lock::SRWLock::TryReadLock() {
    return ::TryAcquireSRWLockShared(&mLock);
}

Lock::SRWLockGuard::SRWLockGuard(SRWLockMode mode, SRWLock& lock, SRWLockTry tryMode)
    : mLock{ lock }, mMode{ mode }, mLocked{ false } {
    if (SRWLockTry::SRW_FORCE == tryMode) {
        if (SRWLockMode::SRW_SHARED == mode) {
            mLock.ReadLock();
        }
        else {
            mLock.WriteLock();
        }

        mLocked = true;
    }
    else {
        if (SRWLockMode::SRW_SHARED == mode) {
            mLocked = mLock.TryReadLock();
        }
        else {
            mLocked = mLock.TryWriteLock();
        }
    }

#ifdef _DEBUG || DEBUG
    if (false == mLocked) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "SRWLock -> Locking Failure");
    }
#endif
}

bool Lock::SRWLockGuard::IsLocking() const {
    return mLocked;
}

Lock::SRWLockGuard::~SRWLockGuard() {
    if (false == mLocked) {
        return;
    }

    if (SRWLockMode::SRW_SHARED == mMode) {
        mLock.ReadUnlock();
    }
    else {
        mLock.WriteUnlock();
    }
}
