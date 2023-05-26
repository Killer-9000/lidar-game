#include "SceneLoader.h"

#include <util/MappedFile.h>
#include <views/WorldView.h>

static constexpr uint32_t SceneFormatVer = 1;

// Helpers
namespace
{
	bool CompareValue(uint8_t* data, const char* value, size_t size)
	{
		return memcmp(data, value, size) == 0;
	}

	bool CompareValue(uint8_t* data, int8_t value, size_t size = 1)
	{
		return *(int8_t*)data == value;
	}
	bool CompareValue(uint8_t* data, uint8_t value, size_t size = 1)
	{
		return *data == value;
	}

	bool CompareValue(uint8_t* data, int16_t value, size_t size = 2)
	{
		return *(int16_t*)data == value;
	}
	bool CompareValue(uint8_t* data, uint16_t value, size_t size = 2)
	{
		return *(uint16_t*)data == value;
	}

	bool CompareValue(uint8_t* data, int32_t value, size_t size = 4)
	{
		return *(int32_t*)data == value;
	}
	bool CompareValue(uint8_t* data, uint32_t value, size_t size = 4)
	{
		return *(uint32_t*)data == value;
	}

	bool CompareValue(uint8_t* data, int64_t value, size_t size = 8)
	{
		return *(int64_t*)data == value;
	}
	bool CompareValue(uint8_t* data, uint64_t value, size_t size = 8)
	{
		return *(uint64_t*)data == value;
	}
}

bool SceneLoader::Load(WorldView* worldView, const std::string& filename)
{
	MappedFile file(filename, MappedFileMode::EREAD, MappedFileOpenMode::EOPEN_EXISTING);

	if (!file.IsOpen())
	{
		printf("Failed to open model file '%s'.\n", filename.c_str());
		return false;
	}

	uint8_t* data = file.GetData();
	size_t size = file.GetSize();

	if (size < 4)
		return false;

	if (!CompareValue(data, 'SCNE', 4))
		return false;
	data += 4;

	if (!CompareValue(data, 4, 4))
		return false;
	data += 4;

	if (!CompareValue(data, SceneFormatVer))
		return false;
	data += 4;

	while (data < file.GetData() + file.GetSize())
	{
		char* magic_char = (char*)data;
		uint32_t magic = *(uint32_t*)data; data += 4;
		uint32_t size = *(uint32_t*)data; data += 4;

		switch (magic)
		{
		case 'OBJS':
		{
			// TODO: Implement
			data += size;
		} break;
		default:
			printf("Unknown chunk '%c%c%c%c' found whilst parsing Scene file '%s'.\n", magic_char[0], magic_char[1], magic_char[2], magic_char[3], filename.c_str());
			data += size;
		}
	}

	return true;
}

bool SceneLoader::Save(WorldView* worldView, const std::string& filename)
{
	uint8_t* data = (uint8_t*)calloc(1024, 1);
	assert(data && "Failed to allocate data array for Scene saving.");

	// Scene chunk
	memcpy(data, "SCNE", 4); data += 4;
	memset(data, 4, 4); data += 4;
	memset(data, SceneFormatVer, 4); data += 4;

	// Objects chunk
	// TODO: Implement
	memcpy(data, "OBJS", 4); data += 4;
	memset(data, 0, 4); data += 4;

	return true;
}
