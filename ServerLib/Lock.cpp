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

Lock::SRWLockGuard::SRWLockGuard(SRW_MODE mode, SRWLock& lock)
    : mLock{ lock }, mMode{ mode }, mLocked { false } {
    if (SRW_MODE::SRW_READ == mode) {
        mLock.ReadLock();
        mLocked.exchange(true);
    }
    else {
        mLock.WriteLock();
        mLocked.exchange(true);
    }
}

Lock::SRWLockGuard::~SRWLockGuard() {
    if (SRW_MODE::SRW_READ == mMode) {
        mLock.ReadUnlock();
        mLocked.exchange(false);
    }
    else {
        mLock.WriteUnlock();
        mLocked.exchange(false);
    }
}
