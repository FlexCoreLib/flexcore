
#include <string>

namespace fc
{

/**
 * token for testing that cannot be copied only moved.
 */
class move_token
{
public:
	move_token(std::string v) noexcept : value_(v) {}
	move_token() = default;
	move_token(move_token&) = delete;
	move_token(move_token&&) = default;
	move_token& operator= (move_token&) = delete;
	move_token& operator= (move_token&&) = default;

	std::string& value() { return value_; }
	const std::string& value() const { return value_; }

	bool operator< (const move_token& other) const { return value_ < other.value_; }

private:
	std::string value_;
};

} // namespace fc


