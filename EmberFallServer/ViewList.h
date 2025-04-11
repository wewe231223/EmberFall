#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewList.h
// 
// 2025 - 02 - 12 : 패킷양을 줄이기 위한 ViewList
//                  매번 업데이트마다 시야 범위 내에 오브젝트가 있는지 검사하고  정보를 보낸다.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViewList {
public:
    ViewList();
    ~ViewList();

    ViewList(const ViewList& other);
    ViewList(ViewList&& other) noexcept;
    ViewList& operator=(const ViewList& other);
    ViewList& operator=(ViewList&& other) noexcept;

public:
    bool IsInList(NetworkObjectIdType id) const;
    bool TryInsert(NetworkObjectIdType id);
    std::unordered_set<NetworkObjectIdType> GetCurrViewList() const;

public:
    GameUnits::GameUnit<GameUnits::Meter> mViewRange{ 100.0m };

private:
    std::unordered_set<NetworkObjectIdType> mViewList{ };
};