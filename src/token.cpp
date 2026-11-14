#include "token.hpp"

#include <utility>

std::unique_ptr<Token_Value> create_token(Token_Type token_type, std::string token_name, Source_Location location)
{
    return std::make_unique<Token_Value>(Token_Value{token_type, std::move(token_name), location});
}
