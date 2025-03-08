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

bool Lock::SRWLock::TryWrietLock() {
    return ::TryAcquireSRWLockExclusive(&mLock);
}

bool Lock::SRWLock::TryReadLock() {
    return ::TryAcquireSRWLockShared(&mLock);
}

Lock::SRWLockGuard::SRWLockGuard(SRWLockMode mode, SRWLock& lock)
    : mLock{ lock }, mMode{ mode }, mLocked { false } {
    if (SRWLockMode::SRW_SHARED == mode) {
        mLock.ReadLock();
        mLocked.exchange(true);
    }
    else {
        mLock.WriteLock();
        mLocked.exchange(true);
    }
}

Lock::SRWLockGuard::~SRWLockGuard() {
    if (SRWLockMode::SRW_SHARED == mMode) {
        mLock.ReadUnlock();
        mLocked.exchange(false);
    }
    else {
        mLock.WriteUnlock();
        mLocked.exchange(false);
    }
}
