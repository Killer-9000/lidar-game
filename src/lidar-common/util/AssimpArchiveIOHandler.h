#pragma once

#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

class AssimpArchiveIOHandler : public Assimp::IOSystem
{
  class ArchiveStream : public Assimp::IOStream
  {
  public:
    ArchiveStream(uint8_t* data, size_t size) : Assimp::IOStream()
    {
      m_data = data;
      m_size = size;
    }

    ~ArchiveStream()
    {
      free(m_data);
    }

    virtual size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override
    {
      if (pvBuffer == nullptr)
        return 0;
      else if (pSize * pCount == 0)
        return 0;
      else if (pSize * pCount > m_size)
        return 0;
      else if (m_pos + pSize * pCount > m_size)
        return 0;
      else if (pSize * pCount < 0)
        return 0;

      memcpy(pvBuffer, m_data + m_pos, pSize * pCount);
      return pCount;
    }

    virtual size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override
    {
      // NOTE: Not currently possible.
      return 0;
    }

    virtual aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override
    {
      switch (pOrigin)
      {
      case aiOrigin_SET:
        m_pos = pOffset;
        break;
      case aiOrigin_CUR:
        m_pos += pOffset;
        break;
      case aiOrigin_END:
        m_pos = m_size - pOffset;
        break;
      }

      if (m_pos < 0 || m_pos > m_size)
        return aiReturn_FAILURE;

      return aiReturn_SUCCESS;
    }

    virtual size_t Tell() const override
    {
      return m_pos;
    }

    virtual size_t FileSize() const override
    {
      return m_size;
    }

    virtual void Flush() override
    {
      // NOTE: Not currently possible.
    }

    uint8_t* m_data;
    size_t m_size;
    size_t m_pos = 0;
  };
public:
	AssimpArchiveIOHandler(std::string baseDir) : Assimp::IOSystem()
	{
    m_baseDir = baseDir;
	}

  virtual bool Exists(const char* pFile) const override
  {
    return SArchiveMgr->FileExists((m_baseDir / pFile).string());
  }

  virtual char getOsSeparator() const override
  {
    return '/';
  }

  virtual Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override
  {
    uint8_t* fileData; uint32_t fileSize;
    if (!SArchiveMgr->OpenFile((m_baseDir / pFile).string(), &fileData, &fileSize))
    {
      printf("Failed to open file '%s'.\n", pFile);
      return nullptr;
    }
    return new ArchiveStream(fileData, fileSize);
  }

  virtual void Close(Assimp::IOStream* pFile) override
  {
    delete (ArchiveStream*)pFile;
  }

  std::filesystem::path m_baseDir;
};