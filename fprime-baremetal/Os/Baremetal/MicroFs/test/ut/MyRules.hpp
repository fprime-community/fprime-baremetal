

// ------------------------------------------------------------------------------------------------------
// Rule:  InitFileSystem
//
// ------------------------------------------------------------------------------------------------------
struct InitFileSystem : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    InitFileSystem(U32 numBins, U32 fileSize, U32 numFiles);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    U32 numBins;
    U32 fileSize;
    U32 numFiles;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenFile
//
// ------------------------------------------------------------------------------------------------------
struct OpenFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  Cleanup
//
// ------------------------------------------------------------------------------------------------------
struct Cleanup : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    Cleanup();

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
};

// ------------------------------------------------------------------------------------------------------
// Rule:  WriteData
//
// ------------------------------------------------------------------------------------------------------
struct WriteData : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    WriteData(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ReadData
//
// ------------------------------------------------------------------------------------------------------
struct ReadData : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    ReadData(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ResetFile
//
// ------------------------------------------------------------------------------------------------------
struct ResetFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    ResetFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CloseFile
//
// ------------------------------------------------------------------------------------------------------
struct CloseFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    CloseFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  Listings
//
// ------------------------------------------------------------------------------------------------------
struct Listings : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    Listings(U16 numBins, U16 numFiles);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    U16 numBins;
    U16 numFiles;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  FreeSpace
//
// ------------------------------------------------------------------------------------------------------
struct FreeSpace : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    FreeSpace();

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenNoPerm
//
// ------------------------------------------------------------------------------------------------------
struct OpenNoPerm : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenNoPerm(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CheckFileSize
//
// ------------------------------------------------------------------------------------------------------
struct CheckFileSize : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    CheckFileSize(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenRead
//
// ------------------------------------------------------------------------------------------------------
struct OpenRead : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenRead(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenFileNotExist
//
// ------------------------------------------------------------------------------------------------------
struct OpenFileNotExist : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenFileNotExist(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenReadEarly
//
// ------------------------------------------------------------------------------------------------------
struct OpenReadEarly : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenReadEarly(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenCreate
//
// ------------------------------------------------------------------------------------------------------
struct OpenCreate : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenCreate(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenAppend
//
// ------------------------------------------------------------------------------------------------------
struct OpenAppend : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenAppend(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  RemoveFile
//
// ------------------------------------------------------------------------------------------------------
struct RemoveFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    RemoveFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  RemoveBusyFile
//
// ------------------------------------------------------------------------------------------------------
struct RemoveBusyFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    RemoveBusyFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  IsFileOpen
//
// ------------------------------------------------------------------------------------------------------
struct IsFileOpen : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    IsFileOpen(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  MoveFile
//
// ------------------------------------------------------------------------------------------------------
struct MoveFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    MoveFile(const char* sourcefile, const char* destfile);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* sourcefile;
    const char* destfile;
    Os::Tester::FileModel* sourceModel;
    Os::Tester::FileModel* destModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  Directory
//
// ------------------------------------------------------------------------------------------------------
struct Directory : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    Directory(const char* dirpath, bool offNominal);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* dirpath;
    bool offNominal;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  SeekFile
//
// ------------------------------------------------------------------------------------------------------
struct SeekFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    SeekFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  SeekNFile
//
// ------------------------------------------------------------------------------------------------------
struct SeekNFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    SeekNFile(const char* filename, NATIVE_INT_TYPE seek);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
    NATIVE_INT_TYPE seek;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CalcCRC32
//
// ------------------------------------------------------------------------------------------------------
struct CalcCRC32 : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    CalcCRC32(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  SeekNotOpen
//
// ------------------------------------------------------------------------------------------------------
struct SeekNotOpen : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    SeekNotOpen(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  SeekBadSize
//
// ------------------------------------------------------------------------------------------------------
struct SeekBadSize : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    SeekBadSize(const char* filename, NATIVE_INT_TYPE seek);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
    NATIVE_INT_TYPE seek;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  SeekRelative
//
// ------------------------------------------------------------------------------------------------------
struct SeekRelative : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    SeekRelative(const char* filename, NATIVE_INT_TYPE seek);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
    NATIVE_INT_TYPE seek;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ReadNotOpen
//
// ------------------------------------------------------------------------------------------------------
struct ReadNotOpen : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    ReadNotOpen(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  WriteNotOpen
//
// ------------------------------------------------------------------------------------------------------
struct WriteNotOpen : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    WriteNotOpen(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  BulkWriteNoOpen
//
// ------------------------------------------------------------------------------------------------------
struct BulkWriteNoOpen : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    BulkWriteNoOpen(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  FlushFile
//
// ------------------------------------------------------------------------------------------------------
struct FlushFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    FlushFile(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  GetErrors
//
// ------------------------------------------------------------------------------------------------------
struct GetErrors : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    GetErrors(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
    Os::Tester::FileModel* fileModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CopyFile
//
// ------------------------------------------------------------------------------------------------------
struct CopyFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    CopyFile(const char* srcFile, const char* destFile);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* srcFile;
    const char* destFile;
    Os::Tester::FileModel* srcModel;
    Os::Tester::FileModel* destModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  AppendFile
//
// ------------------------------------------------------------------------------------------------------
struct AppendFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    AppendFile(const char* srcFile, const char* destFile);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* srcFile;
    const char* destFile;
    Os::Tester::FileModel* srcModel;
    Os::Tester::FileModel* destModel;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  MoveInvalid
//
// ------------------------------------------------------------------------------------------------------
struct MoveInvalid : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    MoveInvalid(const char* sourceFile, const char* destFile);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* sourceFile;
    const char* destFile;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  MoveBusy
//
// ------------------------------------------------------------------------------------------------------
struct MoveBusy : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    MoveBusy(const char* sourceFile, const char* destFile);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* sourceFile;
    const char* destFile;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ReadDirInvalid
//
// ------------------------------------------------------------------------------------------------------
struct ReadDirInvalid : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    ReadDirInvalid(const char* binPath);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* binPath;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  RemoveInvalid
//
// ------------------------------------------------------------------------------------------------------
struct RemoveInvalid : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    RemoveInvalid(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  GetFileSizeInvalid
//
// ------------------------------------------------------------------------------------------------------
struct GetFileSizeInvalid : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    GetFileSizeInvalid(const char* filename);

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );

    const char* filename;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  OpenRandomFile
//
// ------------------------------------------------------------------------------------------------------
struct OpenRandomFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    OpenRandomFile();

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CloseRandomFile
//
// ------------------------------------------------------------------------------------------------------
struct CloseRandomFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    CloseRandomFile();

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
};

// ------------------------------------------------------------------------------------------------------
// Rule:  WriteRandomFile
//
// ------------------------------------------------------------------------------------------------------
struct WriteRandomFile : public STest::Rule<Os::Tester> {
    // ----------------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------------

    //! Constructor
    WriteRandomFile();

    // ----------------------------------------------------------------------
    // Public member functions
    // ----------------------------------------------------------------------

    //! Precondition
    bool precondition(const Os::Tester& state  //!< The test state
    );

    //! Action
    void action(Os::Tester& state  //!< The test state
    );
};
