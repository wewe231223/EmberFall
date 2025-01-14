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

        bool TryWrietLock();
        bool TryReadLock();

    private:
        SRWLOCK mLock{ };
    };

    enum class SRW_MODE : BYTE {
        SRW_READ,
        SRW_WRITE = 0x01,
        SRW_TRY_READ = 0xF0,
        SRW_TRY_WRITE = 0xF1,

        MODE_TRY = 0xF0,
    };

    class SRWLockGuard {
    public:
        explicit SRWLockGuard(SRW_MODE mode, SRWLock& lock);
        ~SRWLockGuard();

    private:
        SRWLock& mLock;
        std::atomic_bool mLocked{ false };
        SRW_MODE mMode{ };
    };
}