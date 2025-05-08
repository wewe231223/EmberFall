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

//#ifdef _DEBUG || DEBUG
//    if (false == mLocked) {
//        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "SRWLock -> Locking Failure");
//    }
//#endif
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

Lock::ScopedSRWLock::ScopedSRWLock(SRWLock& lock1, SRWLockMode mode1, SRWLock& lock2, SRWLockMode mode2) {
    auto sort = std::addressof(lock1) > std::addressof(lock2);
    if (sort) {
        mLockPairs = std::make_pair(std::make_pair(std::addressof(lock2), mode2), std::make_pair(std::addressof(lock1), mode1));
    }
    else {
        mLockPairs = std::make_pair(std::make_pair(std::addressof(lock1), mode1), std::make_pair(std::addressof(lock2), mode2));
    }

    // locking
    if (SRWLockMode::SRW_SHARED == mLockPairs.first.second) {
        mLockPairs.first.first->ReadLock();
    }
    else {
        mLockPairs.first.first->WriteLock();
    }

    if (SRWLockMode::SRW_SHARED == mLockPairs.second.second) {
        mLockPairs.second.first->ReadLock();
    }
    else {
        mLockPairs.second.first->WriteLock();
    }
}

Lock::ScopedSRWLock::~ScopedSRWLock() {
    // unlocking
    if (SRWLockMode::SRW_SHARED == mLockPairs.second.second) {
        mLockPairs.second.first->ReadUnlock();
    }
    else {
        mLockPairs.second.first->WriteUnlock();
    }

    if (SRWLockMode::SRW_SHARED == mLockPairs.first.second) {
        mLockPairs.first.first->ReadUnlock();
    }
    else {
        mLockPairs.first.first->WriteUnlock();
    }
}
