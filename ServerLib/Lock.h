#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Lock.h
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: SRWLock 추가
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Lock {
    class SRWLock {
    public:
        SRWLock();
        ~SRWLock();

        SRWLock(const SRWLock&) = delete;
        SRWLock(SRWLock&&) noexcept = delete;
        SRWLock& operator=(const SRWLock&) = delete;
        SRWLock& operator=(SRWLock&&) noexcept = delete;

    public:
        void WriteLock();
        void WriteUnlock();
        void ReadLock();
        void ReadUnlock();

        bool TryWriteLock();
        bool TryReadLock();

    private:
        SRWLOCK mLock{ };
    };

    enum class SRWLockMode : BYTE {
        SRW_SHARED,
        SRW_EXCLUSIVE,

    };

    enum class SRWLockTry : BYTE {
        SRW_FORCE,
        SRW_TRY
    };

    class SRWLockGuard {
    public:
        explicit SRWLockGuard(SRWLockMode mode, SRWLock& lock, SRWLockTry tryMode = SRWLockTry::SRW_FORCE);
        ~SRWLockGuard();

    public:
        bool IsLocking() const;

    private:
        SRWLock& mLock;
        bool mLocked{ false };
        SRWLockMode mMode{ };
    };
}