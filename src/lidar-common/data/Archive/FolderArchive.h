#pragma once

#include <data/Archive/IArchive.h>
#include <util/MappedFile.h>

#include <filesystem>

class FolderArchive : public IArchive
{
public:
	FolderArchive(std::string folder) : IArchive(folder, IArchive::Type::Folder)
	{
		m_path = folder;
	}

	virtual bool LoadArchive() override { return std::filesystem::exists(m_path); }
	virtual bool UnloadArchive() override { return true; }

	virtual bool ContainsFile(std::string_view filename) override
	{
		return std::filesystem::exists(m_path / filename);
	}
	virtual bool OpenFile(std::string_view filename, uint8_t** out_data, uint32_t* out_size) override
	{
		if (!out_data || !out_size)
		{
			printf("Tried to open file with null outputs.\n");
			return false;
		}

		MappedFile file((m_path / filename).string(), MappedFileMode::EREAD, MappedFileOpenMode::EOPEN_EXISTING);
		if (!file.IsOpen())
		{
			printf("Tried to open file but didn't open.\n");
			return false;
		}
		*out_data = (uint8_t*)malloc(file.GetSize());
		*out_size = (uint32_t)file.GetSize();
		memcpy(*out_data, file.GetData(), file.GetSize());

		return true;
	}
	virtual bool SaveFile(std::string_view filename, uint8_t* data, uint32_t size) override
	{
		if (!data)
		{
			printf("Tried to open file with null outputs.\n");
			return false;
		}

		MappedFile file((m_path / filename).string(), size, MappedFileMode::EWRITE, MappedFileOpenMode::ECREATE_ALWAYS);
		if (!file.IsOpen())
		{
			printf("Tried to open file but didn't open.\n");
			return false;
		}
		memcpy(file.GetData(), data, size);
		return true;
	}

private:
	std::filesystem::path m_path;
};