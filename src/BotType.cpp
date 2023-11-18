#include "BotType.hpp"

namespace Bot
{

std::string ListingBotStatus::str()
{
    std::stringstream os;
    os << *this;
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const ListingBotStatus& status)
{
    switch (status.status)
    {
        case ListingBotStatus::Status::Aborted:
            os << "Listing bot aborted"; 
            break;
        case ListingBotStatus::Status::OpenPosition:
            os << "Listing bot aborted with *** OPEN POSITION ***";
            break;
        case ListingBotStatus::Status::Ok:
             os << "Pnl: " << status.pnl << " USDT, " << (status.pnl/status.amount ) * Quantity("100") << "%."
                << " Buy: " << status.buyPrice << " USDT."
                << " Sell: " << status.sellPrice << " USDT."
                << " Amount invested: " << status.amount << " USDT.";
            break;
        default:
            os << "Unkonwn status " << magic_enum::enum_name(status.status);
    }
    return os;
}

}