#pragma once

namespace NetworkUtil {

    template <typename MaybeIter>
    concept IsIterator = std::contiguous_iterator<MaybeIter> or std::random_access_iterator<MaybeIter>
        or std::bidirectional_iterator<MaybeIter> or std::forward_iterator<MaybeIter>;

    template <std::contiguous_iterator Iter>
    inline typename Iter::value_type* AddressOf(Iter iter)
    {
        return &(*iter);
    }

    template <std::random_access_iterator Iter>
    inline typename Iter::value_type* AddressOf(Iter iter)
    {
        return &(*iter);
    }

    template <typename T>
    bool SetSocketOpt(SOCKET socket, int level, int option, T optval)
    {
        return SOCKET_ERROR != ::setsockopt(socket, level, option, reinterpret_cast<T>(optval), sizeof(optval));
    }
}
