#ifndef SRC_CORE_NAMED_HPP_
#define SRC_CORE_NAMED_HPP_

namespace fc
{

/**
 * \brief Base class for named objects
 *
 * The name can be set and is chained with a parents name.
 * A derived class can choose to supply a parent.
 */
class named
{
public:
	explicit named(std::string n = "")
		: own_name_(n)
	{}
	virtual ~named() {}

	virtual std::string own_name() const { return own_name_; }
	std::string full_name() const
	{
		auto p = parent();
		if (p)
			return p->full_name() + "/" + own_name();
		return this->own_name();
	}

	virtual named* name(const std::string& n)
	{
		own_name_ = n;
		return this; // return this for parameter chaining
	}

protected:
	virtual named* parent() const { return 0; }

private:
	std::string own_name_;
};

} // namespace fc

#endif /* SRC_CORE_NAMED_HPP_ */
