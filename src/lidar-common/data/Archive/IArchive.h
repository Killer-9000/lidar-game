#pragma once

#include <string>
#include <string_view>

class IArchive
{
public:
	enum class Type
	{
		None,
		Folder
	};

	virtual bool LoadArchive() = 0;
	virtual bool UnloadArchive() = 0;

	virtual bool ContainsFile(std::string_view) = 0;
	virtual bool OpenFile(std::string_view, uint8_t**, uint32_t*) = 0;
	virtual bool SaveFile(std::string_view, uint8_t*, uint32_t) = 0;

	Type GetArchiveType() const { return m_type; }
	const std::string& GetArchiveName() const { return m_filename; }

protected:
	IArchive(std::string name, Type type)
		: m_filename{ name }, m_type{ type }
	{

	}

	std::string m_filename;
	Type m_type;
};
