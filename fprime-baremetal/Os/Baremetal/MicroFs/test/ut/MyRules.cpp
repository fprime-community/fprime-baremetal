
    
#include "Tester.hpp"
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <Os/MicroFs/MicroFs.hpp>
#include <Fw/Types/AlignedAllocator.hpp>
#include <Fw/Test/UnitTest.hpp>



  // ------------------------------------------------------------------------------------------------------
  // Rule:  InitFileSystem
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::InitFileSystem::InitFileSystem(U32 numBins, U32 fileSize, U32 numFiles) :
        STest::Rule<Os::Tester>("InitFileSystem")
  {
    this->numBins = numBins;
    this->fileSize = fileSize;
    this->numFiles = numFiles;
  }


  bool Os::Tester::InitFileSystem::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::InitFileSystem::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s \n", this->name);

    ASSERT_LE(this->numBins, Os::MAX_MICROFS_BINS);

    Os::MicroFsSetCfgBins(state.testCfg, this->numBins);


    for (U16 i=0; i < this->numBins; i++)
    {
        Os::MicroFsAddBin(state.testCfg, i, this->fileSize, this->numFiles);
    }
    
    Os::MicroFsInit(state.testCfg, 0, state.alloc);

    for (I32 i=0; i < MAX_TOTAL_FILES; i++) 
    {
      state.fileModels[i].clear();
    }

    state.simFileSystem = new SimFileSystem(this->numBins, this->numFiles, Tester::FILE_SIZE);

  }


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenFile::OpenFile(const char *filename) :
        STest::Rule<Os::Tester>("OpenFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);

      return ((fileModel->mode == Os::Tester::FileModel::CLOSED) ||
              (fileModel->mode == Os::Tester::FileModel::DOESNT_EXIST));
  }

  
  void Os::Tester::OpenFile::action(
            Os::Tester& state //!< The test state
        ) 
  {

    printf("--> Rule: %s %s\n", this->name, this->filename);

    this->fileModel->curPtr = 0;
    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_WRITE);
    ASSERT_EQ(Os::File::OP_OK, stat);
    
    // This is just a dummy call to get code coverage.  Nothing happens here for this file system.
    stat = this->fileModel->fileDesc.prealloc(0,0);
    ASSERT_EQ(Os::File::OP_OK, stat);

    this->fileModel->mode = Os::Tester::FileModel::OPEN_WRITE;
    if (this->fileModel->size == -1)
    {
      this->fileModel->size = 0;
    }
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  Cleanup
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::Cleanup::Cleanup() :
        STest::Rule<Os::Tester>("Cleanup")
  {
  }


  bool Os::Tester::Cleanup::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::Cleanup::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s \n", this->name);

    Os::MicroFsCleanup(0,state.alloc);
    delete state.simFileSystem;

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  WriteData
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::WriteData::WriteData(const char *filename) :
        STest::Rule<Os::Tester>("WriteData")
  {
    this->filename = filename;
    
  }


  bool Os::Tester::WriteData::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
    this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
    return  (fileModel->mode == Os::Tester::FileModel::OPEN_WRITE);

  }

  
  void Os::Tester::WriteData::action(
            Os::Tester& state //!< The test state
        ) 
  {
    
    NATIVE_UINT_TYPE fillSize;

    // Randomize how many bytes are written to the file
    NATIVE_INT_TYPE randSize = rand() % Tester::FILE_SIZE + 1;
    
    if ((fileModel->curPtr + randSize) > Tester::FILE_SIZE) {
      fillSize = Tester::FILE_SIZE - fileModel->curPtr;
    } else {
      fillSize = randSize;
    }

    printf("--> Rule: %s %s %d bytes\n", this->name, this->filename, fillSize);


    // Fill the memory buffer with random numbers between 0 and 0xFF inclusive

    NATIVE_INT_TYPE offset = fileModel->curPtr;
    for (U32 i=0; i < fillSize; i++)
    {
      assert(offset + i < Tester::FILE_SIZE);
      this->fileModel->buffOut[offset + i] = rand() % 256;
    }
    
    NATIVE_INT_TYPE retSize = randSize;
    Os::File::Status stat = fileModel->fileDesc.write(this->fileModel->buffOut + this->fileModel->curPtr, retSize);
    ASSERT_EQ(stat, Os::File::OP_OK);
    ASSERT_LE(this->fileModel->curPtr + retSize, Tester::FILE_SIZE);

    //Update FileModel
    this->fileModel->curPtr += fillSize;
    // Check if the currSize is to be increased.
    if (fileModel->curPtr > fileModel->size)
    {
        fileModel->size = fileModel->curPtr;
    }


  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  ReadData
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::ReadData::ReadData(const char *filename) :
        STest::Rule<Os::Tester>("ReadData")
  {
    this->filename = filename;
  }


  bool Os::Tester::ReadData::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return  ((fileModel->mode == Os::Tester::FileModel::OPEN_READ) ||
               (fileModel->mode == Os::Tester::FileModel::OPEN_WRITE));
      

  }

  
  void Os::Tester::ReadData::action(
            Os::Tester& state //!< The test state
        ) 
  {

      // Randomize how much data is read
      NATIVE_INT_TYPE randSize = rand() % Tester::FILE_SIZE + 1;

      BYTE buffIn[state.testCfg.bins[0].fileSize];
      memset(buffIn,0xA5,sizeof(buffIn));
      ASSERT_LE(randSize, sizeof(buffIn));
      NATIVE_INT_TYPE retSize = randSize;
      Os::File::Status stat = this->fileModel->fileDesc.read(buffIn, retSize);

      ASSERT_EQ(stat, Os::File::OP_OK);

      ASSERT_LE(retSize, fileModel->size - fileModel->curPtr);

      // Check the returned data
      ASSERT_LE(fileModel->curPtr + retSize, Tester::FILE_SIZE);

      ASSERT_EQ(0,memcmp(buffIn, this->fileModel->buffOut + this->fileModel->curPtr, retSize));

      // Update the FileModel
      fileModel->curPtr += retSize;

      printf("--> Rule: %s %s %d bytes\n", this->name, this->filename, retSize);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  ResetFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::ResetFile::ResetFile(const char *filename) :
        STest::Rule<Os::Tester>("ResetFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::ResetFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode != Os::Tester::FileModel::CLOSED) &&
              (fileModel->mode != Os::Tester::FileModel::DOESNT_EXIST));
      
  }

  
  void Os::Tester::ResetFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      // seek back to beginning
      ASSERT_EQ(Os::File::OP_OK, this->fileModel->fileDesc.seek(0));
      fileModel->curPtr = 0;
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  CloseFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::CloseFile::CloseFile(const char *filename) :
        STest::Rule<Os::Tester>("CloseFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::CloseFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode != Os::Tester::FileModel::CLOSED) && 
              (fileModel->mode != Os::Tester::FileModel::DOESNT_EXIST)); 
      
  }

  
  void Os::Tester::CloseFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    // close file
    this->fileModel->fileDesc.close();
    this->fileModel->mode = Os::Tester::FileModel::CLOSED;

  }


  // ------------------------------------------------------------------------------------------------------
  // Rule:  Listings
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::Listings::Listings(U16 numBins, U16 numFiles) :
        STest::Rule<Os::Tester>("Listings")
  {
    this->numBins = numBins;
    this->numFiles = numFiles;
  }


  bool Os::Tester::Listings::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::Listings::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s \n", this->name);

    Fw::String listDir;
    Fw::String expectedFile;
    Fw::String bins[MAX_BINS];
    Fw::String files[MAX_FILES_PER_BIN];
    U32 totalBins = MAX_BINS + 1; // oversize to check return

    COMMENT("Listing /");
    listDir = "/";

    // get root directory listing
    ASSERT_EQ(Os::FileSystem::OP_OK,
        Os::FileSystem::readDirectory(listDir.toChar(), MAX_BINS, bins, totalBins));
    ASSERT_EQ(this->numBins, totalBins);

    for (U16 binIndex=0;  binIndex < this->numBins; binIndex++)
    {
        printf("%s\n", bins[binIndex].toChar());
        expectedFile.format("/%s%d", MICROFS_BIN_STRING, binIndex);
        ASSERT_EQ(0,strcmp(expectedFile.toChar(), bins[binIndex].toChar()));
    }

    U32 totalFiles = MAX_FILES_PER_BIN + 1; // oversize to check return
    for (U16 binIndex=0; binIndex < this->numBins; binIndex++)
    {
        // get file listing
        ASSERT_EQ(Os::FileSystem::OP_OK,
            Os::FileSystem::readDirectory(bins[binIndex].toChar(), this->numFiles, files, totalFiles));
        ASSERT_EQ(this->numFiles, totalFiles);
        COMMENT(bins[binIndex].toChar());
        for (U16 fileIndex=0; fileIndex < this->numFiles; fileIndex++)
        {
            printf("%s\n", files[fileIndex].toChar());
            expectedFile.format("/%s%d/%s%d", MICROFS_BIN_STRING, binIndex, MICROFS_FILE_STRING, fileIndex);
            ASSERT_EQ(0,strcmp(expectedFile.toChar(), files[fileIndex].toChar()));
        }
    }
  }

  // ------------------------------------------------------------------------------------------------------
  // Rule:  FreeSpace
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::FreeSpace::FreeSpace() :
        STest::Rule<Os::Tester>("FreeSpace")
  {
  }


  bool Os::Tester::FreeSpace::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::FreeSpace::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s \n", this->name);

    FwSizeType total;
    FwSizeType free;

    FileSystem::Status stat = FileSystem::getFreeSpace("", total, free);
    ASSERT_EQ(Os::File::OP_OK, stat);

    ASSERT_EQ(state.m_expFreeBytes,free);
    ASSERT_EQ(state.m_expTotalBytes,total);

  }


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenNoPerm
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenNoPerm::OpenNoPerm(const char *filename) :
        STest::Rule<Os::Tester>("OpenNoPerm")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenNoPerm::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return (this->fileModel->mode != Os::Tester::FileModel::CLOSED);
  }

  
  void Os::Tester::OpenNoPerm::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    this->fileModel->curPtr = 0;
    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_WRITE);
    ASSERT_EQ(Os::File::NO_PERMISSION, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  CheckFileSize
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::CheckFileSize::CheckFileSize(const char *filename) :
        STest::Rule<Os::Tester>("CheckFileSize")
  {
    this->filename = filename;
  }


  bool Os::Tester::CheckFileSize::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return true;
  }

  
  void Os::Tester::CheckFileSize::action(
            Os::Tester& state //!< The test state
        ) 
  {
      FwSizeType actualSize;
      FileSystem::Status stat = FileSystem::getFileSize(this->filename, actualSize);
      ASSERT_EQ(FileSystem::OP_OK, stat);

      ASSERT_EQ(actualSize, this->fileModel->size);
      printf("--> Rule: %s %s, size = %d\n", this->name, this->filename, this->fileModel->size);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenRead
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenRead::OpenRead(const char* filename) :
        STest::Rule<Os::Tester>("OpenRead")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenRead::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return (this->fileModel->mode == Os::Tester::FileModel::CLOSED);
  }

  
  void Os::Tester::OpenRead::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_READ);
    ASSERT_EQ(Os::File::OP_OK, stat);

    this->fileModel->curPtr = 0;
    this->fileModel->mode = Os::Tester::FileModel::OPEN_READ;
    if (this->fileModel->size == -1)
    {
      this->fileModel->size = 0;
    }

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenFileNotExist
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenFileNotExist::OpenFileNotExist(const char* filename) :
        STest::Rule<Os::Tester>("OpenFileNotExist")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenFileNotExist::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::OpenFileNotExist::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    Os::File fileDesc;
    Os::File::Status stat = fileDesc.open(this->filename, Os::File::OPEN_READ);
    ASSERT_EQ(Os::File::DOESNT_EXIST, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenReadEarly
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenReadEarly::OpenReadEarly(const char* filename) :
        STest::Rule<Os::Tester>("OpenReadEarly")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenReadEarly::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return (this->fileModel->mode == Os::Tester::FileModel::DOESNT_EXIST);
  }

  
  void Os::Tester::OpenReadEarly::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);
    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_READ);
    ASSERT_EQ(Os::File::DOESNT_EXIST, stat);
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenCreate
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenCreate::OpenCreate(const char* filename) :
        STest::Rule<Os::Tester>("OpenCreate")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenCreate::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((this->fileModel->mode != Os::Tester::FileModel::OPEN_READ) &&
             (this->fileModel->mode != Os::Tester::FileModel::OPEN_WRITE));
  }

  
  void Os::Tester::OpenCreate::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);
    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_CREATE);
    ASSERT_EQ(Os::File::OP_OK, stat);

    this->fileModel->mode = Os::Tester::FileModel::OPEN_WRITE;
    this->fileModel->curPtr = 0;
    this->fileModel->size = 0;

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenAppend
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenAppend::OpenAppend(const char* filename) :
        STest::Rule<Os::Tester>("OpenAppend")
  {
    this->filename = filename;
  }


  bool Os::Tester::OpenAppend::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((this->fileModel->mode != Os::Tester::FileModel::OPEN_READ) &&
             (this->fileModel->mode != Os::Tester::FileModel::OPEN_WRITE));

  }

  
  void Os::Tester::OpenAppend::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    Os::File::Status stat = this->fileModel->fileDesc.open(this->filename, Os::File::OPEN_APPEND);
    ASSERT_EQ(Os::File::OP_OK, stat);

    this->fileModel->mode = Os::Tester::FileModel::OPEN_WRITE;
    if (this->fileModel->size == -1)
    {
      this->fileModel->size = 0;
    }
      
    this->fileModel->curPtr = this->fileModel->size;

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  RemoveFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::RemoveFile::RemoveFile(const char* filename) :
        STest::Rule<Os::Tester>("RemoveFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::RemoveFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return (this->fileModel->mode == Os::Tester::FileModel::CLOSED);

  }

  
  void Os::Tester::RemoveFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    Os::FileSystem::Status stat = Os::FileSystem::removeFile(this->filename);
    ASSERT_EQ(Os::FileSystem::OP_OK, stat);

    this->fileModel->mode = Os::Tester::FileModel::DOESNT_EXIST;
    this->fileModel->curPtr = 0;
    this->fileModel->size = -1;

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  RemoveBusyFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::RemoveBusyFile::RemoveBusyFile(const char* filename) :
        STest::Rule<Os::Tester>("RemoveBusyFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::RemoveBusyFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((this->fileModel->mode != Os::Tester::FileModel::CLOSED) &&
              (this->fileModel->mode != Os::Tester::FileModel::DOESNT_EXIST));
      
  }

  
  void Os::Tester::RemoveBusyFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
    printf("--> Rule: %s %s\n", this->name, this->filename);

    Os::FileSystem::Status stat = Os::FileSystem::removeFile(this->filename);
    ASSERT_EQ(Os::FileSystem::BUSY, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  IsFileOpen
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::IsFileOpen::IsFileOpen(const char* filename) :
        STest::Rule<Os::Tester>("IsFileOpen")
  {
    this->filename = filename;
  }


  bool Os::Tester::IsFileOpen::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return true;
  }

  
  void Os::Tester::IsFileOpen::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);
      bool expState;
      switch (this->fileModel->mode)
      {
        case Os::Tester::FileModel::CLOSED:
        case Os::Tester::FileModel::DOESNT_EXIST:
          expState = false;
          break;
        case Os::Tester::FileModel::OPEN_READ:
        case Os::Tester::FileModel::OPEN_WRITE:
          expState = true;
          break;
      }
      ASSERT_EQ(expState, this->fileModel->fileDesc.isOpen());
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  MoveFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::MoveFile::MoveFile(const char* sourcefile, const char* destfile) :
        STest::Rule<Os::Tester>("MoveFile")
  {
    this->sourcefile = sourcefile;
    this->destfile = destfile;
  }


  bool Os::Tester::MoveFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->sourceModel = const_cast<Os::Tester&>(state).getFileModel(this->sourcefile);
      this->destModel = const_cast<Os::Tester&>(state).getFileModel(this->destfile);

      return (this->sourceModel->mode == Os::Tester::FileModel::CLOSED);

  }

  
  void Os::Tester::MoveFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s source: %s, dest: %s\n", this->name, this->sourcefile, this->destfile);
      Os::FileSystem::Status stat = Os::FileSystem::moveFile(this->sourcefile, this->destfile);
      ASSERT_EQ(Os::FileSystem::Status::OP_OK, stat);

      memcpy(this->sourceModel->buffOut, this->destModel->buffOut, this->sourceModel->size);
      this->destModel->size = this->sourceModel->size;

      // Delete the original file
      this->sourceModel->mode = Os::Tester::FileModel::DOESNT_EXIST;
      this->sourceModel->curPtr = 0;
      this->sourceModel->size = -1;


  }



    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  Directory
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::Directory::Directory(const char* dirpath, bool offNominal) :
        STest::Rule<Os::Tester>("Directory")
  {
    this->dirpath = dirpath;
    this->offNominal = offNominal;
  }


  bool Os::Tester::Directory::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::Directory::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->dirpath);

      Os::FileSystem::Status stat = Os::FileSystem::createDirectory(this->dirpath);

      if (this->offNominal)
      {
        ASSERT_EQ(Os::FileSystem::Status::NO_PERMISSION, stat);
      } else {
        ASSERT_EQ(Os::FileSystem::Status::OP_OK, stat);
      }

      stat = Os::FileSystem::removeDirectory(this->dirpath);

      if (this->offNominal)
      {
        ASSERT_EQ(Os::FileSystem::Status::NO_PERMISSION, stat);
      } else {
        ASSERT_EQ(Os::FileSystem::Status::OP_OK, stat);
      }

  }



    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  SeekFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::SeekFile::SeekFile(const char* filename) :
        STest::Rule<Os::Tester>("SeekFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::SeekFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode ==  Os::Tester::FileModel::OPEN_READ) ||
              (fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE));
  }

  
  void Os::Tester::SeekFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      // Seek random
      NATIVE_INT_TYPE randSeek = rand() % Tester::FILE_SIZE;
      Os::File::Status stat = this->fileModel->fileDesc.seek(randSeek);
      ASSERT_EQ(Os::File::OP_OK, stat);

      // Update the model
      I32 oldSize = this->fileModel->size;
      this->fileModel->curPtr = randSeek;
      if (this->fileModel->curPtr > this->fileModel->size)
      {
        this->fileModel->size = this->fileModel->curPtr;
      }

      // fill with zeros if seek went past old size
      if (this->fileModel->size > oldSize) {
        memset(&this->fileModel->buffOut[oldSize], 0, this->fileModel->size - oldSize);
      }

      
      printf("--> Rule: %s %s %d\n", this->name, this->filename, randSeek);


  }


  // ------------------------------------------------------------------------------------------------------
  // Rule:  SeekNFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::SeekNFile::SeekNFile(const char* filename, NATIVE_INT_TYPE seek) :
        STest::Rule<Os::Tester>("SeekNFile")
  {
    this->filename = filename;
    this->seek = seek;
  }


  bool Os::Tester::SeekNFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode ==  Os::Tester::FileModel::OPEN_READ) ||
              (fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE));
  }

  
  void Os::Tester::SeekNFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      Os::File::Status stat = this->fileModel->fileDesc.seek(this->seek);
      ASSERT_EQ(Os::File::OP_OK, stat);

      // Update the model
      I32 oldSize = this->fileModel->size;

      this->fileModel->curPtr = this->seek;
      if (this->fileModel->curPtr > this->fileModel->size)
      {
        this->fileModel->size = this->fileModel->curPtr;
      }
      
      // fill with zeros if seek went past old size
      if (this->fileModel->size > oldSize) {
        memset(&this->fileModel->buffOut[oldSize], 0, this->fileModel->size - oldSize);
      }

      printf("--> Rule: %s %s %d\n", this->name, this->filename, this->seek);


  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  BulkWrite
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::BulkWrite::BulkWrite(const char* filename) :
        STest::Rule<Os::Tester>("BulkWrite")
  {
    this->filename = filename;
  }


  bool Os::Tester::BulkWrite::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE) ||
              (fileModel->mode ==  Os::Tester::FileModel::OPEN_READ));
  }

  
  void Os::Tester::BulkWrite::action(
            Os::Tester& state //!< The test state
        ) 
  {

    NATIVE_UINT_TYPE fillSize = FILE_SIZE;

    if ((fileModel->curPtr + fillSize) > Tester::FILE_SIZE) {
      fillSize = Tester::FILE_SIZE - fileModel->curPtr;
    }

    Os::File::Status stat = fileModel->fileDesc.bulkWrite(this->fileModel->buffOut + this->fileModel->curPtr, fillSize, fillSize);
    ASSERT_EQ(stat, Os::File::OP_OK);

    //Update FileModel
    this->fileModel->curPtr += fillSize;
    // Check if the currSize is to be increased.
    if (fileModel->curPtr > fileModel->size)
    {
        fileModel->size = fileModel->curPtr;
    }

    printf("--> Rule: %s %s %d bytes\n", this->name, this->filename, fillSize);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  CalcCRC32
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::CalcCRC32::CalcCRC32(const char* filename) :
        STest::Rule<Os::Tester>("CalcCRC32")
  {
    this->filename = filename;
  }


  bool Os::Tester::CalcCRC32::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return true;
  }

  
  void Os::Tester::CalcCRC32::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      U32 crc;
      Os::File::Status stat = fileModel->fileDesc.calculateCRC32(crc);

      if ((fileModel->mode ==  Os::Tester::FileModel::OPEN_READ) ||
          (fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE))
      {

          ASSERT_EQ(stat, Os::File::OP_OK);

          // Update model
          this->fileModel->curPtr = this->fileModel->size;
      
      } else {

          ASSERT_EQ(stat, Os::File::NOT_OPENED);

      }


  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  SeekNotOpen
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::SeekNotOpen::SeekNotOpen(const char* filename) :
        STest::Rule<Os::Tester>("SeekNotOpen")
  {
    this->filename = filename;
  }


  bool Os::Tester::SeekNotOpen::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode !=  Os::Tester::FileModel::OPEN_WRITE) &&
              (fileModel->mode !=  Os::Tester::FileModel::OPEN_READ));

  }

  
  void Os::Tester::SeekNotOpen::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      Os::File::Status stat = this->fileModel->fileDesc.seek(0);
      ASSERT_EQ(Os::File::NOT_OPENED, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  SeekBadSize
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::SeekBadSize::SeekBadSize(const char* filename, NATIVE_INT_TYPE seek) :
        STest::Rule<Os::Tester>("SeekBadSize")
  {
    this->filename = filename;
    this->seek = seek;
  }


  bool Os::Tester::SeekBadSize::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE) ||
              (fileModel->mode ==  Os::Tester::FileModel::OPEN_READ));

  }

  
  void Os::Tester::SeekBadSize::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      Os::File::Status stat = this->fileModel->fileDesc.seek(this->seek);
      ASSERT_EQ(Os::File::BAD_SIZE, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  SeekRelative
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::SeekRelative::SeekRelative(const char* filename, NATIVE_INT_TYPE seek) :
        STest::Rule<Os::Tester>("SeekRelative")
  {
    this->filename = filename;
    this->seek = seek;
  }


  bool Os::Tester::SeekRelative::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE) ||
              (fileModel->mode ==  Os::Tester::FileModel::OPEN_READ));

  }

  
  void Os::Tester::SeekRelative::action(
            Os::Tester& state //!< The test state
        ) 
  {
      Os::File::Status stat = this->fileModel->fileDesc.seek(this->seek, false);
      if (this->fileModel->curPtr + this->seek >= FILE_SIZE)
      {
        ASSERT_EQ(Os::File::BAD_SIZE, stat);

      } else {

        ASSERT_EQ(Os::File::OP_OK, stat);

        // Update the model
        I32 oldSize = this->fileModel->size;

        this->fileModel->curPtr += this->seek;
        if (this->fileModel->curPtr > this->fileModel->size)
        {
          this->fileModel->size = this->fileModel->curPtr;
        }
        
        // fill with zeros if seek went past old size
        if (this->fileModel->size > oldSize) {
          memset(&this->fileModel->buffOut[oldSize], 0, this->fileModel->size - oldSize);
        }

      }

      printf("--> Rule: %s %s %d\n", this->name, this->filename, this->seek);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  ReadNotOpen
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::ReadNotOpen::ReadNotOpen(const char* filename) :
        STest::Rule<Os::Tester>("ReadNotOpen")
  {
    this->filename = filename;
  }


  bool Os::Tester::ReadNotOpen::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode !=  Os::Tester::FileModel::OPEN_WRITE) &&
              (fileModel->mode !=  Os::Tester::FileModel::OPEN_READ));

  }

  
  void Os::Tester::ReadNotOpen::action(
            Os::Tester& state //!< The test state
        ) 
  {
      BYTE buffIn[FILE_SIZE];
      NATIVE_INT_TYPE size = FILE_SIZE;

      printf("--> Rule: %s %s\n", this->name, this->filename);
      Os::File::Status stat = this->fileModel->fileDesc.read(buffIn, size);
      ASSERT_EQ(Os::File::NOT_OPENED, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  WriteNotOpen
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::WriteNotOpen::WriteNotOpen(const char* filename) :
        STest::Rule<Os::Tester>("WriteNotOpen")
  {
    this->filename = filename;
  }


  bool Os::Tester::WriteNotOpen::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode !=  Os::Tester::FileModel::OPEN_WRITE) &&
              (fileModel->mode !=  Os::Tester::FileModel::OPEN_READ));
  }

  
  void Os::Tester::WriteNotOpen::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);
      NATIVE_INT_TYPE size = FILE_SIZE;
      Os::File::Status stat = fileModel->fileDesc.write(this->fileModel->buffOut + this->fileModel->curPtr, size);
      ASSERT_EQ(Os::File::NOT_OPENED, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  BulkWriteNoOpen
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::BulkWriteNoOpen::BulkWriteNoOpen(const char* filename) :
        STest::Rule<Os::Tester>("BulkWriteNoOpen")
  {
    this->filename = filename;
  }


  bool Os::Tester::BulkWriteNoOpen::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return ((fileModel->mode !=  Os::Tester::FileModel::OPEN_WRITE) &&
              (fileModel->mode !=  Os::Tester::FileModel::OPEN_READ));
  }

  
  void Os::Tester::BulkWriteNoOpen::action(
            Os::Tester& state //!< The test state
        ) 
  {
      NATIVE_UINT_TYPE size = FILE_SIZE;
      Os::File::Status stat = fileModel->fileDesc.bulkWrite(this->fileModel->buffOut + this->fileModel->curPtr, size, size);
      ASSERT_EQ(stat, Os::File::NOT_OPENED);

      printf("--> Rule: %s %s\n", this->name, this->filename);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  FlushFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::FlushFile::FlushFile(const char* filename) :
        STest::Rule<Os::Tester>("FlushFile")
  {
    this->filename = filename;
  }


  bool Os::Tester::FlushFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return true;
  }

  
  void Os::Tester::FlushFile::action(
            Os::Tester& state //!< The test state
        ) 
  {

      printf("--> Rule: %s %s\n", this->name, this->filename);

      Os::File::Status stat = fileModel->fileDesc.flush();
      if ((fileModel->mode ==  Os::Tester::FileModel::OPEN_WRITE) ||
          (fileModel->mode ==  Os::Tester::FileModel::OPEN_READ))
      {

        ASSERT_EQ(stat, Os::File::OP_OK);

      } else {

        ASSERT_EQ(stat, Os::File::NOT_OPENED);

      }

  }

  // ------------------------------------------------------------------------------------------------------
  // Rule:  GetErrors
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::GetErrors::GetErrors(const char* filename) :
        STest::Rule<Os::Tester>("GetErrors")
  {
    this->filename = filename;
  }


  bool Os::Tester::GetErrors::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->fileModel = const_cast<Os::Tester&>(state).getFileModel(this->filename);
      return true;
  }

  
  void Os::Tester::GetErrors::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      NATIVE_INT_TYPE lastError = fileModel->fileDesc.getLastError();
      ASSERT_EQ(lastError, 0);

      const char* lastErrorString = fileModel->fileDesc.getLastErrorString();
      ASSERT_STREQ(lastErrorString, "Success");



  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  CopyFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::CopyFile::CopyFile(const char* srcFile, const char* destFile) :
        STest::Rule<Os::Tester>("CopyFile")
  {
    this->srcFile = srcFile;
    this->destFile = destFile;
  }


  bool Os::Tester::CopyFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->srcModel = const_cast<Os::Tester&>(state).getFileModel(this->srcFile);
      this->destModel = const_cast<Os::Tester&>(state).getFileModel(this->destFile);

      return (this->srcModel->mode == Os::Tester::FileModel::CLOSED);
  }

  
  void Os::Tester::CopyFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s Copy %s to %s\n", this->name, this->srcFile, this->destFile);
      Os::FileSystem::Status stat = Os::FileSystem::copyFile(this->srcFile, this->destFile);
      ASSERT_EQ(Os::FileSystem::OP_OK, stat);

      memcpy(this->destModel->buffOut, this->srcModel->buffOut, this->srcModel->size);
      this->destModel->mode = Os::Tester::FileModel::CLOSED;
      this->destModel->size = this->srcModel->size;
      this->destModel->curPtr = 0;

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  AppendFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::AppendFile::AppendFile(const char* srcFile, const char* destFile) :
        STest::Rule<Os::Tester>("AppendFile")
  {
    this->srcFile = srcFile;
    this->destFile = destFile;
  }


  bool Os::Tester::AppendFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      this->srcModel = const_cast<Os::Tester&>(state).getFileModel(this->srcFile);
      this->destModel = const_cast<Os::Tester&>(state).getFileModel(this->destFile);
      return ((this->srcModel->mode == Os::Tester::FileModel::CLOSED) &&
              (this->destModel->mode == Os::Tester::FileModel::CLOSED));
  }

  
  void Os::Tester::AppendFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s Append %s to %s\n", this->name, this->srcFile, this->destFile);
      Os::FileSystem::Status stat = Os::FileSystem::appendFile(this->srcFile, this->destFile);
      ASSERT_EQ(Os::FileSystem::OP_OK, stat);

      I32 addSize = this->srcModel->size;
      // If the size is going to overflow, then limit the addition size
      if ((this->destModel->size + this->srcModel->size) > FILE_SIZE)
      {
        addSize = FILE_SIZE - this->destModel->size;
      }

      memcpy(this->destModel->buffOut + this->destModel->size, this->srcModel->buffOut, addSize);
      this->destModel->size += addSize;
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  MoveInvalid
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::MoveInvalid::MoveInvalid(const char* sourceFile, const char* destFile) :
        STest::Rule<Os::Tester>("MoveInvalid")
  {
    this->sourceFile = sourceFile;
    this->destFile = destFile;
  }


  bool Os::Tester::MoveInvalid::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::MoveInvalid::action(
            Os::Tester& state //!< The test state
        ) 
  {
      
      Os::FileSystem::Status stat = Os::FileSystem::moveFile(this->sourceFile, this->destFile);
      ASSERT_EQ(Os::FileSystem::Status::INVALID_PATH, stat);

      printf("--> Rule: %s %s to %s\n", this->name, this->sourceFile, this->destFile);
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  MoveBusy
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::MoveBusy::MoveBusy(const char* sourceFile, const char* destFile) :
        STest::Rule<Os::Tester>("MoveBusy")
  {
    this->sourceFile = sourceFile;
    this->destFile = destFile;
  }


  bool Os::Tester::MoveBusy::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::MoveBusy::action(
            Os::Tester& state //!< The test state
        ) 
  {
      Os::FileSystem::Status stat = Os::FileSystem::moveFile(this->sourceFile, this->destFile);
      ASSERT_EQ(Os::FileSystem::Status::BUSY, stat);

      printf("--> Rule: %s %s to %s\n", this->name, this->sourceFile, this->destFile);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  ReadDirInvalid
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::ReadDirInvalid::ReadDirInvalid(const char* binPath) :
        STest::Rule<Os::Tester>("ReadDirInvalid")
  {
    this->binPath = binPath;
  }


  bool Os::Tester::ReadDirInvalid::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::ReadDirInvalid::action(
            Os::Tester& state //!< The test state
        ) 
  {
      Fw::String bins[MAX_BINS];
      U32 totalBins = MAX_BINS;

      printf("--> Rule: %s %s\n", this->name, this->binPath);

      Os::FileSystem::Status stat = Os::FileSystem::readDirectory(this->binPath, MAX_BINS, bins, totalBins);
      ASSERT_EQ(stat, Os::FileSystem::INVALID_PATH);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  RemoveInvalid
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::RemoveInvalid::RemoveInvalid(const char* filename) :
        STest::Rule<Os::Tester>("RemoveInvalid")
  {
    this->filename = filename;
  }


  bool Os::Tester::RemoveInvalid::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::RemoveInvalid::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      Os::FileSystem::Status stat = Os::FileSystem::removeFile(this->filename);
      ASSERT_EQ(Os::FileSystem::INVALID_PATH, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  GetFileSizeInvalid
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::GetFileSizeInvalid::GetFileSizeInvalid(const char* filename) :
        STest::Rule<Os::Tester>("GetFileSizeInvalid")
  {
    this->filename = filename;
  }


  bool Os::Tester::GetFileSizeInvalid::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return true;
  }

  
  void Os::Tester::GetFileSizeInvalid::action(
            Os::Tester& state //!< The test state
        ) 
  {
      printf("--> Rule: %s %s\n", this->name, this->filename);

      FwSizeType actualSize;
      FileSystem::Status stat = FileSystem::getFileSize(this->filename, actualSize);
      ASSERT_EQ(FileSystem::INVALID_PATH, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  OpenRandomFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::OpenRandomFile::OpenRandomFile() :
        STest::Rule<Os::Tester>("OpenRandomFile")
  {
  }


  bool Os::Tester::OpenRandomFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return state.simFileSystem->canOpenFile();

  }

  
  void Os::Tester::OpenRandomFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      std::string filename = state.simFileSystem->openFile();
      printf("--> Rule: %s %s\n", this->name, filename.c_str());

      Os::File *filePtr = state.simFileSystem->getFileDesc(filename);
      Os::File::Status stat = filePtr->open(filename.c_str(), Os::File::OPEN_WRITE); 
      ASSERT_EQ(Os::File::OP_OK, stat);

  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  CloseRandomFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::CloseRandomFile::CloseRandomFile() :
        STest::Rule<Os::Tester>("CloseRandomFile")
  {
  }


  bool Os::Tester::CloseRandomFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return state.simFileSystem->canCloseFile();

  }

  
  void Os::Tester::CloseRandomFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      std::string filename = state.simFileSystem->closeFile();
      printf("--> Rule: %s %s\n", this->name, filename.c_str());

      Os::File *filePtr = state.simFileSystem->getFileDesc(filename);
      filePtr->close(); 
  }


    


  // ------------------------------------------------------------------------------------------------------
  // Rule:  WriteRandomFile
  //
  // ------------------------------------------------------------------------------------------------------
  
  Os::Tester::WriteRandomFile::WriteRandomFile() :
        STest::Rule<Os::Tester>("WriteRandomFile")
  {
  }


  bool Os::Tester::WriteRandomFile::precondition(
            const Os::Tester& state //!< The test state
        ) 
  {
      return state.simFileSystem->canWriteFile();
  }

  
  void Os::Tester::WriteRandomFile::action(
            Os::Tester& state //!< The test state
        ) 
  {
      std::vector<uint8_t> vbytes;

      // Randomize how many bytes are written to the file
      NATIVE_INT_TYPE fillSize = rand() % Tester::FILE_SIZE + 1;
      
      // Fill the memory buffer with random numbers between 0 and 0xFF inclusive
      for (NATIVE_INT_TYPE i=0; i < fillSize; i++) {
        vbytes.push_back(rand() % 256);
      }

      std::string filename = state.simFileSystem->writeToFile(vbytes);
      printf("--> Rule: %s %s %d bytes\n", this->name, filename.c_str(), fillSize);

      Os::File *filePtr = state.simFileSystem->getFileDesc(filename);
      Os::File::Status stat = filePtr->write(vbytes.data(), fillSize);
      ASSERT_EQ(stat, Os::File::OP_OK);


  }

