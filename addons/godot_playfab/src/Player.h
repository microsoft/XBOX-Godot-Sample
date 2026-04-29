#pragma once

// Title's representation of a Player (Not PlayFab specific)
class Player
{
public:
    Player() noexcept = default;
    Player(Player const&) noexcept = default;
    Player& operator=(Player const&) noexcept = default;
    virtual ~Player() noexcept = default;

    // Id associated with this player. Implementation will be platform specific
    virtual std::string const& Id() const = 0;

    // Get default player. Implementation will be platform specific
    static std::shared_ptr<Player> GetPlayer();
};
