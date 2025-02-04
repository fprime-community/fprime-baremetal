// ======================================================================
// \title  DataRecorder/test/ut/Tester.hpp
// \author watney
// \brief  hpp file for DataRecorder test harness implementation class
// ======================================================================

#ifndef TESTER_HPP
#define TESTER_HPP

#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <Os/MicroFs/MicroFs.hpp>
#include <Fw/Types/MallocAllocator.hpp>

#include "SimFileSystem.h"



#include "RulesHeaders.hpp"

namespace Os {

  class Tester
  {

      enum {
        MAX_BINS = 10,
        MAX_FILES_PER_BIN = 10,
        MAX_TOTAL_FILES = MAX_BINS * MAX_FILES_PER_BIN,
        FILE_SIZE = 100
      };



      // ----------------------------------------------------------------------
      // Construction and destruction
      // ----------------------------------------------------------------------

    public:

      class FileModel {
          public:
            enum Mode {
              DOESNT_EXIST,
              CLOSED,
              OPEN_READ,
              OPEN_WRITE
            };

            FileModel();
            void clear();

            Mode mode;
            Os::File fileDesc;
            BYTE buffOut[FILE_SIZE];
            NATIVE_INT_TYPE curPtr;
            I32 size;
      };

      #include "MyRules.hpp"

      //! Construct object Tester
      //!
      Tester();

      //! Destroy object Tester
      //!
      ~Tester();

      FileModel fileModels[MAX_TOTAL_FILES];
      SimFileSystem *simFileSystem;

      Fw::MallocAllocator alloc;
      Os::MicroFsConfig testCfg;

    public:

      // ----------------------------------------------------------------------
      // Tests
      // ----------------------------------------------------------------------

      //!
      void InitTest();
      void OpenWriteReadTest();
      void OpenWriteTwiceReadOnceTest();
      void OpenWriteOnceReadTwiceTest();
      void ListTest();
      void OpenStressTest();
      void OpenFreeSpaceTest();
      void ReWriteTest();
      void BadOpenTest();
      void FileSizeTest();
      void NukeTest();
      void OddTests();
      void MoveTest();
      void DirectoryTest();
      void SeekTest();
      void BulkTest();
      void CrcTest();
      void OffNominalTests();
      void CopyTest();
      void AppendTest();
      void SimFileTest();
      void NewTest();

      // Helper functions
      void clearFileBuffer();
      FileModel* getFileModel(const char *filename);
      void getFileNames(char fileNames[][20], U16 numBins, U16 numFiles);


    private:

     

    private:

      // ----------------------------------------------------------------------
      // Helper methods
      // ----------------------------------------------------------------------

      //! Connect ports
      //!
      void connectPorts();

      //! Initialize components
      //!
      void initComponents();

      I16 getIndex(const char *fileName) const;


    private:

      // ----------------------------------------------------------------------
      // Variables
      // ----------------------------------------------------------------------

      // Free space test values
      FwSizeType m_expTotalBytes;
      FwSizeType m_expFreeBytes;
      FileSystem::Status m_expStat;  //! Expected status for internal filesystem calls

  };

} // end namespace Os

#endif
