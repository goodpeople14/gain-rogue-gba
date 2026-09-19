#ifndef BGM_ID_H
#define BGM_ID_H

enum class BgmId
{
    NONE,
    STAGE
};

[[nodiscard]] constexpr bool should_start_bgm(BgmId current, BgmId requested)
{
    return requested != BgmId::NONE && current != requested;
}

#endif
