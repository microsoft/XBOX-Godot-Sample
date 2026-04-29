#include "pch.h"

class GenericPlayer : public Player
{
public:
    GenericPlayer(std::string&& id);
    std::string const& Id() const noexcept override;

private:
    std::string m_id;
};

GenericPlayer::GenericPlayer(std::string&& id) : m_id{ std::move(id) }
{
}

std::string const& GenericPlayer::Id() const noexcept
{
    return m_id;
}

std::shared_ptr<Player> Player::GetPlayer()
{
    static const char* s_defaultPlayerId = s_defaultCustomId;
    return std::make_shared<GenericPlayer>(std::string(s_defaultPlayerId));
}