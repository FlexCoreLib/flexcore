#ifndef SRC_SERIALISATION_CONFIGFILE_HPP_
#define SRC_SERIALISATION_CONFIGFILE_HPP_

#include <cereal/archives/json.hpp>

#include <memory>

namespace fc
{

class config_file
{
public:
	typedef cereal::JSONInputArchive archive_t;

	explicit config_file(std::istream& stream)
	: iarchive(stream)
	, config_elements()
	{
	}

	void load()
	{
		for (auto& config : config_elements)
			config->load(iarchive);
	}

	/**
	 * \brief creates port returning value of type data_t corresponding to name
	 *
	 * \tparam data_t type of data corresponding to this port
	 * \return a passive source, which provides state of type data_t
	 *
	 */
	template<class data_t>
	auto out_value(const std::string& name);

private:
	archive_t iarchive;

	class archive_loadable
	{
	public:
		virtual void load(archive_t&) = 0;
		virtual ~archive_loadable()
		{
		}
	};

	template<class data_t>
	class config_value: public archive_loadable
	{
	public:
		config_value(const std::string& name, const data_t intial_value) :
				stored_value(intial_value), name(name)
		{
		}

		data_t stored_value;
		std::string name;

		void load(archive_t& archive) override
		{
			archive(cereal::make_nvp(name, stored_value));
		}
	};

	std::vector<std::shared_ptr<archive_loadable>> config_elements;

};

} /* namespace fc */

template<class data_t>
inline auto fc::config_file::out_value(const std::string& name)
{
	//currently forces data_t to be default constructible. is this acceptable?
	auto config = std::make_shared<config_value<data_t>>(name, data_t());
	config_elements.push_back(config);
	return [config]()
	{
		return config->stored_value;
	};
}

#endif /* SRC_SERIALISATION_CONFIGFILE_HPP_ */
